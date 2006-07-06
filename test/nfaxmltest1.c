/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *
 *  $Id: nfaxmltest1.c,v 1.3 2006-07-06 13:10:31 heikki Exp $
 *
 */


#include <stdio.h>
#include <string.h>
#include <yaz/nfa.h>
#include <yaz/nmem.h>
#include <yaz/test.h>
#include <yaz/nfaxml.h>

#if YAZ_HAVE_XML2
#include <libxml/parser.h>


/** \brief  Test parsing of a minimal, valid xml string */
void test1() {
    char *xmlstr="<ruleset> "
                 "<rule> "
                 "  <fromstring>foo</fromstring> "
                 "  <tostring>bar</tostring> "
                 "</rule>"
                 "</ruleset>";
    yaz_nfa *nfa=yaz_nfa_parse_xml_memory(xmlstr);
    YAZ_CHECK(nfa);
}



/** \brief  Test parsing of a minimal, invalid xml string */
void test2() {
    char *xmlstr="<ruleset> "
                 "<rule> "
                 "  <fromstring>foo</fromstring> "
                 "  <tostring>bar</tostring> "
                 "</rule>";
                 /* missing "</ruleset>" */
    yaz_log(YLOG_LOG,"Parsing bad xml, expecting errors:");
    yaz_nfa *nfa=yaz_nfa_parse_xml_memory(xmlstr);
    YAZ_CHECK(!nfa);
}


int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    nmem_init ();

    test1();
    test2();

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
