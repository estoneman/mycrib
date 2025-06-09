#include "util/mhd.h"

#include <stdlib.h>
#include <string.h>

void mem_free(void *mem) {
  if (mem) free(mem);
}

enum MHD_Result send_response(struct MHD_Connection *connection,
                              const char *buf, size_t bufsize, int code) {
  struct MHD_Response *response;
  int ret;

  response = MHD_create_response_from_buffer(bufsize, (void *)buf,
                                             MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE,
                          "application/json");
  ret = MHD_queue_response(connection, code, response);
  MHD_destroy_response(response);

  return ret;
}

enum MHD_Result send_response_free_callback(
    struct MHD_Connection *connection, const char *buf, size_t bufsize,
    int code, MHD_ContentReaderFreeCallback crfc) {
  struct MHD_Response *response;
  int ret;

  response = MHD_create_response_from_buffer_with_free_callback(
      bufsize, (void *)buf, crfc);
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE,
                          "application/json");
  ret = MHD_queue_response(connection, code, response);
  MHD_destroy_response(response);

  return ret;
}

enum MHD_Result dh_check_method(const char *method) {
  int cond;

  cond = strcmp(method, MHD_HTTP_METHOD_GET) &&
         strcmp(method, MHD_HTTP_METHOD_POST);
  return cond ? MHD_NO : MHD_YES;
}
