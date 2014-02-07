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

#if HAVE_LIBMEMCACHED_MEMCACHED_H
#if HAVE_MEMCACHED_RETURN_T
#else
typedef memcached_return memcached_return_t;
#endif
#endif

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

#if HAVE_LIBMEMCACHED_MEMCACHED_H
/* memcached wrapper.. Because memcached function do not exist in older libs */
static memcached_st *yaz_memcached_wrap(const char *conf)
{
#if HAVE_MEMCACHED
    return memcached(conf, strlen(conf));
#else
    char **darray;
    int i, num;
    memcached_st *mc = memcached_create(0);
    NMEM nmem = nmem_create();
    memcached_return_t rc;

    nmem_strsplit_blank(nmem, conf, &darray, &num);
    for (i = 0; mc && i < num; i++)
    {
        if (!yaz_strncasecmp(darray[i], "--SERVER=", 9))
        {
            char *host = darray[i] + 9;
            char *port = strchr(host, ':');
            char *weight = strstr(host, "/?");
            if (port)
                *port++ = '\0';
            if (weight)
            {
                *weight = '\0';
                weight += 2;
            }
            rc = memcached_server_add(mc, host, port ? atoi(port) : 11211);
            yaz_log(YLOG_LOG, "memcached_server_add host=%s rc=%u %s",
                    host, (unsigned) rc, memcached_strerror(mc, rc));
            if (rc != MEMCACHED_SUCCESS)
            {
                memcached_free(mc);
                mc = 0;
            }
        }
        else
        {
            /* bad directive */
            memcached_free(mc);
            mc = 0;
        }
    }
    nmem_destroy(nmem);
    return mc;
#endif
}
#endif

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
        c->mc_st = yaz_memcached_wrap(val);
        if (!c->mc_st)
        {
            ZOOM_set_error(c, ZOOM_ERROR_MEMCACHED,
                           "could not create memcached");
            return -1;
        }
        memcached_behavior_set(c->mc_st, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);
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
    if (r->req_facets)
        wrbuf_puts(r->mc_key, r->req_facets);
#endif
}

void ZOOM_memcached_search(ZOOM_connection c, ZOOM_resultset resultset)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    if (c->mc_st && resultset->live_set == 0)
    {
        size_t v_len;
        uint32_t flags;
        memcached_return_t rc;
        char *v = memcached_get(c->mc_st, wrbuf_buf(resultset->mc_key),
                                wrbuf_len(resultset->mc_key),
                                &v_len, &flags, &rc);
        /* count;precision (ASCII) + '\0' + BER buffer for otherInformation */
        if (v)
        {
            ZOOM_Event event;
            size_t lead_len = strlen(v) + 1;

            resultset->size = odr_atoi(v);

            yaz_log(YLOG_LOG, "For key %s got value %s lead_len=%d len=%d",
                    wrbuf_cstr(resultset->mc_key), v, (int) lead_len,
                    (int) v_len);
            if (v_len > lead_len)
            {
                Z_OtherInformation *oi = 0;
                int oi_len = v_len - lead_len;
                odr_setbuf(resultset->odr, v + lead_len, oi_len, 0);
                if (!z_OtherInformation(resultset->odr, &oi, 0, 0))
                {
                    yaz_log(YLOG_WARN, "oi decoding failed");
                    free(v);
                    return;
                }
                ZOOM_handle_search_result(c, resultset, oi);
                ZOOM_handle_facet_result(c, resultset, oi);
            }
            free(v);
            event = ZOOM_Event_create(ZOOM_EVENT_RECV_SEARCH);
            ZOOM_connection_put_event(c, event);
            resultset->live_set = 1;
        }
    }
#endif
}

void ZOOM_memcached_hitcount(ZOOM_connection c, ZOOM_resultset resultset,
                             Z_OtherInformation *oi, const char *precision)
{
#if HAVE_LIBMEMCACHED_MEMCACHED_H
    if (c->mc_st && resultset->live_set == 0)
    {
        uint32_t flags = 0;
        memcached_return_t rc;
        time_t expiration = 36000;
        char *str;
        ODR odr = odr_createmem(ODR_ENCODE);
        char *oi_buf = 0;
        int oi_len = 0;
        char *key;

        str = odr_malloc(odr, 20 + strlen(precision));
        /* count;precision (ASCII) + '\0' + BER buffer for otherInformation */
        sprintf(str, ODR_INT_PRINTF ";%s", resultset->size, precision);
        if (oi)
        {
            z_OtherInformation(odr, &oi, 0, 0);
            oi_buf = odr_getbuf(odr, &oi_len, 0);
        }
        key = odr_malloc(odr, strlen(str) + 1 + oi_len);
        strcpy(key, str);
        if (oi_len)
            memcpy(key + strlen(str) + 1, oi_buf, oi_len);

        rc = memcached_set(c->mc_st,
                           wrbuf_buf(resultset->mc_key),
                           wrbuf_len(resultset->mc_key),
                           key, strlen(str) + 1 + oi_len, expiration, flags);
        yaz_log(YLOG_LOG, "Store hit count key=%s value=%s oi_len=%d rc=%u %s",
                wrbuf_cstr(resultset->mc_key), str, oi_len, (unsigned) rc,
                memcached_strerror(c->mc_st, rc));
        odr_destroy(odr);
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
        WRBUF rec_sha1 = wrbuf_alloc();
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

        wrbuf_sha1_write(rec_sha1, rec_buf, rec_len, 1);

        rc = memcached_set(r->connection->mc_st,
                           wrbuf_buf(k), wrbuf_len(k),
                           wrbuf_buf(rec_sha1), wrbuf_len(rec_sha1),
                           expiration, flags);

        yaz_log(YLOG_LOG, "Store record key=%s val=%s rc=%u %s",
                wrbuf_cstr(k), wrbuf_cstr(rec_sha1), (unsigned) rc,
                memcached_strerror(r->connection->mc_st, rc));

        rc = memcached_add(r->connection->mc_st,
                           wrbuf_buf(rec_sha1), wrbuf_len(rec_sha1),
                           rec_buf, rec_len,
                           expiration, flags);

        yaz_log(YLOG_LOG, "Add record key=%s rec_len=%d rc=%u %s",
                wrbuf_cstr(rec_sha1), rec_len, (unsigned) rc,
                memcached_strerror(r->connection->mc_st, rc));

        odr_destroy(odr);
        wrbuf_destroy(k);
        wrbuf_destroy(rec_sha1);
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
        char *sha1_buf;
        size_t sha1_len;
        uint32_t flags;
        memcached_return_t rc;

        wrbuf_write(k, wrbuf_buf(r->mc_key), wrbuf_len(r->mc_key));
        wrbuf_printf(k, ";%d;%s;%s;%s", pos,
                     syntax ? syntax : "",
                     elementSetName ? elementSetName : "",
                     schema ? schema : "");

        yaz_log(YLOG_LOG, "Lookup record %s", wrbuf_cstr(k));
        sha1_buf = memcached_get(r->connection->mc_st,
                                 wrbuf_buf(k), wrbuf_len(k),
                                 &sha1_len, &flags, &rc);

        wrbuf_destroy(k);
        if (sha1_buf)
        {
            size_t v_len;
            char *v_buf;

            yaz_log(YLOG_LOG, "Lookup record %.*s", (int) sha1_len, sha1_buf);
            v_buf = memcached_get(r->connection->mc_st, sha1_buf, sha1_len,
                                  &v_len, &flags, &rc);
            free(sha1_buf);
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

