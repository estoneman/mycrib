#ifndef _HANDLER_H
#define _HANDLER_H

#include <jansson.h>
#include <microhttpd.h>

json_t *root_handler(struct MHD_Connection *connection);

/**
 * @deprecated
 *
 * Returns the record of a movie, if it exists. This performs an exact lookup on
 * the title
 *
 * @param connection object that stores the title to be searched
 * @return JSON-formatted response, regardless if the operation succeeded or not
 *
 * GET /movie?title=<title>
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
char *movie_handler(struct MHD_Connection *connection);

/**
 * Returns records of movies that match the given search pattern and type
 *
 * @param connection object that stores data to manipulate the sql query
 * @return JSON-formatted response, regardless if the operation succeeded or not
 *
 * GET
 * /movies?search_type=<exact,startswith,endswith,contains>,search_pattern=<pattern>
 *
 * {
 *     "status": 200,
 *     "result": [
 *       {
 *         "cast"         : "cast",
 *         "director"     : "director",
 *         "duration"     : "duration",
 *         "genre"        : "genre",
 *         "poster_url"   : "poster_url",
 *         "rating_family": "rating_family",
 *         "rating_imdb"  : "rating_imdb",
 *         "title":       : "title",
 *         "year"         : "year"
 *       },
 *       ...
 *     ]
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
 *     "status": 404,
 *     "result": {
 *       "error": "%s not found"
 *     }
 *  }
 *
 *  {
 *    "status": 500,
 *    "result": {
 *      "error": "<error message>"
 *    }
 */
json_t *movies_handler(struct MHD_Connection *connection);

#endif  // _HANDLER_H
