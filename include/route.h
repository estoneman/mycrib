#ifndef _ROUTE_H
#define _ROUTE_H

#include <microhttpd.h>

typedef struct {
  char *path;
  char *(*handler)(struct MHD_Connection *);
} Route;

uint16_t small_crc16_8005(const char *, size_t);
void new_route(Route *, const char *, char *(*)(struct MHD_Connection *));
void del_route(Route *, const char *);

#endif  // _ROUTE_H
