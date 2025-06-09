#include <assert.h>
#include <jansson.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "handler.h"
#include "route.h"

#define PORT 8080
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MAX_ROUTE_LEN 64
#define ROUTE_TABLE_SIZE (2 << 15)
#define ERR_TEMPLATE "{\"status\": %i, \"result\": \"%s\"}"

#define SERVERKEYFILE "../pki/mycrib.key"
#define SERVERCERTFILE "../pki/mycrib.pem"

static long get_file_size(const char *filename) {
  FILE *fp;

  fp = fopen(filename, "rb");
  if (fp) {
    long size;

    if ((0 != fseek(fp, 0, SEEK_END)) || (-1 == (size = ftell(fp)))) size = 0;

    fclose(fp);

    return size;
  }
  return 0;
}

static char *load_file(const char *filename) {
  FILE *fp;
  char *buffer;
  long size;

  size = get_file_size(filename);
  if (0 == size) return NULL;

  fp = fopen(filename, "rb");
  if (!fp) return NULL;

  buffer = malloc(size + 1);
  if (!buffer) {
    fclose(fp);
    return NULL;
  }
  buffer[size] = '\0';

  if (size != (long)fread(buffer, 1, size, fp)) {
    free(buffer);
    buffer = NULL;
  }

  fclose(fp);
  return buffer;
}

void mem_free(void *mem) {
  if (mem) free(mem);
}

enum MHD_Result send_response(struct MHD_Connection *connection,
                              const char *buf, size_t bufsize, int code) {
  struct MHD_Response *response;
  int ret;

  response = MHD_create_response_from_buffer(bufsize, (void *)buf,
                                             MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE,
                          "application/json");
  ret = MHD_queue_response(connection, code, response);
  MHD_destroy_response(response);

  return ret;
}

enum MHD_Result send_response_free_callback(
    struct MHD_Connection *connection, const char *buf, size_t bufsize,
    int code, MHD_ContentReaderFreeCallback crfc) {
  struct MHD_Response *response;
  int ret;

  response = MHD_create_response_from_buffer_with_free_callback(
      bufsize, (void *)buf, crfc);
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE,
                          "application/json");
  ret = MHD_queue_response(connection, code, response);
  MHD_destroy_response(response);

  return ret;
}

enum MHD_Result router(Route *routes, struct MHD_Connection *connection,
                       const char *url) {
  char *response;
  size_t url_len;
  json_t *result, *status_json;
  int status;
  unsigned id;

  char *error;
  int required_len;

  url_len = strlen(url);
  if (url_len > MAX_ROUTE_LEN) {
    required_len = snprintf(NULL, 0, ERR_TEMPLATE, MHD_HTTP_BAD_REQUEST,
                            MHD_get_reason_phrase_for(MHD_HTTP_BAD_REQUEST));
    if ((error = malloc(required_len + 1)) == NULL) {
      return send_response(
          connection, MHD_get_reason_phrase_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
          MHD_get_reason_phrase_len_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
          MHD_HTTP_INTERNAL_SERVER_ERROR);
    }
    snprintf(error, required_len + 1, ERR_TEMPLATE, MHD_HTTP_BAD_REQUEST,
             MHD_get_reason_phrase_for(MHD_HTTP_BAD_REQUEST));

    return send_response_free_callback(connection, error, required_len,
                                       MHD_HTTP_BAD_REQUEST, &mem_free);
  }

  id = small_crc16_8005(url, url_len);

  if (!routes[id].path) {
    required_len = snprintf(NULL, 0, ERR_TEMPLATE, MHD_HTTP_NOT_FOUND,
                            MHD_get_reason_phrase_for(MHD_HTTP_NOT_FOUND));
    if ((error = malloc(required_len + 1)) == NULL) {
      return send_response(
          connection, MHD_get_reason_phrase_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
          MHD_get_reason_phrase_len_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
          MHD_HTTP_INTERNAL_SERVER_ERROR);
    }
    snprintf(error, required_len + 1, ERR_TEMPLATE, MHD_HTTP_NOT_FOUND,
             MHD_get_reason_phrase_for(MHD_HTTP_NOT_FOUND));

    return send_response_free_callback(connection, error, required_len,
                                       MHD_HTTP_NOT_FOUND, &mem_free);
  }

  result = routes[id].handler(connection);

  response = json_dumps(result, 0);

  status_json = json_object_get(result, "status");
  if (!status_json)
    return send_response(
        connection, MHD_get_reason_phrase_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
        MHD_get_reason_phrase_len_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
        MHD_HTTP_INTERNAL_SERVER_ERROR);

  status = json_integer_value(status_json);
  if (status == 0)
    return send_response(
        connection, MHD_get_reason_phrase_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
        MHD_get_reason_phrase_len_for(MHD_HTTP_INTERNAL_SERVER_ERROR),
        MHD_HTTP_INTERNAL_SERVER_ERROR);

  json_decref(result);

  return send_response_free_callback(connection, response, strlen(response),
                                     status, &mem_free);
}

enum MHD_Result dh_check_method(const char *method) {
  int cond;

  cond = strcmp(method, MHD_HTTP_METHOD_GET) &&
         strcmp(method, MHD_HTTP_METHOD_POST);
  return cond ? MHD_NO : MHD_YES;
}

enum MHD_Result answer_connection(void *cls, struct MHD_Connection *connection,
                                  const char *url, const char *method,
                                  const char *version, const char *upload_data,
                                  size_t *upload_data_size, void **req_cls) {
  (void)method;
  (void)version;
  (void)upload_data;
  (void)upload_data_size;
  (void)req_cls;

  static int ctx;

  int required_len;
  char *error;
  if (dh_check_method(method) == MHD_NO) {
    required_len =
        snprintf(NULL, 0, ERR_TEMPLATE, MHD_HTTP_METHOD_NOT_ALLOWED,
                 MHD_get_reason_phrase_for(MHD_HTTP_METHOD_NOT_ALLOWED));
    if ((error = malloc(required_len + 1)) == NULL) {
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
  Route *routes = *(Route **)cls;

  return router(routes, connection, url);
}

int main(void) {
  struct MHD_Daemon *daemon;
  enum MHD_FLAG flags;
  Route *routes;
  char *key_pem, *cert_pem;

  cert_pem = load_file(SERVERCERTFILE);
  key_pem = load_file(SERVERKEYFILE);

  if ((key_pem == NULL) || (cert_pem == NULL)) {
    printf("the key/certificate files could not be read.\n");
    return EXIT_FAILURE;
  }

  if ((routes = (Route *)calloc(ROUTE_TABLE_SIZE, sizeof(Route))) == NULL) {
    fprintf(stderr, "[FATAL] Failed to allocate route table\n");
    exit(1);
  }

  new_route(routes, "/", root_handler);
  new_route(routes, "/movies", movies_handler);

  flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_THREAD_PER_CONNECTION |
          MHD_USE_ITC | MHD_USE_TCP_FASTOPEN | MHD_USE_TLS |
          MHD_USE_PEDANTIC_CHECKS | MHD_USE_DEBUG;
  daemon =
      MHD_start_daemon(flags, PORT, NULL, NULL, &answer_connection, &routes,
                       MHD_OPTION_HTTPS_MEM_KEY, key_pem,
                       MHD_OPTION_HTTPS_MEM_CERT, cert_pem, MHD_OPTION_END);

  if (NULL == daemon) {
    free(key_pem);
    free(cert_pem);

    return EXIT_FAILURE;
  }

  getchar();

  MHD_stop_daemon(daemon);

  del_route(routes, "/");
  del_route(routes, "/movies");

  if (routes) free(routes);
  if (key_pem) free(key_pem);
  if (cert_pem) free(cert_pem);

  return EXIT_SUCCESS;
}

// vim: sts=2 ts=2 et ai tw=80
