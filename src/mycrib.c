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
#define ROUTE_TABLE_SIZE (2 << 15)

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
  char *error, *response;
  size_t url_len, error_len;

  url_len = strlen (url);
  error_len = 1024 + (url_len > MAX_ROUTE_LEN ? MAX_ROUTE_LEN : url_len);

  if ((error = malloc (error_len + 1)) == NULL)
    {
      page = "{\"error\": \"Internal Server Error\"}";

      return send_response (connection, page, strlen (page),
                            MHD_HTTP_INTERNAL_SERVER_ERROR);
    }

  unsigned id = small_crc16_8005 (url, url_len);

  if (!routes[id].path)
    {
      page = "{\"error\": \"%s '%s'\"}";
      snprintf (error, error_len, page,
                MHD_get_reason_phrase_for (MHD_HTTP_NOT_FOUND), url);

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
  enum MHD_FLAG flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_DEBUG
                        | MHD_USE_PEDANTIC_CHECKS | MHD_USE_TCP_FASTOPEN
                        | MHD_USE_ITC | MHD_USE_THREAD_PER_CONNECTION;

  Route *routes;

  if ((routes = (Route *)calloc (ROUTE_TABLE_SIZE, sizeof (Route))) == NULL)
    {
      fprintf (stderr, "[FATAL] Failed to allocate route table\n");
      exit (1);
    }

  new_route (routes, "/", root_handler);
  new_route (routes, "/movie", movie_handler);
  new_route (routes, "/movies", movies_handler);

  daemon = MHD_start_daemon (flags, PORT, NULL, NULL, &answer_connection,
                             &routes, MHD_OPTION_END);

  if (NULL == daemon)
    {
      return EXIT_FAILURE;
    }

  getchar ();

  MHD_stop_daemon (daemon);

  del_route (routes, "/");
  del_route (routes, "/movie");
  del_route (routes, "/movies");

  return EXIT_SUCCESS;
}
