#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include <stdlib.h>

/**
 * Get the size of a file
 *
 * @param filename name of file of which to calculate its size
 * @return file size as long
 */
long get_file_size(const char *filename);

/**
 * Load a file in memory
 *
 * @param filename name of file to load into memory
 * @param pointer to file contents
 */
char *load_file(const char *filename);

/**
 * Calculate message digest of a string `m` of length `n` characters
 *
 * @param m string of which to calculate message digest
 * @param n length of `m`
 * @return 16-bit unsigned hash of `m`
 */
uint16_t small_crc16_8005(const char *m, size_t n);

#endif  // _UTIL_H
