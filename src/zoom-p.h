/*
 * Private C header for ZOOM C.
 * $Id: zoom-p.h,v 1.1 2003-10-27 12:21:36 adam Exp $
 */

#if HAVE_XSLT
#include <yaz/srw.h>
#endif

#include <yaz/proto.h>
#include <yaz/comstack.h>
#include <yaz/wrbuf.h>
#include <yaz/zoom.h>
#include <yaz/sortspec.h>
typedef struct ZOOM_Event_p *ZOOM_Event;

struct ZOOM_query_p {
    Z_Query *z_query;
    Z_SortKeySpecList *sort_spec;
    int refcount;
    ODR odr;
    char *query_string;
};

#define STATE_IDLE 0
#define STATE_CONNECTING 1
#define STATE_ESTABLISHED 2

#define ZOOM_SELECT_READ 1
#define ZOOM_SELECT_WRITE 2
#define ZOOM_SELECT_EXCEPT 4

struct ZOOM_connection_p {
    enum oid_proto proto;
    COMSTACK cs;
    char *host_port;
    char *path;
    int error;
    char *addinfo;
    const char *diagset;
    int state;
    int mask;
    int reconnect_ok;
    ODR odr_in;
    ODR odr_out;
    char *buf_in;
    int len_in;
    char *buf_out;
    int len_out;
    char *proxy;
    char *charset;
    char *lang;
    char *cookie_out;
    char *cookie_in;
    char *client_IP;
    int async;
    int support_named_resultsets;
    int last_event;
    ZOOM_task tasks;
    ZOOM_options options;
    ZOOM_resultset resultsets;
    ZOOM_Event m_queue_front;
    ZOOM_Event m_queue_back;
};

struct ZOOM_options_entry {
    char *name;
    char *value;
    struct ZOOM_options_entry *next;
};

struct ZOOM_options_p {
    int refcount;
    void *callback_handle;
    ZOOM_options_callback callback_func;
    struct ZOOM_options_entry *entries;
    ZOOM_options parent1;
    ZOOM_options parent2;
};

typedef struct ZOOM_record_cache_p *ZOOM_record_cache;

struct ZOOM_resultset_p {
    Z_SortKeySpecList *r_sort_spec;
    ZOOM_query query;
    int refcount;
    int size;
    int start;
    int count;
    int step;
    int piggyback;
    char *setname;
    char *schema;
    ODR odr;
    ZOOM_record_cache record_cache;
    ZOOM_options options;
    ZOOM_connection connection;
    ZOOM_resultset next;
};

struct ZOOM_record_p {
    ODR odr;
    WRBUF wrbuf_marc;
    WRBUF wrbuf_iconv;
    WRBUF wrbuf_opac;
    Z_NamePlusRecord *npr;
};

struct ZOOM_record_cache_p {
    struct ZOOM_record_p rec;
    char *elementSetName;
    char *syntax;
    char *schema;
    int pos;
    ZOOM_record_cache next;
};

struct ZOOM_scanset_p {
    int refcount;
    ODR odr;
    ZOOM_options options;
    ZOOM_connection connection;
    Z_AttributesPlusTerm *termListAndStartPoint;
    Z_AttributeSetId *attributeSet;
    Z_ScanResponse *scan_response;
};

struct ZOOM_package_p {
    int refcount;
    ODR odr_out;
    ZOOM_options options;
    ZOOM_connection connection;
    char *buf_out;
    int len_out;
};

struct ZOOM_task_p {
    int running;
    int which;
    union {
#define ZOOM_TASK_SEARCH 1
        struct {
            ZOOM_resultset resultset;
        } search;
#define ZOOM_TASK_RETRIEVE 2
        struct {
            int start;
            ZOOM_resultset resultset;
            int count;
        } retrieve;
#define ZOOM_TASK_CONNECT 3
#define ZOOM_TASK_SCAN 4
        struct {
            ZOOM_scanset scan;
        } scan;
#define ZOOM_TASK_PACKAGE 5
        ZOOM_package package;
    } u;
    ZOOM_task next;
};

struct ZOOM_Event_p {
    int kind;
    ZOOM_Event next;
    ZOOM_Event prev;
};

void ZOOM_options_addref (ZOOM_options opt);
