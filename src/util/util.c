#include "util/util.h"

#include <stdio.h>
#include <stdlib.h>

long get_file_size(const char *filename) {
  FILE *fp;

  fp = fopen(filename, "rb");
  if (fp) {
    long size;

    if ((0 != fseek(fp, 0, SEEK_END)) || (-1 == (size = ftell(fp)))) size = 0;

    fclose(fp);

    return size;
  }
  return 0;
}

char *load_file(const char *filename) {
  FILE *fp;
  char *buffer;
  long size;

  size = get_file_size(filename);
  if (0 == size) return NULL;

  fp = fopen(filename, "rb");
  if (!fp) return NULL;

  buffer = malloc(size + 1);
  if (!buffer) {
    fclose(fp);
    return NULL;
  }
  buffer[size] = '\0';

  if (size != (long)fread(buffer, 1, size, fp)) {
    free(buffer);
    buffer = NULL;
  }

  fclose(fp);
  return buffer;
}
