/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstccl.c,v 1.6 2005-02-01 17:23:36 adam Exp $
 */

/* CCL test */

#include <stdlib.h>
#include <string.h>
#include <yaz/ccl.h>

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
    {0, 0}
};

void tst1(int pass, int *number_of_errors)
{
    CCL_parser parser = ccl_parser_create ();
    CCL_bibset bibset = ccl_qual_mk();
    int i;
    char tstline[128];

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
	exit(23);
    }

    parser->bibset = bibset;

    for (i = 0; query_str[i].query; i++)
    {
	struct ccl_token *token_list =
	    ccl_parser_tokenize(parser, query_str[i].query);
	struct ccl_rpn_node *rpn = ccl_parser_find(parser, token_list);
	ccl_token_del (token_list);
	if (rpn)
	{
	    WRBUF wrbuf = wrbuf_alloc();
	    ccl_pquery(wrbuf, rpn);

	    if (!query_str[i].result)
	    {
		printf ("Failed %s\n", query_str[i].query);
		printf (" got:%s:\n", wrbuf_buf(wrbuf));
		printf (" expected failure\n");
		(*number_of_errors)++;
	    }
	    else if (strcmp(wrbuf_buf(wrbuf), query_str[i].result))
	    {
		printf ("Failed %s\n", query_str[i].query);
		printf (" got:%s:\n", wrbuf_buf(wrbuf));
		printf (" expected:%s:\n", query_str[i].result);
		(*number_of_errors)++;
	    }
	    ccl_rpn_delete(rpn);
	    wrbuf_free(wrbuf, 1);
	}
	else if (query_str[i].result)
	{
	    printf ("Failed %s\n", query_str[i].query);
	    printf (" got failure\n");
	    printf (" expected:%s:\n", query_str[i].result);
	    (*number_of_errors)++;
	}
    }	
    ccl_parser_destroy (parser);
    ccl_qual_rm(&bibset);
}

int main(int argc, char **argv)
{
    int number_of_errors = 0;
    tst1(0, &number_of_errors);
    if (number_of_errors)
	exit(1);
    tst1(1, &number_of_errors);
    if (number_of_errors)
	exit(1);
    tst1(2, &number_of_errors);
    if (number_of_errors)
	exit(1);
    exit(0);
}
