/*
 * Private C header for ZOOM C.
 * $Id: zoom-p.h,v 1.1 2001-10-23 21:00:20 adam Exp $
 */
#include <yaz/proto.h>
#include <yaz/comstack.h>
#include <yaz/wrbuf.h>
#include <yaz/zoom.h>

struct Z3950_search_p {
    Z_Query *query;
    Z_SortKeySpecList *sort_spec;
    int refcount;
    ODR odr;
};

#define STATE_IDLE 0
#define STATE_CONNECTING 1
#define STATE_ESTABLISHED 2

#define Z3950_SELECT_READ 1
#define Z3950_SELECT_WRITE 2

struct Z3950_connection_p {
    COMSTACK cs;
    char *host_port;
    int error;
    char *addinfo;
    int state;
    int mask;
    ODR odr_in;
    ODR odr_out;
    char *buf_in;
    int len_in;
    char *buf_out;
    int len_out;
    char *proxy;
    char *cookie_out;
    char *cookie_in;
    int async;
    int event_pending;
    Z3950_task tasks;
    Z3950_options options;
    Z3950_resultset resultsets;
};


struct Z3950_options_entry {
    char *name;
    char *value;
    struct Z3950_options_entry *next;
};

struct Z3950_options_p {
    int refcount;
    void *callback_handle;
    Z3950_options_callback callback_func;
    struct Z3950_options_entry *entries;
    Z3950_options parent;
};

typedef struct Z3950_record_cache_p *Z3950_record_cache;

struct Z3950_resultset_p {
    Z_Query *r_query;
    Z_SortKeySpecList *r_sort_spec;
    Z3950_search search;
    int refcount;
    int size;
    int start;
    int count;
    int piggyback;
    ODR odr;
    Z3950_record_cache record_cache;
    Z3950_options options;
    Z3950_connection connection;
    Z3950_resultset next;
};

struct Z3950_record_p {
    ODR odr;
    WRBUF wrbuf_marc;
    Z_NamePlusRecord *npr;
};

struct Z3950_record_cache_p {
    struct Z3950_record_p rec;
    char *elementSetName;
    int pos;
    Z3950_record_cache next;
};

struct Z3950_task_p {
    int running;
    int which;
    union {
#define Z3950_TASK_SEARCH 1
	Z3950_resultset resultset;
#define Z3950_TASK_RETRIEVE 2
	/** also resultset here */

    } u;
    Z3950_task next;
};

#ifndef YAZ_DATE
COMSTACK cs_create_host(const char *type_and_host, int blocking, void **vp);
Odr_oid *yaz_str_to_z3950oid (ODR o, int oid_class, const char *str);
Z_SortKeySpecList *yaz_sort_spec (ODR out, const char *arg);
#else
#include <yaz/sortspec.h>
#endif
