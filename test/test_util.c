#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "util/util.h"

int test_load_file(void) {
  const char *tempfile = "/tmp/test.txt";
  const char *expected = "Hello, Sailor!\n";
  size_t expected_len = strlen(expected);
  char *actual;
  int cmp;

  FILE *fp = fopen(tempfile, "wb");
  if (!fp) return -1;

  fputs(expected, fp);
  fclose(fp);

  actual = load_file(tempfile);
  cmp = strncmp(expected, actual, expected_len);

  free(actual);
  remove(tempfile);

  return cmp;
}

int test_get_file_size(void) {
  const char *tempfile = "/tmp/test.txt";
  const char *expected = "Hello, Sailor!\n";
  size_t expected_len = strlen(expected);
  long actual_len;
  int cmp;

  FILE *fp = fopen(tempfile, "wb");
  if (!fp) return -1;

  fputs(expected, fp);
  fclose(fp);

  actual_len = get_file_size(tempfile);
  cmp = (actual_len == (long)expected_len);
  remove(tempfile);

  return cmp;
}

int main(void) {
  assert(test_load_file() == 0);
  assert(test_get_file_size());

  return 0;
}
