#ifndef _MHD_H
#define _MHD_H

#include <microhttpd.h>

/**
 * Check whether request method should be allowed
 *
 * @param method HTTP method to check
 * @return MHD_YES when the request is allowed, MHD_NO when not
 */
enum MHD_Result dh_check_method(const char *method);

/**
 * Memory deallocator to use for `send_response_free_callback`
 *
 * @param mem pointer to free
 */
void mem_free(void *mem);

/**
 * Wrapper to microhttpd's `MHD_create_response_from_buffer`
 *
 * @param connection see microhttpd doc
 * @param buf see microhttpd doc
 * @param bufsize see microhttpd doc
 * @param code see microhttpd doc
 */
enum MHD_Result send_response(struct MHD_Connection *connection,
                              const char *buf, size_t bufsize, int code);

/**
 * Wrapper to microhttpd's `MHD_create_response_from_buffer_with_free_callback`
 *
 * @param connection see microhttpd doc
 * @param buf see microhttpd doc
 * @param bufsize see microhttpd doc
 * @param code see microhttpd doc
 * @param crfc see microhttpd doc
 */
enum MHD_Result send_response_free_callback(struct MHD_Connection *connection,
                                            const char *buf, size_t bufsize,
                                            int code,
                                            MHD_ContentReaderFreeCallback crfc);

#endif  // MHD_H_
