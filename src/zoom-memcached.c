/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-memcached.c
 * \brief Implements query/record caching using memcached
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
#include <yaz/diagbib1.h>

void ZOOM_memcached_init(ZOOM_connection c)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    c->mc_st = 0;
#endif
}

void ZOOM_memcached_destroy(ZOOM_connection c)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    if (c->mc_st)
        memcached_free(c->mc_st);
#endif
}

int ZOOM_memcached_configure(ZOOM_connection c)
{
    const char *val;
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    if (c->mc_st)
    {
        memcached_free(c->mc_st);
        c->mc_st = 0;
    }
#endif
    val = ZOOM_options_get(c->options, "memcached");
    if (val && *val)
    {
#if HAVE_LIBMEMCACHED_MEMCACHED_H
        c->mc_st = memcached(val, strlen(val));
        if (!c->mc_st)
        {
            ZOOM_set_error(c, ZOOM_ERROR_MEMCACHED, val);
            return -1;
        }
#else
        ZOOM_set_error(c, ZOOM_ERROR_MEMCACHED, "not enabled");
        return -1;
#endif
    }
    return 0;
}

void ZOOM_memcached_resultset(ZOOM_resultset r, ZOOM_query q)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    ZOOM_connection c = r->connection;
    r->mc_key = wrbuf_alloc();
    wrbuf_puts(r->mc_key, "0;");
    wrbuf_puts(r->mc_key, c->host_port);
    wrbuf_puts(r->mc_key, ";");
    if (c->user)
        wrbuf_puts(r->mc_key, c->user);
    wrbuf_puts(r->mc_key, ";");
    if (c->group)
        wrbuf_puts(r->mc_key, c->group);
    wrbuf_puts(r->mc_key, ";");
    if (c->password)
        wrbuf_sha1_puts(r->mc_key, c->password, 1);
    wrbuf_puts(r->mc_key, ";");
    {
        WRBUF w = wrbuf_alloc();
        ZOOM_query_get_hash(q, w);
        wrbuf_sha1_puts(r->mc_key, wrbuf_cstr(w), 1);
        wrbuf_destroy(w);
    }
    wrbuf_puts(r->mc_key, ";");
#endif
}

void ZOOM_memcached_search(ZOOM_connection c, ZOOM_resultset resultset)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    /* TODO: add sorting */
    if (c->mc_st && resultset->live_set == 0)
    {
        size_t v_len;
        uint32_t flags;
        memcached_return_t rc;
        char *v = memcached_get(c->mc_st, wrbuf_buf(resultset->mc_key),
                                wrbuf_len(resultset->mc_key),
                                &v_len, &flags, &rc);
        if (v)
        {
            ZOOM_Event event;
            WRBUF w = wrbuf_alloc();

            wrbuf_write(w, v, v_len);
            free(v);
            resultset->size = odr_atoi(wrbuf_cstr(w));

            yaz_log(YLOG_LOG, "For key %s got value %s",
                    wrbuf_cstr(resultset->mc_key), wrbuf_cstr(w));

            wrbuf_destroy(w);
            event = ZOOM_Event_create(ZOOM_EVENT_RECV_SEARCH);
            ZOOM_connection_put_event(c, event);
            resultset->live_set = 1;
        }
    }
#endif
}

void ZOOM_memcached_hitcount(ZOOM_connection c, ZOOM_resultset resultset)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    if (c->mc_st && resultset->live_set == 0)
    {
        uint32_t flags = 0;
        memcached_return_t rc;
        time_t expiration = 36000;
        char str[40];

        sprintf(str, ODR_INT_PRINTF, resultset->size);
        rc = memcached_set(c->mc_st,
                           wrbuf_buf(resultset->mc_key),wrbuf_len(resultset->mc_key),
                           str, strlen(str), expiration, flags);
        yaz_log(YLOG_LOG, "Store hit count key=%s value=%s rc=%u %s",
                wrbuf_cstr(resultset->mc_key), str, (unsigned) rc,
                memcached_last_error_message(c->mc_st));
    }
#endif
}

void ZOOM_memcached_add(ZOOM_resultset r, Z_NamePlusRecord *npr,
                        int pos,
                        const char *syntax, const char *elementSetName,
                        const char *schema,
                        Z_SRW_diagnostic *diag)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    if (r->connection->mc_st &&
        !diag && npr->which == Z_NamePlusRecord_databaseRecord)
    {
        WRBUF k = wrbuf_alloc();
        uint32_t flags = 0;
        memcached_return_t rc;
        time_t expiration = 36000;
        ODR odr = odr_createmem(ODR_ENCODE);
        char *rec_buf;
        int rec_len;

        z_NamePlusRecord(odr, &npr, 0, 0);
        rec_buf = odr_getbuf(odr, &rec_len, 0);

        wrbuf_write(k, wrbuf_buf(r->mc_key), wrbuf_len(r->mc_key));
        wrbuf_printf(k, ";%d;%s;%s;%s", pos,
                     syntax ? syntax : "",
                     elementSetName ? elementSetName : "",
                     schema ? schema : "");
        rc = memcached_set(r->connection->mc_st,
                           wrbuf_buf(k),wrbuf_len(k),
                           rec_buf, rec_len,
                           expiration, flags);

        yaz_log(YLOG_LOG, "Store record lkey=%s len=%d rc=%u %s",
                wrbuf_cstr(k), rec_len, (unsigned) rc,
                memcached_last_error_message(r->connection->mc_st));
        odr_destroy(odr);
        wrbuf_destroy(k);
    }
#endif
}

Z_NamePlusRecord *ZOOM_memcached_lookup(ZOOM_resultset r, int pos,
                                        const char *syntax,
                                        const char *elementSetName,
                                        const char *schema)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    if (r->connection && r->connection->mc_st)
    {
        WRBUF k = wrbuf_alloc();
        size_t v_len;
        char *v_buf;
        uint32_t flags;
        memcached_return_t rc;

        wrbuf_write(k, wrbuf_buf(r->mc_key), wrbuf_len(r->mc_key));
        wrbuf_printf(k, ";%d;%s;%s;%s", pos,
                     syntax ? syntax : "",
                     elementSetName ? elementSetName : "",
                     schema ? schema : "");

        yaz_log(YLOG_LOG, "Lookup record %s", wrbuf_cstr(k));
        v_buf = memcached_get(r->connection->mc_st, wrbuf_buf(k), wrbuf_len(k),
                              &v_len, &flags, &rc);
        wrbuf_destroy(k);
        if (v_buf)
        {
            Z_NamePlusRecord *npr = 0;

            odr_setbuf(r->odr, v_buf, v_len, 0);
            z_NamePlusRecord(r->odr, &npr, 0, 0);
            free(v_buf);
            if (npr)
                yaz_log(YLOG_LOG, "returned memcached copy");
            return npr;
        }
    }
#endif
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

