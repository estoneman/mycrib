#ifndef _DB_H
#define _DB_H

#include <jansson.h>
#include <sqlite3.h>

/**
 * Open connection sqlite3 database
 *
 * @return pointer to sqlite3 database, NULL on error
 */
sqlite3 *db_connect(void);

/**
 * Close database handle
 *
 * @param db handle to sqlite3 database
 * @return 0 on success, -1 on error
 */
int db_close(sqlite3 *db);

/**
 * Returns records of movies that match the given search pattern and type
 *
 * @param connection object that stores data to manipulate the sql query
 * @return JSON-formatted response, regardless if the operation succeeded or not
 *
 */
json_t *db_read_movies(const char *search_pattern, const char *search_type,
                       const char *search_by);

/**
 * Run prepared statement against database
 *
 * @param db handle to the sqlite database
 * @param pp_stmt sqlite prepared statement to execute against db
 * @return JSON array of result sets
 */
json_t *db_exec_read_movies(sqlite3 *db, sqlite3_stmt *pp_stmt);

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
