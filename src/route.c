#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

#define MAXPAGE 1024

uint16_t
small_crc16_8005 (const char *m, size_t n)
{
  uint32_t crc = 0;

  for (size_t i = 0; i < n; i++)
    {
      uint8_t d = *(m++);
      uint32_t x = ((crc ^ d) & 0xff) << 8;
      uint32_t y = x;

      x ^= x << 1;
      x ^= x << 2;
      x ^= x << 4;

      x = (x & 0x8000) | (y >> 1);

      crc = (crc >> 8) ^ (x >> 15) ^ (x >> 1) ^ x;
    }
  return crc;
}

void
new_route (Route *routes, const char *path,
           char *(*handler) (struct MHD_Connection *))
{
  Route route;

  size_t path_len = strlen (path);
  unsigned int id = small_crc16_8005 (path, path_len);
  if ((route.path = (char *)malloc (path_len + 1)) == NULL)
    {
      fprintf (stderr, "[ERROR] failed to allocate new route, OOM\n");
      exit (1);
    }

  strncpy (route.path, path, path_len);
  route.path[path_len] = 0x00;

  route.handler = handler;

  routes[id] = route;

  fprintf (stderr, "[INFO] route '%s' added (id=%d)\n", route.path, id);
}

void
del_route (Route *routes, const char *path)
{
  size_t path_len = strlen (path);
  unsigned int id = small_crc16_8005 (path, path_len);

  if (routes[id].path)
    free (routes[id].path);
}

char *
root_handler (struct MHD_Connection *connection)
{
  (void)connection;

  char *page;
  if ((page = malloc (MAXPAGE)) == NULL)
    {
      fprintf (stderr, "[FATAL] ran out of memory\n");
      return NULL;
    }


  strcpy (page, "{\"handlers\": [\"/\", \"/movie\", \"/movies\"]}");

  return page;
}

char *
movie_handler (struct MHD_Connection *connection)
{
  (void)connection;
  char *page;

  if ((page = malloc (MAXPAGE)) == NULL)
    {
      fprintf (stderr, "[FATAL] ran out of memory\n");
      return NULL;
    }

  strcpy (page, "{\"handler\": \"MOVIE HANDLER\"}");

  return page;
}

char *
movies_handler (struct MHD_Connection *connection)
{
  (void)connection;
  char *page;

  if ((page = malloc (MAXPAGE)) == NULL)
    {
      fprintf (stderr, "[FATAL] ran out of memory\n");
      return NULL;
    }

  strcpy (page, "{\"handler\": \"MOVIES HANDLER\"}");

  return page;
}
