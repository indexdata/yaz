/*
 * Public header for ZOOM C.
 * $Id: zoom.h,v 1.11 2002-01-28 09:27:48 adam Exp $
 */

#include <yaz/yconfig.h>

#define ZOOM_EXPORT YAZ_EXPORT
#define ZOOM_BEGIN_CDECL YAZ_BEGIN_CDECL
#define ZOOM_END_CDECL YAZ_END_CDECL

ZOOM_BEGIN_CDECL

/* ----------------------------------------------------------- */
/* the types we use */

typedef struct ZOOM_options_p *ZOOM_options;
typedef struct ZOOM_query_p *ZOOM_query;
typedef struct ZOOM_connection_p *ZOOM_connection;
typedef	struct ZOOM_resultset_p *ZOOM_resultset;
typedef struct ZOOM_task_p *ZOOM_task;
typedef struct ZOOM_record_p *ZOOM_record;
typedef struct ZOOM_scanset_p *ZOOM_scanset;

/* ----------------------------------------------------------- */
/* connections */

/* create connection, connect to host, if portnum is 0, then port is
read from host string (e.g. myhost:9821) */
ZOOM_EXPORT
ZOOM_connection ZOOM_connection_new (const char *host, int portnum);

/* create connection, don't connect, apply options */
ZOOM_EXPORT
ZOOM_connection ZOOM_connection_create (ZOOM_options options);

/* connect given existing connection */
ZOOM_EXPORT
void ZOOM_connection_connect(ZOOM_connection c, const char *host,
			      int portnum);

/* destroy connection (close connection also) */
ZOOM_EXPORT
void ZOOM_connection_destroy (ZOOM_connection c);

/* get/set option for connection */
ZOOM_EXPORT
const char *ZOOM_connection_option_get (ZOOM_connection c, const char *key);

ZOOM_EXPORT
void ZOOM_connection_option_set (ZOOM_connection c, const char *key,
                                  const char *val);

/* return error code (0 == success, failure otherwise). cp
   holds error string on failure, addinfo holds addititional info (if any)
*/
ZOOM_EXPORT
int ZOOM_connection_error (ZOOM_connection c, const char **cp,
			    const char **addinfo);

/* returns error code */
ZOOM_EXPORT
int ZOOM_connection_errcode (ZOOM_connection c);
/* returns error message */
ZOOM_EXPORT
const char *ZOOM_connection_errmsg (ZOOM_connection c);
/* returns additional info */
ZOOM_EXPORT
const char *ZOOM_connection_addinfo (ZOOM_connection c);

#define ZOOM_ERROR_NONE 0
#define ZOOM_ERROR_CONNECT 10000
#define ZOOM_ERROR_MEMORY  10001
#define ZOOM_ERROR_ENCODE  10002
#define ZOOM_ERROR_DECODE  10003
#define ZOOM_ERROR_CONNECTION_LOST 10004
#define ZOOM_ERROR_INIT 10005
#define ZOOM_ERROR_INTERNAL 10006
#define ZOOM_ERROR_TIMEOUT 10007

ZOOM_EXPORT
int ZOOM_connection_last_event(ZOOM_connection cs);

#define ZOOM_EVENT_NONE 0
#define ZOOM_EVENT_CONNECT 1
#define ZOOM_EVENT_SEND_DATA  2
#define ZOOM_EVENT_RECV_DATA 3
#define ZOOM_EVENT_TIMEOUT 4
#define ZOOM_EVENT_UNKNOWN 5
#define ZOOM_EVENT_SEND_APDU 6
#define ZOOM_EVENT_RECV_APDU 7

/* ----------------------------------------------------------- */
/* result sets */

/* create result set given a search */
ZOOM_EXPORT
ZOOM_resultset ZOOM_connection_search(ZOOM_connection, ZOOM_query q);
/* create result set given PQF query */
ZOOM_EXPORT
ZOOM_resultset ZOOM_connection_search_pqf(ZOOM_connection c, const char *q);

/* destroy result set */
ZOOM_EXPORT
void ZOOM_resultset_destroy(ZOOM_resultset r);

/* result set option */
ZOOM_EXPORT
const char *ZOOM_resultset_option_get (ZOOM_resultset r, const char *key);
ZOOM_EXPORT
void ZOOM_resultset_option_set (ZOOM_resultset r, const char *key, const char *val);

/* return size of result set (alias hit count AKA result count) */
ZOOM_EXPORT
size_t ZOOM_resultset_size (ZOOM_resultset r);

/* retrieve records */
ZOOM_EXPORT
void ZOOM_resultset_records (ZOOM_resultset r, ZOOM_record *recs,
                             size_t start, size_t count);

/* return record object at pos. Returns 0 if unavailable */
ZOOM_EXPORT
ZOOM_record ZOOM_resultset_record (ZOOM_resultset s, size_t pos);

/* like ZOOM_resultset_record - but never blocks .. */
ZOOM_EXPORT
ZOOM_record ZOOM_resultset_record_immediate (ZOOM_resultset s, size_t pos);

/* ----------------------------------------------------------- */
/* records */

/* get record information, in a form given by type */
ZOOM_EXPORT
const char *ZOOM_record_get (ZOOM_record rec, const char *type, int *len);

/* destroy record */
ZOOM_EXPORT
void ZOOM_record_destroy (ZOOM_record rec);

/* return copy of record */
ZOOM_EXPORT
ZOOM_record ZOOM_record_clone (ZOOM_record srec);

/* ----------------------------------------------------------- */
/* queries */

/* create search object */
ZOOM_EXPORT
ZOOM_query ZOOM_query_create(void);
/* destroy it */
ZOOM_EXPORT
void ZOOM_query_destroy(ZOOM_query s);
/* specify prefix query for search */
ZOOM_EXPORT
int ZOOM_query_prefix(ZOOM_query s, const char *str);
/* specify sort criteria for search */
ZOOM_EXPORT
int ZOOM_query_sortby(ZOOM_query s, const char *criteria);

/* ----------------------------------------------------------- */
/* scan */
ZOOM_EXPORT
ZOOM_scanset ZOOM_connection_scan (ZOOM_connection c, const char *startterm);

ZOOM_EXPORT
const char * ZOOM_scanset_term(ZOOM_scanset scan, size_t pos,
                               int *occ, int *len);

ZOOM_EXPORT
size_t ZOOM_scanset_size(ZOOM_scanset scan);

ZOOM_EXPORT
void ZOOM_scanset_destroy (ZOOM_scanset scan);

ZOOM_EXPORT
const char *ZOOM_scanset_option_get (ZOOM_scanset scan, const char *key);

ZOOM_EXPORT
void ZOOM_scanset_option_set (ZOOM_scanset scan, const char *key,
                              const char *val);
/* ----------------------------------------------------------- */
/* options */
typedef const char *(*ZOOM_options_callback)(void *handle, const char *name);

ZOOM_EXPORT
ZOOM_options_callback ZOOM_options_set_callback (ZOOM_options opt,
						   ZOOM_options_callback c,
						   void *handle);
ZOOM_EXPORT
ZOOM_options ZOOM_options_create (void);

ZOOM_EXPORT
ZOOM_options ZOOM_options_create_with_parent (ZOOM_options parent);

ZOOM_EXPORT
const char *ZOOM_options_get (ZOOM_options opt, const char *name);

ZOOM_EXPORT
void ZOOM_options_set (ZOOM_options opt, const char *name, const char *v);

ZOOM_EXPORT
void ZOOM_options_destroy (ZOOM_options opt);

ZOOM_EXPORT
int ZOOM_options_get_bool (ZOOM_options opt, const char *name, int defa);

ZOOM_EXPORT
int ZOOM_options_get_int (ZOOM_options opt, const char *name, int defa);

ZOOM_EXPORT
void ZOOM_options_set_int(ZOOM_options opt, const char *name, int value);

ZOOM_EXPORT
void ZOOM_options_addref (ZOOM_options opt);

/* ----------------------------------------------------------- */
/* events */
/* poll for events on a number of connections. Returns positive
   integer if event occurred ; zero if none occurred and no more
   events are pending. The positive integer specifies the
   connection for which the event occurred. */
ZOOM_EXPORT
int ZOOM_event (int no, ZOOM_connection *cs);

ZOOM_END_CDECL
