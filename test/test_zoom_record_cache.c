/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include "../src/zoom-p.h"

#if YAZ_HAVE_XML2
/* Deliver a searchRetrieveResponse holding num records to a pending search
   for count records. Record i gets recordPosition positions[i] if positions
   are given and 1 + i * stride otherwise. */
static ZOOM_resultset search_response(ZOOM_connection c, int count, int num,
                                      int stride, const int *positions)
{
    ZOOM_resultset r;
    WRBUF w = wrbuf_alloc();
    Z_GDU *gdu = z_get_HTTP_Response(c->odr_in, 200);
    Z_HTTP_Response *hres = gdu->u.HTTP_Response;
    zoom_ret cret = zoom_pending;
    char *addinfo = 0;
    int i;

    c->async = 1; /* search is left pending; the response is given below */
    c->sru_mode = zoom_sru_get;
    ZOOM_options_set_int(c->options, "count", count);
    r = ZOOM_connection_search_pqf(c, "water");
    YAZ_CHECK(c->tasks && c->tasks->which == ZOOM_TASK_SEARCH);

    wrbuf_puts(w, "<zs:searchRetrieveResponse "
               "xmlns:zs=\"http://www.loc.gov/zing/srw/\">"
               "<zs:version>1.1</zs:version>");
    wrbuf_printf(w, "<zs:numberOfRecords>%d</zs:numberOfRecords>"
                 "<zs:records>", num);
    for (i = 0; i < num; i++)
        wrbuf_printf(w, "<zs:record><zs:recordPosition>%d"
                     "</zs:recordPosition></zs:record>",
                     positions ? positions[i] : 1 + i * stride);
    wrbuf_puts(w, "</zs:records></zs:searchRetrieveResponse>");

    z_HTTP_header_add_content_type(c->odr_in, &hres->headers, "text/xml", 0);
    hres->content_buf = (char *) wrbuf_buf(w);
    hres->content_len = (int) wrbuf_len(w);
    YAZ_CHECK_EQ(ZOOM_handle_sru(c, hres, &cret, &addinfo), 0);
    YAZ_CHECK_EQ(cret, zoom_complete);
    YAZ_CHECK_EQ(ZOOM_connection_error(c, 0, 0), 0);
    wrbuf_destroy(w);
    return r;
}

/* The cache grows with the number of records it holds */
static void check_many_records(void)
{
    const int num = 40 * RECORD_HASH_SIZE;
    ZOOM_connection c = ZOOM_connection_create(0);
    ZOOM_resultset r = search_response(c, num, num, 1, 0);
    size_t i;

    for (i = 0; i < (size_t) num; i++)
        if (!ZOOM_resultset_record_immediate(r, i))
            break;
    YAZ_CHECK_EQ(i, num);
    YAZ_CHECK(!ZOOM_resultset_record_immediate(r, num));

    YAZ_CHECK_EQ(r->record_hash_count, num);
    YAZ_CHECK(r->record_hash_size >= r->record_hash_count);

    ZOOM_resultset_cache_reset(r);
    YAZ_CHECK_EQ(r->record_hash_count, 0);
    YAZ_CHECK(!ZOOM_resultset_record_immediate(r, 0));
    YAZ_CHECK(!ZOOM_resultset_record_immediate(r, num - 1));

    ZOOM_resultset_destroy(r);
    ZOOM_connection_destroy(c);
}

/* The server must not decide where in the cache its records go: a
   recordPosition is only used if it is one of the positions asked for. */
static void check_record_positions(void)
{
    /* 1: as calculated, 1000: outside the 3 records asked for, 3: as
       calculated. With 1000 ignored that record is the second one. */
    static const int positions[3] = {1, 1000, 3};
    const int num = 20 * RECORD_HASH_SIZE;
    ZOOM_connection c = ZOOM_connection_create(0);
    ZOOM_resultset r = search_response(c, 3, 3, 1, positions);
    size_t i;

    YAZ_CHECK(ZOOM_resultset_record_immediate(r, 0));
    YAZ_CHECK(ZOOM_resultset_record_immediate(r, 1));
    YAZ_CHECK(ZOOM_resultset_record_immediate(r, 2));
    YAZ_CHECK(!ZOOM_resultset_record_immediate(r, 999));
    ZOOM_resultset_destroy(r);
    ZOOM_connection_destroy(c);

    /* a gap inside the range asked for is kept */
    c = ZOOM_connection_create(0);
    r = search_response(c, 5, 2, 3, 0);
    YAZ_CHECK(ZOOM_resultset_record_immediate(r, 0));
    YAZ_CHECK(!ZOOM_resultset_record_immediate(r, 1));
    YAZ_CHECK(ZOOM_resultset_record_immediate(r, 3));
    ZOOM_resultset_destroy(r);
    ZOOM_connection_destroy(c);

    /* no records asked for, and all positions offered are in the same
       chain of the initial hash table */
    c = ZOOM_connection_create(0);
    r = search_response(c, 0, num, RECORD_HASH_SIZE, 0);
    for (i = 0; i < (size_t) num; i++)
        if (!ZOOM_resultset_record_immediate(r, i))
            break;
    YAZ_CHECK_EQ(i, num);
    YAZ_CHECK(!ZOOM_resultset_record_immediate(r, num));
    YAZ_CHECK_EQ(r->record_hash_count, num);
    ZOOM_resultset_destroy(r);
    ZOOM_connection_destroy(c);
}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
#if YAZ_HAVE_XML2
    check_many_records();
    check_record_positions();
#endif
    YAZ_CHECK_TERM;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
