#ifndef _ROUTE_H
#define _ROUTE_H

#include <jansson.h>
#include <microhttpd.h>

#include "handler.h"

typedef struct {
  char *path;
  Handler handler;
} Route;

/**
 * Create a new route and assign various properties to it
 *
 * @param routes pointer to currently configured routes
 * @param path which URL should be handled
 * @param type type of handler (one of: HANDLER_ARG, HANDLER_VOID)
 */
void new_route(Route *routes, const char *path, Handler handler);

/**
 * Delete a route for a given path
 *
 * @param routes currently configured routes
 * @param path which URL should be deleted
 */
void del_route(Route *routes, const char *path);

/**
 * Route requests to the proper handler
 *
 * @param routes currently configured routes
 * @param req_ctx context for the handler to use (e.g., method, url, etc.)
 * @return result of the handler's operation
 */
enum MHD_Result router(Route *routes, const RequestContext *req_ctx);

#endif  // _ROUTE_H
