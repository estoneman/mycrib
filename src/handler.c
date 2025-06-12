#include "handler.h"

#include <assert.h>
#include <jansson.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#include "db.h"
#include "route.h"

#define JSON_ERROR_FMT "{s:i,s:s}"
#define JSON_ROOT_FMT JSON_ERROR_FMT

json_t *root_handler(void) {
  json_t *result;

  result = json_pack(JSON_ROOT_FMT, "status", MHD_HTTP_OK, "result",
                     "['/', '/movies']");

  return result;
}

json_t *movies_handler(const RequestContext *ctx) {
  json_t *rows;

  if (strncmp(ctx->method, MHD_HTTP_METHOD_GET, strlen(MHD_HTTP_METHOD_GET)) ==
      0) {
    const char *search_type, *search_pattern;

    search_type = MHD_lookup_connection_value(
        ctx->connection, MHD_GET_ARGUMENT_KIND, "search_type");

    if (!search_type) search_type = "contains";

    search_pattern = MHD_lookup_connection_value(
        ctx->connection, MHD_GET_ARGUMENT_KIND, "search_pattern");

    if (!search_pattern)
      return json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_BAD_REQUEST, "error",
                       "A search pattern must be provided");

    rows = db_read_movies(search_pattern, search_type);
    if (!rows)
      return json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                       "error", "Internal Server Error");

    return rows;
  } else {
    fprintf(stderr, "Unimplemented handler for HTTP %s\n", ctx->method);
    assert(0);
  }
}
