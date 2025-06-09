#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include <stdlib.h>

long get_file_size(const char *filename);
char *load_file(const char *filename);
uint16_t small_crc16_8005(const char *m, size_t n);

#endif  // _UTIL_H
