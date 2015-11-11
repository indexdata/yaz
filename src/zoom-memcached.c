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
#if HAVE_LIBMEMCACHED
    c->mc_st = 0;
#endif
#if HAVE_HIREDIS
    c->redis_c = 0;
#endif
    c->expire_search = 600;
    c->expire_record = 1200;
}

void ZOOM_memcached_destroy(ZOOM_connection c)
{
#if HAVE_LIBMEMCACHED
    if (c->mc_st)
        memcached_free(c->mc_st);
#endif
#if HAVE_HIREDIS
    if (c->redis_c)
        redisFree(c->redis_c);
#endif
}

#if HAVE_LIBMEMCACHED
static memcached_st *create_memcached(const char *conf,
                                      int *expire_search, int *expire_record)
{
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
        else if (!yaz_strncasecmp(darray[i], "--EXPIRE=", 9))
        {
            *expire_search = atoi(darray[i] + 9);
            *expire_record = 600 + *expire_search;
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
}
#endif

#if HAVE_HIREDIS
static redisContext *create_redis(const char *conf,
                                  int *expire_search, int *expire_record)
{
    char **darray;
    int i, num;
    NMEM nmem = nmem_create();
    redisContext *context = 0;

    nmem_strsplit_blank(nmem, conf, &darray, &num);
    for (i = 0; i < num; i++)
    {
        if (!yaz_strncasecmp(darray[i], "--SERVER=", 9))
        {
            struct timeval timeout = { 1, 500000 }; /* 1.5 seconds */
            char *host = darray[i] + 9;
            char *port = strchr(host, ':');
            if (port)
                *port++ = '\0';
            context = redisConnectWithTimeout(host,
                                              port ? atoi(port) : 6379,
                                              timeout);
        }
        else if (!yaz_strncasecmp(darray[i], "--EXPIRE=", 9))
        {
            *expire_search = atoi(darray[i] + 9);
            *expire_record = 600 + *expire_search;
        }
    }
    nmem_destroy(nmem);
    return context;
}
#endif

int ZOOM_memcached_configure(ZOOM_connection c)
{
    const char *val;
#if HAVE_HIREDIS
    if (c->redis_c)
    {
        redisFree(c->redis_c);
        c->redis_c = 0;
    }
#endif
#if HAVE_LIBMEMCACHED
    if (c->mc_st)
    {
        memcached_free(c->mc_st);
        c->mc_st = 0;
    }
#endif

    val = ZOOM_options_get(c->options, "redis");
    if (val && *val)
    {
#if HAVE_HIREDIS
        c->redis_c = create_redis(val,
                                  &c->expire_search, &c->expire_record);
        if (c->redis_c == 0 || c->redis_c->err)
        {
            ZOOM_set_error(c, ZOOM_ERROR_MEMCACHED,
                           "could not create redis");
            return -1;
        }
        return 0; /* don't bother with memcached if redis is enabled */
#else
        ZOOM_set_error(c, ZOOM_ERROR_MEMCACHED, "not enabled");
        return -1;
#endif
    }
    val = ZOOM_options_get(c->options, "memcached");
    if (val && *val)
    {
#if HAVE_LIBMEMCACHED
        c->mc_st = create_memcached(val, &c->expire_search, &c->expire_record);
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

static void wrbuf_vary_puts(WRBUF w, const char *v)
{
    if (v)
    {
        if (strlen(v) > 40)
        {
            wrbuf_sha1_puts(w, v, 1);
        }
        else
        {
            wrbuf_puts(w, v);
        }
    }
}

void ZOOM_memcached_resultset(ZOOM_resultset r, ZOOM_query q)
{
    ZOOM_connection c = r->connection;

    r->mc_key = wrbuf_alloc();
    wrbuf_puts(r->mc_key, "1;");
    wrbuf_vary_puts(r->mc_key, c->host_port);
    wrbuf_puts(r->mc_key, ";");
    wrbuf_vary_puts(r->mc_key, ZOOM_resultset_option_get(r, "extraArgs"));
    wrbuf_puts(r->mc_key, ";");
    wrbuf_vary_puts(r->mc_key, c->user);
    wrbuf_puts(r->mc_key, ";");
    wrbuf_vary_puts(r->mc_key, c->group);
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
    wrbuf_vary_puts(r->mc_key, r->req_facets);
}

void ZOOM_memcached_search(ZOOM_connection c, ZOOM_resultset resultset)
{
#if HAVE_HIREDIS
    if (c->redis_c && resultset->live_set == 0)
    {
        redisReply *reply;
        const char *argv[2];

        argv[0] = "GET";
        argv[1] = wrbuf_cstr(resultset->mc_key);

        reply = redisCommandArgv(c->redis_c, 2, argv, 0);
        /* count;precision (ASCII) + '\0' + BER buffer for otherInformation */
        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            char *v = reply->str;
            int v_len = reply->len;
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
                    freeReplyObject(reply);
                    return;
                }
                ZOOM_handle_search_result(c, resultset, oi);
                ZOOM_handle_facet_result(c, resultset, oi);
            }
            event = ZOOM_Event_create(ZOOM_EVENT_RECV_SEARCH);
            ZOOM_connection_put_event(c, event);
            resultset->live_set = 1;
        }
        if (reply)
            freeReplyObject(reply);
    }
#endif
#if HAVE_LIBMEMCACHED
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

#if HAVE_HIREDIS
static void expire_redis(redisContext *redis_c,
                         const char *buf, size_t len, int exp)
{
    redisReply *reply;
    const char *argv[3];
    size_t argvlen[3];
    char key_val[20];

    sprintf(key_val, "%d", exp);

    argv[0] = "EXPIRE";
    argvlen[0] = 6;
    argv[1] = buf;
    argvlen[1] = len;
    argv[2] = key_val;
    argvlen[2] = strlen(key_val);
    reply = redisCommandArgv(redis_c, 3, argv, argvlen);
    freeReplyObject(reply);
}
#endif

void ZOOM_memcached_hitcount(ZOOM_connection c, ZOOM_resultset resultset,
                             Z_OtherInformation *oi, const char *precision)
{
#if HAVE_HIREDIS
    if (c->redis_c && resultset->live_set == 0)
    {
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

        {
            redisReply *reply;
            const char *argv[3];
            size_t argvlen[3];
            argv[0] = "SET";
            argvlen[0] = 3;
            argv[1] = wrbuf_buf(resultset->mc_key);
            argvlen[1] = wrbuf_len(resultset->mc_key);
            argv[2] = key;
            argvlen[2] = strlen(str) + 1 + oi_len;
            reply = redisCommandArgv(c->redis_c, 3, argv, argvlen);
            freeReplyObject(reply);
        }
        expire_redis(c->redis_c,
                     wrbuf_buf(resultset->mc_key),
                     wrbuf_len(resultset->mc_key),
                     c->expire_search);
        odr_destroy(odr);
    }
#endif
#if HAVE_LIBMEMCACHED
    if (c->mc_st && resultset->live_set == 0)
    {
        uint32_t flags = 0;
        memcached_return_t rc;
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
                           key, strlen(str) + 1 + oi_len,
                           c->expire_search, flags);
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
#if HAVE_HIREDIS
    if (r->connection->redis_c &&
        !diag && npr->which == Z_NamePlusRecord_databaseRecord)
    {
        WRBUF k = wrbuf_alloc();
        WRBUF rec_sha1 = wrbuf_alloc();
        ODR odr = odr_createmem(ODR_ENCODE);
        char *rec_buf;
        int rec_len;
        const char *argv[3];
        size_t argvlen[3];
        redisReply *reply;

        z_NamePlusRecord(odr, &npr, 0, 0);
        rec_buf = odr_getbuf(odr, &rec_len, 0);

        wrbuf_write(k, wrbuf_buf(r->mc_key), wrbuf_len(r->mc_key));
        wrbuf_printf(k, ";%d;%s;%s;%s", pos,
                     syntax ? syntax : "",
                     elementSetName ? elementSetName : "",
                     schema ? schema : "");

        wrbuf_sha1_write(rec_sha1, rec_buf, rec_len, 1);

        argv[0] = "SET";
        argvlen[0] = 3;
        argv[1] = wrbuf_buf(k);
        argvlen[1] = wrbuf_len(k);
        argv[2] = wrbuf_buf(rec_sha1);
        argvlen[2] = wrbuf_len(rec_sha1);

        reply = redisCommandArgv(r->connection->redis_c, 3, argv, argvlen);
        yaz_log(YLOG_LOG, "Store record key=%s val=%s",
                wrbuf_cstr(k), wrbuf_cstr(rec_sha1));
        freeReplyObject(reply);

        expire_redis(r->connection->redis_c, argv[1], argvlen[1],
                     r->connection->expire_search);

        argv[1] = wrbuf_buf(rec_sha1);
        argvlen[1] = wrbuf_len(rec_sha1);
        argv[2] = rec_buf;
        argvlen[2] = rec_len;

        reply = redisCommandArgv(r->connection->redis_c, 3, argv, argvlen);
        yaz_log(YLOG_LOG, "Add record key=%s rec_len=%d",
                wrbuf_cstr(rec_sha1), rec_len);
        freeReplyObject(reply);

        expire_redis(r->connection->redis_c, argv[1], argvlen[1],
                     r->connection->expire_record);

        odr_destroy(odr);
        wrbuf_destroy(k);
        wrbuf_destroy(rec_sha1);
    }
#endif
#if HAVE_LIBMEMCACHED
    if (r->connection->mc_st &&
        !diag && npr->which == Z_NamePlusRecord_databaseRecord)
    {
        WRBUF k = wrbuf_alloc();
        WRBUF rec_sha1 = wrbuf_alloc();
        uint32_t flags = 0;
        memcached_return_t rc;
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
                           r->connection->expire_search, flags);

        yaz_log(YLOG_LOG, "Store record key=%s val=%s rc=%u %s",
                wrbuf_cstr(k), wrbuf_cstr(rec_sha1), (unsigned) rc,
                memcached_strerror(r->connection->mc_st, rc));

        rc = memcached_add(r->connection->mc_st,
                           wrbuf_buf(rec_sha1), wrbuf_len(rec_sha1),
                           rec_buf, rec_len,
                           r->connection->expire_record, flags);

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
#if HAVE_HIREDIS
    if (r->connection && r->connection->redis_c)
    {
        WRBUF k = wrbuf_alloc();
        const char *argv[2];
        size_t argvlen[2];
        redisReply *reply1;

        wrbuf_write(k, wrbuf_buf(r->mc_key), wrbuf_len(r->mc_key));
        wrbuf_printf(k, ";%d;%s;%s;%s", pos,
                     syntax ? syntax : "",
                     elementSetName ? elementSetName : "",
                     schema ? schema : "");

        yaz_log(YLOG_LOG, "Lookup record %s", wrbuf_cstr(k));
        argv[0] = "GET";
        argvlen[0] = 3;
        argv[1] = wrbuf_buf(k);
        argvlen[1] = wrbuf_len(k);
        reply1 = redisCommandArgv(r->connection->redis_c, 2, argv, argvlen);

        wrbuf_destroy(k);
        if (reply1 && reply1->type == REDIS_REPLY_STRING)
        {
            redisReply *reply2;
            char *sha1_buf = reply1->str;
            int sha1_len = reply1->len;

            yaz_log(YLOG_LOG, "Lookup record %.*s", (int) sha1_len, sha1_buf);

            argv[0] = "GET";
            argvlen[0] = 3;
            argv[1] = sha1_buf;
            argvlen[1] = sha1_len;

            reply2 = redisCommandArgv(r->connection->redis_c, 2, argv, argvlen);
            if (reply2 && reply2->type == REDIS_REPLY_STRING)
            {
                Z_NamePlusRecord *npr = 0;
                char *v_buf = reply2->str;
                int v_len = reply2->len;

                odr_setbuf(r->odr, v_buf, v_len, 0);
                z_NamePlusRecord(r->odr, &npr, 0, 0);
                if (npr)
                    yaz_log(YLOG_LOG, "returned redis copy");
                freeReplyObject(reply2);
                freeReplyObject(reply1);
                return npr;
            }
            freeReplyObject(reply2);
        }
        freeReplyObject(reply1);
    }
#endif
#if HAVE_LIBMEMCACHED
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

