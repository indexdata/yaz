/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *
 *  $Id: nfaxmltest1.c,v 1.1 2006-07-05 08:12:48 heikki Exp $
 *
 */


#include <stdio.h>
#include <string.h>
#include <yaz/nfa.h>
#include <yaz/nmem.h>
#include <yaz/test.h>
#include <yaz/nfaxml.h>

#if HAVE_XML2
#include <libxml/parser.h>


void test1() {
    char *xmlstr="<ruleset> "
                 "<rule> "
                 "  <fromstring>foo</fromstring> "
                 "  <tostring>bar</tostring> "
                 "</rule>"
                 "</ruleset>";
    xmlDocPtr doc = xmlParseMemory(xmlstr, strlen(xmlstr));
    YAZ_CHECK(doc);
    if (!doc)
        return;
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    nmem_init ();

    test1();

    nmem_exit ();
    YAZ_CHECK_TERM;
}

#else
int main(int argc, char **argv) {
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_TERM;
}

#endif

/* 
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
