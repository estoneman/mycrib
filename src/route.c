/* SUPPORTED API ROUTES
 *   - Home page: GET /
 *   - Get all movies: GET /movies
 *   - Get movie: GET /movie?title=<title>&search_type=<type>
 */

#include "route.h"

#include <microhttpd.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"

#define MAXPAGE 1024

uint16_t small_crc16_8005(const char *m, size_t n) {
  uint32_t crc = 0;

  for (size_t i = 0; i < n; i++) {
    uint8_t d = *(m++);
    uint32_t x = ((crc ^ d) & 0xff) << 8;
    uint32_t y = x;

    x ^= x << 1;
    x ^= x << 2;
    x ^= x << 4;

    x = (x & 0x8000) | (y >> 1);

    crc = (crc >> 8) ^ (x >> 15) ^ (x >> 1) ^ x;
  }
  return crc;
}

void new_route(Route *routes, const char *path,
               char *(*handler)(struct MHD_Connection *)) {
  Route route;

  size_t path_len = strlen(path);
  unsigned int id = small_crc16_8005(path, path_len);
  if ((route.path = (char *)malloc(path_len + 1)) == NULL) {
    fprintf(stderr, "[ERROR] failed to allocate new route, OOM\n");
    exit(1);
  }

  strncpy(route.path, path, path_len);
  route.path[path_len] = '\0';

  route.handler = handler;

  routes[id] = route;

  fprintf(stderr, "[INFO] new route: '%s'\n", route.path);
}

void del_route(Route *routes, const char *path) {
  size_t path_len = strlen(path);
  unsigned int id = small_crc16_8005(path, path_len);

  if (routes[id].path) free(routes[id].path);
}

char *root_handler(struct MHD_Connection *connection) {
  (void)connection;

  char *page;
  if ((page = malloc(MAXPAGE)) == NULL) {
    fprintf(stderr, "[FATAL] ran out of memory\n");
    return NULL;
  }

  strcpy(page, "{\"handlers\":[\"/\",\"/movie\",\"/movies\"]}");

  return page;
}

char *movie_handler(struct MHD_Connection *connection) {
  (void)connection;
  char *page;
  sqlite3 *db;

  if ((page = malloc(MAXPAGE)) == NULL) {
    fprintf(stderr, "[FATAL] ran out of memory\n");
    return NULL;
  }

  if ((db = db_connect()) == NULL) {
    strcpy(page, "{\"error\":\"Database failed to open\"}");
    return page;
  }

  strcpy(page, "{\"handler\":\"MOVIE HANDLER\"}");

  db_close(db);

  return page;
}

char *movies_handler(struct MHD_Connection *connection) {
  char *page;
  const char *search_term, *search_by;

  /* - Get movie: GET /movie?search_term=<term>&search_by=<column> */
  search_term =
      MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "title");
  search_by = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND,
                                          "search_type");

  fprintf(stderr, "[INFO] received search_term=%s, search_by=%s\n", search_term,
          search_by);

  if ((page = malloc(MAXPAGE)) == NULL) {
    fprintf(stderr, "[FATAL] ran out of memory\n");
    return NULL;
  }

  strcpy(page, "{\"handler\":\"MOVIES HANDLER\"}");

  return page;
}
