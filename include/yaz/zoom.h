/*
 * Public header for ZOOM C.
 * $Id: zoom.h,v 1.2 2001-10-24 12:24:43 adam Exp $
 */
/* the types we use */

#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

typedef struct Z3950_options_p *Z3950_options;
typedef struct Z3950_search_p *Z3950_search;
typedef struct Z3950_connection_p *Z3950_connection;
typedef	struct Z3950_resultset_p *Z3950_resultset;
typedef struct Z3950_task_p *Z3950_task;
typedef struct Z3950_record_p *Z3950_record;

/* ----------------------------------------------------------- */
/* connections */

/* create connection, connect to host, if portnum is 0, then port is
read from host string (e.g. myhost:9821) */
YAZ_EXPORT
Z3950_connection Z3950_connection_new (const char *host, int portnum);

/* create connection, don't connect, apply options */
YAZ_EXPORT
Z3950_connection Z3950_connection_create (Z3950_options options);

/* connect given existing connection */
YAZ_EXPORT
void Z3950_connection_connect(Z3950_connection c, const char *host,
			      int portnum);

/* destroy connection (close connection also *) */
YAZ_EXPORT
void Z3950_connection_destroy (Z3950_connection c);

/* set option for connection */
YAZ_EXPORT
const char *Z3950_connection_option (Z3950_connection c, const char *key,
				     const char *val);
/* return host for connection */
YAZ_EXPORT
const char *Z3950_connection_host (Z3950_connection c);

/* return error code (0 == success, failure otherwise). cp
   holds error string on failure, addinfo holds addititional info (if any)
*/
YAZ_EXPORT
int Z3950_connection_error (Z3950_connection c, const char **cp,
			    const char **addinfo);

/* returns error code */
YAZ_EXPORT
int Z3950_connection_errcode (Z3950_connection c);
/* returns error message */
YAZ_EXPORT
const char *Z3950_connection_errmsg (Z3950_connection c);
/* returns additional info */
YAZ_EXPORT
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
YAZ_EXPORT
Z3950_resultset Z3950_connection_search(Z3950_connection, Z3950_search q);
/* create result set given PQF query */
YAZ_EXPORT
Z3950_resultset Z3950_connection_search_pqf(Z3950_connection c, const char *q);

/* destroy result set */
YAZ_EXPORT
void Z3950_resultset_destroy(Z3950_resultset r);

/* result set option */
YAZ_EXPORT
const char *Z3950_resultset_option (Z3950_resultset r, const char *key,
				    const char *val);
/* return size of result set (hit count, AKA resultCount) */
YAZ_EXPORT
int Z3950_resultset_size (Z3950_resultset r);

/* return record at pos (starting from ), render given spec in type */
YAZ_EXPORT
void *Z3950_resultset_get (Z3950_resultset s, int pos, const char *type,
			   int *len);
/* retrieve records */
YAZ_EXPORT
void Z3950_resultset_records (Z3950_resultset r, Z3950_record *recs,
			      size_t *cnt);

/* return record object at pos. Returns 0 if unavailable */
YAZ_EXPORT
Z3950_record Z3950_resultset_record (Z3950_resultset s, int pos);

/* like Z3950_resultset_record - but never blocks .. */
YAZ_EXPORT
Z3950_record Z3950_resultset_record_immediate (Z3950_resultset s, int pos);

/* ----------------------------------------------------------- */
/* records */

/* Get record information, in a form given by type */
YAZ_EXPORT
void *Z3950_record_get (Z3950_record rec, const char *type, int *len);

/* Destroy record */
YAZ_EXPORT
void Z3950_record_destroy (Z3950_record rec);

/* ----------------------------------------------------------- */
/* searches */

/* create search object */
YAZ_EXPORT
Z3950_search Z3950_search_create(void);
/* destroy it */
YAZ_EXPORT
void Z3950_search_destroy(Z3950_search s);
/* specify prefix query for search */
YAZ_EXPORT
int Z3950_search_prefix(Z3950_search s, const char *str);
/* specify sort criteria for search */
YAZ_EXPORT
int Z3950_search_sortby(Z3950_search s, const char *criteria);


/* ----------------------------------------------------------- */
/* options */
typedef const char *(*Z3950_options_callback)(void *handle, const char *name);

YAZ_EXPORT
Z3950_options_callback Z3950_options_set_callback (Z3950_options opt,
						   Z3950_options_callback c,
						   void *handle);
YAZ_EXPORT
Z3950_options Z3950_options_create (void);

YAZ_EXPORT
Z3950_options Z3950_options_create_with_parent (Z3950_options parent);

YAZ_EXPORT
const char *Z3950_options_get (Z3950_options opt, const char *name);

YAZ_EXPORT
void Z3950_options_set (Z3950_options opt, const char *name, const char *v);

YAZ_EXPORT
void Z3950_options_destroy (Z3950_options opt);

YAZ_EXPORT
int Z3950_options_get_bool (Z3950_options opt, const char *name, int defa);

YAZ_EXPORT
int Z3950_options_get_int (Z3950_options opt, const char *name, int defa);

YAZ_EXPORT
void Z3950_options_addref (Z3950_options opt);

/* ----------------------------------------------------------- */
/* events */
/* poll for events on a number of connections. Returns positive
   integer if event occurred ; zero if none occurred and no more
   events are pending. The positive integer specifies the
   connection for which the event occurred. There's no way to get
   the details yet, sigh. */
YAZ_EXPORT
int Z3950_event (int no, Z3950_connection *cs);

YAZ_END_CDECL
