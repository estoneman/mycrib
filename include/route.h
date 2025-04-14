#ifndef _ROUTE_H
#define _ROUTE_H

typedef struct {
  const char *path;
  char *(*handler)(void);
} Route;

char * movie_handler (void);
char * movies_handler (void);

#endif
