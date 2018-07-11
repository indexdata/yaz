/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/retrieval.h>
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include <string.h>
#include <yaz/log.h>
#include <yaz/oid_db.h>

#if YAZ_HAVE_XSLT

#include <libxml/parser.h>
#include <libxml/tree.h>

yaz_retrieval_t conv_configure(const char *xmlstring, WRBUF w)
{
    xmlDocPtr doc = xmlParseMemory(xmlstring, strlen(xmlstring));
    if (!doc)
    {
        wrbuf_printf(w, "xmlParseMemory");
        return 0;
    }
    else
    {
        xmlNodePtr ptr = xmlDocGetRootElement(doc);
        yaz_retrieval_t p = yaz_retrieval_create();

        if (p)
        {
            const char *srcdir = getenv("srcdir");
            if (srcdir)
                yaz_retrieval_set_path(p, srcdir);
        }
        if (!ptr)
        {
            wrbuf_printf(w, "xmlDocGetRootElement");
            yaz_retrieval_destroy(p);
            p = 0;
        }
        else if (!p)
        {
            wrbuf_printf(w, "yaz_retrieval_create");
        }
        else
        {
            int r = yaz_retrieval_configure(p, ptr);

            if (r)
            {
                wrbuf_puts(w, yaz_retrieval_get_error(p));
                yaz_retrieval_destroy(p);
                p = 0;
            }
        }
        xmlFreeDoc(doc);
        return p;
    }
}

int conv_configure_test(const char *xmlstring, const char *expect_error,
                        yaz_retrieval_t *pt)
{
    WRBUF w = wrbuf_alloc();
    int ret;

    yaz_retrieval_t p = conv_configure(xmlstring, w);

    if (!p)
    {
        if (expect_error && !strcmp(wrbuf_cstr(w), expect_error))
            ret = 1;
        else
        {
            ret = 0;
            printf("%s\n", wrbuf_cstr(w));
        }
    }
    else
    {
        if (expect_error)
        {
            ret = 0;
            yaz_retrieval_destroy(p);
        }
        else
        {
            if (pt)
                *pt = p;
            else
                yaz_retrieval_destroy(p);
            ret = 1;
        }
    }
    wrbuf_destroy(w);
    return ret;
}

static void tst_configure(void)
{
    YAZ_CHECK(conv_configure_test(
                  "<bad",
                  "xmlParseMemory", 0));

    YAZ_CHECK(conv_configure_test(
                  "<bad/>",
                  "Expected element <retrievalinfo>", 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo/>", 0, 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo><bad/></retrievalinfo>",
                  "Element <retrievalinfo>:"
                  " expected element <retrieval>, got <bad>",
                  0));

    YAZ_CHECK(conv_configure_test("<retrievalinfo><retrieval/>"
                                  "</retrievalinfo>",
                                  "Missing 'syntax' attribute", 0));


    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " unknown=\"unknown\""
                  ">"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <retrieval>:  expected attributes "
                  "'syntax', identifier' or 'name', got "
                  "'unknown'", 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"unknown_synt\""
                  ">"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <retrieval>:  unknown attribute "
                  "value syntax='unknown_synt'", 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  "/>"
                  "</retrievalinfo>",
                  0, 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  " name=\"marcxml\"/>"
                  "</retrievalinfo>",
                  0, 0));


    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  " name=\"marcxml\""
                  " identifier=\"info:srw/schema/1/marcxml-v1.1\""
                  "/>"
                  "</retrievalinfo>",
                  0, 0));



    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  " identifier=\"info:srw/schema/1/marcxml-v1.1\""
                  " name=\"marcxml\">"
                  "<convert/>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <retrieval>: expected zero or one element "
                  "<backend>, got <convert>", 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  " identifier=\"info:srw/schema/1/marcxml-v1.1\""
                  " name=\"marcxml\">"
                  " <backend syntax=\"usmarc\""
                  " schema=\"marcxml\""
                  "/>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <backend>: expected attributes 'syntax' or 'name,"
                  " got 'schema'", 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  " identifier=\"info:srw/schema/1/marcxml-v1.1\""
                  " name=\"marcxml\">"
                  " <backend syntax=\"usmarc\""
                  " name=\"marcxml\""
                  "/>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  0, 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  " identifier=\"info:srw/schema/1/marcxml-v1.1\""
                  " name=\"marcxml\">"
                  " <backend syntax=\"unknown\""
                  "/>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <backend syntax='unknown'>: "
                  "attribute 'syntax' has invalid value "
                  "'unknown'", 0));


    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval"
                  " syntax=\"usmarc\""
                  " identifier=\"info:srw/schema/1/marcxml-v1.1\""
                  " name=\"marcxml\">"
                  " <backend syntax=\"usmarc\""
                  " unknown=\"silly\""
                  "/>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <backend>: expected attributes "
                  "'syntax' or 'name, got 'unknown'", 0));


    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval syntax=\"usmarc\">"
                  "<backend syntax=\"xml\" name=\"dc\">"
                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                  "<marc"
                  " inputcharset=\"utf-8\""
                  " outputcharset=\"non-existent\""
                  " inputformat=\"xml\""
                  " outputformat=\"marc\""
                  "/>"
                  "</backend>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <marc inputcharset='utf-8'"
                  " outputcharset='non-existent'>: Unsupported character"
                  " set mapping defined by attribute values", 0));

    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval syntax=\"usmarc\">"
                  "<backend syntax=\"xml\" name=\"dc\">"
                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                  "<marc"
                  " inputcharset=\"utf-8\""
                  " outputcharset=\"marc-8\""
                  " inputformat=\"not-existent\""
                  " outputformat=\"marc\""
                  "/>"
                  "</backend>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <marc inputformat='not-existent'>:  Unsupported"
                  " input format defined by attribute value", 0));


    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval syntax=\"usmarc\">"
                  "<backend syntax=\"xml\" name=\"dc\">"
                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                  "<marc"
                  " inputcharset=\"utf-8\""
                  " outputcharset=\"marc-8\""
                  " inputformat=\"xml\""
                  " outputformat=\"marc\""
                  "/>"
                  "</backend>"
                  "<backend/>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  "Element <retrieval>: "
                  "only one <backend> allowed", 0));


    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo>"
                  "<retrieval syntax=\"usmarc\">"
                  "<backend syntax=\"xml\" name=\"dc\">"
                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                  "<marc"
                  " inputcharset=\"utf-8\""
                  " outputcharset=\"marc-8\""
                  " inputformat=\"xml\""
                  " outputformat=\"marc\""
                  "/>"
                  "</backend>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  0, 0));

    yaz_retrieval_t p = 0;
    YAZ_CHECK(conv_configure_test(
                  "<retrievalinfo "
                  " xmlns=\"http://indexdata.com/yaz\" version=\"1.0\">"
                  "<retrieval syntax=\"grs-1\"/>"
                  "<retrieval syntax=\"usmarc\" name=\"F\"/>"
                  "<retrieval syntax=\"usmarc\" name=\"B\"/>"
                  "<retrieval syntax=\"xml\" name=\"marcxml\" "
                  "           identifier=\"info:srw/schema/1/marcxml-v1.1\">"
                  "  <backend syntax=\"usmarc\" name=\"F\">"
                  "    <marc inputformat=\"marc\" outputformat=\"marcxml\" "
                  "            inputcharset=\"marc-8\"/>"
                  "  </backend>"
                  "</retrieval>"
                  "<retrieval syntax=\"danmarc\" name=\"f1\" split=\":\">"
                  "  <backend syntax=\"usmarc\" name=\"F\">"
                  "    <marc inputformat=\"marc\" outputformat=\"marcxchange\" "
                  "          inputcharset=\"marc-8\"/>"
                  "  </backend>"
                  "</retrieval>"
                  "<retrieval syntax=\"xml\" name=\"dc\" "
                  "           identifier=\"info:srw/schema/1/dc-v1.1\">"
                  "  <backend syntax=\"usmarc\" name=\"F\">"
                  "    <marc inputformat=\"marc\" outputformat=\"marcxml\" "
                  "          inputcharset=\"marc-8\"/>"
                  "    <xslt stylesheet=\"test_record_conv.xsl\"/> "
                  "  </backend>"
                  "</retrieval>"
                  "</retrievalinfo>",
                  0, &p));

    const char *match_schema = 0;
    Odr_oid *match_syntax = 0;
    const char *backend_schema = 0;
    Odr_oid *backend_syntax = 0;
    int r;
    r = yaz_retrieval_request(p, "F", yaz_oid_recsyn_usmarc,
                              &match_schema, &match_syntax,
                              0, &backend_schema, &backend_syntax);
    YAZ_CHECK(r == 0);
    YAZ_CHECK(backend_schema && !strcmp(backend_schema, "F"));
    YAZ_CHECK(!oid_oidcmp(backend_syntax, yaz_oid_recsyn_usmarc));

    r = yaz_retrieval_request(p, 0, yaz_oid_recsyn_usmarc,
                              &match_schema, &match_syntax,
                              0, &backend_schema, &backend_syntax);
    YAZ_CHECK(r == 0);
    YAZ_CHECK(backend_schema == 0);
    YAZ_CHECK(!oid_oidcmp(backend_syntax, yaz_oid_recsyn_usmarc));

    r = yaz_retrieval_request(p, 0, yaz_oid_recsyn_usmarc,
                              &match_schema, &match_syntax,
                              0, &backend_schema, &backend_syntax);
    YAZ_CHECK(r == 0);
    YAZ_CHECK(backend_schema == 0);
    YAZ_CHECK(!oid_oidcmp(backend_syntax, yaz_oid_recsyn_usmarc));

    r = yaz_retrieval_request(p, 0, yaz_oid_recsyn_xml,
                              &match_schema, &match_syntax,
                              0, &backend_schema, &backend_syntax);
    YAZ_CHECK(r == 0);
    YAZ_CHECK(backend_schema && !strcmp(backend_schema, "F"));
    YAZ_CHECK(!oid_oidcmp(backend_syntax, yaz_oid_recsyn_usmarc));

    r = yaz_retrieval_request(p, "f1:9988", yaz_oid_recsyn_danmarc,
                              &match_schema, &match_syntax,
                              0, &backend_schema, &backend_syntax);
    YAZ_CHECK(r == 0);
    YAZ_CHECK(backend_schema && !strcmp(backend_schema, "F:9988"));
    YAZ_CHECK(!oid_oidcmp(backend_syntax, yaz_oid_recsyn_usmarc));

    r = yaz_retrieval_request(p, 0, yaz_oid_recsyn_danmarc,
                              &match_schema, &match_syntax,
                              0, &backend_schema, &backend_syntax);
    YAZ_CHECK(r == 0);
    YAZ_CHECK(backend_schema && !strcmp(backend_schema, "F"));
    YAZ_CHECK(!oid_oidcmp(backend_syntax, yaz_oid_recsyn_usmarc));

    yaz_retrieval_destroy(p);
}

#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);

    yaz_log_xml_errors(0, 0 /* disable it */);

#if YAZ_HAVE_XSLT
    tst_configure();
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

