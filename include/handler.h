#ifndef _HANDLER_H
#define _HANDLER_H

#include <jansson.h>

typedef struct {
  struct MHD_Connection *connection;
  const char *url;
  const char *method;
  const char *upload_data;
  const size_t *upload_data_size;
} RequestContext;

typedef json_t *(*Handler)(const RequestContext *req_ctx);

/**
 * Handle requests to '/'
 *
 * @param req_ctx context to use in order to react differently based on request
 * data
 * @return JSON-formatted response
 */
json_t *root_handler(const RequestContext *req_ctx);

/**
 * Handle requests to '/movies'
 *
 * @param req_ctx context to use in order to react differently based on request
 * data
 * @return JSON-formatted response
 *
 * This handler expects the following query string parameters
 *   - search_type: type of search (contains[default], exact, startswith,
 *     endswith)
 *   - search_pattern: string to search for
 *   - search_by: column to search by (e.g., title, cast, release_date, etc.)
 *     see database schema for available fields
 */
json_t *movies_handler(const RequestContext *req_ctx);

#endif  // _HANDLER_H
