/* SUPPORTED API ROUTES
 *   - Home page: GET /
 *   - Get all movies: GET /movies
 *   - Get movie: GET /movie?title=<title>&search_type=<type>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

#define PORT 8888
#define ARRAY_LEN(arr) (sizeof (arr) / sizeof ((arr)[0]))
#define MAX_ROUTE_LEN 64

const char *ROUTES[] = { "/", "/movies", "/movie" };
size_t n_routes = ARRAY_LEN (ROUTES);

int
valid_route (const char *req_url)
{
  for (size_t i = 0; i < n_routes; ++i)
    {
      if (strcmp (ROUTES[i], req_url) == 0)
        return 0;
    }

  return -1;
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
  ret = MHD_queue_response (connection, code, response);
  MHD_destroy_response (response);

  return ret;
}

enum MHD_Result
router (struct MHD_Connection *connection, const char *url)
{
  char *buf;
  size_t urllen, buflen;
  const char *page;

  urllen = strlen (url);
  buflen = 1024 + (urllen > MAX_ROUTE_LEN ? MAX_ROUTE_LEN : urllen);

  if ((buf = malloc (buflen + 1)) == NULL)
    {
      page = "<html><body>Internal Server Error</body></html>";

      return send_response (connection, page, strlen (page),
                            MHD_HTTP_INTERNAL_SERVER_ERROR);
    }

  if (valid_route (url) == -1)
    {
      page = "<html><body>Invalid route: %s</body></html>";
      snprintf (buf, buflen, page, url);

      return send_response_free_callback (connection, buf, strlen (buf),
                                          MHD_HTTP_NOT_FOUND, &mem_free);
    }

  page = "<html><body>%s</body></html>";
  snprintf (buf, buflen, page, url);

  return send_response_free_callback (connection, buf, strlen (buf),
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
  (void)cls;
  (void)req_cls;

  if (dh_check_method (method) == MHD_NO)
    {
      const char *page;
      page = "<html><body>Method not allowed</body></html>\n";

      return send_response (connection, page, strlen (page),
                            MHD_HTTP_METHOD_NOT_ALLOWED);
    }

  return router (connection, url);
}

int
main (void)
{
  struct MHD_Daemon *daemon;
  enum MHD_FLAG flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_DEBUG
                        | MHD_USE_PEDANTIC_CHECKS;

  daemon = MHD_start_daemon (flags, PORT, NULL, NULL, &answer_connection,
                             NULL, MHD_OPTION_END);

  if (NULL == daemon)
    {
      return EXIT_FAILURE;
    }

  getchar ();

  MHD_stop_daemon (daemon);

  return EXIT_SUCCESS;
}
