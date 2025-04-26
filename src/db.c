#include <sqlite3.h>
#include <stdio.h>

#include "db.h"

#define UNUSED(x) (void)x
#define SQL_DUMP "SELECT * FROM movies;"

static int
callback (void *NotUsed, int argc, char **argv, char **azColName)
{
  UNUSED (NotUsed);

  int i;
  for (i = 0; i < argc; i++)
    {
      printf ("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
  printf ("\n");
  return 0;
}

int
db_connect (void)
{
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  const char *db_file = "../sql/mycrib.db";

  rc = sqlite3_open (db_file, &db);
  if (rc)
    {
      fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return (1);
    }
  rc = sqlite3_exec (db, SQL_DUMP, callback, 0, &zErrMsg);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free (zErrMsg);
    }
  sqlite3_close (db);
  return 0;
}
