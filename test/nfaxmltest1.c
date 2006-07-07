/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *
 *  $Id: nfaxmltest1.c,v 1.6 2006-07-07 13:39:05 heikki Exp $
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
    char *xmlstr = "<ruleset> "
                   "<rule> "
                   "  <fromstring>foo</fromstring> "
                   "  <tostring>bar</tostring> "
                   "</rule>"
                   "</ruleset>";
    yaz_nfa *nfa = yaz_nfa_parse_xml_memory(xmlstr);
    YAZ_CHECK_TODO(nfa);
}


/** \brief  Test parsing of a minimal, invalid xml string */
void test2() {
    yaz_nfa *nfa;
    char *xmlstr = "<ruleset> "
                   "<rule> "
                   "  <fromstring>foo</fromstring> "
                   "  <tostring>bar</tostring> "
                   "</rule>";
                 /* missing "</ruleset>" */
    yaz_log(YLOG_LOG,"Parsing bad xml, expecting errors:");
    nfa = yaz_nfa_parse_xml_memory(xmlstr);
    YAZ_CHECK(!nfa);
}

/** \brief  Test parsing a few minimal xml files */
void test3() {
    char *goodfilenames[] = {
             "nfaxml-simple.xml",
             "nfaxml-main.xml", /* x-includes nfaxml-include */
              0};
    char *badfilenames[] = {
             "nfaxml-missing.xml",  /* file not there at all */
             "nfaxml-badinclude.xml",  /* bad xinclude in it */
              0};
    yaz_nfa *nfa;
    char **f = goodfilenames;
    do {
        yaz_log(YLOG_LOG,"Parsing (good) xml file '%s'", *f);
        nfa=yaz_nfa_parse_xml_file(*f);
        YAZ_CHECK_TODO(nfa);
    } while (*++f);

    f = badfilenames;
    do {
        yaz_log(YLOG_LOG,"Parsing bad xml file '%s'. Expecting errors", *f);
        nfa = yaz_nfa_parse_xml_file(*f);
        YAZ_CHECK(!nfa);
    } while (*++f);
}


int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    nmem_init ();

    test1();
    test2();
    test3();

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
