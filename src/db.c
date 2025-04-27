#include "db.h"

#include <sqlite3.h>
#include <stdio.h>

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

  return db;
}

int db_close(sqlite3 *db) {
  int rc;

  if ((rc = sqlite3_close(db)) != SQLITE_OK) {
    fprintf(stderr, "Can't close database: rc=%d\n", rc);
    return -1;
  }

  return 0;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  (void)NotUsed;

  int i;
  for (i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int db_query_exec(sqlite3 *db, const char *query) {
  char *errmsg = 0;

  sqlite3_exec(db, query, callback, 0, &errmsg);

  return 0;
}
