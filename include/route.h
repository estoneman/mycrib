#ifndef _ROUTE_H
#define _ROUTE_H

#include <jansson.h>
#include <microhttpd.h>

typedef union {
  json_t *(*handler_arg)(struct MHD_Connection *, const char *);
  json_t *(*handler_void)(void);
} Handler;

typedef enum { HANDLER_ARG, HANDLER_VOID } HandlerType;

typedef struct {
  char *path;
  HandlerType type;
  Handler handler;
} Route;

uint16_t small_crc16_8005(const char *m, size_t n);
void new_route(Route *routes, const char *path, HandlerType type,
               Handler handler);
void del_route(Route *routes, const char *path);
enum MHD_Result router(Route *routes, struct MHD_Connection *connection,
                       const char *method, const char *url);

#endif  // _ROUTE_H
