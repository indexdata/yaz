/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/test.h>
#include <yaz/log.h>
#include <yaz/wrbuf.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"

#if YAZ_HAVE_XML2
static int search(void *handle, bend_search_rr *rr)
{
    rr->hits = 0;
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    rr->bend_search = search;
    return &result;
}

static int get(COMSTACK cs, char **buf, int *size)
{
    *buf = xstrdup((const char *) cs->user);
    *size = (int) strlen(*buf);
    return *size;
}

static int more(COMSTACK cs)
{
    return 0;
}

static const char *addrstr(COMSTACK cs)
{
    return "test";
}

/* Collect log entries, one per line. */
static void log_handler(int level, const char *message, void *info)
{
    WRBUF log = (WRBUF) info;
    wrbuf_puts(log, message);
    wrbuf_puts(log, "\n");
}

/* Send a wire-format request through the actual HTTP/SRU frontend and
   check that control characters from the peer are escaped in the log. */
static void check_request_log(void)
{
    struct comstack cs;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    WRBUF log = wrbuf_alloc();
    association *assoc;

    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_HTTP;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    cs.user = "GET /Default\x1b?version=1.2&operation=searchRetrieve"
        "&query=a%0Ab%1Bc%7F%C3%A6&maximumRecords=0 HTTP/1.1\r\n"
        "Host: localhost\r\n\r\n";
    yaz_log_set_handler(log_handler, log);
    assoc = create_association(chan, &cs, 0);
    iochan_setdata(chan, assoc);
    assoc->cs_get_mask = EVENT_INPUT;
    ir_session(chan, EVENT_INPUT);
    yaz_log_set_handler(0, 0);
    YAZ_CHECK(request_head(&assoc->outgoing) != 0);

    YAZ_CHECK(strstr(wrbuf_cstr(log), "GET /Default\\x1B?version=1.2") != 0);
    YAZ_CHECK(strstr(wrbuf_cstr(log),
                     " cql: a\\x0Ab\\x1Bc\\x7F\xc3\xa6\n") != 0);
    YAZ_CHECK(strchr(wrbuf_cstr(log), 27) == 0);

    destroy_association(assoc);
    xfree(chan);
    wrbuf_destroy(log);
}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if YAZ_HAVE_XML2
    /* No log file. Entries are passed to log_handler only. */
    yaz_log_init_file(0);
    yaz_log_init_level(yaz_log_mask_str("request"));
    statserv_getcontrol()->bend_init = init;
    check_request_log();
#endif
    YAZ_CHECK_TERM;
}
