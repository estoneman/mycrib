/* SUPPORTED API ROUTES
 *   - Home page: GET /
 *   - Get all movies: GET /movies
 *   - Get movie: GET /movie?title=<title>&search_type=<type>
 */

#include "route.h"

#include <assert.h>
#include <microhttpd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "util/mhd.h"
#include "util/util.h"

#define MAX_ROUTE_LEN 64
#define ERR_TEMPLATE "{\"status\": %i, \"result\": \"%s\"}"

void new_route(Route *routes, const char *path, HandlerType type,
               Handler handler) {
  Route route;

  size_t path_len = strlen(path);
  unsigned int id = small_crc16_8005(path, path_len);
  if ((route.path = (char *)malloc(path_len + 1)) == NULL) {
    fprintf(stderr, "[ERROR] failed to allocate new route, OOM\n");
    exit(1);
  }

  snprintf(route.path, path_len + 1, "%s", path);
  route.path[path_len] = '\0';

  route.type = type;
  route.handler = handler;

  routes[id] = route;

  fprintf(stderr, "[INFO] new route: '%s'\n", route.path);
}

void del_route(Route *routes, const char *path) {
  size_t path_len = strlen(path);
  unsigned int id = small_crc16_8005(path, path_len);

  if (routes[id].path) free(routes[id].path);
}

enum MHD_Result router(Route *routes, struct MHD_Connection *connection,
                       const char *method, const char *url) {
  (void)method;
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

  switch (routes[id].type) {
    case HANDLER_ARG:
      result = routes[id].handler.handler_arg(connection, method);
      break;
    case HANDLER_VOID:
      result = routes[id].handler.handler_void();
      break;
    default:
      assert(0 && "UNIMPLEMENTED");
  }

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
