/* $Id: srw-gateway.c,v 1.2 2003-01-20 13:04:50 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

/*
 * TODO:
 *
 * TTL for targets. Separate thread for cleanup.
 * External target config and aliases.
 */

/* note that soapH.h defines _REENTRANT so we check for it here */
#ifdef _REENTRANT
#include <pthread.h>
#define USE_THREADS 1
#else
#define USE_THREADS 0
#endif

#include <yaz/srw-util.h>
#include <yaz/xmalloc.h>
#include <yaz/zoom.h>
#include <yaz/log.h>
#include <yaz/options.h>
#include <yaz/wrbuf.h>

#define RESULT_SETS 0
#define SRW_DEBUG 1

struct tset {
    ZOOM_resultset m_r;
    long m_expiry_sec;   /* date of creation */
    int m_idle_time;
    char *m_query;
    char *m_schema;
    struct tset *m_next;
};

struct target {
    ZOOM_connection m_c;
    char *m_name;
    int m_in_use;
    struct tset *m_sets;
    struct target *next;
};

struct srw_prop {
    int optimize_level;
    int idle_time;
    int max_sets;
    xslt_maps maps;
};

static cql_transform_t cql_transform_handle = 0;
static struct target *target_list = 0;
#if USE_THREADS
static pthread_mutex_t target_mutex = PTHREAD_MUTEX_INITIALIZER;
#define mylock(x) pthread_mutex_lock(x)
#define myunlock(x) pthread_mutex_unlock(x)
#else
#define mylock(x)
#define myunlock(x)
#endif

#define ERROR_NO_TARGET -1
#define ERROR_BAD_CQL   10

static int diag_bib1_to_srw (int code)
{
    static int map[] = {
        1, 1,
        2, 2,
        3, 11,
        4, 35,
        5, 12,
        6, 30,
        7, 30,
        8, 32,
        9, 29,
        10, 10,
        11, 12,
        13, 61,
        14, 63,
        15, 68,
        16, 70,
        17, 70,
        18, 50,
        19, 55,
        20, 56, 
        21, 52,
        22, 50,
        /* 23-26 no map */
        27, 51,
        28, 52,
        29, 52,
        30, 51,
        31, 52,
        32, 52,
        33, 52,
        /* 100 -105 */
        106, 66,
        107, 11,
        108, 10,
        109, 2,
        110, 37,
        /* 111- 112 */
        113, 10,
        114, 16,
        115, 16,
        116, 16,
        117, 19,
        118, 22,
        119, 32,
        120, 28,
        121, 15,
        122, 32,
        123, 22,
        124, 24,
        125, 36,
        126, 36, 
        127, 36,
        128, 51,
        129, 39,
        130, 43,
        131, 40,
        132, 42,
        201, 44,
        202, 41,
        203, 43,
        /* 205 */
        0
    };
    const int *p = map;
    while (*p)
    {
        if (code == *p)
            return p[1];
        p += 2;
    }
    return 0;
}

static int searchRetrieve(void *userinfo,
                          struct soap * soap,
                          xsd__string  *query,
                          struct xcql__operandType *xQuery,	
                          xsd__string *sortKeys,
                          struct xsort__xSortKeysType *xSortKeys,
                          xsd__integer *startRecord,
                          xsd__integer *maximumRecords,
                          xsd__string *recordSchema,
                          xsd__string *recordPacking,
                          struct zs__searchRetrieveResponse *res);
static int explain (void *userinfo,
                    struct soap *soap,
                    struct zs__explainResponse *explainResponse);

struct target *target_use (const char *action, const char *query,
                           const char *schema, struct tset **set,
                           struct srw_prop *prop)
{
    char name[80];
    struct target *l = 0;
    struct timeval tv;
    struct tset **ssp = 0;
    long now;
    int no_sets = 0;

    gettimeofday(&tv, 0);
    now = tv.tv_sec;

    if (strlen(action) >= 80)
        action = "localhost:210";
    if (strchr(action, '/') || strchr(action, ':'))
        strcpy (name, action);
    else
    {
        strcpy (name, "localhost/");
        if (*action == '\0')
            strcat (name, "Default");
        else
            strcat (name, action);
    }

    /* See if we have the target and the same query */
    if (query)
        for (l = target_list; l; l = l->next)
            if (!l->m_in_use && !strcmp (l->m_name, name))
            {
                struct tset *s = l->m_sets;
                for (; s; s = s->m_next)
                    if (!strcmp(s->m_query, query) && 
                        !strcmp (s->m_schema, schema))
                    {
                        *set = s;
                        return l;
                    }
            }
    
    /* OK, see if we have the target, then.. */
    for (l = target_list; l; l = l->next)
        if (!strcmp (l->m_name, name) && !l->m_in_use)
        {
            struct tset *s = l->m_sets;
            for (; s; s = s->m_next)
                /* if m_expiry_sec is 0, the condition is true below */
                if (s->m_expiry_sec < now)
                {
                    xfree (s->m_query);
                    s->m_query = xstrdup("");
                    xfree (s->m_schema);
                    s->m_schema = xstrdup("");
                    ZOOM_resultset_destroy(s->m_r);
                    s->m_r = 0;
                    *set = s;
                    return l;
                }
            break;
        }
    if (!l)
    {
        /* allright. Have to make a new one */
        l = xmalloc (sizeof(*l));
        l->m_name = xstrdup (name);
        l->m_in_use = 1;
        l->m_c = 0;
        l->m_sets = 0;
        l->next = target_list;
        target_list = l;
    }
    for (ssp = &l->m_sets; *ssp; ssp = &(*ssp)->m_next)
        no_sets++;
    *ssp = xmalloc(sizeof(**ssp));
    (*ssp)->m_next = 0;
    (*ssp)->m_query = xstrdup("");
    (*ssp)->m_schema = xstrdup("");
    (*ssp)->m_r = 0;
    (*ssp)->m_expiry_sec = 0;
    (*ssp)->m_idle_time = (no_sets >= prop->max_sets ? 0 : prop->idle_time);
    *set = *ssp;
    return l;
}

static void target_destroy (struct target *t)
{
    struct target **tp;

    mylock(&target_mutex);

    for (tp = &target_list; *tp; tp = &(*tp)->next)
        if (*tp == t)
        {
            struct tset *s = t->m_sets;
            while (s)
            {
                struct tset *s_next = s->m_next;
                xfree (s->m_query);
                xfree (s->m_schema);
                ZOOM_resultset_destroy (s->m_r);
                xfree (s);
                s = s_next;
            }

            *tp = t->next;

            ZOOM_connection_destroy (t->m_c);

            xfree (t->m_name);
            xfree (t);
            break;
        }
    myunlock(&target_mutex);
}
    
static void target_leave (struct target *l)
{
    mylock(&target_mutex);
    l->m_in_use = 0;

    if (1)
    {
        struct tset *s = l->m_sets;
        for (; s; s = s->m_next)
            yaz_log(LOG_LOG, " set %s q=%s", 
                    (s->m_r ? ZOOM_resultset_option_get(s->m_r,"setname"):""),
                    s->m_query);
    }
    myunlock(&target_mutex);
}

#if USE_THREADS
static void *p_serve (void *p)
{
    struct soap *soap = p;
    yaz_srw_serve(soap, searchRetrieve, explain);
}
#endif


static void standalone(struct soap *soap, const char *host, int port,
                       int max_thr, struct srw_prop *properties)
{
    struct soap **soap_thr = malloc (sizeof(*soap_thr) * max_thr);
#if USE_THREADS
    pthread_t *tid = malloc (sizeof(pthread_t) * max_thr);
#endif
    int m, s, i;
    int cno = 0;
    int stop = 0;
    char fname[40];
    
    m = soap_bind(soap, 0, port, 100);
    if (m < 0)
    {
        yaz_log (LOG_WARN|LOG_ERRNO, "Cannot bind to %d", port);
        stop = 1;
    }

    for (i = 0; i<max_thr; i++)
        soap_thr[i] = 0;

    while (!stop)
    {
        for (i = 0; i<max_thr; i++)
        {
            s = soap_accept(soap);
            if (s < 0)
                break;
            cno++;
            if (!soap_thr[i])
            {
                soap_thr[i] = soap_new();
                if (!soap_thr[i])
                {
                    stop = 1;
                    break;
                }
            }
            else
            {
#if USE_THREADS
                if (max_thr > 1)          /* static mode for max_thr <= 1 */
                    pthread_join(tid[i], 0);
#endif
                soap_end(soap_thr[i]);
            }

#if SRW_DEBUG
            sprintf (fname, "srw.recv.%05d.log", cno);
	    remove (fname);
            soap_set_recv_logfile(soap_thr[i], fname);
            
            sprintf (fname, "srw.sent.%05d.log", cno);
	    remove (fname);
            soap_set_sent_logfile(soap_thr[i], fname);
            
            sprintf (fname, "srw.test.%05d.log", cno);
	    remove (fname);
            soap_set_test_logfile(soap_thr[i], fname);

            yaz_log (LOG_LOG, "starting session %d %ld.%ld.%ld.%ld", cno,
                     (long) (soap->ip>>24) & 0xff,
                     (long) (soap->ip>>16) & 0xff,
                     (long) (soap->ip>>8) & 0xff,
                     (long) soap->ip & 0xff);
#endif
            soap_thr[i]->encodingStyle = 0;
            soap_thr[i]->socket = s;
            soap_thr[i]->user = properties;
#if USE_THREADS
            if (max_thr <= 1)
                yaz_srw_serve(soap_thr[i],
                              searchRetrieve, explain);  /* static mode .. */
            else
                pthread_create(&tid[i], 0, p_serve, soap_thr[i]);
#else
            yaz_srw_serve(soap_thr[i],
                          searchRetrieve, explain);  /* static mode .. */
#endif
        }
    }
#if USE_THREADS
    free (tid);
#endif
    free (soap_thr);
}

static void reconnect (struct target *t)
{
    struct tset *s;

    for (s = t->m_sets; s; s = s->m_next)
    {
        ZOOM_resultset_destroy(s->m_r);
        s->m_r = 0;
    }
    ZOOM_connection_destroy (t->m_c);

    t->m_c = ZOOM_connection_create (0);
    ZOOM_connection_connect (t->m_c, t->m_name, 0);
}

int explain (void *userinfo,
             struct soap *soap,
             struct zs__explainResponse *explainResponse)
{
    explainResponse->Explain = 
        "<explain>\n"
        "  <!-- not implemented -->\n"
        "</explain>\n";
    return SOAP_OK;
}

int fetchone(struct soap *soap, struct srw_prop *properties,
             ZOOM_record zrec, const char *schema,
             char **rec_data, char **rec_schema)
{
    xslt_map_result res;
    int xml_len;
    const char *xml_rec = ZOOM_record_get(zrec, "xml", &xml_len);
    if (!xml_rec)
    {
        return 65;
    }
    if (!strcmp(schema, "MARC21") || !strcmp(schema, "http://www.loc.gov/marcxml/"))
    {
        *rec_data = soap_malloc (soap, xml_len+1);
        memcpy (*rec_data, xml_rec, xml_len);
        (*rec_data)[xml_len] = 0;
        *rec_schema = "http://www.loc.gov/marcxml/";
    }
    else if ((res = xslt_map (properties->maps, "MARC21",
                              schema, xml_rec, xml_len)))
    {
        int len = xslt_map_result_len(res);
        char *buf = xslt_map_result_buf(res);

        *rec_data = soap_malloc (soap, len+1);
        memcpy (*rec_data, buf, len);
        (*rec_data)[len] = 0;
        
        *rec_schema = soap_malloc(soap,
                                  strlen(xslt_map_result_schema(res)) + 1);
        strcpy(*rec_schema, xslt_map_result_schema(res));
        
        xslt_map_free (res);
    }
    else
    {
        *rec_data = soap_malloc(soap, strlen(schema)+1);
        strcpy(*rec_data, schema);
        return 66;
    }
    return 0;
}
                
int searchRetrieve(void *userinfo,
                   struct soap * soap,
                   xsd__string  *query,
                   struct xcql__operandType *xQuery,	
                   xsd__string *sortKeys,
                   struct xsort__xSortKeysType *xSortKeys,
                   xsd__integer *startRecord,
                   xsd__integer *maximumRecords,
                   xsd__string *recordSchema,
                   xsd__string *recordPacking,
                   struct zs__searchRetrieveResponse *res)
{
    const char *msg = 0, *addinfo = 0;
    const char *schema = recordSchema ? *recordSchema : "";
    struct target *t = 0;
    struct tset *s = 0;
    int error = 0;
    struct srw_prop *properties = (struct srw_prop*) userinfo;
    WRBUF wr_log = wrbuf_alloc();

    char pqf_buf[1024];
    char zurl[81];

    *pqf_buf = '\0';
    *zurl = '\0';
    yaz_log (LOG_LOG, "HTTP: %s", soap->endpoint);
    if (*soap->endpoint)
    {
        const char *cp = strstr(soap->endpoint, "//");
	if (cp)
            cp = cp+2;             /* skip method// */
	else
            cp = soap->endpoint;
	cp = strstr(cp, "/"); 
	if (cp)
        {
            size_t len;
            cp++;
            len = strlen(cp);
            if (len > 80)
                len = 80;
            if (len)
                memcpy (zurl, cp, len);
            zurl[len] = '\0';
        }
    }
    else
    {
        const char *cp = getenv("PATH_INFO");
        if (cp && cp[0] && cp[1])
        {
            size_t len;
            cp++;  /* skip / */
            len = strlen(cp);
            if (len > 80)
                len = 80;
            if (len)
                memcpy (zurl, cp, len);
            zurl[len] = '\0';
        }
    }
    if (query)
    {
        CQL_parser cql_parser = cql_parser_create();
        int r = cql_parser_string(cql_parser, *query);

        if (r)
        {
            yaz_log (LOG_LOG, "cql failed: %s", *query);
            error = ERROR_BAD_CQL;
        }
        else
        {
            struct cql_node *tree = cql_parser_result(cql_parser);
            error = cql_transform_buf (cql_transform_handle, tree,
                                       pqf_buf, sizeof(pqf_buf));
            if (error)
                cql_transform_error(cql_transform_handle, &addinfo);
            cql_parser_destroy(cql_parser);
            yaz_log (LOG_LOG, "cql OK: %s", *query);
        }
    }
    else if (xQuery)
    {
        struct cql_node *tree = xcql_to_cqlnode(xQuery);
        yaz_log (LOG_LOG, "xcql");
        cql_transform_buf (cql_transform_handle, tree,
                           pqf_buf, sizeof(pqf_buf));
        cql_node_destroy(tree);
    }
    if (!error)
    {
        mylock(&target_mutex);
        t = target_use (zurl, *pqf_buf ? pqf_buf : 0, schema, &s, properties);
        myunlock(&target_mutex);
    }

    if (!error && !t->m_c)
    {
        reconnect(t);
        if (ZOOM_connection_error (t->m_c, &msg, &addinfo))
        {
            yaz_log (LOG_LOG, "%s: connect failed", t->m_name);
            error = ERROR_NO_TARGET;
        }
        else
            yaz_log (LOG_LOG, "%s: connect ok", t->m_name);
    }
    if (!error && t->m_c &&  *pqf_buf)
    {
        if (properties->optimize_level <=1 ||
            strcmp (pqf_buf, s->m_query) ||
            strcmp (schema, s->m_schema))
        {
            /* not the same query: remove result set .. */
            ZOOM_resultset_destroy (s->m_r);
            s->m_r = 0;
        }
        else
        {
            /* same query: retrieve (instead of doing search) */
            if (maximumRecords && *maximumRecords > 0)
            {
                int start = startRecord ? *startRecord : 1;
                yaz_log (LOG_LOG, "%s: present start=%d count=%d pqf=%s",
                         t->m_name, start, *maximumRecords, pqf_buf);
                wrbuf_printf (wr_log, "%s: present start=%d count=%d pqf=%s",
                              t->m_name, start, *maximumRecords, pqf_buf);
                ZOOM_resultset_records (s->m_r, 0, start-1, *maximumRecords);
                error = ZOOM_connection_error (t->m_c, &msg, &addinfo);
                if (error == ZOOM_ERROR_CONNECTION_LOST ||
                    error == ZOOM_ERROR_CONNECT)
                {
                    reconnect (t);
                    if ((error = ZOOM_connection_error (t->m_c, &msg,
						     &addinfo)))
                    {
                        yaz_log (LOG_LOG, "%s: connect failed", t->m_name);
                        error = ERROR_NO_TARGET;
                    }
                }
                else if (error)
                {
                    yaz_log (LOG_LOG, "%s: present failed bib1-code=%d",
                             t->m_name, error);
                    error = diag_bib1_to_srw(error);
                }
            }
            else
            {
                yaz_log (LOG_LOG, "%s: matched search pqf=%s",
                         t->m_name, pqf_buf);
                wrbuf_printf (wr_log, "%s: matched search pqf=%s",
                         t->m_name, pqf_buf);
            }
        }
        if (!error && !s->m_r) 
        {   /* no result set usable. We must search ... */
            int pass;
            for (pass = 0; pass < 2; pass++)
            {
                char val[30];
                int start = startRecord ? *startRecord : 1;
                int count = maximumRecords ? *maximumRecords : 0;
                
                sprintf (val, "%d", start-1);
                ZOOM_connection_option_set (t->m_c, "start", val);
                
                sprintf (val, "%d", count);
                ZOOM_connection_option_set (t->m_c, "count", val);
                
                ZOOM_connection_option_set (t->m_c, "preferredRecordSyntax", 
                                            "usmarc");
                
                xfree (s->m_query);
                s->m_query = xstrdup (pqf_buf);

                xfree (s->m_schema);
                s->m_schema = xstrdup (schema);
                
                yaz_log (LOG_LOG, "%s: search start=%d count=%d pqf=%s",
                         t->m_name, start, count, pqf_buf);
                
                wrbuf_printf (wr_log, "%s: search start=%d count=%d pqf=%s",
                              t->m_name, start, count, pqf_buf);
                
                s->m_r = ZOOM_connection_search_pqf (t->m_c, s->m_query);
                
                error = ZOOM_connection_error (t->m_c, &msg, &addinfo);
                if (!error)
                    break;
                if (error != ZOOM_ERROR_CONNECTION_LOST &&
                    error != ZOOM_ERROR_CONNECT)
                {
                    yaz_log (LOG_LOG, "%s: search failed bib1-code=%d",
                             t->m_name, error);
                    error = diag_bib1_to_srw(error);
                    break;
                }
                yaz_log (LOG_LOG, "%s: reconnect (search again)", t->m_name);

                /* try once more */
                reconnect(t);

                error = ZOOM_connection_error (t->m_c, &msg, &addinfo);

                if (error)
                {
                    error = ERROR_NO_TARGET;
                    break;
                }
            }
        }
    }
    
    if (!error && t->m_c && s->m_r)
    {
        yaz_log (LOG_LOG, "%s: %d hits", t->m_name,
                 ZOOM_resultset_size(s->m_r));
        res->numberOfRecords = ZOOM_resultset_size(s->m_r);
        
        if (maximumRecords)
        {
            int i, j = 0;
            int offset = startRecord ? *startRecord -1 : 0;
            res->records.record =
                soap_malloc(soap, sizeof(*res->records.record) *
                            *maximumRecords);
            
            for (i = 0; i < *maximumRecords; i++)
            {
                char *rec_data = 0;
                char *rec_schema = 0;
                ZOOM_record zrec = ZOOM_resultset_record (s->m_r, offset + i);
                if (!zrec)
                {
                    error = 65;
                    addinfo = schema;
                    break;
                }
                error = fetchone(soap, properties, zrec, schema,
                                 &rec_data, &rec_schema);
                if (error)
                {
                    addinfo = rec_data;
                    break;
                }
                res->records.record[j] = 
                    soap_malloc(soap, sizeof(**res->records.record));
                res->records.record[j]->recordData = rec_data;
                res->records.record[j]->recordSchema = rec_schema;
                j++;
            }
            res->records.__sizeRecords = j;
        }
        else
            res->numberOfRecords = 0;
    }
    if (error)
    {
        if (s)
        {
            ZOOM_resultset_destroy (s->m_r);
            s->m_r = 0;
        }
        if (error == ERROR_NO_TARGET)
        {
            addinfo = zurl;
            ZOOM_connection_destroy (t->m_c);
            t->m_c = 0;
        }
        else
        {
            res->diagnostics.__sizeDiagnostics = 1;
            res->diagnostics.diagnostic =
                soap_malloc (soap, sizeof(*res->diagnostics.diagnostic));
            res->diagnostics.diagnostic[0] =
                soap_malloc (soap, sizeof(**res->diagnostics.diagnostic));
            
            res->diagnostics.diagnostic[0]->code = error;
            if (addinfo)
            {
                res->diagnostics.diagnostic[0]->details =
                    soap_malloc (soap, strlen(addinfo) + 1);
                strcpy (res->diagnostics.diagnostic[0]->details, addinfo);
            }
            else
                res->diagnostics.diagnostic[0]->details = 0;
        }
    }
    else
    {
        if (s->m_r)
        {
            struct timeval tv;
            const char *setname = ZOOM_resultset_option_get(s->m_r, "setname");
            if (strcmp(setname, "default") && s->m_idle_time)
            {
                res->resultSetId = soap_malloc(soap, strlen(setname));
                strcpy(res->resultSetId, setname);
                res->resultSetIdleTime = s->m_idle_time;
                gettimeofday(&tv, 0);
                s->m_expiry_sec = res->resultSetIdleTime + tv.tv_sec + 2;
            } else {
                s->m_expiry_sec = 0;
            }
        }
    }

    if (t)
    {
        if (properties->optimize_level > 0)
            target_leave(t);
        else
            target_destroy(t);
    }
    wrbuf_free(wr_log, 1);
    if (error == ERROR_NO_TARGET)
        return soap_receiver_fault(soap, "Cannot connect to Z39.50 target", 0);
    return SOAP_OK;
}

int main(int argc, char **argv)
{
    struct soap soap;
    int ret;
    int port = 0;
    int no_threads = 40;
    char *arg;
    const char *host = 0;
    struct srw_prop properties;

    properties.optimize_level = 2;
    properties.idle_time = 300;
    properties.max_sets = 30;
    properties.maps = 0;
            
    while ((ret = options("dO:T:l:hVp:s:x:i:", argv, argc, &arg)) != -2)
    {
        switch(ret)
        {
        case 0:
            port = atoi(arg);
            break;
        case 'O':
            properties.optimize_level = atoi(arg);
            break;
        case 'T':
            no_threads = atoi(arg);
            if (no_threads < 1 || no_threads > 200)
                no_threads = 40;
            break;
        case 's':
            if (!properties.maps)
            {
                properties.maps = xslt_maps_create();
                if (xslt_maps_file(properties.maps, arg))
                {
                    fprintf (stderr, "maps file %s could not be opened\n",
                             arg);
                    exit(1);
                }
            }
            break;
        case 'l':
            yaz_log_init_file(arg);
            break;
        case 'V':
            puts ("Version: $Id: srw-gateway.c,v 1.2 2003-01-20 13:04:50 adam Exp $"
#if SRW_DEBUG
            " DEBUG"
#endif
                );
            exit (0);
        case 'p':
            if (cql_transform_handle == 0)
                cql_transform_handle = cql_transform_open_fname(arg);
            break;
        case 'x':
            properties.max_sets = atoi(arg);
            break;
        case 'i':
            properties.idle_time = atoi(arg);
            break;
        case 'h':
            printf ("srw-gateway [options] <port>\n");
            printf ("  port  port for standalone service; If port is omitted, CGI is used.\n");
            printf ("  -O n     optimize level. >= 1 cache connections, >=2 cache result sets.\n");
#if USE_THREADS
            printf ("  -T n     number of threads.\n");
#else
            printf ("  -T       unsupported in this version.\n");
#endif
            printf ("  -l file  log to file (instead of stderr).\n");
            printf ("  -p file  PQF properties.\n");
            printf ("  -s file  schema maps.\n");
            printf ("  -i time  idle time.\n");
            printf ("  -x sets  live sets.\n");
            printf ("  -V       show version.\n");
            exit (1);
        default:
            fprintf (stderr, "srw-gateway: bad option -%s ; use -h for help\n",
                     arg);
            exit (1);
            break;
            
        }
    }
    if (!cql_transform_handle)
        cql_transform_handle = cql_transform_open_fname("pqf.properties");
    if (!properties.maps)
    {
        properties.maps = xslt_maps_create();
        xslt_maps_file(properties.maps, "maps.xml");
    }
    soap.encodingStyle = 0;
    if (port == 0 && getenv("QUERY_STRING"))
    {
        properties.optimize_level = 0;

        yaz_log_init_file("srw.log");
        yaz_log (LOG_LOG, "CGI begin");
        soap_init(&soap);
        soap.user = &properties;

        yaz_srw_serve(&soap, searchRetrieve, explain);

        soap_end(&soap);
        yaz_log (LOG_LOG, "CGI end");
    }
    else if (port)
    {
        if (!cql_transform_handle)
        {
            fprintf(stderr, "no properties file; use option -p to specify\n");
            exit (1);
        }
        yaz_log (LOG_LOG, "standalone service on port %d", port);
        
        soap_init(&soap);

        standalone(&soap, host, port, no_threads, &properties);
    }
    else
    {
        fprintf(stderr, "srw-gateway: no port specified. Use -h for help\n");
    }
    xslt_maps_free(properties.maps);
    return 0;
}
