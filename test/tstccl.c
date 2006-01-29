/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstccl.c,v 1.10 2006-01-29 21:59:13 adam Exp $
 */

/* CCL test */

#include <string.h>
#include <yaz/ccl.h>
#include <yaz/test.h>

struct ccl_tst {
    char *query;
    char *result;
};

static struct ccl_tst query_str[] = {
    { "x1", "@attr 4=2 @attr 1=1016 x1 "},
    { "(((((x1)))))", "@attr 4=2 @attr 1=1016 x1 "},
    {"x1 and x2", "@and @attr 4=2 @attr 1=1016 x1 @attr 4=2 @attr 1=1016 x2 "},
    { "ti=x3", "@attr 4=2 @attr 1=4 x3 "},
    { "dc.title=x4", "@attr 1=/my/title x4 "},
    { "x1 and", 0},
    { "tix=x5", 0},
    { "spid%æserne", "@prox 0 1 0 2 k 2 @attr 4=2 @attr 1=1016 spid @attr 4=2 @attr 1=1016 æserne "},
    { "date=1980", "@attr 2=3 1980 "},
    { "date=234-1990", "@and @attr 2=4 234 @attr 2=2 1990 "},
    { "date=234- 1990", "@and @attr 2=4 234 @attr 2=2 1990 "},
    { "date=234 -1990", "@and @attr 2=4 234 @attr 2=2 1990 "},
    { "date=234 - 1990", "@and @attr 2=4 234 @attr 2=2 1990 "},
    { "date=-1980", "@attr 2=2 1980 "},
    { "date=- 1980", "@attr 2=2 1980 "},
    { "x=-1980", "@attr 2=3 -1980 "},
    { "x=- 1980", "@attr 2=2 1980 "},
    { "x= -1980", "@attr 2=3 -1980 "},
    { "x=234-1990", "@attr 2=3 234-1990 "},
    { "x=234 - 1990", "@and @attr 2=4 234 @attr 2=2 1990 "},
    { "ti=a,b", "@attr 4=1 @attr 1=4 a,b "},
    { "ti=a, b", "@attr 4=1 @attr 1=4 a,\\ b "},
    { "ti=a-b", "@attr 4=2 @attr 1=4 a-b "},
    { "ti=a - b", "@attr 4=1 @attr 1=4 a\\ -\\ b "},
    {0, 0}
};

void tst1(int pass)
{
    CCL_parser parser = ccl_parser_create ();
    CCL_bibset bibset = ccl_qual_mk();
    int i;
    char tstline[128];

    YAZ_CHECK(parser);
    YAZ_CHECK(bibset);
    switch(pass)
    {
    case 0:
        ccl_qual_fitem(bibset, "u=4    s=pw t=l,r", "ti");
        ccl_qual_fitem(bibset, "1=1016 s=al,pw",    "term");
        ccl_qual_fitem(bibset, "1=/my/title",         "dc.title");
        ccl_qual_fitem(bibset, "r=r",         "date");
        ccl_qual_fitem(bibset, "r=o",         "x");
        break;
    case 1:
        strcpy(tstline, "ti u=4    s=pw t=l,r");
        ccl_qual_line(bibset, tstline);

        strcpy(tstline, "term 1=1016 s=al,pw   # default term");
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
                     "term 1=1016 s=al,pw\r\n"
                     "\n"
                     "dc.title 1=/my/title\n"
                     "date r=r\n" 
                     "x r=o\n"
            );
        break;
    default:
        YAZ_CHECK(0);
    }

    parser->bibset = bibset;

    for (i = 0; query_str[i].query; i++)
    {
        struct ccl_token *token_list;
        struct ccl_rpn_node *rpn;

        token_list = ccl_parser_tokenize(parser, query_str[i].query);
        rpn = ccl_parser_find(parser, token_list);
        ccl_token_del (token_list);
        if (rpn)
        {
            /* parse ok. check that result is there and match */
            WRBUF wrbuf = wrbuf_alloc();
            ccl_pquery(wrbuf, rpn);

            /* check expect a result and that it matches */
            YAZ_CHECK(query_str[i].result);
            if (query_str[i].result)
            {
                YAZ_CHECK(!strcmp(wrbuf_buf(wrbuf), query_str[i].result));
            }
            ccl_rpn_delete(rpn);
            wrbuf_free(wrbuf, 1);
        }
        else 
        {
            /* parse failed. So we expect no result either */
            YAZ_CHECK(!query_str[i].result);
        }
    }   
    ccl_parser_destroy (parser);
    ccl_qual_rm(&bibset);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst1(0);
    tst1(1);
    tst1(2);
    YAZ_CHECK_TERM;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

