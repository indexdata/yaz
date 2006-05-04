/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tst_record_conv.c,v 1.3 2006-05-04 18:22:59 adam Exp $
 *
 */
#include <yaz/record_conv.h>
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include <string.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_XSLT

#include <libxml/parser.h>
#include <libxml/tree.h>

yaz_record_conv_t conv_configure(const char *xmlstring, WRBUF w)
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
        yaz_record_conv_t p = yaz_record_conv_create();

        if (p)
        {
            const char *srcdir = getenv("srcdir");
            if (srcdir)
                yaz_record_conv_set_path(p, srcdir);
        }
        if (!ptr)
        {
            wrbuf_printf(w, "xmlDocGetRootElement");
            yaz_record_conv_destroy(p);
            p = 0;
        }
        else if (!p)
        {
            wrbuf_printf(w, "yaz_record_conv_create");
        }
        else
        {
            int r = yaz_record_conv_configure(p, ptr);
            
            if (r)
            {
                wrbuf_puts(w, yaz_record_conv_get_error(p));
                yaz_record_conv_destroy(p);
                p = 0;
            }
        }
        xmlFreeDoc(doc);
        return p;
    }    
}

int conv_configure_test(const char *xmlstring, const char *expect_error,
                        yaz_record_conv_t *pt)
{
    WRBUF w = wrbuf_alloc();
    int ret;

    yaz_record_conv_t p = conv_configure(xmlstring, w);

    if (!p)
    {
        if (expect_error && !strcmp(wrbuf_buf(w), expect_error))
            ret = 1;
        else
        {
            ret = 0;
            printf("%.*s\n", wrbuf_len(w), wrbuf_buf(w));
        }
    }
    else
    {
        if (expect_error)
        {
            ret = 0;
            yaz_record_conv_destroy(p);
        }
        else
        {
            if (pt)
                *pt = p;
            else
                yaz_record_conv_destroy(p);
            ret = 1;
        }
    }
    wrbuf_free(w, 1);
    return ret;
}

static void tst_configure()
{
    YAZ_CHECK(conv_configure_test("<bad", "xmlParseMemory", 0));
    YAZ_CHECK(conv_configure_test("<bad/>", "Missing 'convert' element", 0));
    YAZ_CHECK(conv_configure_test("<convert/>", 0, 0));
    YAZ_CHECK(conv_configure_test("<convert><bad/></convert>",
                                  "Bad element 'bad'."
                                  "Expected marc, xslt, ..", 0));
    YAZ_CHECK(conv_configure_test("<convert>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"marc-8\""
                                  " outputcharset=\"marc-8\""
                                  "/>"
                                  "</convert>",
                                  "Attribute 'inputformat' required", 0));
    YAZ_CHECK(conv_configure_test("<convert>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "</convert>",
                                  0, 0));
}

static int conv_convert_test(yaz_record_conv_t p,
                             const char *input_record,
                             const char *output_expect_record)
{
    int ret = 0;
    if (!p)
    {
        YAZ_CHECK(ret);
    }
    else
    {
        WRBUF output_record = wrbuf_alloc();
        int r = yaz_record_conv_record(p, input_record, output_record);
        if (r)
        {
            if (output_expect_record)
            {
                printf("yaz_record_conv error=%s\n",
                       yaz_record_conv_get_error(p));
                ret = 0;
            }
            else
                ret = 1;
        }
        else
        {
            if (!output_expect_record)
            {
                ret = 0;
            }
            else if (strlen(output_expect_record) != wrbuf_len(output_record))
            {
                int expect_len = strlen(output_expect_record);
                ret = 0;
                printf("output_record expect-len=%d got-len=%d\n", expect_len,
                       wrbuf_len(output_record));
                printf("got-output_record = %.*s\n",
                       wrbuf_len(output_record), wrbuf_buf(output_record));
                printf("output_expect_record = %s\n",
                       output_expect_record);
            }
            else if (memcmp(output_expect_record, wrbuf_buf(output_record),
                            strlen(output_expect_record)))
            {
                ret = 0;
                printf("got-output_record = %.*s\n",
                       wrbuf_len(output_record), wrbuf_buf(output_record));
                printf("output_expect_record = %s\n",
                       output_expect_record);
            }
            else
            {
                ret = 1;
            }
        }
        wrbuf_free(output_record, 1);
    }
    return ret;
}

static void tst_convert()
{
    yaz_record_conv_t p = 0;
    const char *marcxml_rec =
        "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
        "  <leader>00080nam a22000498a 4500</leader>\n"
        "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
        "  <datafield tag=\"010\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"a\">   11224466 </subfield>\n"
        "  </datafield>\n"
        "</record>\n";
    const char *iso2709_rec =
        "\x30\x30\x30\x38\x30\x6E\x61\x6D\x20\x61\x32\x32\x30\x30\x30\x34"
        "\x39\x38\x61\x20\x34\x35\x30\x30\x30\x30\x31\x30\x30\x31\x33\x30"
        "\x30\x30\x30\x30\x30\x31\x30\x30\x30\x31\x37\x30\x30\x30\x31\x33"
        "\x1E\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20\x1E\x20\x20"
        "\x1F\x61\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20\x1E\x1D";

    YAZ_CHECK(conv_configure_test("<convert>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "</convert>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, marcxml_rec, iso2709_rec));

    YAZ_CHECK(conv_configure_test("<convert>"
                                  "<marc"
                                  " outputcharset=\"utf-8\""
                                  " inputcharset=\"marc-8\""
                                  " outputformat=\"marcxml\""
                                  " inputformat=\"marc\""
                                  "/>"
                                  "</convert>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, iso2709_rec, marcxml_rec));


    YAZ_CHECK(conv_configure_test("<convert>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "<marc"
                                  " outputcharset=\"utf-8\""
                                  " inputcharset=\"marc-8\""
                                  " outputformat=\"marcxml\""
                                  " inputformat=\"marc\""
                                  "/>"
                                  "</convert>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, marcxml_rec, marcxml_rec));



    YAZ_CHECK(conv_configure_test("<convert>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<marc"
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "<marc"
                                  " inputcharset=\"marc-8\""
                                  " outputformat=\"marcxml\""
                                  " inputformat=\"marc\""
                                  "/>"
                                  "</convert>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, marcxml_rec, marcxml_rec));
}

#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if HAVE_XML2
    tst_configure();
    tst_convert();
#endif
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

