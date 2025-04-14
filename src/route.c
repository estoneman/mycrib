#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

#define MAXPAGE 1024

char *
movie_handler (void)
{
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
movies_handler (void)
{
  char *page;

  if ((page = malloc (MAXPAGE)) == NULL)
    {
      fprintf (stderr, "[FATAL] ran out of memory\n");
      return NULL;
    }

  strcpy (page, "{\"handler\": \"MOVIES HANDLER\"}");

  return page;
}
