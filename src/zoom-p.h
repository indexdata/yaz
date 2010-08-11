/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file zoom-p.h
 * \brief Internal header for ZOOM implementation
 */
#include <yaz/proto.h>
#include <yaz/oid_db.h>
#include <yaz/comstack.h>
#include <yaz/wrbuf.h>
#include <yaz/zoom.h>
#include <yaz/srw.h>
#include <yaz/mutex.h>

#define SHPTR 1
#define ZOOM_RESULT_LISTS 0

typedef struct ZOOM_Event_p *ZOOM_Event;

typedef enum {
    zoom_sru_error,
    zoom_sru_soap,
    zoom_sru_get,
    zoom_sru_post
} zoom_sru_mode;
    

typedef struct ZOOM_task_p *ZOOM_task;

#define STATE_IDLE 0
#define STATE_CONNECTING 1
#define STATE_ESTABLISHED 2

#if ZOOM_RESULT_LISTS
typedef struct ZOOM_resultsets_p *ZOOM_resultsets;
#endif

struct ZOOM_connection_p {
    enum oid_proto proto;
    COMSTACK cs;
    char *host_port;
    char *path;
    int error;
    char *addinfo;
    char *diagset;
    int state;
    int mask;
    int reconnect_ok;
    ODR odr_in;
    ODR odr_out;
    ODR odr_print;
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
    char *sru_version;

    char *user;
    char *group;
    char *password;

    int async;
    int support_named_resultsets;
    int last_event;

    int maximum_record_size;
    int preferred_message_size;

    ZOOM_task tasks;
    ZOOM_options options;
#if ZOOM_RESULT_LISTS
    ZOOM_resultsets resultsets;
#else
    ZOOM_resultset resultsets;
#endif
    ZOOM_Event m_queue_front;
    ZOOM_Event m_queue_back;
    zoom_sru_mode sru_mode;
    int no_redirects; /* 0 for no redirects. >0 for number of redirects */

    int log_details;
    int log_api;
};

#if ZOOM_RESULT_LISTS
struct ZOOM_resultsets_p {
    ZOOM_resultset resultset;
    ZOOM_resultsets next;
};
#endif

struct ZOOM_options_entry {
    char *name;
    char *value;
    int len;                  /* of `value', which may contain NULs */
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

#define RECORD_HASH_SIZE  131

struct ZOOM_resultset_p {
    Z_SortKeySpecList *r_sort_spec;
    ZOOM_query query;
    int refcount;
    Odr_int size;
    int step;
    int piggyback;
    char *setname;
    char *schema;
    ODR odr;
    ZOOM_record_cache record_hash[RECORD_HASH_SIZE];
    ZOOM_options options;
    ZOOM_connection connection;
    char **databaseNames;
    int num_databaseNames;
    YAZ_MUTEX mutex;
#if SHPTR
    struct WRBUF_shptr *record_wrbuf;
#endif
#if ZOOM_RESULT_LISTS
#else
    ZOOM_resultset next;
#endif
    ZOOM_facet_field *facets;
    int num_facets;
    char **facets_names;
};

struct ZOOM_record_p {
    ODR odr;
#if SHPTR
    struct WRBUF_shptr *record_wrbuf;
#else
    WRBUF wrbuf;
#endif

    Z_NamePlusRecord *npr;
    const char *schema;

    const char *diag_uri;
    const char *diag_message;
    const char *diag_details;
    const char *diag_set;
};

struct facet_term_p {
    char *term;
    int frequency;
};

struct ZOOM_facet_field_p {
    char *facet_name;
    int num_terms;
    struct facet_term_p *facet_terms;
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
    ZOOM_query query;
    Z_ScanResponse *scan_response;
    Z_SRW_scanResponse *srw_scan_response;

    char **databaseNames;
    int num_databaseNames;
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
            int count;
            int start;
            ZOOM_resultset resultset;
            char *syntax;
            char *elementSetName;
            int recv_search_fired;
        } search;
#define ZOOM_TASK_RETRIEVE 2
        struct {
            int start;
            ZOOM_resultset resultset;
            int count;
            char *syntax;
            char *elementSetName;
        } retrieve;
#define ZOOM_TASK_CONNECT 3
#define ZOOM_TASK_SCAN 4
        struct {
            ZOOM_scanset scan;
        } scan;
#define ZOOM_TASK_PACKAGE 5
        ZOOM_package package;
#define ZOOM_TASK_SORT 6
        struct {
            ZOOM_resultset resultset;
            ZOOM_query q;
        } sort;
    } u;
    ZOOM_task next;
};

struct ZOOM_Event_p {
    int kind;
    ZOOM_Event next;
    ZOOM_Event prev;
};

typedef enum {
    zoom_pending,
    zoom_complete
} zoom_ret;

void ZOOM_options_addref (ZOOM_options opt);

void ZOOM_handle_Z3950_apdu(ZOOM_connection c, Z_APDU *apdu);

void ZOOM_set_dset_error(ZOOM_connection c, int error,
                         const char *dset,
                         const char *addinfo, const char *addinfo2);

void ZOOM_set_error(ZOOM_connection c, int error, const char *addinfo);

ZOOM_Event ZOOM_Event_create(int kind);
void ZOOM_connection_put_event(ZOOM_connection c, ZOOM_Event event);

zoom_ret ZOOM_connection_Z3950_send_search(ZOOM_connection c);
zoom_ret send_Z3950_present(ZOOM_connection c);
zoom_ret ZOOM_connection_Z3950_send_scan(ZOOM_connection c);
zoom_ret ZOOM_send_buf(ZOOM_connection c);
zoom_ret send_Z3950_sort(ZOOM_connection c, ZOOM_resultset resultset);
char **ZOOM_connection_get_databases(ZOOM_connection con, ZOOM_options options,
                                     int *num, ODR odr);
zoom_ret ZOOM_connection_Z3950_send_init(ZOOM_connection c);

ZOOM_task ZOOM_connection_add_task(ZOOM_connection c, int which);
void ZOOM_connection_remove_task(ZOOM_connection c);
int ZOOM_test_reconnect(ZOOM_connection c);

ZOOM_record ZOOM_record_cache_lookup(ZOOM_resultset r, int pos,
                                     const char *syntax,
                                     const char *elementSetName);
void ZOOM_record_cache_add(ZOOM_resultset r, Z_NamePlusRecord *npr, 
                           int pos,
                           const char *syntax, const char *elementSetName,
                           const char *schema,
                           Z_SRW_diagnostic *diag);

Z_Query *ZOOM_query_get_Z_Query(ZOOM_query s);
Z_SortKeySpecList *ZOOM_query_get_sortspec(ZOOM_query s);
char *ZOOM_query_get_query_string(ZOOM_query s);

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

