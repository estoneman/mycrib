// #include <microhttpd.h>
#include "handler.h"

#include <jansson.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"

#define JSON_ERROR_FMT "{s:i,s:s}"

char *root_handler(struct MHD_Connection *connection) {
  (void)connection;
  return NULL;
}

char *movie_handler(struct MHD_Connection *connection) {
  const char *query, *title;
  char *title_like;
  int rc;
  sqlite3 *db;
  sqlite3_stmt *pp_stmt;
  size_t len_title, len_title_like;

  json_t *record_set, *err;

  title =
      MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "title");

  if (!title) {
    err = json_pack(JSON_ERROR_FMT, "status code", MHD_HTTP_BAD_REQUEST,
                    "error", "Title was not provided");

    return json_dumps(err, 0);
  }

  len_title = strlen(title);
  len_title_like = len_title + 2;  // 2 for '%'

  if ((title_like = malloc(len_title_like + 1)) == NULL) {
    err =
        json_pack(JSON_ERROR_FMT, "status code", MHD_HTTP_INTERNAL_SERVER_ERROR,
                  "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  snprintf(title_like, len_title_like + 1, "%%%s%%", title);

  if ((db = db_connect()) == NULL) {
    err =
        json_pack(JSON_ERROR_FMT, "status code", MHD_HTTP_INTERNAL_SERVER_ERROR,
                  "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  query = "SELECT * FROM movies WHERE title LIKE ?;";
  if (build_stmt_va(&pp_stmt, db, query, 1, title_like) == -1) {
    err =
        json_pack(JSON_ERROR_FMT, "status code", MHD_HTTP_INTERNAL_SERVER_ERROR,
                  "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  record_set = db_query_exec(db, pp_stmt, TYPE_MOVIE);

  if (!record_set) {
    err =
        json_pack(JSON_ERROR_FMT, "status code", MHD_HTTP_INTERNAL_SERVER_ERROR,
                  "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  if ((rc = db_close(db)) != 0) {
    err =
        json_pack(JSON_ERROR_FMT, "status code", MHD_HTTP_INTERNAL_SERVER_ERROR,
                  "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  return json_dumps(record_set, 0);
}

char *movies_handler(struct MHD_Connection *connection) {
  (void)connection;
  return NULL;
}
