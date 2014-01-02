/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/wrbuf.h>
#include <yaz/sortspec.h>
#include <yaz/log.h>
#include <yaz/test.h>

static int cql(const char *arg, const char *expected_result)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_SortKeySpecList *sort_spec = yaz_sort_spec(odr, arg);
    int ret = 0;

    if (!sort_spec)
    {
        yaz_log(YLOG_WARN, "yaz_sort_spec : parse error: %s", arg);
    }
    else
    {
        WRBUF w = wrbuf_alloc();
        int r = yaz_sort_spec_to_cql(sort_spec, w);

        if (!expected_result && r)
            ret = 1;
        else if (expected_result && r == 0)
        {
            if (strcmp(wrbuf_cstr(w), expected_result) == 0)
                ret = 1;
            else
            {
                yaz_log(YLOG_WARN, "sort: diff: %s", arg);
                yaz_log(YLOG_WARN, " expected %s", expected_result);
                yaz_log(YLOG_WARN, " got      %s", wrbuf_cstr(w));
            }
        }
        else if (r)
        {
            yaz_log(YLOG_WARN, "sort: diff %s", arg);
            yaz_log(YLOG_WARN, " expected %s", expected_result);
            yaz_log(YLOG_WARN, " got error %d", r);
        }
        else if (r == 0)
        {
            yaz_log(YLOG_WARN, "sort: diff %s", arg);
            yaz_log(YLOG_WARN, " expected error");
            yaz_log(YLOG_WARN, " got %s", wrbuf_cstr(w));
        }
        wrbuf_destroy(w);
    }
    odr_destroy(odr);
    return ret;
}

static int type7(const char *arg, const char *expected_result)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_SortKeySpecList *sort_spec = yaz_sort_spec(odr, arg);
    int ret = 0;

    if (!sort_spec)
    {
        yaz_log(YLOG_WARN, "yaz_sort_spec : parse error: %s", arg);
    }
    else
    {
        WRBUF w = wrbuf_alloc();
        int r;

        wrbuf_puts(w, "q");
        r = yaz_sort_spec_to_type7(sort_spec, w);

        if (!expected_result && r)
            ret = 1;
        else if (expected_result && r == 0)
        {
            if (strcmp(wrbuf_cstr(w), expected_result) == 0)
                ret = 1;
            else
            {
                yaz_log(YLOG_WARN, "sort: diff: %s", arg);
                yaz_log(YLOG_WARN, " expected %s", expected_result);
                yaz_log(YLOG_WARN, " got      %s", wrbuf_cstr(w));
            }
        }
        else if (r)
        {
            yaz_log(YLOG_WARN, "sort: diff %s", arg);
            yaz_log(YLOG_WARN, " expected %s", expected_result);
            yaz_log(YLOG_WARN, " got error %d", r);
        }
        else if (r == 0)
        {
            yaz_log(YLOG_WARN, "sort: diff %s", arg);
            yaz_log(YLOG_WARN, " expected error");
            yaz_log(YLOG_WARN, " got %s", wrbuf_cstr(w));
        }
        wrbuf_destroy(w);
    }
    odr_destroy(odr);
    return ret;
}

static int strategy_sortkeys(const char *arg, const char *expected_result, int (*strategy) (Z_SortKeySpecList *, WRBUF))
{
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_SortKeySpecList *sort_spec = yaz_sort_spec(odr, arg);
    int ret = 0;

    if (!sort_spec)
    {
        yaz_log(YLOG_WARN, "yaz_sort_spec : parse error: %s", arg);
    }
    else
    {
        WRBUF w = wrbuf_alloc();
        int r = (strategy)(sort_spec, w);

        if (!expected_result && r)
            ret = 1;
        else if (expected_result && r == 0)
        {
            if (strcmp(wrbuf_cstr(w), expected_result) == 0)
                ret = 1;
            else
            {
                yaz_log(YLOG_WARN, "sort: diff: %s", arg);
                yaz_log(YLOG_WARN, " expected %s", expected_result);
                yaz_log(YLOG_WARN, " got      %s", wrbuf_cstr(w));
            }
        }
        else if (r)
        {
            yaz_log(YLOG_WARN, "sort: diff %s", arg);
            yaz_log(YLOG_WARN, " expected %s", expected_result);
            yaz_log(YLOG_WARN, " got error %d", r);
        }
        else if (r == 0)
        {
            yaz_log(YLOG_WARN, "sort: diff %s", arg);
            yaz_log(YLOG_WARN, " expected error");
            yaz_log(YLOG_WARN, " got %s", wrbuf_cstr(w));
        }
        wrbuf_destroy(w);
    }
    odr_destroy(odr);
    return ret;
}

static int srw_sortkeys(const char *arg, const char *expected_result) {
    return strategy_sortkeys(arg, expected_result, yaz_sort_spec_to_srw_sortkeys);
}

static int solr_sortkeys(const char *arg, const char *expected_result) {
    return strategy_sortkeys(arg, expected_result, yaz_sort_spec_to_solr_sortkeys);
}



static int check_sortkeys_to_sort_spec(const char *arg,
                                           const char *expected_result, int (*sort_strategy)(const char *, WRBUF))
{
    WRBUF w = wrbuf_alloc();
    int ret = 0;
    int r = sort_strategy(arg, w);

    if (!expected_result && r)
        ret = 1;
    else if (expected_result && r == 0)
    {
        if (strcmp(wrbuf_cstr(w), expected_result) == 0)
            ret = 1;
        else
        {
            yaz_log(YLOG_WARN, "sort: diff: %s", arg);
            yaz_log(YLOG_WARN, " expected %s", expected_result);
            yaz_log(YLOG_WARN, " got      %s", wrbuf_cstr(w));
        }
    }
    else if (r)
    {
        yaz_log(YLOG_WARN, "sort: diff %s", arg);
        yaz_log(YLOG_WARN, " expected %s", expected_result);
        yaz_log(YLOG_WARN, " got error %d", r);
    }
    else if (r == 0)
    {
        yaz_log(YLOG_WARN, "sort: diff %s", arg);
        yaz_log(YLOG_WARN, " expected error");
        yaz_log(YLOG_WARN, " got %s", wrbuf_cstr(w));
    }
    wrbuf_destroy(w);
    return ret;
}

static int check_srw_sortkeys_to_sort_spec(const char *arg, const char *expected_result) {
    return check_sortkeys_to_sort_spec(arg, expected_result, yaz_srw_sortkeys_to_sort_spec);
}

static int check_solr_sortkeys_to_sort_spec(const char *arg, const char *expected_result) {
    return check_sortkeys_to_sort_spec(arg, expected_result, yaz_solr_sortkeys_to_sort_spec);
}

static void tst(void)
{
    YAZ_CHECK(cql("title a",
                  " SORTBY title/ascending/ignoreCase"));
    YAZ_CHECK(cql("title a date ds",
                  " SORTBY title/ascending/ignoreCase"
                  " date/descending/respectCase"));
    YAZ_CHECK(cql("1=4,2=3 a", 0));
    YAZ_CHECK(cql("date a=1900",
                  " SORTBY date/ascending/ignoreCase/missingValue=1900"));

    YAZ_CHECK(type7("title a",
                  "@or q @attr 1=title @attr 7=1 0"));
    YAZ_CHECK(type7("title a date ds",
                    "@or @or q @attr 1=title @attr 7=1 0"
                    " @attr 1=date @attr 7=2 1"));
    YAZ_CHECK(type7("1=4,2=3 a",
                  "@or q @attr 1=4 @attr 2=3 @attr 7=1 0"));
    YAZ_CHECK(type7("date a=1900",
                  "@or q @attr 1=date @attr 7=1 0"));

    YAZ_CHECK(srw_sortkeys("title a",
                           "title,,1,0,highValue"));
    YAZ_CHECK(srw_sortkeys("title a date ds",
                           "title,,1,0,highValue "
                           "date,,0,1,highValue"));
    YAZ_CHECK(srw_sortkeys("1=4,2=3 a", 0));
    YAZ_CHECK(srw_sortkeys("date a=1900",
                           "date,,1,0,1900"));
    YAZ_CHECK(check_srw_sortkeys_to_sort_spec(
                  "date,,1,0,1900",
                  "date ai=1900"));

    YAZ_CHECK(solr_sortkeys("title a",
                           "title asc"));
    YAZ_CHECK(solr_sortkeys("title a date ds",
                           "title asc"
                            ",date desc"));
    YAZ_CHECK(solr_sortkeys("1=4,2=3 a", 0));

    YAZ_CHECK(check_solr_sortkeys_to_sort_spec(
                  "date asc",
                  "date ai"));

}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst();
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

