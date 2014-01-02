/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <yaz/test.h>
#include <yaz/srw.h>
#include <yaz/soap.h>

#if YAZ_HAVE_XML2
#include <libxml/parser.h>

static void tst_srw(void)
{
    const char *charset = 0;
    char *content_buf = 0;
    int content_len;
    int ret;
    ODR o = odr_createmem(ODR_ENCODE);
    Z_SOAP_Handler h[2] = {
        {"http://www.loc.gov/zing/srw/", 0, (Z_SOAP_fun) yaz_srw_codec},
        {0, 0, 0}
    };
    Z_SRW_PDU *sr = yaz_srw_get(o, Z_SRW_searchRetrieve_request);
    Z_SOAP *p = (Z_SOAP *) odr_malloc(o, sizeof(*p));

    YAZ_CHECK(o);
    YAZ_CHECK(sr);
    YAZ_CHECK(p);
#if 0
    sr->u.request->query = "jordb" "\xe6" "r";
#else
    sr->u.request->query = "jordbaer";
#endif

    p->which = Z_SOAP_generic;
    p->u.generic = (Z_SOAP_Generic *) odr_malloc(o, sizeof(*p->u.generic));
    p->u.generic->no = 0;
    p->u.generic->ns = 0;
    p->u.generic->p = sr;
    p->ns = "http://schemas.xmlsoap.org/soap/envelope/";

    ret = z_soap_codec_enc(o, &p, &content_buf, &content_len, h, charset);
    odr_destroy(o);
    YAZ_CHECK(ret == 0);  /* codec failed ? */
}
#endif

static void tst_array_to_uri(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    char **names;
    char **values;
    char *query_string;
    int r;

    r = yaz_uri_to_array("&", odr, &names, &values);
    YAZ_CHECK_EQ(r, 0);
    if (r == 0)
    {
        YAZ_CHECK(names[0] == 0);
        YAZ_CHECK(values[0] == 0);
    }
    r = yaz_uri_to_array("&&", odr, &names, &values);
    YAZ_CHECK_EQ(r, 0);
    if (r == 0)
    {
        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, ""));

        YAZ_CHECK(names[0] == 0);
        YAZ_CHECK(values[0] == 0);
    }
    r = yaz_uri_to_array("a=AA&bb=%42B", odr, &names, &values);
    YAZ_CHECK_EQ(r, 2);
    if (r == 2)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], "BB"));

        YAZ_CHECK(names[2] == 0);
        YAZ_CHECK(values[2] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=BB"));
    }
    r = yaz_uri_to_array("a=AA&bb&x", odr, &names, &values);
    YAZ_CHECK_EQ(r, 3);
    if (r == 3)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], ""));

        YAZ_CHECK(names[2] && !strcmp(names[2], "x"));
        YAZ_CHECK(values[2] && !strcmp(values[1], ""));

        YAZ_CHECK(names[3] == 0);
        YAZ_CHECK(values[3] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=&x="));
    }

    r = yaz_uri_to_array("a=AA&bb=&x=", odr, &names, &values);
    YAZ_CHECK_EQ(r, 3);
    if (r == 3)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], ""));

        YAZ_CHECK(names[2] && !strcmp(names[2], "x"));
        YAZ_CHECK(values[2] && !strcmp(values[1], ""));

        YAZ_CHECK(names[3] == 0);
        YAZ_CHECK(values[3] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=&x="));
    }

    r = yaz_uri_to_array("a=AA&&&bb&x&&", odr, &names, &values);
    YAZ_CHECK_EQ(r, 3);
    if (r == 3)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], ""));

        YAZ_CHECK(names[2] && !strcmp(names[2], "x"));
        YAZ_CHECK(values[2] && !strcmp(values[2], ""));

        YAZ_CHECK(names[3] == 0);
        YAZ_CHECK(values[3] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=&x="));
    }

    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if YAZ_HAVE_XML2
    LIBXML_TEST_VERSION;
    tst_srw();
#endif
    tst_array_to_uri();
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

