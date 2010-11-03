/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
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

struct ZOOM_query_p {
    Z_Query *z_query;
    Z_SortKeySpecList *sort_spec;
    int refcount;
    ODR odr;
    char *query_string;
};

Z_Query *ZOOM_query_get_Z_Query(ZOOM_query s)
{
    return s->z_query;
}


Z_SortKeySpecList *ZOOM_query_get_sortspec(ZOOM_query s)
{
    return s->sort_spec;
}

static void cql2pqf_wrbuf_puts(const char *buf, void *client_data)
{
    WRBUF wrbuf = (WRBUF) client_data;
    wrbuf_puts(wrbuf, buf);
}

char *ZOOM_query_get_query_string(ZOOM_query s)
{
    return s->query_string;
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
                              cql2pqf_wrbuf_puts, wrbuf_result);
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
    s->odr = odr_createmem(ODR_ENCODE);
    s->query_string = 0;

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
        odr_destroy(s->odr);
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
    s->query_string = odr_strdup(s->odr, str);
    s->z_query = (Z_Query *) odr_malloc(s->odr, sizeof(*s->z_query));
    s->z_query->which = Z_Query_type_1;
    s->z_query->u.type_1 =  p_query_rpn(s->odr, str);
    if (!s->z_query->u.type_1)
    {
        s->z_query = 0;
        return -1;
    }
    return 0;
}

ZOOM_API(int)
    ZOOM_query_cql(ZOOM_query s, const char *str)
{
    Z_External *ext;

    s->query_string = odr_strdup(s->odr, str);

    ext = (Z_External *) odr_malloc(s->odr, sizeof(*ext));
    ext->direct_reference = odr_oiddup(s->odr, yaz_oid_userinfo_cql);
    ext->indirect_reference = 0;
    ext->descriptor = 0;
    ext->which = Z_External_CQL;
    ext->u.cql = s->query_string;
    
    s->z_query = (Z_Query *) odr_malloc(s->odr, sizeof(*s->z_query));
    s->z_query->which = Z_Query_type_104;
    s->z_query->u.type_104 =  ext;

    return 0;
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
    s->sort_spec = yaz_sort_spec(s->odr, criteria);
    if (!s->sort_spec)
        return -1;
    return 0;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

