/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstsoap1.c,v 1.1 2005-05-06 11:11:37 adam Exp $
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
	xmlDocPtr doc = xmlNewDoc("1.0");
#if 0
	const char *val = "jordbær"; /* makes xmlDocDumpMemory hang .. */
#else
	const char *val = "jordbaer"; /* OK */
#endif
	xmlNodePtr top = xmlNewNode(0, "top");
	
	xmlNewTextChild(top, 0, "sub", val);
	xmlDocSetRootElement(doc, top);
	
	xmlDocDumpMemory(doc, &buf_out, &len_out);
	printf("%*s", len_out, buf_out);
    }
#endif
    return 0;
}


