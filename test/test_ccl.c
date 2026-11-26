/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/ccl_xml.h>
#include <yaz/log.h>
#include <yaz/test.h>
#include <yaz/zoom.h>

#include "xml_util.h"
#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#endif

static int tst_ccl_query(CCL_bibset bibset,
                         const char *query,
                         const char *result)
{
    CCL_parser parser = ccl_parser_create(bibset);
    int ret = 0;

    if (parser && bibset)
    {
        struct ccl_rpn_node *rpn = ccl_parser_find_str(parser, query);
        if (rpn)
        {
            /* parse ok. check that result is there and match */
            WRBUF wrbuf = wrbuf_alloc();
            ccl_pquery(wrbuf, rpn);

            /* check expect a result and that it matches */
            if (result && !strcmp(wrbuf_cstr(wrbuf), result))
                ret = 1;
            else
            {
                yaz_log(YLOG_WARN, "%s: result does not match", query);
                yaz_log(YLOG_WARN, " expected %s", result);
                yaz_log(YLOG_WARN, " got      %s", wrbuf_cstr(wrbuf));
                ret = 0;
            }
            ccl_rpn_delete(rpn);
            wrbuf_destroy(wrbuf);
        }
        else
        {
            if (result)
            {
                yaz_log(YLOG_WARN, "%s: parse failed", query);
                ret = 0;
            }
            else
                ret = 1;
        }
    }
    ccl_parser_destroy (parser);
    return ret;
}

void tst1(int pass)
{
    CCL_bibset bibset = ccl_qual_mk();
    char tstline[128];

    YAZ_CHECK(bibset);
    if (!bibset)
        return;

    if (pass == 1)
    {
        CCL_bibset nbibset = ccl_qual_dup(bibset);
        ccl_qual_rm(&bibset);
        bibset = nbibset;
    }

    switch(pass)
    {
    case 0:
        ccl_qual_fitem(bibset, "term dc.title", "comb");
        ccl_qual_fitem(bibset, "u=4    s=pw t=l,r", "ti");
        ccl_qual_fitem(bibset, "1=1016 s=al,pw t=r",    "term");
        ccl_qual_fitem(bibset, "t=x", "reg");
        ccl_qual_fitem(bibset, "t=z", "z");
        ccl_qual_fitem(bibset, "1=/my/title",       "dc.title");
        ccl_qual_fitem(bibset, "r=r,omiteq",        "date");
        ccl_qual_fitem(bibset, "r=o",         "x");
        ccl_qual_fitem(bibset, "dc.title", "title");
        ccl_qual_fitem(bibset, "s=ag", "ag");
        ccl_qual_fitem(bibset, "s=sl u=2", "splitlist");
        ccl_qual_fitem(bibset, "s=sl u=2 u=3", "s2");
        break;
    case 1:
        strcpy(tstline, "ti u=4    s=pw t=l,r");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "term 1=1016 s=al,pw t=r  # default term");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "reg t=x");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "z t=z");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "dc.title 1=/my/title");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "date r=r,omiteq # ordered relation");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "x r=o # ordered relation");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "title dc.title # alias");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "comb term dc.title # combination");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "ag s=ag");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "splitlist s=sl u=2");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "s2 s=sl u=2 u=3");
        ccl_qual_line(bibset, tstline);
        break;
    case 2:
        ccl_qual_buf(bibset, "ti u=4    s=pw t=l,r\n"
                     "term 1=1016 s=al,pw t=r\r\n"
                     "\n"
                     "reg t=x\r\n"
                     "z t=z\r\n"
                     "dc.title 1=/my/title\n"
                     "date r=r,omiteq\n"
                     "x r=o\n"
                     "title dc.title\n"
                     "comb term dc.title\n"
                     "ag s=ag\n"
                     "splitlist s=sl u=2\n"
                     "s2 s=sl u=2 u=3\n"
            );
        break;
    case 3:
#if YAZ_HAVE_XML2
        if (1)
        {
            const char *addinfo = 0;
            const char *xml_str =
                "<cclmap>\n"
                " <qual name=\"ti\">\n"
                "   <attr type=\"u\" value=\"4\"/>\n"
                "   <attr type=\"s\" value=\"pw\"/>\n"
                "   <attr type=\"t\" value=\"l,r\"/>\n"
                " </qual>\n"
                " <qual name=\"term\">\n"
                "   <attr type=\"1\" value=\"1016\"/>\n"
                "   <attr type=\"s\" value=\"al,pw\"/>\n"
                "   <attr type=\"t\" value=\"r\"/>\n"
                " </qual>\n"
                " <qual name=\"reg\">\n"
                "   <attr type=\"t\" value=\"x\"/>\n"
                " </qual>\n"
                " <qual name=\"z\">\n"
                "   <attr type=\"t\" value=\"z\"/>\n"
                " </qual>\n"
                " <qual name=\"dc.title\">\n"
                "   <attr type=\"1\" value=\"/my/title\"/>\n"
                " </qual>\n"
                " <qual name=\"date\">\n"
                "   <attr type=\"r\" value=\"r,omiteq\"/>\n"
                " </qual>\n"
                " <qual name=\"x\">\n"
                "   <attr type=\"r\" value=\"o\"/>\n"
                " </qual>\n"
                " <qual name=\"title\">\n"
                "   <qual name=\"dc.title\"/>\n"
                " </qual>\n"
                " <qual name=\"comb\">\n"
                "   <qual name=\"term\"/>\n"
                "   <qual name=\"dc.title\"/>\n"
                " </qual>\n"
                " <qual name=\"ag\">\n"
                "   <attr type=\"s\" value=\"ag\"/>\n"
                " </qual>\n"
                " <qual name=\"splitlist\">\n"
                "   <attr type=\"s\" value=\"sl\"/>\n"
                "   <attr type=\"u\" value=\"2\"/>\n"
                " </qual>\n"
                " <qual name=\"s2\">\n"
                "   <attr type=\"s\" value=\"sl\"/>\n"
                "   <attr type=\"u\" value=\"2\"/>\n"
                "   <attr type=\"u\" value=\"3\"/>\n"
                " </qual>\n"
                "</cclmap>\n";

            xmlDocPtr doc = yaz_xml_parse_memory(xml_str, strlen(xml_str));
            YAZ_CHECK(doc);

            int r = ccl_xml_config(bibset, xmlDocGetRootElement(doc), &addinfo);
            YAZ_CHECK_EQ(r, 0);

            xmlFreeDoc(doc);
        }
        break;
#else
        return;
#endif
    default:
        YAZ_CHECK(0);
        return;
    }

    YAZ_CHECK(tst_ccl_query(bibset, "x1", "@attr 4=2 @attr 1=1016 x1 "));

    YAZ_CHECK(tst_ccl_query(bibset, "k\xc3\xb8" "benhavn", "@attr 4=2 @attr 1=1016 k\xc3\xb8" "benhavn "));

    YAZ_CHECK(tst_ccl_query(bibset, "k\xf8" "benhavn", "@attr 4=2 @attr 1=1016 ""k\xf8" "benhavn "));

    YAZ_CHECK(tst_ccl_query(bibset, "(((((x1)))))", "@attr 4=2 @attr 1=1016 x1 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x1 and x2",
                  "@and "
                  "@attr 4=2 @attr 1=1016 x1 "
                  "@attr 4=2 @attr 1=1016 x2 "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=x3", "@attr 4=2 @attr 1=4 x3 "));
    YAZ_CHECK(tst_ccl_query(bibset, "dc.title=x4", "@attr 1=/my/title x4 "));
    YAZ_CHECK(tst_ccl_query(bibset, "dc.title=(x4)", "@attr 1=/my/title x4 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x1 and", 0));
    YAZ_CHECK(tst_ccl_query(bibset, "tix=x5", 0));

    YAZ_CHECK(tst_ccl_query(bibset, "a%b",
                  "@prox 0 1 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));
    YAZ_CHECK(tst_ccl_query(bibset, "a%(b)",
                  "@prox 0 1 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));
    YAZ_CHECK(tst_ccl_query(bibset, "(a)%(b)",
                  "@prox 0 1 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));
    YAZ_CHECK(tst_ccl_query(bibset, "a%1b",
                  "@prox 0 1 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "a%2b",
                  "@prox 0 2 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "(a)%2(b)",
                  "@prox 0 2 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "a%19b",
                  "@prox 0 19 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "spid%\xe6serne",
                  "@prox 0 1 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 spid "
                  "@attr 4=2 @attr 1=1016 \xe6serne "));

    YAZ_CHECK(tst_ccl_query(bibset, "a!b",
                  "@prox 0 1 1 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));
    YAZ_CHECK(tst_ccl_query(bibset, "a!2b",
                  "@prox 0 2 1 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "a% (b or dc.title=c)",
                            "@prox 0 1 0 2 k 2 "
                            "@attr 4=2 @attr 1=1016 a "
                            "@or @attr 4=2 @attr 1=1016 b "
                            "@attr 4=2 @attr 1=/my/title c "
    ));

    YAZ_CHECK(tst_ccl_query(bibset, "(a b) % (c)",
                            "@prox 0 1 0 2 k 2 @and "
                            "@attr 4=2 @attr 1=1016 a @attr 4=2 @attr 1=1016 b "
                            "@attr 4=2 @attr 1=1016 c " ));

    YAZ_CHECK(tst_ccl_query(bibset, "date=1980",
                            "1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "(date=1980)",
                            "1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date>1980",
                            "@attr 2=5 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date>=1980",
                            "@attr 2=4 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date<1980",
                            "@attr 2=1 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date<=1980",
                            "@attr 2=2 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234-1990",
                            "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234- 1990",
                            "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234 -1990",
                            "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234 - 1990",
                            "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=-1980",
                            "@attr 2=2 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=- 1980",
                            "@attr 2=2 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=-1980",
                            "@attr 2=3 -1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=- 1980",
                            "@attr 2=2 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x= -1980",
                            "@attr 2=3 -1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=234-1990",
                            "@attr 2=3 234-1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=234 - 1990",
                            "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a,b",
                            "@attr 4=1 @attr 1=4 a,b "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a, b",
                            "@attr 4=1 @attr 1=4 \"a, b\" "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a-b",
                            "@attr 4=2 @attr 1=4 a-b "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a - b",
                            "@attr 4=1 @attr 1=4 \"a - b\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "a?",
                            "@attr 5=1 @attr 4=2 @attr 1=1016 a "));
    YAZ_CHECK(tst_ccl_query(bibset, "a b",
                            "@and @attr 4=2 @attr 1=1016 a "
                            "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "a b?",
                            "@and @attr 4=2 @attr 1=1016 a "
                            "@attr 5=1 @attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "title=a",
                            "@attr 1=/my/title a "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=a?b#\"c?\"",
                            "@attr 5=102 a.*b.c\\\\? "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=a?b#\"c?\"",
                            "@attr 5=104 a?b#c\\\\? "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=\\(",
                            "( "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=\\(",
                            "( "));

    YAZ_CHECK(tst_ccl_query(bibset, "z=a b#",
                            "@attr 5=104 \"a b#\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=\\\"",
                            "\"\\\"\" "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=\\\"",
                            "\"\\\"\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=.",
                            ". "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=.",
                            ". "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=\".\"",
                            ". "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=\".\"",
                            ". "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=?\\?",
                            "@attr 5=102 .*\\\\? "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=?\\?",
                            "@attr 5=104 ?\\\\? "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=\"?\\?\"",
                            "?? "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=\"?\\?\"",
                            "?? "));

    YAZ_CHECK(tst_ccl_query(bibset, "reg=\\\\",
                            "\\\\ "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=\\\\",
                            "\\\\ "));

    YAZ_CHECK(tst_ccl_query(bibset, "\\\\",
                            "@attr 4=2 @attr 1=1016 \\\\ "));

    YAZ_CHECK(tst_ccl_query(bibset, "comb=a",
                            "@or @attr 4=2 @attr 1=1016 a "
                            "@attr 1=/my/title a "));

    YAZ_CHECK(tst_ccl_query(bibset, "a? b?",
                            "@and @attr 5=1 @attr 4=2 @attr 1=1016 a "
                            "@attr 5=1 @attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "\"a\"? \"b?\"",
                            "@and @attr 5=1 @attr 4=2 @attr 1=1016 a "
                            "@attr 4=2 @attr 1=1016 b? "));

    YAZ_CHECK(tst_ccl_query(bibset, "@and",
                            "@attr 4=2 @attr 1=1016 \\@and "));

    YAZ_CHECK(tst_ccl_query(bibset, "a@and",
                            "@attr 4=2 @attr 1=1016 a@and "));

    YAZ_CHECK(tst_ccl_query(bibset, "}",
                            "@attr 4=2 @attr 1=1016 } "));

    YAZ_CHECK(tst_ccl_query(bibset, "{",
                            "@attr 4=2 @attr 1=1016 \"{\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "\"a b c\"",
                            "@attr 4=1 @attr 1=1016 \"a b c\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "\"a b  c  \"",
                            "@attr 4=1 @attr 1=1016 \"a b  c  \" "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=a",
                            "@attr 4=2 a "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=a b",
                            "@attr 4=2 \"a b\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=a b \"c d\"",
                            "@and @attr 4=2 \"a b\" @attr 4=1 \"c d\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=a b \"c\"",
                            "@attr 4=2 \"a b c\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=a b \"\"",
                            "@attr 4=2 \"a b \" "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=a \"b c\" d",
                            "@and @and "
                            "@attr 4=2 a @attr 4=1 \"b c\" @attr 4=2 d "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=\"a b c\"",
                            "@attr 4=1 \"a b c\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "ag=\"a b c\" \"d e\"",
                            "@and @attr 4=1 \"a b c\" @attr 4=1 \"d e\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "splitlist=a", "@attr 1=2 a "));
    YAZ_CHECK(tst_ccl_query(bibset, "splitlist=a b", "@or "
                            "@and @attr 1=2 a @attr 1=2 b @attr 1=2 \"a b\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "splitlist=a b c", "@or @or "
                            "@and "
                            "@attr 1=2 a "
                            "@or @and @attr 1=2 b @attr 1=2 c "
                            "@attr 1=2 \"b c\" "
                            "@and @attr 1=2 \"a b\" @attr 1=2 c "
                            "@attr 1=2 \"a b c\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "splitlist=a?", 0));
    YAZ_CHECK(tst_ccl_query(bibset, "splitlist=a b?", 0));
    YAZ_CHECK(tst_ccl_query(bibset, "splitlist=a b c?", 0));
    YAZ_CHECK(tst_ccl_query(bibset, "splitlist=a b c d?", 0));

    YAZ_CHECK(tst_ccl_query(bibset, "s2=a", "@or @attr 1=2 a @attr 1=3 a "));

    YAZ_CHECK(tst_ccl_query(bibset, "s2=a b", "@or "
                            "@and " "@or @attr 1=2 a @attr 1=3 a "
                            "@or @attr 1=2 b @attr 1=3 b "
                            "@or @attr 1=2 \"a b\" @attr 1=3 \"a b\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "s2=a? b", 0));
    YAZ_CHECK(tst_ccl_query(bibset, "s2=a b?", 0));
    YAZ_CHECK(tst_ccl_query(bibset, "s2=a b? c", 0));

    YAZ_CHECK(tst_ccl_query(bibset, "z=(ti=b)", "@attr 4=2 @attr 1=4 b "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=(ti=b?)",
                            "@attr 5=1 @attr 4=2 @attr 1=4 b "));
    YAZ_CHECK(tst_ccl_query(bibset, "z=(a? and ti=b? and c?)",
                            "@and @and "
                            "@attr 5=104 a? "
                            "@attr 5=1 @attr 4=2 @attr 1=4 b "
                            "@attr 5=104 c? "));

    ccl_qual_rm(&bibset);
}

void tst2(void)
{
    CCL_bibset bibset = ccl_qual_mk();

    YAZ_CHECK(bibset);
    if (!bibset)
        return;

    ccl_qual_fitem(bibset, "u=4    s=pw t=l,r", "ti");
    ccl_qual_fitem(bibset, "1=1016 s=al,pw t=z",    "term");

    YAZ_CHECK(tst_ccl_query(bibset, "a?#",
                            "@attr 5=104 @attr 4=2 @attr 1=1016 a?# "));

    YAZ_CHECK(tst_ccl_query(bibset, "a b?#",
                            "@and @attr 4=2 @attr 1=1016 a @attr 5=104 @attr 4=2 @attr 1=1016 b?# "));

    YAZ_CHECK(tst_ccl_query(bibset, "a*",
                            "@attr 4=2 @attr 1=1016 a* "));

    YAZ_CHECK(tst_ccl_query(bibset, "a?",
                            "@attr 5=104 @attr 4=2 @attr 1=1016 a? "));

    ccl_qual_fitem(bibset, "*", "@truncation");
    YAZ_CHECK(tst_ccl_query(bibset, "a*",
                            "@attr 5=104 @attr 4=2 @attr 1=1016 a? "));

    YAZ_CHECK(tst_ccl_query(bibset, "a?",
                            "@attr 4=2 @attr 1=1016 a? "));

    ccl_qual_fitem(bibset, "?", "@mask");
    YAZ_CHECK(tst_ccl_query(bibset, "a?",
                            "@attr 5=104 @attr 4=2 @attr 1=1016 a# "));


    ccl_qual_fitem(bibset, "", "@mask");
    ccl_qual_fitem(bibset, "", "@truncation");
    YAZ_CHECK(tst_ccl_query(bibset, "a?#",
                            "@attr 4=2 @attr 1=1016 a?# "));

    ccl_qual_fitem(bibset, "og", "@and");
    ccl_qual_fitem(bibset, "eller", "@or");
    ccl_qual_fitem(bibset, "ikke", "@not");

    YAZ_CHECK(tst_ccl_query(bibset, "a og b eller c ikke d",
                            "@not @or @and @attr 4=2 @attr 1=1016 a "
                            "@attr 4=2 @attr 1=1016 b "
                            "@attr 4=2 @attr 1=1016 c "
                            "@attr 4=2 @attr 1=1016 d "));
    ccl_qual_rm(&bibset);
}


void tst3(void)
{
    CCL_bibset b = ccl_qual_mk();

    YAZ_CHECK(b);
    if (!b)
        return;

    ccl_qual_fitem(b, "2=102", "rk");
    ccl_qual_fitem(b, "1=9903 r=r", "lex");
    ccl_qual_fitem(b, "4=109 r=o", "st-numeric");

    YAZ_CHECK(tst_ccl_query(b, "rk=(lex=9)", "@attr 2=3 @attr 1=9903 9 "));
    YAZ_CHECK(tst_ccl_query(b, "rk=(lex>9)", "@attr 2=5 @attr 1=9903 9 "));
    YAZ_CHECK(tst_ccl_query(b, "rk=(lex,st-numeric=300-600)",
                            "@and "
                            "@attr 2=4 @attr 4=109 @attr 1=9903 300 "
                            "@attr 2=2 @attr 4=109 @attr 1=9903 600 "));
    YAZ_CHECK(tst_ccl_query(b, "rk=(lex=9) and b",
                            "@and @attr 2=3 @attr 1=9903 9 b "));
    YAZ_CHECK(tst_ccl_query(b, "rk=(lex=9 and b)",
                            "@and @attr 2=3 @attr 1=9903 9 @attr 2=102 b "));
    ccl_qual_rm(&b);
}

/* r=o / r=r qualifier with operand that is not a single term */
static void tst_relation_operand(void)
{
    CCL_bibset b = ccl_qual_mk();

    YAZ_CHECK(b);
    if (!b)
        return;

    ccl_qual_fitem(b, "u=30 r=r", "date");
    ccl_qual_fitem(b, "u=30 u=31 r=o", "dates");

    YAZ_CHECK(tst_ccl_query(b, "date=a", "@attr 2=3 @attr 1=30 a "));
    YAZ_CHECK(tst_ccl_query(b, "date=a%b",
                            "@prox 0 1 0 2 k 2 "
                            "@attr 1=30 a @attr 1=30 b "));
    YAZ_CHECK(tst_ccl_query(b, "date=(a) b",
                            "@prox 0 2 0 1 k 2 "
                            "@attr 1=30 a @attr 1=30 b "));
    YAZ_CHECK(tst_ccl_query(b, "date=(a) b )", 0));
    YAZ_CHECK(tst_ccl_query(b, "date=(a or b)",
                            "@or @attr 1=30 a @attr 1=30 b "));
    YAZ_CHECK(tst_ccl_query(b, "date>(set=1)", "@set 1 "));
    YAZ_CHECK(tst_ccl_query(b, "set=1 and date>(set=1)",
                            "@and @set 1 @set 1 "));
    YAZ_CHECK(tst_ccl_query(b, "date=(date=1-2)",
                            "@and "
                            "@attr 2=4 @attr 1=30 @attr 1=30 1 "
                            "@attr 2=2 @attr 1=30 @attr 1=30 2 "));
    YAZ_CHECK(tst_ccl_query(b, "dates=1980",
                            "@or @attr 1=30 1980 @attr 1=31 1980 "));
    ccl_qual_rm(&b);
}

void tst_addinfo(void)
{
    const char *addinfo;
    CCL_bibset bibset = ccl_qual_mk();

    int r = ccl_qual_fitem2(bibset, "u=4    s=pw t=l,r", "ti", &addinfo);
    YAZ_CHECK(r == 0 && addinfo == 0);

    r = ccl_qual_fitem2(bibset, "1=1016 s=al,pw t=z", "term", &addinfo);
    YAZ_CHECK(r == 0 && addinfo == 0);

    r = ccl_qual_fitem2(bibset, "x=", "term", &addinfo);
    YAZ_CHECK(r != 0 && addinfo != 0);

    r = ccl_qual_fitem2(bibset, "12=3", "term", &addinfo);
    YAZ_CHECK(r == 0 && addinfo == 0);

    r = ccl_qual_fitem2(bibset, "ab=3", "term", &addinfo);
    YAZ_CHECK(r != 0 && addinfo != 0);

    r = ccl_qual_fitem2(bibset, "x=ab", "term", &addinfo);
    YAZ_CHECK(r != 0 && addinfo != 0);

    r = ccl_qual_fitem2(bibset, "s=ab", "term", &addinfo);
    YAZ_CHECK(r == 0 && addinfo == 0);

    ccl_qual_rm(&bibset);
}

static void check_complexity(CCL_parser parser, const char *query, int error)
{
    int pos = -1;
    struct ccl_rpn_node *rpn = ccl_parser_find_str(parser, query);
    YAZ_CHECK_EQ(ccl_parser_get_error(parser, &pos), error);
    YAZ_CHECK(error ? rpn == 0 : rpn != 0);
    if (error)
        YAZ_CHECK(pos >= 0 && pos <= (int) strlen(query));
    if (rpn)
    {
        WRBUF out = wrbuf_alloc();
        ccl_pquery(out, rpn);
        YAZ_CHECK(wrbuf_len(out) > 0);
        wrbuf_destroy(out);
    }
    ccl_rpn_delete(rpn);
}

static void tst_complexity(void)
{
    CCL_bibset bibset = ccl_qual_mk();
    WRBUF query = wrbuf_alloc();
    int i, j, error, pos;
    const char *error_string = 0;
    ZOOM_query zoom = ZOOM_query_create();
    const char *prefix[] = { "(", "a and (", "a or (", "a not (", "ti=(" };

    ccl_qual_fitem(bibset, "u=4", "ti");
    ccl_qual_fitem(bibset, "s=sl u=2", "splitlist");
    ccl_qual_fitem(bibset, "s=al", "words");
    ccl_qual_fitem(bibset, "words words", "both");
    CCL_parser parser = ccl_parser_create(bibset);
    for (j = 0; j < (int) (sizeof(prefix) / sizeof(*prefix)); j++)
    {
        for (int depth = CCL_MAX_DEPTH - 1; depth <= CCL_MAX_DEPTH; depth++)
        {
            wrbuf_rewind(query);
            for (i = 0; i < depth; i++)
                wrbuf_puts(query, prefix[j]);
            wrbuf_puts(query, "a");
            for (i = 0; i < depth; i++)
                wrbuf_puts(query, ")");
            check_complexity(parser, wrbuf_cstr(query),
                             depth < CCL_MAX_DEPTH ? 0 : CCL_ERR_COMPLEXITY);
            check_complexity(parser, "a", 0);
        }
    }
    /* A large malformed input must hit the limit before stack exhaustion. */
    wrbuf_rewind(query);
    for (i = 0; i < 20000; i++)
        wrbuf_puts(query, "(");
    check_complexity(parser, wrbuf_cstr(query), CCL_ERR_COMPLEXITY);
    struct ccl_rpn_node *rpn =
        ccl_find_str(bibset, wrbuf_cstr(query), &error, &pos);
    YAZ_CHECK(!rpn);
    YAZ_CHECK_EQ(error, CCL_ERR_COMPLEXITY);
    ccl_rpn_delete(rpn);
    YAZ_CHECK_EQ(ZOOM_query_ccl2rpn(zoom, wrbuf_cstr(query), 0,
                                  &error, &error_string, &pos), -1);
    YAZ_CHECK_EQ(error, CCL_ERR_COMPLEXITY);
    YAZ_CHECK(error_string &&
              !strcmp(error_string, "Query complexity limit exceeded"));
    YAZ_CHECK_EQ(ZOOM_query_ccl2rpn(zoom, "a", 0,
                                  &error, &error_string, &pos), 0);

    /* One expression step, two per term, one per result-set operand. */
    wrbuf_rewind(query);
    for (i = 0; i < (CCL_MAX_STEPS - 2) / 2; i++)
        wrbuf_puts(query, i ? " and a" : "a");
    wrbuf_puts(query, " and set=s");
    check_complexity(parser, wrbuf_cstr(query), 0); /* exactly 4096 */
    wrbuf_puts(query, " and set=s");
    check_complexity(parser, wrbuf_cstr(query), CCL_ERR_COMPLEXITY);
    check_complexity(parser, "a", 0);

    /* Split-list recursion shares the expression depth budget. */
    wrbuf_rewind(query);
    wrbuf_puts(query, "splitlist=");
    for (i = 0; i < CCL_MAX_DEPTH - 1; i++)
        wrbuf_puts(query, "a ");
    check_complexity(parser, wrbuf_cstr(query), 0);
    wrbuf_puts(query, "a");
    check_complexity(parser, wrbuf_cstr(query), CCL_ERR_COMPLEXITY);

    /* Term-list expansion must obey the work limit too. */
    wrbuf_rewind(query);
    wrbuf_puts(query, "words=");
    for (i = 0; i < CCL_MAX_STEPS; i++)
        wrbuf_puts(query, "a ");
    check_complexity(parser, wrbuf_cstr(query), CCL_ERR_COMPLEXITY);
    /* Fail in the second qualifier expansion after the first built a tree. */
    wrbuf_rewind(query);
    wrbuf_puts(query, "both=");
    for (i = 0; i < CCL_MAX_STEPS / 2 + 1; i++)
        wrbuf_puts(query, "a ");
    check_complexity(parser, wrbuf_cstr(query), CCL_ERR_COMPLEXITY);
    check_complexity(parser, "a ! (", CCL_ERR_TERM_EXPECTED);

    /* Each term gets the attributes of all qualifiers in its list: one
       count per qualifier and one per attribute, for every term. */
    wrbuf_rewind(query);
    for (i = 0; i < 16; i++)
        wrbuf_puts(query, i ? ",ti" : "ti");
    wrbuf_puts(query, "=a");
    for (i = 1; i < CCL_MAX_APPLIED / 32; i++)
        wrbuf_puts(query, "!a");
    check_complexity(parser, wrbuf_cstr(query), 0); /* exactly 65536 */
    wrbuf_puts(query, "!a");
    check_complexity(parser, wrbuf_cstr(query), CCL_ERR_COMPLEXITY);
    check_complexity(parser, "ti,ti=a", 0);

    /* A nested list inherits, and so counts, the outer qualifiers too:
       42 inherited, then 43 qualifiers and 43 attributes for the term. */
    wrbuf_rewind(query);
    for (i = 0; i < 42; i++)
        wrbuf_puts(query, i ? ",ti" : "ti");
    wrbuf_puts(query, "=(ti=a");
    for (i = 1; i < CCL_MAX_APPLIED / 128; i++)
        wrbuf_puts(query, " and ti=a");
    wrbuf_puts(query, ")");
    check_complexity(parser, wrbuf_cstr(query), 0); /* exactly 65536 */
    wrbuf_cut_right(query, 1);
    wrbuf_puts(query, " and ti=a)");
    check_complexity(parser, wrbuf_cstr(query), CCL_ERR_COMPLEXITY);
    check_complexity(parser, "a", 0);

    ZOOM_query_destroy(zoom);
    ccl_parser_destroy(parser);
    ccl_qual_rm(&bibset);
    wrbuf_destroy(query);
}

static void tst_range_error_position(void)
{
    const char *ranges[] = { "abc-", "abc-def", "-abc", "abc -def" };
    CCL_bibset bibset = ccl_qual_mk();
    WRBUF query = wrbuf_alloc();
    int i, error, pos, expected;
    struct ccl_rpn_node *rpn;

    ccl_qual_fitem(bibset, "r=r", "date");
    CCL_parser parser = ccl_parser_create(bibset);
    for (size_t j = 0; j < sizeof(ranges) / sizeof(*ranges); j++)
    {
        wrbuf_rewind(query);
        for (i = 0; i < (CCL_MAX_STEPS - 2) / 2; i++)
            wrbuf_puts(query, i ? " and a" : "a");
        wrbuf_printf(query, " and date=%s", ranges[j]);
        expected = strstr(wrbuf_cstr(query), "abc") - wrbuf_cstr(query);
        rpn = ccl_parser_find_str(parser, wrbuf_cstr(query));
        YAZ_CHECK(!rpn);
        YAZ_CHECK_EQ(ccl_parser_get_error(parser, &pos), CCL_ERR_COMPLEXITY);
        YAZ_CHECK_EQ(pos, expected);
        ccl_rpn_delete(rpn);
        rpn = ccl_find_str(bibset, wrbuf_cstr(query), &error, &pos);
        YAZ_CHECK(!rpn);
        YAZ_CHECK_EQ(error, CCL_ERR_COMPLEXITY);
        YAZ_CHECK_EQ(pos, expected);
        ccl_rpn_delete(rpn);
        check_complexity(parser, "date=abc-", 0);
    }
    ccl_parser_destroy(parser);
    ccl_qual_rm(&bibset);
    wrbuf_destroy(query);
}

static int tst_stop_words_query(CCL_bibset bibset, const char *query,
                                const char *result)
{
    int ret = 0, error, pos;
    struct ccl_rpn_node *rpn = ccl_find_str(bibset, query, &error, &pos);

    if (rpn)
    {
        ccl_stop_words_t csw = ccl_stop_words_create();
        WRBUF wrbuf = wrbuf_alloc();

        ccl_stop_words_tree(csw, bibset, &rpn);
        ccl_pquery(wrbuf, rpn);
        if (!strcmp(wrbuf_cstr(wrbuf), result))
            ret = 1;
        else
        {
            yaz_log(YLOG_WARN, "%s: result does not match", query);
            yaz_log(YLOG_WARN, " expected %s", result);
            yaz_log(YLOG_WARN, " got      %s", wrbuf_cstr(wrbuf));
        }
        wrbuf_destroy(wrbuf);
        ccl_stop_words_destroy(csw);
        ccl_rpn_delete(rpn);
    }
    else
        yaz_log(YLOG_WARN, "%s: parse failed", query);
    return ret;
}

static void tst_stop_words(void)
{
    CCL_bibset bibset = ccl_qual_mk();

    YAZ_CHECK(bibset);
    if (!bibset)
        return;
    ccl_qual_fitem(bibset, "s=al", "term");
    ccl_qual_add_special(bibset, "stop.*", "the of a");

    YAZ_CHECK(tst_stop_words_query(bibset, "cat", "cat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "the", ""));
    YAZ_CHECK(tst_stop_words_query(bibset, "cat hat the", "@and cat hat "));
    /* stop word removed below the top node */
    YAZ_CHECK(tst_stop_words_query(bibset, "the cat hat", "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "cat the hat", "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "the and cat and hat",
                                   "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "(the or cat) and hat",
                                   "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "cat and (the or hat)",
                                   "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "(the or of) and cat and hat",
                                   "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "(cat and the) or (a and hat)",
                                   "@or cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "(cat not the) and hat",
                                   "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "(the % cat) and hat",
                                   "@and cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "cat % (the or hat)",
                                   "@prox 0 1 0 2 k 2 cat hat "));
    YAZ_CHECK(tst_stop_words_query(bibset, "the of a the of a cat hat the",
                                   "@and cat hat "));
    ccl_qual_rm(&bibset);
}

static void tst_field_or(void)
{
    CCL_bibset bibset = ccl_qual_mk();
    CCL_parser parser;
    struct ccl_rpn_node *rpn;
    int pos = -1;

    ccl_qual_fitem(bibset, "or", "@field");
    ccl_qual_fitem(bibset, "u=4 s=pw", "ti");
    ccl_qual_fitem(bibset, "u=1003 s=pw", "au");

    YAZ_CHECK(tst_ccl_query(bibset, "ti,au=x",
                            "@or @attr 4=2 @attr 1=4 x "
                            "@attr 4=2 @attr 1=1003 x "));
    YAZ_CHECK(tst_ccl_query(bibset, "nofield,ti=x", 0));

    /* unknown field after known ones: nodes for ti and au must be freed */
    parser = ccl_parser_create(bibset);
    rpn = ccl_parser_find_str(parser, "ti,au,nofield=x");
    YAZ_CHECK(!rpn);
    YAZ_CHECK_EQ(ccl_parser_get_error(parser, &pos), CCL_ERR_UNKNOWN_QUAL);
    YAZ_CHECK_EQ(pos, 6);
    ccl_rpn_delete(rpn);
    ccl_parser_destroy(parser);
    ccl_qual_rm(&bibset);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_range_error_position();
    tst_complexity();
    tst1(0);
    tst1(1);
    tst1(2);
    tst1(3);
    tst2();
    tst3();
    tst_relation_operand();
    tst_addinfo();
    tst_stop_words();
    tst_field_or();
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

