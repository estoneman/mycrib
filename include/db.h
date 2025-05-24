#ifndef _DB_H
#define _DB_H

#include <jansson.h>
#include <sqlite3.h>
#include <stddef.h>
#include <stdlib.h>

typedef enum { TYPE_MOVIE, TYPE_BOOK, TYPE_SONG } ResultSetType;

sqlite3 *db_connect(void);
int db_close(sqlite3 *);

/**
 * Execute a query and return its results, if any
 *
 * @param db handle to the sqlite database
 * @param pp_stmt sqlite prepared statement
 * @param schema schema to use when marshalling database results
 * @return JSON array of result sets
 */
json_t *db_query_exec(sqlite3 *, sqlite3_stmt *, ResultSetType);

/**
 * Builds an sqlite3_stmt to be used by database query executor.
 *
 * @param pp_stmt reference to a sqlite3_stmt* (this value will be filled upon
 * success)
 * @param db database connection handle
 * @param query sqlite query to execute
 * @param n number of arguments needing to be bound into query
 * @param ... the values of the n arguments (so far only char* types are
 * supported)
 * @return 0 on success, -1 on error
 */
int build_stmt_va(sqlite3_stmt **, sqlite3 *, const char *, size_t, ...);

#endif  // _DB_H
