#ifndef _DB_H
#define _DB_H

#include <sqlite3.h>

sqlite3 *db_connect(void);
int db_close(sqlite3 *);
int db_query_exec(sqlite3 *, const char *);

#endif  // _DB_H
