#ifndef _ROUTE_H
#define _ROUTE_H

#include <jansson.h>
#include <microhttpd.h>

typedef union {
  json_t *(*handler_arg)(void *req_ctx);
  json_t *(*handler_void)(void);
} Handler;

typedef enum { HANDLER_ARG, HANDLER_VOID } HandlerType;

typedef struct {
  char *path;
  HandlerType type;
  Handler handler;
} Route;

typedef struct {
  struct MHD_Connection *connection;
  const char *method;
  const char *upload_data;
  const size_t *upload_data_size;
} RequestContext;

uint16_t small_crc16_8005(const char *m, size_t n);
void new_route(Route *routes, const char *path, HandlerType type,
               Handler handler);
void del_route(Route *routes, const char *path);
enum MHD_Result router(Route *routes, const char *url, RequestContext *req_ctx);

#endif  // _ROUTE_H
