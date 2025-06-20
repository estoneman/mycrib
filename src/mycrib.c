#include <assert.h>
#include <microhttpd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "route.h"
#include "util/mhd.h"
#include "util/util.h"

#define PORT 8080
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#define ROUTE_TABLE_SIZE (2 << 15)
#define ERR_TEMPLATE "{\"status\": %i, \"result\": \"%s\"}"

#define SERVERKEYFILE "../pki/mycrib.key"
#define SERVERCERTFILE "../pki/mycrib.pem"

static volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig) {
  (void)sig;
  keep_running = 0;
  fprintf(stderr, "[INFO] shutting down server\n");
}

enum MHD_Result answer_connection(void *cls, struct MHD_Connection *connection,
                                  const char *url, const char *method,
                                  const char *version, const char *upload_data,
                                  size_t *upload_data_size, void **req_cls) {
  (void)version;
  (void)upload_data;
  (void)upload_data_size;
  (void)req_cls;

  static int ctx;
  Route *routes;
  int required_len;
  char *error;

  if (dh_check_method(method) == MHD_NO) {
    required_len =
        snprintf(NULL, 0, ERR_TEMPLATE, MHD_HTTP_METHOD_NOT_ALLOWED,
                 MHD_get_reason_phrase_for(MHD_HTTP_METHOD_NOT_ALLOWED));
    if (!(error = malloc(required_len + 1))) {
      return send_response(
          connection, MHD_get_reason_phrase_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
          MHD_get_reason_phrase_len_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
          MHD_HTTP_INTERNAL_SERVER_ERROR);
    }
    snprintf(error, required_len + 1, ERR_TEMPLATE, MHD_HTTP_METHOD_NOT_ALLOWED,
             MHD_get_reason_phrase_for(MHD_HTTP_METHOD_NOT_ALLOWED));

    return send_response_free_callback(connection, error, required_len,
                                       MHD_HTTP_METHOD_NOT_ALLOWED, &mem_free);
  }

  if (&ctx != *req_cls) {
    *req_cls = &ctx;
    return MHD_YES;
  }
  *req_cls = NULL;
  routes = *(Route **)cls;

  // MAYBE TODO: pass different types of contexts based on request
  RequestContext req_ctx = {.connection = connection,
                            .url = url,
                            .method = method,
                            .upload_data = upload_data,
                            .upload_data_size = upload_data_size};

  return router(routes, &req_ctx);
}

int main(void) {
  struct MHD_Daemon *daemon;
  enum MHD_FLAG flags;
  Route *routes;
  char *key_pem, *cert_pem;
  struct sigaction sa;
  pid_t self;

  cert_pem = load_file(SERVERCERTFILE);
  key_pem = load_file(SERVERKEYFILE);

  if ((!key_pem) || (!cert_pem)) {
    printf("the key/certificate files could not be read.\n");
    return EXIT_FAILURE;
  }

  if ((routes = (Route *)calloc(ROUTE_TABLE_SIZE, sizeof(Route))) == NULL) {
    free(key_pem);
    free(cert_pem);

    fprintf(stderr, "[FATAL] Failed to allocate route table\n");
    exit(1);
  }

  new_route(routes, "/", root_handler);
  new_route(routes, "/movies", movies_handler);

  flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_THREAD_PER_CONNECTION |
          MHD_USE_ITC | MHD_USE_TLS | MHD_USE_PEDANTIC_CHECKS | MHD_USE_DEBUG;
#ifdef _LINUX
  flags |= MHD_USE_TCP_FASTOPEN;
#endif
  daemon =
      MHD_start_daemon(flags, PORT, NULL, NULL, &answer_connection, &routes,
                       MHD_OPTION_HTTPS_MEM_KEY, key_pem,
                       MHD_OPTION_HTTPS_MEM_CERT, cert_pem, MHD_OPTION_END);

  if (!daemon) {
    del_route(routes, "/");
    del_route(routes, "/movies");

    free(routes);
    free(key_pem);
    free(cert_pem);

    return EXIT_FAILURE;
  }

  sa.sa_handler = signal_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");

    del_route(routes, "/");
    del_route(routes, "/movies");

    free(routes);
    free(key_pem);
    free(cert_pem);

    return EXIT_FAILURE;
  }

  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("sigaction");

    del_route(routes, "/");
    del_route(routes, "/movies");

    free(key_pem);
    free(cert_pem);
    free(routes);

    return EXIT_FAILURE;
  }

  self = getpid();
  fprintf(stderr,
          "[INFO] [%d] started server on port %d, press CTRL+C to stop\n", self,
          PORT);

  while (keep_running) sleep(1);

  MHD_stop_daemon(daemon);

  del_route(routes, "/");
  del_route(routes, "/movies");

  free(key_pem);
  free(cert_pem);
  free(routes);

  return EXIT_SUCCESS;
}

// vim: sts=2 ts=2 et ai tw=80
