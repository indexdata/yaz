/*
 * Copyright (c) 2002-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: tstccl.c,v 1.1 2003-10-27 12:21:38 adam Exp $
 */

/* CCL test */

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
    {0, 0}
};

void tst1(void)
{
    CCL_parser parser = ccl_parser_create ();
    CCL_bibset bibset = ccl_qual_mk();
    int i;

    ccl_qual_fitem(bibset, "u=4    s=pw t=l,r", "ti");
    ccl_qual_fitem(bibset, "1=1016 s=al,pw",    "term");
    ccl_qual_fitem(bibset, "1=/my/title",         "dc.title");

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
		exit(3);
	    }
	    else if (strcmp(wrbuf_buf(wrbuf), query_str[i].result))
	    {
		printf ("Failed %s\n", query_str[i].query);
		printf (" got:%s:\n", wrbuf_buf(wrbuf));
		printf (" expected:%s:\n", query_str[i].result);
		exit(2);
	    }
	    ccl_rpn_delete(rpn);
	    wrbuf_free(wrbuf, 1);
	}
	else if (query_str[i].result)
	{
	    printf ("Failed %s\n", query_str[i].query);
	    printf (" got failure\n");
	    printf (" expected:%s:\n", query_str[i].result);
	    exit(4);
	}
    }	
    ccl_parser_destroy (parser);
    ccl_qual_rm(&bibset);
}

int main(int argc, char **argv)
{
    tst1();
    exit(0);
}
