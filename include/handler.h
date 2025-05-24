#ifndef _HANDLER_H
#define _HANDLER_H

#include <microhttpd.h>

char *root_handler(struct MHD_Connection *);

/**
 * Returns the record of a movie, if it exists
 *
 * GET /movie?title=<title>
 *
 * @param connection MHD connection that stores the title to be searched
 * @return JSON-formatted response, whether the operation succeeded or not
 *
 * {
 *     "status": 200,
 *     "result": {
 *       "cast"         : "cast",
 *       "director"     : "director",
 *       "duration"     : "duration",
 *       "genre"        : "genre",
 *       "poster_url"   : "poster_url",
 *       "rating_family": "rating_family",
 *       "rating_imdb"  : "rating_imdb",
 *       "title":       : "title",
 *       "year"         : "year"
 *     }
 *  }
 *
 *  {
 *     "status": 404,
 *     "result": {
 *       "error": "%s not found"
 *     }
 *  }
 *
 *  {
 *     "status": 400
 *     "result": {
 *       "error": "title not provided"
 *     }
 *  }
 *
 *  {
 *    "status": 500,
 *    "result": {
 *      "error": "<error message>"
 *    }
 */
char *movie_handler(struct MHD_Connection *);

char *movies_handler(struct MHD_Connection *);

#endif  // _HANDLER_H
