/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstsoap1.c,v 1.4 2005-08-22 20:34:23 adam Exp $
 */

#include <stdlib.h>
#if HAVE_XML2
#include <libxml/parser.h>
#endif

int main(int argc, char **argv)
{
#if HAVE_XML2
    LIBXML_TEST_VERSION;

    if (argc <= 1)
    {
        xmlChar *buf_out;
        int len_out;
        xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
#if 0
        const char *val = "jordbær"; /* makes xmlDocDumpMemory hang .. */
#else
        const char *val = "jordbaer"; /* OK */
#endif
        xmlNodePtr top = xmlNewNode(0, BAD_CAST "top");
        
        xmlNewTextChild(top, 0, BAD_CAST "sub", BAD_CAST val);
        xmlDocSetRootElement(doc, top);
        
        xmlDocDumpMemory(doc, &buf_out, &len_out);
#if 0
        printf("%*s", len_out, buf_out);
#endif
    }
#endif
    return 0;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

