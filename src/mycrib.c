/* SUPPORTED API ROUTES
 *   - Home page: GET /
 *   - Get all movies: GET /movies
 *   - Get movie: GET /movie?title=<title>&search_type=<type>
 */

#include <assert.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

#define PORT 8888
#define ARRAY_LEN(arr) (sizeof (arr) / sizeof ((arr)[0]))
#define MAX_ROUTE_LEN 64

const char *ROUTES[] = { "/movie", "/movies" };
#define N_ROUTES ARRAY_LEN (ROUTES)

Route routes[N_ROUTES];
size_t curr_route = 0;

int
route_id (const char *req_url)
{
  for (size_t i = 0; i < N_ROUTES; ++i)
    {
      if (strcmp (ROUTES[i], req_url) == 0)
        return i;
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
router (struct MHD_Connection *connection, const char *url)
{
  const char *page;
  char *error, *response;
  size_t url_len, error_len;
  int route;

  url_len = strlen (url);
  error_len = 1024 + (url_len > MAX_ROUTE_LEN ? MAX_ROUTE_LEN : url_len);

  if ((error = malloc (error_len + 1)) == NULL)
    {
      page = "{\"error\": \"Internal Server Error\"}";

      return send_response (connection, page, strlen (page),
                            MHD_HTTP_INTERNAL_SERVER_ERROR);
    }

  if ((route = route_id (url)) == -1)
    {
      page = "{\"error\": \"Invalid route: \"%s\"}";
      snprintf (error, error_len, page, url);

      return send_response_free_callback (connection, error, strlen (error),
                                          MHD_HTTP_NOT_FOUND, &mem_free);
    }

  response = routes[route].handler ();

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
  (void)cls;
  (void)req_cls;

  if (dh_check_method (method) == MHD_NO)
    {
      const char *page;
      page = "{\"error\": \"Method not allowed\"}";

      return send_response (connection, page, strlen (page),
                            MHD_HTTP_METHOD_NOT_ALLOWED);
    }

  return router (connection, url);
}

void
add_route (const char *path, char *(*handler) (void))
{
  assert (curr_route < N_ROUTES);

  routes[curr_route].path = path;
  routes[curr_route++].handler = handler;
}

int
main (void)
{
  struct MHD_Daemon *daemon;
  enum MHD_FLAG flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_DEBUG
                        | MHD_USE_PEDANTIC_CHECKS;

  add_route ("/movie", movie_handler);
  add_route ("/movies", movies_handler);

  daemon = MHD_start_daemon (flags, PORT, NULL, NULL, &answer_connection, NULL,
                             MHD_OPTION_END);

  if (NULL == daemon)
    {
      return EXIT_FAILURE;
    }

  getchar ();

  MHD_stop_daemon (daemon);

  return EXIT_SUCCESS;
}
