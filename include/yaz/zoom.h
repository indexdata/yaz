/*
 * Public header for ZOOM C.
 * $Id: zoom.h,v 1.4 2001-11-15 08:58:29 adam Exp $
 */

/* 1. Renamed type Z3950_search to Z3950_query and the functions
      that manipulate it..
      Changed positions/sizes to be of type size_t rather than int.
   2. Deleted Z3950_resultset_get. Added Z3950_record_dup. Record
      reference(s) returned by Z350_resultset_records and
      Z3950_resultset_record are "owned" by result set.
*/
#include <yaz/yconfig.h>

#define ZOOM_EXPORT YAZ_EXPORT
#define ZOOM_BEGIN_CDECL YAZ_BEGIN_CDECL
#define ZOOM_END_CDECL YAZ_END_CDECL

ZOOM_BEGIN_CDECL

/* ----------------------------------------------------------- */
/* the types we use */

typedef struct Z3950_options_p *Z3950_options;
typedef struct Z3950_query_p *Z3950_query;
typedef struct Z3950_connection_p *Z3950_connection;
typedef	struct Z3950_resultset_p *Z3950_resultset;
typedef struct Z3950_task_p *Z3950_task;
typedef struct Z3950_record_p *Z3950_record;

/* ----------------------------------------------------------- */
/* connections */

/* create connection, connect to host, if portnum is 0, then port is
read from host string (e.g. myhost:9821) */
ZOOM_EXPORT
Z3950_connection Z3950_connection_new (const char *host, int portnum);

/* create connection, don't connect, apply options */
ZOOM_EXPORT
Z3950_connection Z3950_connection_create (Z3950_options options);

/* connect given existing connection */
ZOOM_EXPORT
void Z3950_connection_connect(Z3950_connection c, const char *host,
			      int portnum);

/* destroy connection (close connection also) */
ZOOM_EXPORT
void Z3950_connection_destroy (Z3950_connection c);

/* set option for connection */
ZOOM_EXPORT
const char *Z3950_connection_option (Z3950_connection c, const char *key,
				     const char *val);
/* return host for connection */
ZOOM_EXPORT
const char *Z3950_connection_host (Z3950_connection c);

/* return error code (0 == success, failure otherwise). cp
   holds error string on failure, addinfo holds addititional info (if any)
*/
ZOOM_EXPORT
int Z3950_connection_error (Z3950_connection c, const char **cp,
			    const char **addinfo);

/* returns error code */
ZOOM_EXPORT
int Z3950_connection_errcode (Z3950_connection c);
/* returns error message */
ZOOM_EXPORT
const char *Z3950_connection_errmsg (Z3950_connection c);
/* returns additional info */
ZOOM_EXPORT
const char *Z3950_connection_addinfo (Z3950_connection c);

#define Z3950_ERROR_NONE 0
#define Z3950_ERROR_CONNECT 10000
#define Z3950_ERROR_MEMORY  10001
#define Z3950_ERROR_ENCODE  10002
#define Z3950_ERROR_DECODE  10003
#define Z3950_ERROR_CONNECTION_LOST 10004
#define Z3950_ERROR_INIT 10005
#define Z3950_ERROR_INTERNAL 10006
#define Z3950_ERROR_TIMEOUT 10007

/* ----------------------------------------------------------- */
/* result sets */

/* create result set given a search */
ZOOM_EXPORT
Z3950_resultset Z3950_connection_search(Z3950_connection, Z3950_query q);
/* create result set given PQF query */
ZOOM_EXPORT
Z3950_resultset Z3950_connection_search_pqf(Z3950_connection c, const char *q);

/* destroy result set */
ZOOM_EXPORT
void Z3950_resultset_destroy(Z3950_resultset r);

/* result set option */
ZOOM_EXPORT
const char *Z3950_resultset_option (Z3950_resultset r, const char *key,
				    const char *val);
/* return size of result set (alias hit count AKA result count) */
ZOOM_EXPORT
size_t Z3950_resultset_size (Z3950_resultset r);

/* retrieve records */
ZOOM_EXPORT
void Z3950_resultset_records (Z3950_resultset r, Z3950_record *recs,
			      size_t start, size_t count);

/* return record object at pos. Returns 0 if unavailable */
ZOOM_EXPORT
Z3950_record Z3950_resultset_record (Z3950_resultset s, size_t pos);

/* like Z3950_resultset_record - but never blocks .. */
ZOOM_EXPORT
Z3950_record Z3950_resultset_record_immediate (Z3950_resultset s, size_t pos);

/* ----------------------------------------------------------- */
/* records */

/* get record information, in a form given by type */
ZOOM_EXPORT
void *Z3950_record_get (Z3950_record rec, const char *type, size_t *len);

/* destroy record */
ZOOM_EXPORT
void Z3950_record_destroy (Z3950_record rec);

/* return copy of record */
ZOOM_EXPORT
Z3950_record Z3950_record_dup (Z3950_record srec);

/* ----------------------------------------------------------- */
/* searches */

/* create search object */
ZOOM_EXPORT
Z3950_query Z3950_query_create(void);
/* destroy it */
ZOOM_EXPORT
void Z3950_query_destroy(Z3950_query s);
/* specify prefix query for search */
ZOOM_EXPORT
int Z3950_query_prefix(Z3950_query s, const char *str);
/* specify sort criteria for search */
ZOOM_EXPORT
int Z3950_query_sortby(Z3950_query s, const char *criteria);

/* ----------------------------------------------------------- */
/* options */
typedef const char *(*Z3950_options_callback)(void *handle, const char *name);

ZOOM_EXPORT
Z3950_options_callback Z3950_options_set_callback (Z3950_options opt,
						   Z3950_options_callback c,
						   void *handle);
ZOOM_EXPORT
Z3950_options Z3950_options_create (void);

ZOOM_EXPORT
Z3950_options Z3950_options_create_with_parent (Z3950_options parent);

ZOOM_EXPORT
const char *Z3950_options_get (Z3950_options opt, const char *name);

ZOOM_EXPORT
void Z3950_options_set (Z3950_options opt, const char *name, const char *v);

ZOOM_EXPORT
void Z3950_options_destroy (Z3950_options opt);

ZOOM_EXPORT
int Z3950_options_get_bool (Z3950_options opt, const char *name, int defa);

ZOOM_EXPORT
int Z3950_options_get_int (Z3950_options opt, const char *name, int defa);

ZOOM_EXPORT
void Z3950_options_addref (Z3950_options opt);

/* ----------------------------------------------------------- */
/* events */
/* poll for events on a number of connections. Returns positive
   integer if event occurred ; zero if none occurred and no more
   events are pending. The positive integer specifies the
   connection for which the event occurred. There's no way to get
   the details yet, sigh. */
ZOOM_EXPORT
int Z3950_event (int no, Z3950_connection *cs);

ZOOM_END_CDECL
