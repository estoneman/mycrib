#ifndef _HANDLER_H
#define _HANDLER_H

#include <jansson.h>

#include "route.h"

/**
 * Handle requests to '/'
 *
 * @return JSON-formatted response
 */
json_t *root_handler(void);

/**
 * Handle requests to '/movies'
 *
 * @param ctx context to use in order to react differently based on request data
 * @return JSON-formatted response
 */
json_t *movies_handler(const RequestContext *ctx);

#endif  // _HANDLER_H
