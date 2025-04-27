#include <assert.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "route.h"

#define PORT 8080
#define ARRAY_LEN(arr) (sizeof (arr) / sizeof ((arr)[0]))
#define MAX_ROUTE_LEN 64
#define ROUTE_TABLE_SIZE (2 << 15)

#define SERVERKEYFILE "../pki/mycrib.key"
#define SERVERCERTFILE "/etc/pki/ca-trust/source/anchors/mycrib.pem"

static long
get_file_size (const char *filename)
{
  FILE *fp;

  fp = fopen (filename, "rb");
  if (fp)
    {
      long size;

      if ((0 != fseek (fp, 0, SEEK_END)) || (-1 == (size = ftell (fp))))
        size = 0;

      fclose (fp);

      return size;
    }
  else
    return 0;
}

static char *
load_file (const char *filename)
{
  FILE *fp;
  char *buffer;
  long size;

  size = get_file_size (filename);
  if (0 == size)
    return NULL;

  fp = fopen (filename, "rb");
  if (!fp)
    return NULL;

  buffer = malloc (size + 1);
  if (!buffer)
    {
      fclose (fp);
      return NULL;
    }
  buffer[size] = '\0';

  if (size != (long)fread (buffer, 1, size, fp))
    {
      free (buffer);
      buffer = NULL;
    }

  fclose (fp);
  return buffer;
}

void
mem_free (void *mem)
{
  if (mem)
    free (mem);
}

enum MHD_Result
send_response (struct MHD_Connection *connection, const char *buf,
               size_t bufsize, int code)
{
  struct MHD_Response *response;
  int ret;

  response = MHD_create_response_from_buffer (bufsize, (void *)buf,
                                              MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header (response, MHD_HTTP_HEADER_CONTENT_TYPE,
                           "application/json");
  ret = MHD_queue_response (connection, code, response);
  MHD_destroy_response (response);

  return ret;
}

enum MHD_Result
send_response_free_callback (struct MHD_Connection *connection,
                             const char *buf, size_t bufsize, int code,
                             MHD_ContentReaderFreeCallback crfc)
{
  struct MHD_Response *response;
  int ret;

  response = MHD_create_response_from_buffer_with_free_callback (
      bufsize, (void *)buf, crfc);
  MHD_add_response_header (response, MHD_HTTP_HEADER_CONTENT_TYPE,
                           "application/json");
  ret = MHD_queue_response (connection, code, response);
  MHD_destroy_response (response);

  return ret;
}

enum MHD_Result
router (Route *routes, struct MHD_Connection *connection, const char *url)
{
  const char *page;
  char *error, *response, san_url[MAX_ROUTE_LEN + 1];
  size_t url_len, error_len;

  url_len = strlen (url);
  error_len = 1024 + (url_len > MAX_ROUTE_LEN ? MAX_ROUTE_LEN : url_len);
  strncpy (san_url, url, MAX_ROUTE_LEN);
  san_url[MAX_ROUTE_LEN - 3] = '.';
  san_url[MAX_ROUTE_LEN - 2] = '.';
  san_url[MAX_ROUTE_LEN - 1] = '.';
  san_url[MAX_ROUTE_LEN] = '\0';

  if ((error = malloc (error_len + 1)) == NULL)
    {
      page = "{\"error\": \"Internal Server Error\"}";

      return send_response (connection, page, strlen (page),
                            MHD_HTTP_INTERNAL_SERVER_ERROR);
    }

  unsigned id = small_crc16_8005 (san_url, url_len);

  if (!routes[id].path)
    {
      page = "{\"error\": \"%s '%s'\"}";
      snprintf (error, error_len, page,
                MHD_get_reason_phrase_for (MHD_HTTP_NOT_FOUND), san_url);

      return send_response_free_callback (connection, error, strlen (error),
                                          MHD_HTTP_NOT_FOUND, &mem_free);
    }

  response = routes[id].handler (connection);

  return send_response_free_callback (connection, response, strlen (response),
                                      MHD_HTTP_OK, &mem_free);
}

enum MHD_Result
dh_check_method (const char *method)
{
  int cond;

  cond = strcmp (method, MHD_HTTP_METHOD_GET)
         && strcmp (method, MHD_HTTP_METHOD_POST);
  return cond ? MHD_NO : MHD_YES;
}

enum MHD_Result
answer_connection (void *cls, struct MHD_Connection *connection,
                   const char *url, const char *method, const char *version,
                   const char *upload_data, size_t *upload_data_size,
                   void **req_cls)
{
  (void)method;
  (void)version;
  (void)upload_data;
  (void)upload_data_size;
  (void)req_cls;

  static int ctx;

  if (dh_check_method (method) == MHD_NO)
    {
      const char *page;
      page = "{\"error\": \"Method not allowed\"}";

      return send_response (connection, page, strlen (page),
                            MHD_HTTP_METHOD_NOT_ALLOWED);
    }

  if (&ctx != *req_cls)
    {
      *req_cls = &ctx;
      return MHD_YES;
    }
  *req_cls = NULL;
  Route *routes = *(Route **)cls;

  return router (routes, connection, url);
}

int
main (void)
{
  struct MHD_Daemon *daemon;
  enum MHD_FLAG flags;
  Route *routes;
  char *key_pem, *cert_pem;

  cert_pem = load_file (SERVERCERTFILE);
  key_pem = load_file (SERVERKEYFILE);

  if ((key_pem == NULL) || (cert_pem == NULL))
    {
      printf ("the key/certificate files could not be read.\n");
      return EXIT_FAILURE;
    }

  if ((routes = (Route *)calloc (ROUTE_TABLE_SIZE, sizeof (Route))) == NULL)
    {
      fprintf (stderr, "[FATAL] Failed to allocate route table\n");
      exit (1);
    }

  new_route (routes, "/", root_handler);
  new_route (routes, "/movie", movie_handler);
  new_route (routes, "/movies", movies_handler);

  flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_THREAD_PER_CONNECTION
          | MHD_USE_ITC | MHD_USE_TCP_FASTOPEN | MHD_USE_TLS
          | MHD_USE_PEDANTIC_CHECKS | MHD_USE_DEBUG;
  daemon
      = MHD_start_daemon (flags, PORT, NULL, NULL, &answer_connection, &routes,
                          MHD_OPTION_HTTPS_MEM_KEY, key_pem,
                          MHD_OPTION_HTTPS_MEM_CERT, cert_pem, MHD_OPTION_END);

  if (NULL == daemon)
    {
      printf ("%s\n", cert_pem);

      free (key_pem);
      free (cert_pem);

      return EXIT_FAILURE;
    }

  getchar ();

  MHD_stop_daemon (daemon);

  del_route (routes, "/");
  del_route (routes, "/movie");
  del_route (routes, "/movies");

  free (key_pem);
  free (cert_pem);

  return EXIT_SUCCESS;
}

// vim: sts=2 ts=2 et ai tw=80
