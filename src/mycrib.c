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
  int found = -1;

  for (size_t i = 0; i < n_routes; ++i)
    {
      if (strcmp (ROUTES[i], req_url) == 0)
        found = 0;
    }

  return found;
}

void
mem_free (void *mem)
{
  if (mem)
    free (mem);
}

enum MHD_Result
router (struct MHD_Connection *connection, const char *url)
{
  char *buf;
  size_t urllen, buflen;
  int ret;
  const char *page;
  struct MHD_Response *response;

  urllen = strlen (url);
  buflen = 1024 + (urllen > MAX_ROUTE_LEN ? MAX_ROUTE_LEN : urllen);

  if ((buf = malloc (buflen + 1)) == NULL)
    {
      page = "<html><body>Internal Server Error</body></html>";

      response = MHD_create_response_from_buffer (strlen (page), (void *)page,
                                                  MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response (connection, MHD_HTTP_INTERNAL_SERVER_ERROR,
                                response);

      return ret;
    }

  if (valid_route (url) == -1)
    {
      page = "<html><body>Invalid route: %s</body></html>";
      snprintf (buf, buflen, page, url);

      response = MHD_create_response_from_buffer_with_free_callback (
          strlen (buf), (void *)buf, &mem_free);
      ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);

      return ret;
    }

  page = "<html><body>%s</body></html>";
  snprintf (buf, buflen, page, url);

  response = MHD_create_response_from_buffer (strlen (buf), (void *)buf,
                                              MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);

  MHD_destroy_response (response);
  free (buf);

  return ret;
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

  struct MHD_Response *response;
  const char *page;
  int ret;

  if (dh_check_method (method) == MHD_NO)
    {
      page = "<html></body>Method not allowed</body></html>\n";
      response = MHD_create_response_from_buffer (strlen (page), (void *)page,
                                                  MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response (connection, MHD_HTTP_METHOD_NOT_ALLOWED,
                                response);
      MHD_destroy_response (response);

      return ret;
    }

  page = "<html><body>Hello, Sailor!</body></html>\n";

  return router (connection, url);
}

int
main (void)
{
  struct MHD_Daemon *daemon;
  enum MHD_FLAG flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_DEBUG
                        | MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_PEDANTIC_CHECKS;

  daemon = MHD_start_daemon (flags, PORT, NULL, NULL, &answer_connection,
                             (void *)&dh_check_method, MHD_OPTION_END);

  if (NULL == daemon)
    {
      return EXIT_FAILURE;
    }

  getchar ();

  MHD_stop_daemon (daemon);

  return EXIT_SUCCESS;
}
