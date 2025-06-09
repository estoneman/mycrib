#ifndef _MHD_H
#define _MHD_H

#include <microhttpd.h>

enum MHD_Result dh_check_method(const char *method);
void mem_free(void *mem);
enum MHD_Result send_response_free_callback(struct MHD_Connection *connection,
                                            const char *buf, size_t bufsize,
                                            int code,
                                            MHD_ContentReaderFreeCallback crfc);

enum MHD_Result send_response(struct MHD_Connection *connection,
                              const char *buf, size_t bufsize, int code);

#endif  // MHD_H_
