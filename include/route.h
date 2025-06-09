#ifndef _ROUTE_H
#define _ROUTE_H

#include <jansson.h>
#include <microhttpd.h>

typedef struct {
  char *path;
  json_t *(*handler)(struct MHD_Connection *);
} Route;

uint16_t small_crc16_8005(const char *m, size_t n);
void new_route(Route *routes, const char *path,
               json_t *(*handler)(struct MHD_Connection *));
void del_route(Route *routes, const char *path);
enum MHD_Result router(Route *routes, struct MHD_Connection *connection,
                       const char *url);

#endif  // _ROUTE_H
