#include "db.h"

#include <assert.h>
#include <jansson.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int build_stmt_va(sqlite3_stmt **pp_stmt, sqlite3 *db, const char *query,
                  size_t n, ...) {
  int rc;

  if ((rc = sqlite3_prepare_v2(db, query, strlen(query) + 1, pp_stmt, NULL)) !=
      SQLITE_OK) {
    fprintf(stderr, "[ERROR] sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  va_list ap;
  va_start(ap, n);

  for (size_t i = 0; i < n; ++i) {
    char *field = va_arg(ap, char *);

    if ((rc = sqlite3_bind_text(*pp_stmt, i + 1, field, strlen(field),
                                SQLITE_TRANSIENT)) != SQLITE_OK) {
      fprintf(stderr, "[ERROR] failed to bind query parameter to SQL query\n");
      return -1;
    }
  }

  va_end(ap);

  return 0;
}

sqlite3 *db_connect(void) {
  sqlite3 *db;
  int rc;
  const char *db_file = "../sql/mycrib.db";

  rc = sqlite3_open(db_file, &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);

    return NULL;
  }

  int old_val;

  if ((rc = sqlite3_db_config(db, SQLITE_DBCONFIG_DEFENSIVE, 1, &old_val)) !=
      SQLITE_OK) {
    fprintf(stderr, "Could not set SQLITE_DBCONFIG_DEFENSIVE: %s\n",
            sqlite3_errmsg(db));
    sqlite3_close(db);

    return NULL;
  }

  return db;
}

int db_close(sqlite3 *db) {
  int rc;

  if ((rc = sqlite3_close(db)) != SQLITE_OK) {
    fprintf(stderr, "Can't close database: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  return 0;
}

json_t *db_query_exec(sqlite3 *db, sqlite3_stmt *pp_stmt,
                      ResultSetType rs_type) {
  int rc;

  json_t *record_set = json_array();
  json_t *record;

  while ((rc = sqlite3_step(pp_stmt)) == SQLITE_ROW) {
    switch (rs_type) {
      case TYPE_MOVIE:
        record = json_pack("{s:s,s:s,s:i,s:i,s:s,s:f,s:s,s:s,s:f}", "title",
                           sqlite3_column_text(pp_stmt, 1), "genre",
                           sqlite3_column_text(pp_stmt, 2), "year",
                           sqlite3_column_int(pp_stmt, 3), "length",
                           sqlite3_column_int(pp_stmt, 4), "poster_url",
                           sqlite3_column_text(pp_stmt, 5), "rating_family",
                           sqlite3_column_double(pp_stmt, 6), "cast",
                           sqlite3_column_text(pp_stmt, 7), "director",
                           sqlite3_column_text(pp_stmt, 8), "rating_imdb",
                           sqlite3_column_double(pp_stmt, 9));
        if (!record || json_array_append_new(record_set, record)) continue;
        break;
      default:
        assert(0 && "UNIMPLEMENTED");
    }
  }

  if (rc == SQLITE_ERROR || rc == SQLITE_MISUSE) {
    fprintf(stderr, "[ERROR]: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(pp_stmt);

  return record_set;
}

// vim: ts=2 sts=2 sw=2 et ai
