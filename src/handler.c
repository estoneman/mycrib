#include "handler.h"

#include <jansson.h>
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"

#define JSON_ERROR_FMT "{s:i,s:s}"
#define JSON_ROOT_FMT JSON_ERROR_FMT
#define JSON_RESULT_FMT "{s:i,s:O}"

json_t *root_handler(void) {
  json_t *result;

  result = json_pack(JSON_ROOT_FMT, "status", MHD_HTTP_OK, "result",
                     "['/', '/movies']");

  return result;
}

// @deprecated
char *movie_handler(struct MHD_Connection *connection) {
  const char *sql, *title;
  int rc;
  sqlite3 *db;
  sqlite3_stmt *pp_stmt;
  json_t *record_set, *result, *err;

  title =
      MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "title");
  if (!title) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_BAD_REQUEST, "error",
                    "Title was not provided");

    return json_dumps(err, 0);
  }

  if ((db = db_connect()) == NULL) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                    "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  sql = "SELECT * FROM movies WHERE title LIKE ?;";
  if (build_stmt_va(&pp_stmt, db, sql, 1, title) == -1) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                    "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  record_set = db_query_exec(db, pp_stmt, TYPE_MOVIE);

  if (!record_set) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                    "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  if ((rc = db_close(db)) != 0) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                    "error", "Internal Server Error");

    return json_dumps(err, 0);
  }

  result = json_pack("{s:i,s:O}", "status", MHD_HTTP_OK, "result", record_set);
  json_decref(record_set);

  return json_dumps(result, 0);
}

json_t *movies_handler(struct MHD_Connection *connection, const char *method) {
  // TODO: dispatch based off http method
  (void)method;

  const char *sql, *search_type, *search_pattern;
  char *search_pattern_like;
  int rc;
  size_t len_search_pattern, len_search_pattern_like;
  sqlite3 *db;
  sqlite3_stmt *pp_stmt;

  json_t *record_set, *result, *err;

  search_type = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND,
                                            "search_type");

  if (!search_type) {
    search_type = "contains";
  }

  search_pattern = MHD_lookup_connection_value(
      connection, MHD_GET_ARGUMENT_KIND, "search_pattern");

  if (!search_pattern) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_BAD_REQUEST, "error",
                    "A search pattern must be provided");

    return err;
  }

  if ((db = db_connect()) == NULL) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                    "error", "Internal Server Error");

    return err;
  }

  len_search_pattern = strlen(search_pattern);

  if (strncmp(search_type, "contains", strlen("contains")) == 0) {
    len_search_pattern_like = len_search_pattern + 2;  // 2 '%'

    if ((search_pattern_like = malloc(len_search_pattern_like + 1)) == NULL) {
      err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "error", "Internal Server Error");

      return err;
    }

    snprintf(search_pattern_like, len_search_pattern_like + 1, "%%%s%%",
             search_pattern);

    sql = "SELECT * FROM movies WHERE title LIKE ?;";
    if (build_stmt_va(&pp_stmt, db, sql, 1, search_pattern_like) == -1) {
      err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "error", "Internal Server Error");

      return err;
    }

    free(search_pattern_like);
  } else if (strncmp(search_type, "endswith", strlen("endswith")) == 0) {
    len_search_pattern_like = len_search_pattern + 1;  // 1 '%'

    if ((search_pattern_like = malloc(len_search_pattern_like + 1)) == NULL) {
      err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "error", "Internal Server Error");

      return err;
    }

    snprintf(search_pattern_like, len_search_pattern_like + 1, "%%%s",
             search_pattern);

    sql = "SELECT * FROM movies WHERE title LIKE ?;";
    if (build_stmt_va(&pp_stmt, db, sql, 1, search_pattern_like) == -1) {
      err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "error", "Internal Server Error");

      return err;
    }

    free(search_pattern_like);
  } else if (strncmp(search_type, "startswith", strlen("startswith")) == 0) {
    len_search_pattern_like = len_search_pattern + 1;  // 1 '%'

    if ((search_pattern_like = malloc(len_search_pattern_like + 1)) == NULL) {
      err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "error", "Internal Server Error");

      return err;
    }

    snprintf(search_pattern_like, len_search_pattern_like + 1, "%s%%",
             search_pattern);

    sql = "SELECT * FROM movies WHERE title LIKE ?;";
    if (build_stmt_va(&pp_stmt, db, sql, 1, search_pattern_like) == -1) {
      err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "error", "Internal Server Error");

      return err;
    }

    free(search_pattern_like);
  } else if (strncmp(search_type, "exact", strlen("exact")) == 0) {
    sql = "SELECT * FROM movies WHERE title = ?;";
    if (build_stmt_va(&pp_stmt, db, sql, 1, search_pattern) == -1) {
      err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                      "error", "Internal Server Error");

      return err;
    }
  } else {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_BAD_REQUEST, "error",
                    "Unsupported search type");
    return err;
  }

  record_set = db_exec_movie(db, pp_stmt);

  if (!record_set) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                    "error", "Internal Server Error");

    return err;
  }

  result =
      json_pack(JSON_RESULT_FMT, "status", MHD_HTTP_OK, "result", record_set);
  json_decref(record_set);

  if ((rc = db_close(db)) != 0) {
    err = json_pack(JSON_ERROR_FMT, "status", MHD_HTTP_INTERNAL_SERVER_ERROR,
                    "error", "Internal Server Error");

    return err;
  }

  return result;
}
