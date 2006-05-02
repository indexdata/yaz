/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tst_record_conv.c,v 1.1 2006-05-02 20:47:46 adam Exp $
 *
 */
#include <yaz/record_conv.h>
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include <string.h>

#if HAVE_XML2

#include <libxml/parser.h>
#include <libxml/tree.h>

yaz_record_conv_t conv_from_xml(const char *xmlstring, WRBUF w)
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

int conv_from_xml_compare(const char *xmlstring, const char *expect_error,
                          yaz_record_conv_t *pt)
{
    WRBUF w = wrbuf_alloc();
    int ret;

    yaz_record_conv_t p = conv_from_xml(xmlstring, w);

    if (!p)
    {
        if (expect_error && !strcmp(wrbuf_buf(w), expect_error))
            ret = 1;
        else
            ret = 0;
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

static void tst()
{
    YAZ_CHECK(conv_from_xml_compare("<bad", "xmlParseMemory", 0));
    YAZ_CHECK(conv_from_xml_compare("<bad/>", "Missing 'convert' element", 0));
    YAZ_CHECK(conv_from_xml_compare("<convert/>", 0, 0));
    YAZ_CHECK(conv_from_xml_compare("<convert><bad/></convert>",
                                    "Bad element 'bad'."
                                    "Expected xslt, marc_to_xml,...", 0));
    YAZ_CHECK(conv_from_xml_compare("<convert>"
                                    "<xslt stylesheet=\"x.xsl\"/>"
                                    "<marc_to_xml charset=\"marc-8\"/>"
                                    "</convert>",
                                    0, 0));
}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if HAVE_XML2
    tst();
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

