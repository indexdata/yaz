/*
 * Private C header for ZOOM C.
 * $Id: zoom-p.h,v 1.8 2001-12-30 22:21:11 adam Exp $
 */
#include <yaz/proto.h>
#include <yaz/comstack.h>
#include <yaz/wrbuf.h>
#include <yaz/zoom.h>
#include <yaz/sortspec.h>

typedef struct ZOOM_Event_p *ZOOM_Event;

struct ZOOM_query_p {
    Z_Query *query;
    Z_SortKeySpecList *sort_spec;
    int refcount;
    ODR odr;
};

#define STATE_IDLE 0
#define STATE_CONNECTING 1
#define STATE_ESTABLISHED 2

#define ZOOM_SELECT_READ 1
#define ZOOM_SELECT_WRITE 2
#define ZOOM_SELECT_EXCEPT 4

struct ZOOM_connection_p {
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
    ZOOM_options parent;
};

typedef struct ZOOM_record_cache_p *ZOOM_record_cache;

struct ZOOM_resultset_p {
    Z_Query *r_query;
    Z_SortKeySpecList *r_sort_spec;
    ZOOM_query search;
    int refcount;
    int size;
    int start;
    int count;
    int piggyback;
    char *setname;
    ODR odr;
    ZOOM_record_cache record_cache;
    ZOOM_options options;
    ZOOM_connection connection;
    ZOOM_resultset next;
};

struct ZOOM_record_p {
    ODR odr;
    WRBUF wrbuf_marc;
    Z_NamePlusRecord *npr;
};

struct ZOOM_record_cache_p {
    struct ZOOM_record_p rec;
    char *elementSetName;
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
    } u;
    ZOOM_task next;
};

struct ZOOM_Event_p {
    int kind;
    ZOOM_Event next;
    ZOOM_Event prev;
};


