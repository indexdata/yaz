/*
 * Copyright (c) 2002-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: tstccl.c,v 1.1 2003-06-23 10:22:21 adam Exp $
 */

/* CCL test */

#include <yaz/ccl.h>

static char *query_str[] = {
    "x1",
    "x1 and x2",
    "ti=x3",
    "dc.title=x4",
    0
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

    for (i = 0; query_str[i]; i++)
    {
	struct ccl_token *token_list =
	    ccl_parser_tokenize(parser, query_str[i]);
	struct ccl_rpn_node *rpn = ccl_parser_find(parser, token_list);
	ccl_token_del (token_list);
	if (rpn)
	{
	    ccl_rpn_delete(rpn);
	}
	else
	{
	    printf ("failed %s\n", query_str[i]);
	    exit(1+i);
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
