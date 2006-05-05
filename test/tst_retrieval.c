/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tst_retrieval.c,v 1.2 2006-05-05 18:37:08 adam Exp $
 *
 */
#include <yaz/retrieval.h>
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include <string.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_XSLT

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
    wrbuf_free(w, 1);
    return ret;
}

static void tst_configure()
{
    YAZ_CHECK(conv_configure_test("<bad", 
                                  "xmlParseMemory", 0));

    YAZ_CHECK(conv_configure_test("<bad/>", 
                                  "Missing 'retrievalinfo' element", 0));

    YAZ_CHECK(conv_configure_test("<retrievalinfo/>", 0, 0));

    YAZ_CHECK(conv_configure_test("<retrievalinfo><bad/></retrievalinfo>",
                                  "Bad element 'bad'."
                                  " Expected 'retrieval'", 0));

    YAZ_CHECK(conv_configure_test("<retrievalinfo>"
                                  "<retrieval>"
                                  "<convert>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "</convert>"
                                  "</retrieval>"
                                  "</retrievalinfo>",
                                  0, 0));

    YAZ_CHECK(conv_configure_test("<retrievalinfo>"
                                  "<retrieval>"
                                  "<convert>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "</convert>"
                                  "</retrieval>"
                                  "<retrieval>"
                                  "<convert>"
                                  "<xslt stylesheet=\"tst_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "</convert>"
                                  "</retrieval>"
                                  "</retrievalinfo>",
                                  0, 0));
}

#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if HAVE_XSLT
    tst_configure();
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

