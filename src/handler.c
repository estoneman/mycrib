#include "handler.h"

#include <assert.h>
#include <jansson.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#include "db.h"

#define JSON_ERROR_FMT "{s:i,s:s}"
#define JSON_ROOT_FMT JSON_ERROR_FMT

json_t *
root_handler (const RequestContext *req_ctx)
{
  json_t *result;

  if (strncmp (req_ctx->method, MHD_HTTP_METHOD_GET,
               strlen (MHD_HTTP_METHOD_GET))
      != 0)
    {
      return json_pack (
          JSON_ERROR_FMT, "status", MHD_HTTP_METHOD_NOT_ALLOWED, "result",
          MHD_get_reason_phrase_for (MHD_HTTP_METHOD_NOT_ALLOWED));
    }

  result = json_pack (JSON_ROOT_FMT, "status", MHD_HTTP_OK, "result",
                      "['/', '/movies']");

  return result;
}

json_t *
movies_handler (const RequestContext *req_ctx)
{
  json_t *rows;

  if (strncmp (req_ctx->method, MHD_HTTP_METHOD_GET,
               strlen (MHD_HTTP_METHOD_GET))
      == 0)
    {
      const char *search_type, *search_pattern, *search_by;

      search_type = MHD_lookup_connection_value (
          req_ctx->connection, MHD_GET_ARGUMENT_KIND, "search_type");

      if (!search_type)
        search_type = "contains";

      search_by = MHD_lookup_connection_value (
          req_ctx->connection, MHD_GET_ARGUMENT_KIND, "search_by");

      if (!search_by)
        search_by = "title";

      search_pattern = MHD_lookup_connection_value (
          req_ctx->connection, MHD_GET_ARGUMENT_KIND, "search_pattern");

      if (!search_pattern)
        return json_pack (JSON_ERROR_FMT, "status", MHD_HTTP_BAD_REQUEST,
                          "error", "A search pattern must be provided");

      rows = db_read_movies (search_pattern, search_type, search_by);
      if (!rows)
        return json_pack (JSON_ERROR_FMT, "status",
                          MHD_HTTP_INTERNAL_SERVER_ERROR, "error",
                          "Internal Server Error");

      return rows;
    }
  else
    {
      fprintf (stderr, "Unimplemented handler for HTTP %s\n", req_ctx->method);
      assert (0);
    }
}
