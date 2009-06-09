/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/* CCL test */

#include <string.h>
#include <yaz/ccl_xml.h>
#include <yaz/log.h>
#include <yaz/test.h>


static int tst_ccl_query(CCL_bibset bibset,
                         const char *query,
                         const char *result)
{
    CCL_parser parser = ccl_parser_create(bibset);
    int ret = 0;

    if (parser && bibset)
    {
        struct ccl_rpn_node *rpn;
        
        rpn = ccl_parser_find_str(parser, query);
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

    switch(pass)
    {
    case 0:
        ccl_qual_fitem(bibset, "u=4    s=pw t=l,r", "ti");
        ccl_qual_fitem(bibset, "1=1016 s=al,pw t=r",    "term");
        ccl_qual_fitem(bibset, "1=/my/title",         "dc.title");
        ccl_qual_fitem(bibset, "r=r",         "date");
        ccl_qual_fitem(bibset, "r=o",         "x");
        break;
    case 1:
        strcpy(tstline, "ti u=4    s=pw t=l,r");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "term 1=1016 s=al,pw t=r  # default term");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "dc.title 1=/my/title");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "date r=r # ordered relation");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "x r=o # ordered relation");
        ccl_qual_line(bibset, tstline);
        break;
    case 2:
        ccl_qual_buf(bibset, "ti u=4    s=pw t=l,r\n"
                     "term 1=1016 s=al,pw t=r\r\n"
                     "\n"
                     "dc.title 1=/my/title\n"
                     "date r=r\n" 
                     "x r=o\n"
            );
        break;
    case 3:
#if YAZ_HAVE_XML2
        if (1)
        {
            xmlDocPtr doc;
            int r;
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
                " <qual name=\"dc.title\">\n"
                "   <attr type=\"1\" value=\"/my/title\"/>\n"
                " </qual>\n"
                " <qual name=\"date\">\n"
                "   <attr type=\"r\" value=\"r\"/>\n"
                " </qual>\n"
                " <qual name=\"x\">\n"
                "   <attr type=\"r\" value=\"o\"/>\n"
                " </qual>\n"
                "</cclmap>\n";
            
            doc = xmlParseMemory(xml_str, strlen(xml_str));
            YAZ_CHECK(doc);

            r = ccl_xml_config(bibset, xmlDocGetRootElement(doc), &addinfo);
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
    YAZ_CHECK(tst_ccl_query(bibset, "(((((x1)))))", "@attr 4=2 @attr 1=1016 x1 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x1 and x2",
                  "@and "
                  "@attr 4=2 @attr 1=1016 x1 "
                  "@attr 4=2 @attr 1=1016 x2 "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=x3", "@attr 4=2 @attr 1=4 x3 "));
    YAZ_CHECK(tst_ccl_query(bibset, "dc.title=x4", "@attr 1=/my/title x4 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x1 and", 0));
    YAZ_CHECK(tst_ccl_query(bibset, "tix=x5", 0));

    YAZ_CHECK(tst_ccl_query(bibset, "a%b", 
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

    YAZ_CHECK(tst_ccl_query(bibset, "a%19b", 
                  "@prox 0 19 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "spid%æserne", 
                  "@prox 0 1 0 2 k 2 "
                  "@attr 4=2 @attr 1=1016 spid "
                  "@attr 4=2 @attr 1=1016 æserne "));

    YAZ_CHECK(tst_ccl_query(bibset, "a!b", 
                  "@prox 0 1 1 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));
    YAZ_CHECK(tst_ccl_query(bibset, "a!2b", 
                  "@prox 0 2 1 2 k 2 "
                  "@attr 4=2 @attr 1=1016 a "
                  "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "date=1980", "@attr 2=3 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234-1990", "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234- 1990", "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234 -1990", "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=234 - 1990", "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=-1980", "@attr 2=2 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "date=- 1980", "@attr 2=2 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=-1980", "@attr 2=3 -1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=- 1980", "@attr 2=2 1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x= -1980", "@attr 2=3 -1980 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=234-1990", "@attr 2=3 234-1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "x=234 - 1990", "@and @attr 2=4 234 @attr 2=2 1990 "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a,b", "@attr 4=1 @attr 1=4 a,b "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a, b", "@attr 4=1 @attr 1=4 \"a, b\" "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a-b", "@attr 4=2 @attr 1=4 a-b "));
    YAZ_CHECK(tst_ccl_query(bibset, "ti=a - b", "@attr 4=1 @attr 1=4 \"a - b\" "));

    YAZ_CHECK(tst_ccl_query(bibset, "a?", "@attr 5=1 @attr 4=2 @attr 1=1016 a "));
    YAZ_CHECK(tst_ccl_query(bibset, "a b", 
                            "@and @attr 4=2 @attr 1=1016 a "
                            "@attr 4=2 @attr 1=1016 b "));

    YAZ_CHECK(tst_ccl_query(bibset, "a b?", 
                            "@and @attr 4=2 @attr 1=1016 a "
                            "@attr 5=1 @attr 4=2 @attr 1=1016 b "));

    /* Bug #2895 */
#if 1
    YAZ_CHECK(tst_ccl_query(bibset, "a? b?", 
                            /* incorrect. */
                            "@and @attr 4=2 @attr 1=1016 a? "
                            "@attr 5=1 @attr 4=2 @attr 1=1016 b "));
#else
    YAZ_CHECK(tst_ccl_query(bibset, "a? b?", 
                            /* correct */
                            "@and @attr 5=1 @attr 4=2 @attr 1=1016 a "
                            "@attr 5=1 @attr 4=2 @attr 1=1016 b "));
#endif
    ccl_qual_rm(&bibset);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst1(0);
    tst1(1);
    tst1(2);
    tst1(3);
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

