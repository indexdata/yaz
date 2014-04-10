/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-query.c
 * \brief Implements ZOOM C query interface.
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
#include <yaz/log.h>
#include <yaz/pquery.h>
#include <yaz/cql.h>
#include <yaz/ccl.h>
#include <yaz/sortspec.h>

#define SORT_STRATEGY_Z3950 0
#define SORT_STRATEGY_TYPE7 1
#define SORT_STRATEGY_CQL   2
#define SORT_STRATEGY_SRU11 3
#define SORT_STRATEGY_EMBED 4
#define SORT_STRATEGY_SOLR  5

struct ZOOM_query_p {
    Z_Query *z_query;
    int sort_strategy;
    Z_SortKeySpecList *sort_spec;
    int refcount;
    ODR odr_sort_spec;
    ODR odr_query;
    int query_type;
    char *query_string;
    WRBUF full_query;
    WRBUF sru11_sort_spec;
};

static int generate(ZOOM_query s)
{
    if (s->query_string)
    {
        Z_External *ext;

        wrbuf_rewind(s->full_query);
        wrbuf_puts(s->full_query, s->query_string);
        odr_reset(s->odr_query);

        if (s->sort_spec && (s->sort_strategy == SORT_STRATEGY_SRU11 || s->sort_strategy == SORT_STRATEGY_SOLR))
        {
            int r = 0;
            wrbuf_rewind(s->sru11_sort_spec);

            switch (s->sort_strategy)
            {
            case SORT_STRATEGY_SRU11:
                r = yaz_sort_spec_to_srw_sortkeys(s->sort_spec, s->sru11_sort_spec);
                break;
            case SORT_STRATEGY_SOLR:
                r = yaz_sort_spec_to_solr_sortkeys(s->sort_spec, s->sru11_sort_spec);
                break;
            }
            if (r)
                return r;
        }
        switch (s->query_type)
        {
        case Z_Query_type_1: /* RPN */
            if (s->sort_spec &&
                (s->sort_strategy == SORT_STRATEGY_TYPE7 ||
                 s->sort_strategy == SORT_STRATEGY_EMBED))
            {
                int r = yaz_sort_spec_to_type7(s->sort_spec, s->full_query);
                if (r)
                    return r;
            }
            s->z_query = (Z_Query *) odr_malloc(s->odr_query,
                                                sizeof(*s->z_query));
            s->z_query->which = Z_Query_type_1;
            s->z_query->u.type_1 =
                p_query_rpn(s->odr_query, wrbuf_cstr(s->full_query));
            if (!s->z_query->u.type_1)
            {
                s->z_query = 0;
                return -1;
            }
            break;
        case Z_Query_type_104: /* CQL */
            if (s->sort_spec &&
                (s->sort_strategy == SORT_STRATEGY_CQL ||
                 s->sort_strategy == SORT_STRATEGY_EMBED))
            {
                int r = yaz_sort_spec_to_cql(s->sort_spec, s->full_query);
                if (r)
                    return r;
            }
            ext = (Z_External *) odr_malloc(s->odr_query, sizeof(*ext));
            ext->direct_reference = odr_oiddup(s->odr_query,
                                               yaz_oid_userinfo_cql);
            ext->indirect_reference = 0;
            ext->descriptor = 0;
            ext->which = Z_External_CQL;
            ext->u.cql = odr_strdup(s->odr_query, wrbuf_cstr(s->full_query));

            s->z_query = (Z_Query *) odr_malloc(s->odr_query, sizeof(*s->z_query));
            s->z_query->which = Z_Query_type_104;
            s->z_query->u.type_104 =  ext;

            break;
        }
    }
    return 0;
}

const char *ZOOM_query_get_sru11(ZOOM_query s)
{
    if (wrbuf_len(s->sru11_sort_spec))
        return wrbuf_cstr(s->sru11_sort_spec);
    return 0;
}

Z_Query *ZOOM_query_get_Z_Query(ZOOM_query s)
{
    return s->z_query;
}

Z_SortKeySpecList *ZOOM_query_get_sortspec(ZOOM_query s)
{
    return s->sort_strategy == SORT_STRATEGY_Z3950 ? s->sort_spec : 0;
}

const char *ZOOM_query_get_query_string(ZOOM_query s)
{
    return wrbuf_cstr(s->full_query);
}

void ZOOM_query_get_hash(ZOOM_query s, WRBUF w)
{
    wrbuf_printf(w, "%d;", s->query_type);
    if (s->query_string)
        wrbuf_puts(w, s->query_string);
    wrbuf_printf(w, ";%d;", s->sort_strategy);
    if (s->sort_spec)
        yaz_sort_spec_to_type7(s->sort_spec, w);
}

/*
 * Returns an xmalloc()d string containing RPN that corresponds to the
 * CQL passed in.  On error, sets the Connection object's error state
 * and returns a null pointer.
 * ### We could cache CQL parser and/or transformer in Connection.
 */
static char *cql2pqf(ZOOM_connection c, const char *cql)
{
    CQL_parser parser;
    int error;
    const char *cqlfile;
    cql_transform_t trans;
    char *result = 0;

    parser = cql_parser_create();
    if ((error = cql_parser_string(parser, cql)) != 0) {
        cql_parser_destroy(parser);
        ZOOM_set_error(c, ZOOM_ERROR_CQL_PARSE, cql);
        return 0;
    }

    cqlfile = ZOOM_connection_option_get(c, "cqlfile");
    if (cqlfile == 0)
    {
        ZOOM_set_error(c, ZOOM_ERROR_CQL_TRANSFORM, "no CQL transform file");
    }
    else if ((trans = cql_transform_open_fname(cqlfile)) == 0)
    {
        char buf[512];
        sprintf(buf, "can't open CQL transform file '%.200s': %.200s",
                cqlfile, strerror(errno));
        ZOOM_set_error(c, ZOOM_ERROR_CQL_TRANSFORM, buf);
    }
    else
    {
        WRBUF wrbuf_result = wrbuf_alloc();
        error = cql_transform(trans, cql_parser_result(parser),
                              wrbuf_vp_puts, wrbuf_result);
        if (error != 0) {
            char buf[512];
            const char *addinfo;
            error = cql_transform_error(trans, &addinfo);
            sprintf(buf, "%.200s (addinfo=%.200s)",
                    cql_strerror(error), addinfo);
            ZOOM_set_error(c, ZOOM_ERROR_CQL_TRANSFORM, buf);
        }
        else
        {
            result = xstrdup(wrbuf_cstr(wrbuf_result));
        }
        cql_transform_close(trans);
        wrbuf_destroy(wrbuf_result);
    }
    cql_parser_destroy(parser);
    return result;
}


ZOOM_API(ZOOM_query)
    ZOOM_query_create(void)
{
    ZOOM_query s = (ZOOM_query) xmalloc(sizeof(*s));

    s->refcount = 1;
    s->z_query = 0;
    s->sort_spec = 0;
    s->odr_query = odr_createmem(ODR_ENCODE);
    s->odr_sort_spec = odr_createmem(ODR_ENCODE);
    s->query_string = 0;
    s->full_query = wrbuf_alloc();
    s->sort_strategy = SORT_STRATEGY_Z3950;
    s->sru11_sort_spec = wrbuf_alloc();
    return s;
}

ZOOM_API(void)
    ZOOM_query_destroy(ZOOM_query s)
{
    if (!s)
        return;

    (s->refcount)--;
    if (s->refcount == 0)
    {
        odr_destroy(s->odr_query);
        odr_destroy(s->odr_sort_spec);
        xfree(s->query_string);
        wrbuf_destroy(s->full_query);
        wrbuf_destroy(s->sru11_sort_spec);
        xfree(s);
    }
}

ZOOM_API(void)
    ZOOM_query_addref(ZOOM_query s)
{
    s->refcount++;
}


ZOOM_API(int)
    ZOOM_query_prefix(ZOOM_query s, const char *str)
{
    xfree(s->query_string);
    s->query_string = xstrdup(str);
    s->query_type = Z_Query_type_1;
    return generate(s);
}

ZOOM_API(int)
    ZOOM_query_cql(ZOOM_query s, const char *str)
{
    xfree(s->query_string);
    s->query_string = xstrdup(str);
    s->query_type = Z_Query_type_104;
    return generate(s);
}

/*
 * Translate the CQL string client-side into RPN which is passed to
 * the server.  This is useful for server's that don't themselves
 * support CQL, for which ZOOM_query_cql() is useless.  `conn' is used
 * only as a place to stash diagnostics if compilation fails; if this
 * information is not needed, a null pointer may be used.
 */
ZOOM_API(int)
    ZOOM_query_cql2rpn(ZOOM_query s, const char *str, ZOOM_connection conn)
{
    char *rpn;
    int ret;
    ZOOM_connection freeme = 0;

    if (conn == 0)
        conn = freeme = ZOOM_connection_create(0);

    rpn = cql2pqf(conn, str);
    if (freeme != 0)
        ZOOM_connection_destroy(freeme);
    if (rpn == 0)
        return -1;

    ret = ZOOM_query_prefix(s, rpn);
    xfree(rpn);
    return ret;
}

/*
 * Analogous in every way to ZOOM_query_cql2rpn(), except that there
 * is no analogous ZOOM_query_ccl() that just sends uninterpreted CCL
 * to the server, as the YAZ GFS doesn't know how to handle this.
 */
ZOOM_API(int)
    ZOOM_query_ccl2rpn(ZOOM_query s, const char *str, const char *config,
                       int *ccl_error, const char **error_string,
                       int *error_pos)
{
    int ret;
    struct ccl_rpn_node *rpn;
    CCL_bibset bibset = ccl_qual_mk();

    if (config)
        ccl_qual_buf(bibset, config);

    rpn = ccl_find_str(bibset, str, ccl_error, error_pos);
    if (!rpn)
    {
        *error_string = ccl_err_msg(*ccl_error);
        ret = -1;
    }
    else
    {
        WRBUF wr = wrbuf_alloc();
        ccl_pquery(wr, rpn);
        ccl_rpn_delete(rpn);
        ret = ZOOM_query_prefix(s, wrbuf_cstr(wr));
        wrbuf_destroy(wr);
    }
    ccl_qual_rm(&bibset);
    return ret;
}

ZOOM_API(int)
    ZOOM_query_sortby(ZOOM_query s, const char *criteria)
{
    return ZOOM_query_sortby2(s, "z3950", criteria);
}

ZOOM_API(int)
ZOOM_query_sortby2(ZOOM_query s, const char *strategy, const char *criteria)
{
    if (!strcmp(strategy, "z3950"))
    {
        s->sort_strategy = SORT_STRATEGY_Z3950;
    }
    else if (!strcmp(strategy, "type7"))
    {
        s->sort_strategy = SORT_STRATEGY_TYPE7;
    }
    else if (!strcmp(strategy, "cql"))
    {
        s->sort_strategy = SORT_STRATEGY_CQL;
    }
    else if (!strcmp(strategy, "sru11"))
    {
        s->sort_strategy = SORT_STRATEGY_SRU11;
    }
    else if (!strcmp(strategy, "solr"))
    {
        s->sort_strategy = SORT_STRATEGY_SOLR;
    }
    else if (!strcmp(strategy, "embed"))
    {
        s->sort_strategy = SORT_STRATEGY_EMBED;
    }
    else
        return -1;

    odr_reset(s->odr_sort_spec);
    s->sort_spec = yaz_sort_spec(s->odr_sort_spec, criteria);
    if (!s->sort_spec)
        return -2;
    return generate(s);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

