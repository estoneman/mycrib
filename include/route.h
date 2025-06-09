#ifndef _ROUTE_H
#define _ROUTE_H

#include <jansson.h>
#include <microhttpd.h>

typedef struct {
  char *path;
  json_t *(*handler)(struct MHD_Connection *);
} Route;

uint16_t small_crc16_8005(const char *, size_t);
void new_route(Route *, const char *, json_t *(*)(struct MHD_Connection *));
void del_route(Route *, const char *);

#endif  // _ROUTE_H
