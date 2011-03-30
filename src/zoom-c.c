/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-c.c
 * \brief Implements ZOOM C interface.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>
#include <errno.h>
#include "zoom-p.h"

#include <yaz/yaz-util.h>
#include <yaz/xmalloc.h>
#include <yaz/otherinfo.h>
#include <yaz/log.h>
#include <yaz/diagbib1.h>
#include <yaz/charneg.h>
#include <yaz/query-charset.h>
#include <yaz/snprintf.h>
#include <yaz/facet.h>

#include <yaz/shptr.h>

#if SHPTR
YAZ_SHPTR_TYPE(WRBUF)
#endif

static int log_api0 = 0;
static int log_details0 = 0;

static void resultset_destroy(ZOOM_resultset r);
static zoom_ret do_write_ex(ZOOM_connection c, char *buf_out, int len_out);

static void initlog(void)
{
    static int log_level_initialized = 0;

    if (!log_level_initialized)
    {
        log_api0 = yaz_log_module_level("zoom");
        log_details0 = yaz_log_module_level("zoomdetails");
        log_level_initialized = 1;
    }
}

void ZOOM_connection_remove_tasks(ZOOM_connection c);

void ZOOM_set_dset_error(ZOOM_connection c, int error,
                         const char *dset,
                         const char *addinfo, const char *addinfo2)
{
    char *cp;

    xfree(c->addinfo);
    c->addinfo = 0;
    c->error = error;
    if (!c->diagset || strcmp(dset, c->diagset))
    {
        xfree(c->diagset);
        c->diagset = xstrdup(dset);
        /* remove integer part from SRW diagset .. */
        if ((cp = strrchr(c->diagset, '/')))
            *cp = '\0';
    }
    if (addinfo && addinfo2)
    {
        c->addinfo = (char*) xmalloc(strlen(addinfo) + strlen(addinfo2) + 2);
        strcpy(c->addinfo, addinfo);
        strcat(c->addinfo, addinfo2);
    }
    else if (addinfo)
        c->addinfo = xstrdup(addinfo);
    if (error != ZOOM_ERROR_NONE)
    {
        yaz_log(c->log_api, "%p set_dset_error %s %s:%d %s %s",
                c, c->host_port ? c->host_port : "<>", dset, error,
                addinfo ? addinfo : "",
                addinfo2 ? addinfo2 : "");
        ZOOM_connection_remove_tasks(c);
    }
}

int ZOOM_uri_to_code(const char *uri)
{
    int code = 0;       
    const char *cp;
    if ((cp = strrchr(uri, '/')))
        code = atoi(cp+1);
    return code;
}



void ZOOM_set_error(ZOOM_connection c, int error, const char *addinfo)
{
    ZOOM_set_dset_error(c, error, "ZOOM", addinfo, 0);
}

static void clear_error(ZOOM_connection c)
{
    /*
     * If an error is tied to an operation then it's ok to clear: for
     * example, a diagnostic returned from a search is cleared by a
     * subsequent search.  However, problems such as Connection Lost
     * or Init Refused are not cleared, because they are not
     * recoverable: doing another search doesn't help.
     */

    ZOOM_connection_remove_events(c);
    switch (c->error)
    {
    case ZOOM_ERROR_CONNECT:
    case ZOOM_ERROR_MEMORY:
    case ZOOM_ERROR_DECODE:
    case ZOOM_ERROR_CONNECTION_LOST:
    case ZOOM_ERROR_INIT:
    case ZOOM_ERROR_INTERNAL:
    case ZOOM_ERROR_UNSUPPORTED_PROTOCOL:
        break;
    default:
        ZOOM_set_error(c, ZOOM_ERROR_NONE, 0);
    }
}

void ZOOM_connection_show_task(ZOOM_task task)
{
    switch(task->which)
    {
    case ZOOM_TASK_SEARCH:
        yaz_log(YLOG_LOG, "search p=%p", task);
        break;
    case ZOOM_TASK_RETRIEVE:
        yaz_log(YLOG_LOG, "retrieve p=%p", task);
        break;
    case ZOOM_TASK_CONNECT:
        yaz_log(YLOG_LOG, "connect p=%p", task);
        break;
    case ZOOM_TASK_SCAN:
        yaz_log(YLOG_LOG, "scan p=%p", task);
        break;
    }
}

void ZOOM_connection_show_tasks(ZOOM_connection c)
{
    ZOOM_task task;
    yaz_log(YLOG_LOG, "connection p=%p tasks", c);
    for (task = c->tasks; task; task = task->next)
        ZOOM_connection_show_task(task);
}

ZOOM_task ZOOM_connection_add_task(ZOOM_connection c, int which)
{
    ZOOM_task *taskp = &c->tasks;
    while (*taskp)
        taskp = &(*taskp)->next;
    *taskp = (ZOOM_task) xmalloc(sizeof(**taskp));
    (*taskp)->running = 0;
    (*taskp)->which = which;
    (*taskp)->next = 0;
    clear_error(c);
    return *taskp;
}

ZOOM_API(int) ZOOM_connection_is_idle(ZOOM_connection c)
{
    return c->tasks ? 0 : 1;
}

ZOOM_task ZOOM_connection_insert_task(ZOOM_connection c, int which)
{
    ZOOM_task task = (ZOOM_task) xmalloc(sizeof(*task));

    task->next = c->tasks;
    c->tasks = task;

    task->running = 0;
    task->which = which;
    return task;
}

void ZOOM_connection_remove_task(ZOOM_connection c)
{
    ZOOM_task task = c->tasks;

    if (task)
    {
        c->tasks = task->next;
        switch (task->which)
        {
        case ZOOM_TASK_SEARCH:
            resultset_destroy(task->u.search.resultset);
            xfree(task->u.search.syntax);
            xfree(task->u.search.elementSetName);
            break;
        case ZOOM_TASK_RETRIEVE:
            resultset_destroy(task->u.retrieve.resultset);
            xfree(task->u.retrieve.syntax);
            xfree(task->u.retrieve.elementSetName);
            break;
        case ZOOM_TASK_CONNECT:
            break;
        case ZOOM_TASK_SCAN:
            ZOOM_scanset_destroy(task->u.scan.scan);
            break;
        case ZOOM_TASK_PACKAGE:
            ZOOM_package_destroy(task->u.package);
            break;
        case ZOOM_TASK_SORT:
            resultset_destroy(task->u.sort.resultset);
            ZOOM_query_destroy(task->u.sort.q);
            break;
        default:
            assert(0);
        }
        xfree(task);

        if (!c->tasks)
        {
            ZOOM_Event event = ZOOM_Event_create(ZOOM_EVENT_END);
            ZOOM_connection_put_event(c, event);
        }
    }
}

void ZOOM_connection_remove_tasks(ZOOM_connection c)
{
    while (c->tasks)
        ZOOM_connection_remove_task(c);
}


ZOOM_API(ZOOM_connection)
    ZOOM_connection_create(ZOOM_options options)
{
    ZOOM_connection c = (ZOOM_connection) xmalloc(sizeof(*c));

    initlog();

    c->log_api = log_api0;
    c->log_details = log_details0;

    yaz_log(c->log_api, "%p ZOOM_connection_create", c);

    c->proto = PROTO_Z3950;
    c->cs = 0;
    ZOOM_connection_set_mask(c, 0);
    c->reconnect_ok = 0;
    c->state = STATE_IDLE;
    c->addinfo = 0;
    c->diagset = 0;
    ZOOM_set_error(c, ZOOM_ERROR_NONE, 0);
    c->buf_in = 0;
    c->len_in = 0;
    c->buf_out = 0;
    c->len_out = 0;
    c->resultsets = 0;

    c->options = ZOOM_options_create_with_parent(options);

    c->host_port = 0;
    c->path = 0;
    c->proxy = 0;
    
    c->charset = c->lang = 0;

    c->cookie_out = 0;
    c->cookie_in = 0;
    c->client_IP = 0;
    c->tasks = 0;

    c->user = 0;
    c->group = 0;
    c->password = 0;

    c->maximum_record_size = 0;
    c->preferred_message_size = 0;

    c->odr_in = odr_createmem(ODR_DECODE);
    c->odr_out = odr_createmem(ODR_ENCODE);
    c->odr_print = 0;

    c->async = 0;
    c->support_named_resultsets = 0;
    c->last_event = ZOOM_EVENT_NONE;

    c->m_queue_front = 0;
    c->m_queue_back = 0;

    c->sru_version = 0;
    c->no_redirects = 0;
    return c;
}


/* set database names. Take local databases (if set); otherwise
   take databases given in ZURL (if set); otherwise use Default */
char **ZOOM_connection_get_databases(ZOOM_connection con, ZOOM_options options,
                                     int *num, ODR odr)
{
    char **databaseNames;
    const char *cp = ZOOM_options_get(options, "databaseName");
    
    if ((!cp || !*cp) && con->host_port)
    {
        if (strncmp(con->host_port, "unix:", 5) == 0)
            cp = strchr(con->host_port+5, ':');
        else
            cp = strchr(con->host_port, '/');
        if (cp)
            cp++;
    }
    if (!cp)
        cp = "Default";
    nmem_strsplit(odr_getmem(odr), "+", cp,  &databaseNames, num);
    return databaseNames;
}

ZOOM_API(ZOOM_connection)
    ZOOM_connection_new(const char *host, int portnum)
{
    ZOOM_connection c = ZOOM_connection_create(0);

    ZOOM_connection_connect(c, host, portnum);
    return c;
}

static zoom_sru_mode get_sru_mode_from_string(const char *s)
{
    if (!s || !*s)
        return zoom_sru_soap;
    if (!yaz_matchstr(s, "soap"))
        return zoom_sru_soap;
    else if (!yaz_matchstr(s, "get"))
        return zoom_sru_get;
    else if (!yaz_matchstr(s, "post"))
        return zoom_sru_post;
    else if (!yaz_matchstr(s, "solr"))
        return zoom_sru_solr;
    return zoom_sru_error;
}

ZOOM_API(void)
    ZOOM_connection_connect(ZOOM_connection c,
                            const char *host, int portnum)
{
    const char *val;
    ZOOM_task task;

    initlog();

    yaz_log(c->log_api, "%p ZOOM_connection_connect host=%s portnum=%d",
            c, host ? host : "null", portnum);

    ZOOM_set_error(c, ZOOM_ERROR_NONE, 0);
    ZOOM_connection_remove_tasks(c);

    if (c->odr_print)
    {
        odr_setprint(c->odr_print, 0); /* prevent destroy from fclose'ing */
        odr_destroy(c->odr_print);
    }
    if (ZOOM_options_get_bool(c->options, "apdulog", 0))
    {
        c->odr_print = odr_createmem(ODR_PRINT);
        odr_setprint(c->odr_print, yaz_log_file());
    }
    else
        c->odr_print = 0;

    if (c->cs)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_connect reconnect ok", c);
        c->reconnect_ok = 1;
        return;
    }
    yaz_log(c->log_details, "%p ZOOM_connection_connect connect", c);
    xfree(c->proxy);
    c->proxy = 0;
    val = ZOOM_options_get(c->options, "proxy");
    if (val && *val)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_connect proxy=%s", c, val);
        c->proxy = xstrdup(val);
    }

    xfree(c->charset);
    c->charset = 0;
    val = ZOOM_options_get(c->options, "charset");
    if (val && *val)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_connect charset=%s", c, val);
        c->charset = xstrdup(val);
    }

    xfree(c->lang);
    val = ZOOM_options_get(c->options, "lang");
    if (val && *val)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_connect lang=%s", c, val);
        c->lang = xstrdup(val);
    }
    else
        c->lang = 0;

    if (host)
    {
        xfree(c->host_port);
        if (portnum)
        {
            char hostn[128];
            sprintf(hostn, "%.80s:%d", host, portnum);
            c->host_port = xstrdup(hostn);
        }
        else
            c->host_port = xstrdup(host);
    }        

    {
        /*
         * If the "<scheme>:" part of the host string is preceded by one
         * or more comma-separated <name>=<value> pairs, these are taken
         * to be options to be set on the connection object.  Among other
         * applications, this facility can be used to embed authentication
         * in a host string:
         *          user=admin,password=secret,tcp:localhost:9999
         */
        char *remainder = c->host_port;
        char *pcolon = strchr(remainder, ':');
        char *pcomma;
        char *pequals;
        while ((pcomma = strchr(remainder, ',')) != 0 &&
               (pcolon == 0 || pcomma < pcolon)) {
            *pcomma = '\0';
            if ((pequals = strchr(remainder, '=')) != 0) {
                *pequals = '\0';
                /*printf("# setting '%s'='%s'\n", remainder, pequals+1);*/
                ZOOM_connection_option_set(c, remainder, pequals+1);
            }
            remainder = pcomma+1;
        }

        if (remainder != c->host_port) {
            xfree(c->host_port);
            c->host_port = xstrdup(remainder);
            /*printf("# reset hp='%s'\n", remainder);*/
        }
    }

    val = ZOOM_options_get(c->options, "sru");
    c->sru_mode = get_sru_mode_from_string(val);

    xfree(c->sru_version);
    val = ZOOM_options_get(c->options, "sru_version");
    c->sru_version = xstrdup(val ? val : "1.2");

    ZOOM_options_set(c->options, "host", c->host_port);

    xfree(c->cookie_out);
    c->cookie_out = 0;
    val = ZOOM_options_get(c->options, "cookie");
    if (val && *val)
    { 
        yaz_log(c->log_details, "%p ZOOM_connection_connect cookie=%s", c, val);
        c->cookie_out = xstrdup(val);
    }

    xfree(c->client_IP);
    c->client_IP = 0;
    val = ZOOM_options_get(c->options, "clientIP");
    if (val && *val)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_connect clientIP=%s",
                c, val);
        c->client_IP = xstrdup(val);
    }

    xfree(c->group);
    c->group = 0;
    val = ZOOM_options_get(c->options, "group");
    if (val && *val)
        c->group = xstrdup(val);

    xfree(c->user);
    c->user = 0;
    val = ZOOM_options_get(c->options, "user");
    if (val && *val)
        c->user = xstrdup(val);

    xfree(c->password);
    c->password = 0;
    val = ZOOM_options_get(c->options, "password");
    if (!val)
        val = ZOOM_options_get(c->options, "pass");

    if (val && *val)
        c->password = xstrdup(val);
    
    c->maximum_record_size =
        ZOOM_options_get_int(c->options, "maximumRecordSize", 1024*1024);
    c->preferred_message_size =
        ZOOM_options_get_int(c->options, "preferredMessageSize", 1024*1024);

    c->async = ZOOM_options_get_bool(c->options, "async", 0);
    yaz_log(c->log_details, "%p ZOOM_connection_connect async=%d", c, c->async);
 
    task = ZOOM_connection_add_task(c, ZOOM_TASK_CONNECT);

    if (!c->async)
    {
        while (ZOOM_event(1, &c))
            ;
    }
}

ZOOM_API(void) ZOOM_resultset_release(ZOOM_resultset r)
{
#if ZOOM_RESULT_LISTS
#else
    if (r->connection)
    {
        /* remove ourselves from the resultsets in connection */
        ZOOM_resultset *rp = &r->connection->resultsets;
        while (1)
        {
            assert(*rp);   /* we must be in this list!! */
            if (*rp == r)
            {   /* OK, we're here - take us out of it */
                *rp = (*rp)->next;
                break;
            }
            rp = &(*rp)->next;
        }
        r->connection = 0;
    }
#endif
}

ZOOM_API(void)
    ZOOM_connection_destroy(ZOOM_connection c)
{
#if ZOOM_RESULT_LISTS
    ZOOM_resultsets list;
#else
    ZOOM_resultset r;
#endif
    if (!c)
        return;
    yaz_log(c->log_api, "%p ZOOM_connection_destroy", c);
    if (c->cs)
        cs_close(c->cs);

#if ZOOM_RESULT_LISTS
    // Remove the connection's usage of resultsets
    list = c->resultsets;
    while (list) {
        ZOOM_resultsets removed = list;
        ZOOM_resultset_destroy(list->resultset);
        list = list->next;
        xfree(removed);
    }
#else
    for (r = c->resultsets; r; r = r->next)
        r->connection = 0;
#endif

    xfree(c->buf_in);
    xfree(c->addinfo);
    xfree(c->diagset);
    odr_destroy(c->odr_in);
    odr_destroy(c->odr_out);
    if (c->odr_print)
    {
        odr_setprint(c->odr_print, 0); /* prevent destroy from fclose'ing */
        odr_destroy(c->odr_print);
    }
    ZOOM_options_destroy(c->options);
    ZOOM_connection_remove_tasks(c);
    ZOOM_connection_remove_events(c);
    xfree(c->host_port);
    xfree(c->path);
    xfree(c->proxy);
    xfree(c->charset);
    xfree(c->lang);
    xfree(c->cookie_out);
    xfree(c->cookie_in);
    xfree(c->client_IP);
    xfree(c->user);
    xfree(c->group);
    xfree(c->password);
    xfree(c->sru_version);
    xfree(c);
}

void ZOOM_resultset_addref(ZOOM_resultset r)
{
    if (r)
    {
        yaz_mutex_enter(r->mutex);
        (r->refcount)++;
        yaz_log(log_details0, "%p ZOOM_resultset_addref count=%d",
                r, r->refcount);
        yaz_mutex_leave(r->mutex);
    }
}

static int g_resultsets = 0;
static YAZ_MUTEX g_resultset_mutex = 0;

/* TODO We need to initialize this before running threaded:
 * call resultset_use(0)  */

static int resultset_use(int delta) {
    int resultset_count;
    if (g_resultset_mutex == 0)
        yaz_mutex_create(&g_resultset_mutex);
    yaz_mutex_enter(g_resultset_mutex);
    g_resultsets += delta;
    resultset_count = g_resultsets;
    yaz_mutex_leave(g_resultset_mutex);
    return resultset_count;
}

int resultsets_count(void) {
    return resultset_use(0);
}

ZOOM_resultset ZOOM_resultset_create(void)
{
    int i;
    ZOOM_resultset r = (ZOOM_resultset) xmalloc(sizeof(*r));

    initlog();

    yaz_log(log_details0, "%p ZOOM_resultset_create", r);
    r->refcount = 1;
    r->size = 0;
    r->odr = odr_createmem(ODR_ENCODE);
    r->piggyback = 1;
    r->setname = 0;
    r->schema = 0;
    r->step = 0;
    for (i = 0; i<RECORD_HASH_SIZE; i++)
        r->record_hash[i] = 0;
    r->r_sort_spec = 0;
    r->query = 0;
    r->connection = 0;
    r->databaseNames = 0;
    r->num_databaseNames = 0;
    r->facets = 0;
    r->num_facets = 0;
    r->facets_names = 0;
    r->mutex = 0;
    yaz_mutex_create(&r->mutex);
#if SHPTR
    {
        WRBUF w = wrbuf_alloc();
        YAZ_SHPTR_INIT(r->record_wrbuf, w);
    }
#endif
    resultset_use(1);
    return r;
}

ZOOM_API(ZOOM_resultset)
    ZOOM_connection_search_pqf(ZOOM_connection c, const char *q)
{
    ZOOM_resultset r;
    ZOOM_query s = ZOOM_query_create();

    ZOOM_query_prefix(s, q);

    r = ZOOM_connection_search(c, s);
    ZOOM_query_destroy(s);
    return r;
}

ZOOM_API(ZOOM_resultset)
    ZOOM_connection_search(ZOOM_connection c, ZOOM_query q)
{
    ZOOM_resultset r = ZOOM_resultset_create();
    ZOOM_task task;
    const char *cp;
    int start, count;
    const char *syntax, *elementSetName;
#if ZOOM_RESULT_LISTS
    ZOOM_resultsets set;
#endif

    yaz_log(c->log_api, "%p ZOOM_connection_search set %p query %p", c, r, q);
    r->r_sort_spec = ZOOM_query_get_sortspec(q);
    r->query = q;

    r->options = ZOOM_options_create_with_parent(c->options);
    
    start = ZOOM_options_get_int(r->options, "start", 0);
    count = ZOOM_options_get_int(r->options, "count", 0);
    {
        /* If "presentChunk" is defined use that; otherwise "step" */
        const char *cp = ZOOM_options_get(r->options, "presentChunk");
        r->step = ZOOM_options_get_int(r->options,
                                       (cp != 0 ? "presentChunk": "step"), 0);
    }
    r->piggyback = ZOOM_options_get_bool(r->options, "piggyback", 1);
    cp = ZOOM_options_get(r->options, "setname");
    if (cp)
        r->setname = xstrdup(cp);
    cp = ZOOM_options_get(r->options, "schema");
    if (cp)
        r->schema = xstrdup(cp);

    r->databaseNames = ZOOM_connection_get_databases(c, c->options, &r->num_databaseNames,
                                         r->odr);
    
    r->connection = c;

#if ZOOM_RESULT_LISTS
    yaz_log(log_details, "%p ZOOM_connection_search: Adding new resultset (%p) to resultsets (%p) ", c, r, c->resultsets);
    set = xmalloc(sizeof(*set));
    ZOOM_resultset_addref(r);
    set->resultset = r;
    set->next = c->resultsets;
    c->resultsets = set;
#else
    r->next = c->resultsets;
    c->resultsets = r;
#endif
    if (c->host_port && c->proto == PROTO_HTTP)
    {
        if (!c->cs)
        {
            yaz_log(c->log_details, "ZOOM_connection_search: no comstack");
            ZOOM_connection_add_task(c, ZOOM_TASK_CONNECT);
        }
        else
        {
            yaz_log(c->log_details, "ZOOM_connection_search: reconnect");
            c->reconnect_ok = 1;
        }
    }

    task = ZOOM_connection_add_task(c, ZOOM_TASK_SEARCH);
    task->u.search.resultset = r;
    task->u.search.start = start;
    task->u.search.count = count;
    task->u.search.recv_search_fired = 0;

    syntax = ZOOM_options_get(r->options, "preferredRecordSyntax"); 
    task->u.search.syntax = syntax ? xstrdup(syntax) : 0;
    elementSetName = ZOOM_options_get(r->options, "elementSetName");
    task->u.search.elementSetName = elementSetName 
        ? xstrdup(elementSetName) : 0;
   
    ZOOM_resultset_addref(r);

    ZOOM_query_addref(q);

    if (!c->async)
    {
        while (ZOOM_event(1, &c))
            ;
    }
    return r;
}

ZOOM_API(void)
    ZOOM_resultset_sort(ZOOM_resultset r,
                         const char *sort_type, const char *sort_spec)
{
    (void) ZOOM_resultset_sort1(r, sort_type, sort_spec);
}

ZOOM_API(int)
    ZOOM_resultset_sort1(ZOOM_resultset r,
                         const char *sort_type, const char *sort_spec)
{
    ZOOM_connection c = r->connection;
    ZOOM_task task;
    ZOOM_query newq;

    newq = ZOOM_query_create();
    if (ZOOM_query_sortby(newq, sort_spec) < 0)
        return -1;

    yaz_log(c->log_api, "%p ZOOM_resultset_sort r=%p sort_type=%s sort_spec=%s",
            r, r, sort_type, sort_spec);
    if (!c)
        return 0;

    if (c->host_port && c->proto == PROTO_HTTP)
    {
        if (!c->cs)
        {
            yaz_log(c->log_details, "%p ZOOM_resultset_sort: no comstack", r);
            ZOOM_connection_add_task(c, ZOOM_TASK_CONNECT);
        }
        else
        {
            yaz_log(c->log_details, "%p ZOOM_resultset_sort: prepare reconnect",
                    r);
            c->reconnect_ok = 1;
        }
    }
    
    ZOOM_resultset_cache_reset(r);
    task = ZOOM_connection_add_task(c, ZOOM_TASK_SORT);
    task->u.sort.resultset = r;
    task->u.sort.q = newq;

    ZOOM_resultset_addref(r);  

    if (!c->async)
    {
        while (ZOOM_event(1, &c))
            ;
    }

    return 0;
}

ZOOM_API(void)
    ZOOM_resultset_destroy(ZOOM_resultset r)
{
    resultset_destroy(r);
}

static void resultset_destroy(ZOOM_resultset r)
{
    if (!r)
        return;
    yaz_mutex_enter(r->mutex);
    (r->refcount)--;
    yaz_log(log_details0, "%p ZOOM_resultset_destroy r=%p count=%d",
            r, r, r->refcount);
    if (r->refcount == 0)
    {
        yaz_mutex_leave(r->mutex);

        yaz_log(log_details0, "%p ZOOM_connection resultset_destroy: Deleting resultset (%p) ", r->connection, r);
        ZOOM_resultset_cache_reset(r);
        ZOOM_resultset_release(r);
        ZOOM_query_destroy(r->query);
        ZOOM_options_destroy(r->options);
        odr_destroy(r->odr);
        xfree(r->setname);
        xfree(r->schema);
        yaz_mutex_destroy(&r->mutex);
#if SHPTR
        YAZ_SHPTR_DEC(r->record_wrbuf, wrbuf_destroy);
#endif
        resultset_use(-1);
        xfree(r);
    }
    else
        yaz_mutex_leave(r->mutex);
}

ZOOM_API(size_t)
    ZOOM_resultset_size(ZOOM_resultset r)
{
    return r->size;
}

int ZOOM_test_reconnect(ZOOM_connection c)
{
    ZOOM_Event event;

    if (!c->reconnect_ok)
        return 0;
    ZOOM_connection_close(c);
    c->reconnect_ok = 0;
    c->tasks->running = 0;
    ZOOM_connection_insert_task(c, ZOOM_TASK_CONNECT);

    event = ZOOM_Event_create(ZOOM_EVENT_CONNECT);
    ZOOM_connection_put_event(c, event);

    return 1;
}

static void ZOOM_resultset_retrieve(ZOOM_resultset r,
                                    int force_sync, int start, int count)
{
    ZOOM_task task;
    ZOOM_connection c;
    const char *cp;
    const char *syntax, *elementSetName;

    if (!r)
        return;
    yaz_log(log_details0, "%p ZOOM_resultset_retrieve force_sync=%d start=%d"
            " count=%d", r, force_sync, start, count);
    c = r->connection;
    if (!c)
        return;

    if (c->host_port && c->proto == PROTO_HTTP)
    {
        if (!c->cs)
        {
            yaz_log(log_details0, "%p ZOOM_resultset_retrieve: no comstack", r);
            ZOOM_connection_add_task(c, ZOOM_TASK_CONNECT);
        }
        else
        {
            yaz_log(log_details0, "%p ZOOM_resultset_retrieve: prepare "
                    "reconnect", r);
            c->reconnect_ok = 1;
        }
    }
    task = ZOOM_connection_add_task(c, ZOOM_TASK_RETRIEVE);
    task->u.retrieve.resultset = r;
    task->u.retrieve.start = start;
    task->u.retrieve.count = count;

    syntax = ZOOM_options_get(r->options, "preferredRecordSyntax"); 
    task->u.retrieve.syntax = syntax ? xstrdup(syntax) : 0;
    elementSetName = ZOOM_options_get(r->options, "elementSetName");
    task->u.retrieve.elementSetName = elementSetName 
        ? xstrdup(elementSetName) : 0;

    cp = ZOOM_options_get(r->options, "schema");
    if (cp)
    {
        if (!r->schema || strcmp(r->schema, cp))
        {
            xfree(r->schema);
            r->schema = xstrdup(cp);
        }
    }

    ZOOM_resultset_addref(r);

    if (!r->connection->async || force_sync)
        while (r->connection && ZOOM_event(1, &r->connection))
            ;
}

ZOOM_API(void)
    ZOOM_resultset_records(ZOOM_resultset r, ZOOM_record *recs,
                           size_t start, size_t count)
{
    int force_present = 0;

    if (!r)
        return ;
    yaz_log(log_api0, "%p ZOOM_resultset_records r=%p start=%ld count=%ld",
            r, r, (long) start, (long) count);
    if (count && recs)
        force_present = 1;
    ZOOM_resultset_retrieve(r, force_present, start, count);
    if (force_present)
    {
        size_t i;
        for (i = 0; i< count; i++)
            recs[i] = ZOOM_resultset_record_immediate(r, i+start);
    }
}

ZOOM_API(size_t)
    ZOOM_resultset_facets_size(ZOOM_resultset r) {
    return r->num_facets;
}

ZOOM_API(ZOOM_facet_field)
    ZOOM_resultset_get_facet_field(ZOOM_resultset r, const char *name) {
    int num = r->num_facets;
    ZOOM_facet_field *facets = r->facets;
    int index;
    for (index = 0; index < num; index++) {
        if (!strcmp(facets[index]->facet_name, name)) {
            return facets[index];
        }
    }
    return 0;
}


ZOOM_API(ZOOM_facet_field *)
    ZOOM_resultset_facets(ZOOM_resultset r)
{
    return r->facets;
}

ZOOM_API(const char**)
    ZOOM_resultset_facet_names(ZOOM_resultset r)
{
    return (const char **) r->facets_names;
}

ZOOM_API(const char*)
    ZOOM_facet_field_name(ZOOM_facet_field field)
{
    return field->facet_name;
}

ZOOM_API(size_t)
    ZOOM_facet_field_term_count(ZOOM_facet_field field)
{
    return field->num_terms;
}

ZOOM_API(const char*)
    ZOOM_facet_field_get_term(ZOOM_facet_field field, size_t idx, int *freq) {
    *freq = field->facet_terms[idx].frequency;
    return field->facet_terms[idx].term;
}


static void get_cert(ZOOM_connection c)
{
    char *cert_buf;
    int cert_len;
    
    if (cs_get_peer_certificate_x509(c->cs, &cert_buf, &cert_len))
    {
        ZOOM_connection_option_setl(c, "sslPeerCert",
                                    cert_buf, cert_len);
        xfree(cert_buf);
    }
}

static zoom_ret do_connect_host(ZOOM_connection c,
                                const char *effective_host,
                                const char *logical_url);

static zoom_ret do_connect(ZOOM_connection c)
{
    const char *effective_host;

    if (c->proxy)
        effective_host = c->proxy;
    else
        effective_host = c->host_port;
    return do_connect_host(c, effective_host, c->host_port);
}

static zoom_ret do_connect_host(ZOOM_connection c, const char *effective_host,
    const char *logical_url)
{
    void *add;

    yaz_log(c->log_details, "%p do_connect effective_host=%s", c, effective_host);

    if (c->cs)
        cs_close(c->cs);
    c->cs = cs_create_host(effective_host, 0, &add);

    if (c->cs && c->cs->protocol == PROTO_HTTP)
    {
#if YAZ_HAVE_XML2
        if (logical_url)
        {
            const char *db = 0;
            
            c->proto = PROTO_HTTP;
            cs_get_host_args(logical_url, &db);
            xfree(c->path);
            
            c->path = xmalloc(strlen(db) * 3 + 2);
            yaz_encode_sru_dbpath_buf(c->path, db);
        }
#else
        ZOOM_set_error(c, ZOOM_ERROR_UNSUPPORTED_PROTOCOL, "SRW");
        ZOOM_connection_close(c);
        return zoom_complete;
#endif
    }
    if (c->cs)
    {
        int ret = cs_connect(c->cs, add);
        if (ret == 0)
        {
            ZOOM_Event event = ZOOM_Event_create(ZOOM_EVENT_CONNECT);
            ZOOM_connection_put_event(c, event);
            get_cert(c);
            if (c->proto == PROTO_Z3950)
                ZOOM_connection_Z3950_send_init(c);
            else
            {
                /* no init request for SRW .. */
                assert(c->tasks->which == ZOOM_TASK_CONNECT);
                ZOOM_connection_remove_task(c);
                ZOOM_connection_set_mask(c, 0);
                ZOOM_connection_exec_task(c);
            }
            c->state = STATE_ESTABLISHED;
            return zoom_pending;
        }
        else if (ret > 0)
        {
            int mask = ZOOM_SELECT_EXCEPT;
            if (c->cs->io_pending & CS_WANT_WRITE)
                mask += ZOOM_SELECT_WRITE;
            if (c->cs->io_pending & CS_WANT_READ)
                mask += ZOOM_SELECT_READ;
            ZOOM_connection_set_mask(c, mask);
            c->state = STATE_CONNECTING; 
            return zoom_pending;
        }
    }
    c->state = STATE_IDLE;
    ZOOM_set_error(c, ZOOM_ERROR_CONNECT, logical_url);
    return zoom_complete;
}

/* returns 1 if PDU was sent OK (still pending )
   0 if PDU was not sent OK (nothing to wait for) 
*/

ZOOM_API(ZOOM_record)
    ZOOM_resultset_record_immediate(ZOOM_resultset s,size_t pos)
{
    const char *syntax =
        ZOOM_options_get(s->options, "preferredRecordSyntax"); 
    const char *elementSetName =
        ZOOM_options_get(s->options, "elementSetName");

    return ZOOM_record_cache_lookup(s, pos, syntax, elementSetName);
}

ZOOM_API(ZOOM_record)
    ZOOM_resultset_record(ZOOM_resultset r, size_t pos)
{
    ZOOM_record rec = ZOOM_resultset_record_immediate(r, pos);

    if (!rec)
    {
        /*
         * MIKE: I think force_sync should always be zero, but I don't
         * want to make this change until I get the go-ahead from
         * Adam, in case something depends on the old synchronous
         * behaviour.
         */
        int force_sync = 1;
        if (getenv("ZOOM_RECORD_NO_FORCE_SYNC")) force_sync = 0;
        ZOOM_resultset_retrieve(r, force_sync, pos, 1);
        rec = ZOOM_resultset_record_immediate(r, pos);
    }
    return rec;
}

ZOOM_API(ZOOM_scanset)
    ZOOM_connection_scan(ZOOM_connection c, const char *start)
{
    ZOOM_scanset s;
    ZOOM_query q = ZOOM_query_create();

    ZOOM_query_prefix(q, start);

    s = ZOOM_connection_scan1(c, q);
    ZOOM_query_destroy(q);
    return s;

}

ZOOM_API(ZOOM_scanset)
    ZOOM_connection_scan1(ZOOM_connection c, ZOOM_query q)
{
    ZOOM_scanset scan = 0;
    Z_Query *z_query = ZOOM_query_get_Z_Query(q);

    if (!z_query)
        return 0;
    scan = (ZOOM_scanset) xmalloc(sizeof(*scan));
    scan->connection = c;
    scan->odr = odr_createmem(ODR_DECODE);
    scan->options = ZOOM_options_create_with_parent(c->options);
    scan->refcount = 1;
    scan->scan_response = 0;
    scan->srw_scan_response = 0;

    scan->query = q;
    ZOOM_query_addref(q);
    scan->databaseNames = ZOOM_connection_get_databases(c, c->options,
                                            &scan->num_databaseNames,
                                            scan->odr);

    if (1)
    {
        ZOOM_task task = ZOOM_connection_add_task(c, ZOOM_TASK_SCAN);
        task->u.scan.scan = scan;
        
        (scan->refcount)++;
        if (!c->async)
        {
            while (ZOOM_event(1, &c))
                ;
        }
    }
    return scan;
}

ZOOM_API(void)
    ZOOM_scanset_destroy(ZOOM_scanset scan)
{
    if (!scan)
        return;
    (scan->refcount)--;
    if (scan->refcount == 0)
    {
        ZOOM_query_destroy(scan->query);

        odr_destroy(scan->odr);
        
        ZOOM_options_destroy(scan->options);
        xfree(scan);
    }
}

static zoom_ret send_package(ZOOM_connection c)
{
    ZOOM_Event event;

    yaz_log(c->log_details, "%p send_package", c);
    if (!c->tasks)
        return zoom_complete;
    assert (c->tasks->which == ZOOM_TASK_PACKAGE);
    
    event = ZOOM_Event_create(ZOOM_EVENT_SEND_APDU);
    ZOOM_connection_put_event(c, event);
    
    c->buf_out = c->tasks->u.package->buf_out;
    c->len_out = c->tasks->u.package->len_out;

    return ZOOM_send_buf(c);
}

ZOOM_API(size_t)
    ZOOM_scanset_size(ZOOM_scanset scan)
{
    if (!scan)
        return 0;

    if (scan->scan_response && scan->scan_response->entries)
        return scan->scan_response->entries->num_entries;
    else if (scan->srw_scan_response)
        return scan->srw_scan_response->num_terms;
    return 0;
}

static void ZOOM_scanset_term_x(ZOOM_scanset scan, size_t pos,
                                size_t *occ,
                                const char **value_term, size_t *value_len,
                                const char **disp_term, size_t *disp_len)
{
    size_t noent = ZOOM_scanset_size(scan);
    
    *value_term = 0;
    *value_len = 0;

    *disp_term = 0;
    *disp_len = 0;

    *occ = 0;
    if (pos >= noent)
        return;
    if (scan->scan_response)
    {
        Z_ScanResponse *res = scan->scan_response;
        if (res->entries->entries[pos]->which == Z_Entry_termInfo)
        {
            Z_TermInfo *t = res->entries->entries[pos]->u.termInfo;
            
            *value_term = (const char *) t->term->u.general->buf;
            *value_len = t->term->u.general->len;
            if (t->displayTerm)
            {
                *disp_term = t->displayTerm;
                *disp_len = strlen(*disp_term);
            }
            else if (t->term->which == Z_Term_general)
            {
                *disp_term = (const char *) t->term->u.general->buf;
                *disp_len = t->term->u.general->len;
            }
            *occ = t->globalOccurrences ? *t->globalOccurrences : 0;
        }
    }
    if (scan->srw_scan_response)
    {
        Z_SRW_scanResponse *res = scan->srw_scan_response;
        Z_SRW_scanTerm *t = res->terms + pos;
        if (t)
        {
            *value_term = t->value;
            *value_len = strlen(*value_term);

            if (t->displayTerm)
                *disp_term = t->displayTerm;
            else
                *disp_term = t->value;
            *disp_len = strlen(*disp_term);
            *occ = t->numberOfRecords ? *t->numberOfRecords : 0;
        }
    }
}

ZOOM_API(const char *)
    ZOOM_scanset_term(ZOOM_scanset scan, size_t pos,
                      size_t *occ, size_t *len)
{
    const char *value_term = 0;
    size_t value_len = 0;
    const char *disp_term = 0;
    size_t disp_len = 0;

    ZOOM_scanset_term_x(scan, pos, occ, &value_term, &value_len,
                        &disp_term, &disp_len);
    
    *len = value_len;
    return value_term;
}

ZOOM_API(const char *)
    ZOOM_scanset_display_term(ZOOM_scanset scan, size_t pos,
                              size_t *occ, size_t *len)
{
    const char *value_term = 0;
    size_t value_len = 0;
    const char *disp_term = 0;
    size_t disp_len = 0;

    ZOOM_scanset_term_x(scan, pos, occ, &value_term, &value_len,
                        &disp_term, &disp_len);
    
    *len = disp_len;
    return disp_term;
}

ZOOM_API(const char *)
    ZOOM_scanset_option_get(ZOOM_scanset scan, const char *key)
{
    return ZOOM_options_get(scan->options, key);
}

ZOOM_API(void)
    ZOOM_scanset_option_set(ZOOM_scanset scan, const char *key,
                            const char *val)
{
    ZOOM_options_set(scan->options, key, val);
}


ZOOM_API(ZOOM_package)
    ZOOM_connection_package(ZOOM_connection c, ZOOM_options options)
{
    ZOOM_package p = (ZOOM_package) xmalloc(sizeof(*p));

    p->connection = c;
    p->odr_out = odr_createmem(ODR_ENCODE);
    p->options = ZOOM_options_create_with_parent2(options, c->options);
    p->refcount = 1;
    p->buf_out = 0;
    p->len_out = 0;
    return p;
}

ZOOM_API(void)
    ZOOM_package_destroy(ZOOM_package p)
{
    if (!p)
        return;
    (p->refcount)--;
    if (p->refcount == 0)
    {
        odr_destroy(p->odr_out);
        xfree(p->buf_out);
        
        ZOOM_options_destroy(p->options);
        xfree(p);
    }
}

ZOOM_API(const char *)
    ZOOM_package_option_get(ZOOM_package p, const char *key)
{
    return ZOOM_options_get(p->options, key);
}

ZOOM_API(const char *)
    ZOOM_package_option_getl(ZOOM_package p, const char *key, int *lenp)
{
    return ZOOM_options_getl(p->options, key, lenp);
}

ZOOM_API(void)
    ZOOM_package_option_set(ZOOM_package p, const char *key,
                            const char *val)
{
    ZOOM_options_set(p->options, key, val);
}

ZOOM_API(void)
    ZOOM_package_option_setl(ZOOM_package p, const char *key,
                             const char *val, int len)
{
    ZOOM_options_setl(p->options, key, val, len);
}

ZOOM_API(int)
    ZOOM_connection_exec_task(ZOOM_connection c)
{
    ZOOM_task task = c->tasks;
    zoom_ret ret = zoom_complete;

    if (!task)
        return 0;
    yaz_log(c->log_details, "%p ZOOM_connection_exec_task type=%d run=%d",
            c, task->which, task->running);
    if (c->error != ZOOM_ERROR_NONE)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_exec_task "
                "removing tasks because of error = %d", c, c->error);
        ZOOM_connection_remove_tasks(c);
        return 0;
    }
    if (task->running)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_exec_task "
                "task already running", c);
        return 0;
    }
    task->running = 1;
    ret = zoom_complete;
    if (c->cs || task->which == ZOOM_TASK_CONNECT)
    {
        switch (task->which)
        {
        case ZOOM_TASK_SEARCH:
            if (c->proto == PROTO_HTTP)
                ret = ZOOM_connection_srw_send_search(c);
            else
                ret = ZOOM_connection_Z3950_send_search(c);
            break;
        case ZOOM_TASK_RETRIEVE:
            if (c->proto == PROTO_HTTP)
                ret = ZOOM_connection_srw_send_search(c);
            else
                ret = send_Z3950_present(c);
            break;
        case ZOOM_TASK_CONNECT:
            ret = do_connect(c);
            break;
        case ZOOM_TASK_SCAN:
            if (c->proto == PROTO_HTTP)
                ret = ZOOM_connection_srw_send_scan(c);
            else
                ret = ZOOM_connection_Z3950_send_scan(c);
            break;
        case ZOOM_TASK_PACKAGE:
            ret = send_package(c);
            break;
        case ZOOM_TASK_SORT:
            c->tasks->u.sort.resultset->r_sort_spec = 
                ZOOM_query_get_sortspec(c->tasks->u.sort.q);
            ret = send_Z3950_sort(c, c->tasks->u.sort.resultset);
            break;
        }
    }
    else
    {
        yaz_log(c->log_details, "%p ZOOM_connection_exec_task "
                "remove tasks because no connection exist", c);
        ZOOM_connection_remove_tasks(c);
    }
    if (ret == zoom_complete)
    {
        yaz_log(c->log_details, "%p ZOOM_connection_exec_task "
                "task removed (complete)", c);
        ZOOM_connection_remove_task(c);
        return 0;
    }
    yaz_log(c->log_details, "%p ZOOM_connection_exec_task "
            "task pending", c);
    return 1;
}

#if YAZ_HAVE_XML2
static Z_GDU *get_HTTP_Request_url(ODR odr, const char *url)
{
    Z_GDU *p = z_get_HTTP_Request(odr);
    const char *host = url;
    const char *cp0 = strstr(host, "://");
    const char *cp1 = 0;
    if (cp0)
        cp0 = cp0+3;
    else
        cp0 = host;
    
    cp1 = strchr(cp0, '/');
    if (!cp1)
        cp1 = cp0 + strlen(cp0);
    
    if (cp0 && cp1)
    {
        char *h = (char*) odr_malloc(odr, cp1 - cp0 + 1);
        memcpy (h, cp0, cp1 - cp0);
        h[cp1-cp0] = '\0';
        z_HTTP_header_add(odr, &p->u.HTTP_Request->headers, "Host", h);
    }
    p->u.HTTP_Request->path = odr_strdup(odr, *cp1 ? cp1 : "/");
    return p;
}

static zoom_ret send_HTTP_redirect(ZOOM_connection c, const char *uri,
                                  Z_HTTP_Response *cookie_hres)
{
    struct Z_HTTP_Header *h;
    Z_GDU *gdu = get_HTTP_Request_url(c->odr_out, uri);
    char *combined_cookies;
    int combined_cookies_len = 0;

    gdu->u.HTTP_Request->method = odr_strdup(c->odr_out, "GET");
    z_HTTP_header_add(c->odr_out, &gdu->u.HTTP_Request->headers, "Accept",
                      "text/xml");

    for (h = cookie_hres->headers; h; h = h->next)
    {
        if (!strcmp(h->name, "Set-Cookie"))
        {
            char *cp;

            if (!(cp = strchr(h->value, ';')))
                cp = h->value + strlen(h->value);
            if (cp - h->value >= 1) {
                combined_cookies = xrealloc(combined_cookies, combined_cookies_len + cp - h->value + 3);
                memcpy(combined_cookies+combined_cookies_len, h->value, cp - h->value);
                combined_cookies[combined_cookies_len + cp - h->value] = '\0';
                strcat(combined_cookies,"; ");
                combined_cookies_len = strlen(combined_cookies);
            }
        }
    }

    if (combined_cookies_len)
    {
        z_HTTP_header_add(c->odr_out, &gdu->u.HTTP_Request->headers,
                          "Cookie", combined_cookies);
        xfree(combined_cookies);
    }

    if (c->user && c->password)
    {
        z_HTTP_header_add_basic_auth(c->odr_out, &gdu->u.HTTP_Request->headers,
                                     c->user, c->password);
    }
    if (!z_GDU(c->odr_out, &gdu, 0, 0))
        return zoom_complete;
    if (c->odr_print)
        z_GDU(c->odr_print, &gdu, 0, 0);
    c->buf_out = odr_getbuf(c->odr_out, &c->len_out, 0);

    odr_reset(c->odr_out);
    return ZOOM_send_buf(c);
}

#if YAZ_HAVE_XML2
void ZOOM_set_HTTP_error(ZOOM_connection c, int error,
                         const char *addinfo, const char *addinfo2)
{
    ZOOM_set_dset_error(c, error, "HTTP", addinfo, addinfo2);
}
#endif


static void handle_http(ZOOM_connection c, Z_HTTP_Response *hres)
{
    zoom_ret cret = zoom_complete;
    int ret = -1;
    const char *addinfo = 0;
    const char *connection_head = z_HTTP_header_lookup(hres->headers,
                                                       "Connection");
    const char *location;

    ZOOM_connection_set_mask(c, 0);
    yaz_log(c->log_details, "%p handle_http", c);
    
    if ((hres->code == 301 || hres->code == 302) && c->sru_mode == zoom_sru_get
        && (location = z_HTTP_header_lookup(hres->headers, "Location")))
    {
        c->no_redirects++;
        if (c->no_redirects > 10)
        {
            ZOOM_set_HTTP_error(c, hres->code, 0, 0);
            c->no_redirects = 0;
            ZOOM_connection_close(c);
        }
        else
        {
            /* since redirect may change host we just reconnect. A smarter
               implementation might check whether it's the same server */
            do_connect_host(c, location, 0);
            send_HTTP_redirect(c, location, hres);
            /* we're OK for now. Operation is not really complete */
            ret = 0;
            cret = zoom_pending;
        }
    }
    else 
    {  
        ret = ZOOM_handle_sru(c, hres, &cret);
        if (ret == 0)
        {
            if (c->no_redirects) /* end of redirect. change hosts again */
                ZOOM_connection_close(c);
        }
        c->no_redirects = 0;
    }
    if (ret)
    {
        if (hres->code != 200)
            ZOOM_set_HTTP_error(c, hres->code, 0, 0);
        else
            ZOOM_set_error(c, ZOOM_ERROR_DECODE, addinfo);
        ZOOM_connection_close(c);
    }
    if (cret == zoom_complete)
    {
        yaz_log(c->log_details, "removing tasks in handle_http");
        ZOOM_connection_remove_task(c);
    }
    {
        int must_close = 0;
        if (!strcmp(hres->version, "1.0"))
        {
            /* HTTP 1.0: only if Keep-Alive we stay alive.. */
            if (!connection_head || strcmp(connection_head, "Keep-Alive"))
                must_close = 1;
        }
        else
        {
            /* HTTP 1.1: only if no close we stay alive.. */
            if (connection_head && !strcmp(connection_head, "close"))
                must_close = 1;
        }
        if (must_close)
        {
            ZOOM_connection_close(c);
            if (c->tasks)
            {
                c->tasks->running = 0;
                ZOOM_connection_insert_task(c, ZOOM_TASK_CONNECT);
                c->reconnect_ok = 0;
            }
        }
    }
}
#endif

static int do_read(ZOOM_connection c)
{
    int r, more;
    ZOOM_Event event;
    
    event = ZOOM_Event_create(ZOOM_EVENT_RECV_DATA);
    ZOOM_connection_put_event(c, event);
    
    r = cs_get(c->cs, &c->buf_in, &c->len_in);
    more = cs_more(c->cs);
    yaz_log(c->log_details, "%p do_read len=%d more=%d", c, r, more);
    if (r == 1)
        return 0;
    if (r <= 0)
    {
        if (!ZOOM_test_reconnect(c))
        {
            ZOOM_set_error(c, ZOOM_ERROR_CONNECTION_LOST, c->host_port);
            ZOOM_connection_close(c);
        }
    }
    else
    {
        Z_GDU *gdu;
        ZOOM_Event event;

        odr_reset(c->odr_in);
        odr_setbuf(c->odr_in, c->buf_in, r, 0);
        event = ZOOM_Event_create(ZOOM_EVENT_RECV_APDU);
        ZOOM_connection_put_event(c, event);

        if (!z_GDU(c->odr_in, &gdu, 0, 0))
        {
            int x;
            int err = odr_geterrorx(c->odr_in, &x);
            char msg[100];
            const char *element = odr_getelement(c->odr_in);
            yaz_snprintf(msg, sizeof(msg),
                    "ODR code %d:%d element=%s offset=%d",
                    err, x, element ? element : "<unknown>",
                    odr_offset(c->odr_in));
            ZOOM_set_error(c, ZOOM_ERROR_DECODE, msg);
            if (c->log_api)
            {
                FILE *ber_file = yaz_log_file();
                if (ber_file)
                    odr_dumpBER(ber_file, c->buf_in, r);
            }
            ZOOM_connection_close(c);
        }
        else
        {
            if (c->odr_print)
                z_GDU(c->odr_print, &gdu, 0, 0);
            if (gdu->which == Z_GDU_Z3950)
                ZOOM_handle_Z3950_apdu(c, gdu->u.z3950);
            else if (gdu->which == Z_GDU_HTTP_Response)
            {
#if YAZ_HAVE_XML2
                handle_http(c, gdu->u.HTTP_Response);
#else
                ZOOM_set_error(c, ZOOM_ERROR_DECODE, 0);
                ZOOM_connection_close(c);
#endif
            }
        }
        c->reconnect_ok = 0;
    }
    return 1;
}

static zoom_ret do_write_ex(ZOOM_connection c, char *buf_out, int len_out)
{
    int r;
    ZOOM_Event event;
    
    event = ZOOM_Event_create(ZOOM_EVENT_SEND_DATA);
    ZOOM_connection_put_event(c, event);

    yaz_log(c->log_details, "%p do_write_ex len=%d", c, len_out);
    if ((r = cs_put(c->cs, buf_out, len_out)) < 0)
    {
        yaz_log(c->log_details, "%p do_write_ex write failed", c);
        if (ZOOM_test_reconnect(c))
        {
            return zoom_pending;
        }
        if (c->state == STATE_CONNECTING)
            ZOOM_set_error(c, ZOOM_ERROR_CONNECT, c->host_port);
        else
            ZOOM_set_error(c, ZOOM_ERROR_CONNECTION_LOST, c->host_port);
        ZOOM_connection_close(c);
        return zoom_complete;
    }
    else if (r == 1)
    {    
        int mask = ZOOM_SELECT_EXCEPT;
        if (c->cs->io_pending & CS_WANT_WRITE)
            mask += ZOOM_SELECT_WRITE;
        if (c->cs->io_pending & CS_WANT_READ)
            mask += ZOOM_SELECT_READ;
        ZOOM_connection_set_mask(c, mask);
        yaz_log(c->log_details, "%p do_write_ex write incomplete mask=%d",
                c, c->mask);
    }
    else
    {
        ZOOM_connection_set_mask(c, ZOOM_SELECT_READ|ZOOM_SELECT_EXCEPT);
        yaz_log(c->log_details, "%p do_write_ex write complete mask=%d",
                c, c->mask);
    }
    return zoom_pending;
}

zoom_ret ZOOM_send_buf(ZOOM_connection c)
{
    return do_write_ex(c, c->buf_out, c->len_out);
}


ZOOM_API(const char *)
    ZOOM_connection_option_get(ZOOM_connection c, const char *key)
{
    return ZOOM_options_get(c->options, key);
}

ZOOM_API(const char *)
    ZOOM_connection_option_getl(ZOOM_connection c, const char *key, int *lenp)
{
    return ZOOM_options_getl(c->options, key, lenp);
}

ZOOM_API(void)
    ZOOM_connection_option_set(ZOOM_connection c, const char *key,
                               const char *val)
{
    ZOOM_options_set(c->options, key, val);
}

ZOOM_API(void)
    ZOOM_connection_option_setl(ZOOM_connection c, const char *key,
                                const char *val, int len)
{
    ZOOM_options_setl(c->options, key, val, len);
}

ZOOM_API(const char *)
    ZOOM_resultset_option_get(ZOOM_resultset r, const char *key)
{
    return ZOOM_options_get(r->options, key);
}

ZOOM_API(void)
    ZOOM_resultset_option_set(ZOOM_resultset r, const char *key,
                              const char *val)
{
    ZOOM_options_set(r->options, key, val);
}


ZOOM_API(int)
    ZOOM_connection_errcode(ZOOM_connection c)
{
    return ZOOM_connection_error(c, 0, 0);
}

ZOOM_API(const char *)
    ZOOM_connection_errmsg(ZOOM_connection c)
{
    const char *msg;
    ZOOM_connection_error(c, &msg, 0);
    return msg;
}

ZOOM_API(const char *)
    ZOOM_connection_addinfo(ZOOM_connection c)
{
    const char *addinfo;
    ZOOM_connection_error(c, 0, &addinfo);
    return addinfo;
}

ZOOM_API(const char *)
    ZOOM_connection_diagset(ZOOM_connection c)
{
    const char *diagset;
    ZOOM_connection_error_x(c, 0, 0, &diagset);
    return diagset;
}

ZOOM_API(const char *)
    ZOOM_diag_str(int error)
{
    switch (error)
    {
    case ZOOM_ERROR_NONE:
        return "No error";
    case ZOOM_ERROR_CONNECT:
        return "Connect failed";
    case ZOOM_ERROR_MEMORY:
        return "Out of memory";
    case ZOOM_ERROR_ENCODE:
        return "Encoding failed";
    case ZOOM_ERROR_DECODE:
        return "Decoding failed";
    case ZOOM_ERROR_CONNECTION_LOST:
        return "Connection lost";
    case ZOOM_ERROR_INIT:
        return "Init rejected";
    case ZOOM_ERROR_INTERNAL:
        return "Internal failure";
    case ZOOM_ERROR_TIMEOUT:
        return "Timeout";
    case ZOOM_ERROR_UNSUPPORTED_PROTOCOL:
        return "Unsupported protocol";
    case ZOOM_ERROR_UNSUPPORTED_QUERY:
        return "Unsupported query type";
    case ZOOM_ERROR_INVALID_QUERY:
        return "Invalid query";
    case ZOOM_ERROR_CQL_PARSE:
        return "CQL parsing error";
    case ZOOM_ERROR_CQL_TRANSFORM:
        return "CQL transformation error";
    case ZOOM_ERROR_CCL_CONFIG:
        return "CCL configuration error";
    case ZOOM_ERROR_CCL_PARSE:
        return "CCL parsing error";
    case ZOOM_ERROR_ES_INVALID_ACTION:
        return "Extended Service. invalid action";
    case ZOOM_ERROR_ES_INVALID_VERSION:
        return "Extended Service. invalid version";
    case ZOOM_ERROR_ES_INVALID_SYNTAX:
        return "Extended Service. invalid syntax";
    default:
        return diagbib1_str(error);
    }
}

ZOOM_API(int)
    ZOOM_connection_error_x(ZOOM_connection c, const char **cp,
                            const char **addinfo, const char **diagset)
{
    int error = c->error;
    if (cp)
    {
        if (!c->diagset || !strcmp(c->diagset, "ZOOM"))
            *cp = ZOOM_diag_str(error);
        else if (!strcmp(c->diagset, "HTTP"))
            *cp = z_HTTP_errmsg(c->error);
        else if (!strcmp(c->diagset, "Bib-1"))
            *cp = ZOOM_diag_str(error);
        else if (!strcmp(c->diagset, "info:srw/diagnostic/1"))
            *cp = yaz_diag_srw_str(c->error);
        else
            *cp = "Unknown error and diagnostic set";
    }
    if (addinfo)
        *addinfo = c->addinfo ? c->addinfo : "";
    if (diagset)
        *diagset = c->diagset ? c->diagset : "";
    return c->error;
}

ZOOM_API(int)
    ZOOM_connection_error(ZOOM_connection c, const char **cp,
                          const char **addinfo)
{
    return ZOOM_connection_error_x(c, cp, addinfo, 0);
}

static void ZOOM_connection_do_io(ZOOM_connection c, int mask)
{
    ZOOM_Event event = 0;
    int r = cs_look(c->cs);
    yaz_log(c->log_details, "%p ZOOM_connection_do_io mask=%d cs_look=%d",
            c, mask, r);
    
    if (r == CS_NONE)
    {
        event = ZOOM_Event_create(ZOOM_EVENT_CONNECT);
        ZOOM_set_error(c, ZOOM_ERROR_CONNECT, c->host_port);
        ZOOM_connection_close(c);
        ZOOM_connection_put_event(c, event);
    }
    else if (r == CS_CONNECT)
    {
        int ret = ret = cs_rcvconnect(c->cs);
        yaz_log(c->log_details, "%p ZOOM_connection_do_io "
                "cs_rcvconnect returned %d", c, ret);
        if (ret == 1)
        {
            int mask = ZOOM_SELECT_EXCEPT;
            if (c->cs->io_pending & CS_WANT_WRITE)
                mask += ZOOM_SELECT_WRITE;
            if (c->cs->io_pending & CS_WANT_READ)
                mask += ZOOM_SELECT_READ;
            ZOOM_connection_set_mask(c, mask);
            event = ZOOM_Event_create(ZOOM_EVENT_NONE);
            ZOOM_connection_put_event(c, event);
        }
        else if (ret == 0)
        {
            event = ZOOM_Event_create(ZOOM_EVENT_CONNECT);
            ZOOM_connection_put_event(c, event);
            get_cert(c);
            if (c->proto == PROTO_Z3950)
                ZOOM_connection_Z3950_send_init(c);
            else
            {
                /* no init request for SRW .. */
                assert(c->tasks->which == ZOOM_TASK_CONNECT);
                ZOOM_connection_remove_task(c);
                ZOOM_connection_set_mask(c, 0);
                ZOOM_connection_exec_task(c);
            }
            c->state = STATE_ESTABLISHED;
        }
        else
        {
            ZOOM_set_error(c, ZOOM_ERROR_CONNECT, c->host_port);
            ZOOM_connection_close(c);
        }
    }
    else
    {
        if (mask & ZOOM_SELECT_EXCEPT)
        {
            if (!ZOOM_test_reconnect(c))
            {
                ZOOM_set_error(c, ZOOM_ERROR_CONNECTION_LOST, c->host_port);
                ZOOM_connection_close(c);
            }
            return;
        }
        if (mask & ZOOM_SELECT_READ)
            do_read(c);
        if (c->cs && (mask & ZOOM_SELECT_WRITE))
            ZOOM_send_buf(c);
    }
}

ZOOM_API(int)
    ZOOM_connection_last_event(ZOOM_connection cs)
{
    if (!cs)
        return ZOOM_EVENT_NONE;
    return cs->last_event;
}


ZOOM_API(int) ZOOM_connection_fire_event_timeout(ZOOM_connection c)
{
    if (c->mask)
    {
        ZOOM_Event event = ZOOM_Event_create(ZOOM_EVENT_TIMEOUT);
        /* timeout and this connection was waiting */
        ZOOM_set_error(c, ZOOM_ERROR_TIMEOUT, 0);
        ZOOM_connection_close(c);
        ZOOM_connection_put_event(c, event);
    }
    return 0;
}

ZOOM_API(int)
    ZOOM_connection_process(ZOOM_connection c)
{
    ZOOM_Event event;
    if (!c)
        return 0;

    event = ZOOM_connection_get_event(c);
    if (event)
    {
        ZOOM_Event_destroy(event);
        return 1;
    }
    ZOOM_connection_exec_task(c);
    event = ZOOM_connection_get_event(c);
    if (event)
    {
        ZOOM_Event_destroy(event);
        return 1;
    }
    return 0;
}

ZOOM_API(int)
    ZOOM_event_nonblock(int no, ZOOM_connection *cs)
{
    int i;

    yaz_log(log_details0, "ZOOM_process_event(no=%d,cs=%p)", no, cs);
    
    for (i = 0; i<no; i++)
    {
        ZOOM_connection c = cs[i];

        if (c && ZOOM_connection_process(c))
            return i+1;
    }
    return 0;
}

ZOOM_API(int) ZOOM_connection_fire_event_socket(ZOOM_connection c, int mask)
{
    if (c->mask && mask)
        ZOOM_connection_do_io(c, mask);
    return 0;
}

ZOOM_API(int) ZOOM_connection_get_socket(ZOOM_connection c)
{
    if (c->cs)
        return cs_fileno(c->cs);
    return -1;
}

ZOOM_API(int) ZOOM_connection_set_mask(ZOOM_connection c, int mask)
{
    c->mask = mask;
    if (!c->cs)
        return -1; 
    return 0;
}

ZOOM_API(int) ZOOM_connection_get_mask(ZOOM_connection c)
{
    if (c->cs)
        return c->mask;
    return 0;
}

ZOOM_API(int) ZOOM_connection_get_timeout(ZOOM_connection c)
{
    return ZOOM_options_get_int(c->options, "timeout", 30);
}

ZOOM_API(void) ZOOM_connection_close(ZOOM_connection c)
{
    if (c->cs)
        cs_close(c->cs);
    c->cs = 0;
    ZOOM_connection_set_mask(c, 0);
    c->state = STATE_IDLE;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

