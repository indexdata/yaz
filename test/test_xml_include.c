/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <yaz/xml_include.h>
#include <yaz/test.h>

static void tst_xml_include(void)
{
    const char *srcdir = getenv("srcdir");
    xmlDocPtr doc;
    xmlNodePtr node;
    const char *xml_in = "<x><include src=\"test_xml_inc*.xml\"/></x>";

    if (srcdir == 0)
        srcdir = ".";

    doc = xmlParseMemory(xml_in, strlen(xml_in));
    YAZ_CHECK(doc);
    if (!doc)
        return;
    node = xmlDocGetRootElement(doc);
    YAZ_CHECK(node);
    if (node)
    {
        const char *expect =
            "<?xml version=\"1.0\"?>\n"
            "<x><!-- begin include src=\"test_xml_inc*.xml\" -->"
            "<y>some</y>"
            "<!-- end include src=\"test_xml_inc*.xml\" --></x>\n";

        xmlChar *xml_out;
        int len_out;
        int ret = yaz_xml_include_simple(node, srcdir);
        YAZ_CHECK(ret == 0);
        xmlDocDumpMemory(doc, &xml_out, &len_out);
        YAZ_CHECK(xml_out && len_out > 0);
        if (xml_out && len_out > 0)
        {
            YAZ_CHECK(strlen(expect) == len_out);
            if (strlen(expect) == len_out)
            {
                YAZ_CHECK(memcmp(expect, xml_out, len_out) == 0);
            }
            else
            {
                fwrite(xml_out, 1, len_out, stdout);
                fflush(stdout);
            }
            xmlFree(xml_out);
        }
    }
    xmlFreeDoc(doc);
}


int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_xml_include();
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

