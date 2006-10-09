/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *
 *  $Id: nfaxmltest1.c,v 1.9 2006-10-09 14:22:44 heikki Exp $
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
void test1(void) {
    char *xmlstr = "<ruleset> "
                   "<rule> "
                   "  <fromstring>foo</fromstring> "
                   "  <tostring>bar</tostring> "
                   "</rule>"
                   "</ruleset>";
    yaz_nfa *nfa = yaz_nfa_parse_xml_memory(xmlstr,"test1");
    YAZ_CHECK(nfa);
    yaz_nfa_destroy(nfa);
}


/** \brief  Test parsing of a minimal, invalid xml string */
void test2(void) {
    yaz_nfa *nfa;
    char *xmlstr = "<ruleset> "
                   "<rule> "
                   "  <fromstring>foo</fromstring> "
                   "  <tostring>bar</tostring> "
                   "</rule>";
                 /* missing "</ruleset>" */
    yaz_log(YLOG_LOG,"Parsing bad xml, expecting errors:");
    nfa = yaz_nfa_parse_xml_memory(xmlstr,"test2");
    YAZ_CHECK(!nfa);
    nfa = yaz_nfa_parse_xml_memory(0,"test2-null");
    YAZ_CHECK(!nfa);
}

/** \brief  Test parsing a few minimal xml files */
void test3(void) {
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
        YAZ_CHECK_TODO(nfa);  /* fails on make distcheck, can't find the files*/
    } while (*++f);

    f = badfilenames;
    do {
        yaz_log(YLOG_LOG,"Parsing bad xml file '%s'. Expecting errors", *f);
        nfa = yaz_nfa_parse_xml_file(*f);
        YAZ_CHECK(!nfa);
    } while (*++f);
}

/** \brief  Test parsing of a few minimal xml strings, with logical errors */
void test4(void) {
    yaz_nfa *nfa;
    char *xmls[] = { 
      /*a*/"<missingruleset>   <foo/>   </missingruleset>",
      /*b*/"<ruleset> <missingrule/> </ruleset>",
      /*c*/"<ruleset> <rule> <garbagerule/> </rule> </ruleset>",
      /*d*/"<ruleset><rule>"
              "<fromstring>MissingTo</fromstring>"
           "</rule></ruleset>",
      /*e*/"<ruleset><rule>"
              "<fromstring>DuplicateFrom</fromstring>"
              "<fromstring>Another Fromstring</fromstring>"
           "</rule></ruleset>",
      /*f*/"<ruleset><rule>"
              "<tostring>MissingFrom</tostring>"
           "</rule></ruleset>",
      /*g*/"<ruleset><rule>"
              "<tostring>DuplicateTo</tostring>"
              "<tostring>AnotherTo</tostring>"
           "</rule></ruleset>",
      /*h*/"<ruleset><rule>"
              "<fromstring>GoodUTF:Ã¦Ã¸Ã¥</fromstring>"
           "</rule></ruleset>",
      /*i*/"<ruleset><rule>"
              "<tostring>BadUtf8:Ø</tostring>"
           "</rule></ruleset>",
      /*j*/"<ruleset>"
             "<rule>"
               "<fromstring>ConflictingRules</fromstring>"
               "<tostring>IdenticalStrings</tostring>"
             "</rule>"
             "<rule>"
               "<fromstring>ConflictingRules</fromstring>"
               "<tostring>IdenticalStrings</tostring>"
             "</rule>"
           "</ruleset>",
      /*k*/"", /* empty string! */
      /*l*/"<ruleset>"
             "<rule>"
               "<fromrange>A-Z</fromrange>"
               "<torange>a-x</torange>"
             "</rule>"
           "</ruleset>",
              0 };
    char **xmlp=xmls;    
    char label[]= { 'a', 0 };
    while ( *xmlp ) {
        yaz_log(YLOG_LOG,"test4-%s: Parsing bad xml, expecting errors:",
                label);
        nfa = yaz_nfa_parse_xml_memory(*xmlp,label);
        YAZ_CHECK(!nfa);
        xmlp++;
        label[0]++; 
    }
} /* test4 */

static void test5(void) {
    struct conv_test {
        char *name;
        int expresult;
        char *xml;
        char *from;
        char *to;
    };
    struct conv_test tests[]= {
        { "test5-1",  YAZ_NFA_SUCCESS,
             "<ruleset>"
               "<rule>"
                 "<fromstring>foo</fromstring>"
                 "<tostring>bar</tostring>"
               "</rule>"
             "</ruleset>",
             "this is a foo test ofoofo fo foofoo fofoofooofoooo ",
             "this is a bar test obarfo fo barbar fobarbarobaroo "
        },
        { "test5-2",  YAZ_NFA_SUCCESS,
             "<ruleset>"
               "<rule>"
                 "<fromstring>ooooo</fromstring>"
                 "<tostring>five </tostring>"
               "</rule>"
               "<rule>"
                 "<fromstring>oooo</fromstring>"
                 "<tostring>four </tostring>"
               "</rule>"
               "<rule>"
                 "<fromstring>ooo</fromstring>"
                 "<tostring>three </tostring>"
               "</rule>"
               "<rule>"
                 "<fromstring>oo</fromstring>"
                 "<tostring>two </tostring>"
               "</rule>"
             "</ruleset>",
             "oo-oooo-",
             "two -four -"
        },
        { "test5-4",  YAZ_NFA_SUCCESS, 0, /* same xml */
             "oo-oooo-ooooooo-",
             "two -four -five two -"
        },
        { "test5-3",  YAZ_NFA_OVERRUN, 0, /* could match further oo's */
             "oo-oooo-ooooooo",
             "two -four -five "  
        },
        { "test5-4 (lowercase)",  YAZ_NFA_SUCCESS,
             "<ruleset>"
               "<rule>"
                 "<fromrange>A-Z</fromrange>"
                 "<torange>a-z</torange>"
               "</rule>"
             "</ruleset>",
             "LowerCase TEST with A-Z and a-z",
             "lowercase test with a-z and a-z"
        },
        { "test5-5 (lowercase entities)",  YAZ_NFA_SUCCESS,
             "<ruleset>"
               "<rule>"
                 "<fromrange>&#x41;-Z</fromrange>"
                 "<torange>&#97;-&#x7A;</torange>"
               "</rule>"
             "</ruleset>",
             "LowerCase TEST with A-Z and a-z (and &#41; &#5A; )",
             "lowercase test with a-z and a-z (and &#41; &#5a; )"
        },
        { "test5-6 (danish lowercase)",  YAZ_NFA_SUCCESS,
             "<ruleset>"
               "<rule>"
                 "<fromrange>A-Z</fromrange>"
                 "<torange>a-z</torange>"
               "</rule>"
               "<rule>"
                 "<fromrange>&#xC0;-&#xD6;</fromrange>"
                 "<torange>&#xE0;-&#xF6;</torange>"
               "</rule>"
               "<rule>"
                 "<fromrange>&#xD8;-&#xDF;</fromrange>"
                 "<torange>&#xF8;-&#xFF;</torange>"
               "</rule>"
               "<rule>"
                 "<fromstring>&#xC5;</fromstring>"
                 "<tostring>&#xE5;</tostring>"
               "</rule>"
               "<rule>"
                 "<fromstring>D&#xe4;nish</fromstring>"
                 "<tostring>D&#xc4;NISH</tostring>"
               "</rule>"
             "</ruleset>",
             "LowerCase TEST with Dänish Å !? åæø ÅÆØ XYZ",
             "lowercase test with DÄNISH å !? åæø åæø xyz"
        },
        {0,0,0,0}
    };
    char *xml=0;
#define MAXBUF 2048    
    yaz_nfa *nfa;
    yaz_nfa_char frombuf[MAXBUF];
    yaz_nfa_char tobuf[MAXBUF];
    char charbuf[MAXBUF];
    struct conv_test *thistest=tests;
    char *cp;
    yaz_nfa_char *ycp;
    size_t incharsleft;
    size_t outcharsleft;
    size_t prev_incharsleft;
    int rc;
    yaz_nfa_char *fromp;
    yaz_nfa_char *top;
    while (thistest->name) {
        yaz_log(YLOG_DEBUG,"Starting test %s",thistest->name);
        if (thistest->xml)
            xml=thistest->xml;
        nfa = yaz_nfa_parse_xml_memory(xml, thistest->name);
        YAZ_CHECK(nfa);
        if (nfa) {
            if ( yaz_test_get_verbosity() > 3) {
                yaz_nfa_dump(0,nfa,yaz_nfa_dump_converter);
            }
            ycp=frombuf;
            cp=thistest->from;
            while ( (*ycp++ = (unsigned char)*cp++) )
                ; /* strcpy, but expand to yaz_nfa_chars */
            incharsleft = strlen(thistest->from);
            prev_incharsleft = 0;
            outcharsleft = MAXBUF-1;
            fromp = frombuf;
            top = tobuf;
            rc = YAZ_NFA_SUCCESS;
            while ( (rc == YAZ_NFA_SUCCESS) && (incharsleft>0) && 
                    (prev_incharsleft != incharsleft ) )  /* prevent loops */
            {
                prev_incharsleft=incharsleft;
                rc=yaz_nfa_convert_slice(nfa, &fromp, &incharsleft,
                        &top, &outcharsleft);
            }
            YAZ_CHECK_EQ(rc, thistest->expresult);
            if ( (rc == thistest->expresult) &&
                 (rc == YAZ_NFA_SUCCESS)) {
                YAZ_CHECK_EQ(incharsleft, 0);
                YAZ_CHECK( prev_incharsleft != incharsleft ); 
            }
            ycp=tobuf;
            cp=charbuf;
            while (ycp != top )
                *cp++ = *ycp++;
            *cp=0;
            if ( yaz_test_get_verbosity() > 2) {
                printf("%s from:   '%s' \n",thistest->name, thistest->from);
                printf("%s result: '%s' \n",thistest->name, charbuf);
                printf("%s expect: '%s' \n",thistest->name, thistest->to);
            }
            YAZ_CHECK( 0==strcmp(thistest->to,charbuf) );
            yaz_nfa_destroy(nfa);
        }
        thistest++;
    }
    
} /* test5 */


/* More things to test:
 *
 *   - Empty strings in to/from
 *   - ranges, length mismatches, etc
 */

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    nmem_init ();

    test1();
    test2();
    test3();
    test4();
    test5();

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
