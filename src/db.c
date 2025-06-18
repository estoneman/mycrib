#include "db.h"

#include <assert.h>
#include <jansson.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_OK_CODE 200
#define HTTP_OK_MSG "OK"
#define HTTP_BAD_REQUEST_CODE 400
#define HTTP_BAD_REQUEST_MSG "Bad Request"
#define HTTP_INTERNAL_SERVER_ERROR_CODE 500
#define HTTP_INTERNAL_SERVER_ERROR_MSG "Internal Server Error"

#define JSON_RESULT_FMT "{s:i,s:O}"
#define JSON_ERROR_FMT "{s:i,s:s}"

int
build_stmt_va (sqlite3_stmt **pp_stmt, sqlite3 *db, const char *query,
               size_t n, ...)
{
  int rc;

  if ((rc = sqlite3_prepare_v2 (db, query, strlen (query) + 1, pp_stmt, NULL))
      != SQLITE_OK)
    {
      fprintf (stderr, "[ERROR] sqlite3_prepare_v2 failed: %s\n",
               sqlite3_errmsg (db));
      return -1;
    }

  va_list ap;
  va_start (ap, n);

  for (size_t i = 0; i < n; ++i)
    {
      char *field = va_arg (ap, char *);

      if ((rc = sqlite3_bind_text (*pp_stmt, i + 1, field, strlen (field),
                                   SQLITE_TRANSIENT))
          != SQLITE_OK)
        {
          fprintf (stderr,
                   "[ERROR] failed to bind query parameter to SQL query: %s\n",
                   sqlite3_errmsg (db));
          return -1;
        }
    }

  va_end (ap);

  return 0;
}

sqlite3 *
db_connect (void)
{
  sqlite3 *db;
  int rc, old_val;
  const char *db_file = "../sql/mycrib.db";

  rc = sqlite3_open (db_file, &db);
  if (rc)
    {
      fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);

      return NULL;
    }

  if ((rc = sqlite3_db_config (db, SQLITE_DBCONFIG_DEFENSIVE, 1, &old_val))
      != SQLITE_OK)
    {
      fprintf (stderr, "Could not set SQLITE_DBCONFIG_DEFENSIVE: %s\n",
               sqlite3_errmsg (db));
      sqlite3_close (db);

      return NULL;
    }

  return db;
}

int
db_close (sqlite3 *db)
{
  int rc;

  if ((rc = sqlite3_close (db)) != SQLITE_OK)
    {
      fprintf (stderr, "Can't close database: %s\n", sqlite3_errmsg (db));
      return -1;
    }

  return 0;
}

json_t *
db_read_movies (const char *search_pattern, const char *search_type,
                const char *search_by)
{
  size_t len_search_pattern, len_search_pattern_like;
  char *search_pattern_like, *sql_query_fmt, *sql_query;
  json_t *record_set, *result;
  int rc, sql_query_len;
  sqlite3 *db;
  sqlite3_stmt *pp_stmt;

  if (!(db = db_connect ()))
    return NULL;

  len_search_pattern = strlen (search_pattern);

  if (strncmp (search_type, "contains", strlen ("contains")) == 0)
    {
      len_search_pattern_like = len_search_pattern + 2; // 2 '%'

      if (!(search_pattern_like = malloc (len_search_pattern_like + 1)))
        return NULL;

      snprintf (search_pattern_like, len_search_pattern_like + 1, "%%%s%%",
                search_pattern);

      sql_query_fmt = "SELECT * FROM movies WHERE %s LIKE ?;";
      sql_query_len = snprintf (NULL, 0, sql_query_fmt, search_by);
      if (!(sql_query = malloc (sql_query_len + 1)))
        {
          free (search_pattern_like);
          return NULL;
        }
      snprintf (sql_query, sql_query_len + 1, sql_query_fmt, search_by);

      if (build_stmt_va (&pp_stmt, db, sql_query, 1, search_pattern_like)
          == -1)
        {
          free (sql_query);
          free (search_pattern_like);
          return NULL;
        }

      free (sql_query);
      free (search_pattern_like);
    }
  else if (strncmp (search_type, "endswith", strlen ("endswith")) == 0)
    {
      len_search_pattern_like = len_search_pattern + 1; // 1 '%'

      if (!(search_pattern_like = malloc (len_search_pattern_like + 1)))
        return NULL;

      snprintf (search_pattern_like, len_search_pattern_like + 1, "%%%s",
                search_pattern);

      sql_query_fmt = "SELECT * FROM movies WHERE %s LIKE ?;";
      sql_query_len = snprintf (NULL, 0, sql_query_fmt, search_by);
      if (!(sql_query = malloc (sql_query_len + 1)))
        {
          free (search_pattern_like);
          return NULL;
        }
      snprintf (sql_query, sql_query_len + 1, sql_query_fmt, search_by);

      if (build_stmt_va (&pp_stmt, db, sql_query, 1, search_pattern_like)
          == -1)
        {
          free (sql_query);
          free (search_pattern_like);
          return NULL;
        }

      free (sql_query);
      free (search_pattern_like);
    }
  else if (strncmp (search_type, "startswith", strlen ("startswith")) == 0)
    {
      len_search_pattern_like = len_search_pattern + 1; // 1 '%'

      if (!(search_pattern_like = malloc (len_search_pattern_like + 1)))
        return NULL;

      snprintf (search_pattern_like, len_search_pattern_like + 1, "%s%%",
                search_pattern);

      sql_query = "SELECT * FROM movies WHERE %s LIKE ?;";
      sql_query_fmt = "SELECT * FROM movies WHERE %s LIKE ?;";
      sql_query_len = snprintf (NULL, 0, sql_query_fmt, search_by);
      if (!(sql_query = malloc (sql_query_len + 1)))
        {
          free (search_pattern_like);
          return NULL;
        }
      snprintf (sql_query, sql_query_len + 1, sql_query_fmt, search_by);

      if (build_stmt_va (&pp_stmt, db, sql_query, 1, search_pattern_like)
          == -1)
        {
          free (sql_query);
          free (search_pattern_like);
          return NULL;
        }

      free (sql_query);
      free (search_pattern_like);
    }
  else if (strncmp (search_type, "exact", strlen ("exact")) == 0)
    {
      sql_query_fmt = "SELECT * FROM movies WHERE %s = ?;";
      sql_query_len = snprintf (NULL, 0, sql_query_fmt, search_by);
      if (!(sql_query = malloc (sql_query_len + 1)))
        return NULL;
      snprintf (sql_query, sql_query_len + 1, sql_query_fmt, search_by);

      if (build_stmt_va (&pp_stmt, db, sql_query, 1, search_pattern) == -1)
        {
          free (sql_query);
          return NULL;
        }

      free (sql_query);
    }
  else
    {
      return json_pack (JSON_ERROR_FMT, "status", HTTP_BAD_REQUEST_CODE,
                        "error", "Unsupported search type");
    }

  record_set = db_exec_read_movies (db, pp_stmt);

  if (!record_set)
    return NULL;

  result = json_pack (JSON_RESULT_FMT, "status", HTTP_OK_CODE, "result",
                      record_set);
  json_decref (record_set);

  if ((rc = db_close (db)) != 0)
    return NULL;

  return result;
}

json_t *
db_exec_read_movies (sqlite3 *db, sqlite3_stmt *pp_stmt)
{
  int rc;

  json_t *record_set = json_array ();
  json_t *record;

  while ((rc = sqlite3_step (pp_stmt)) == SQLITE_ROW)
    {
      record = json_pack ("{s:s,s:s,s:i,s:i,s:s,s:f,s:s,s:s,s:f}", "title",
                          sqlite3_column_text (pp_stmt, 1), "genre",
                          sqlite3_column_text (pp_stmt, 2), "year",
                          sqlite3_column_int (pp_stmt, 3), "length",
                          sqlite3_column_int (pp_stmt, 4), "poster_url",
                          sqlite3_column_text (pp_stmt, 5), "rating_family",
                          sqlite3_column_double (pp_stmt, 6), "cast",
                          sqlite3_column_text (pp_stmt, 7), "director",
                          sqlite3_column_text (pp_stmt, 8), "rating_imdb",
                          sqlite3_column_double (pp_stmt, 9));
      if (!record || json_array_append_new (record_set, record))
        continue;
    }

  if (rc == SQLITE_ERROR || rc == SQLITE_MISUSE)
    {
      fprintf (stderr, "[ERROR]: %s\n", sqlite3_errmsg (db));
    }

  sqlite3_finalize (pp_stmt);

  return record_set;
}

// vim: ts=2 sts=2 sw=2 et ai
