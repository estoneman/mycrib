#ifndef _HANDLER_H
#define _HANDLER_H

#include <microhttpd.h>

char *root_handler(struct MHD_Connection *);
char *movie_handler(struct MHD_Connection *);
char *movies_handler(struct MHD_Connection *);

#endif  // _HANDLER_H
