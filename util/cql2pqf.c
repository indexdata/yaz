/* $Id: cql2pqf.c,v 1.2 2003-12-18 16:45:19 mike Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include <stdlib.h>
#include <stdio.h>

#include <yaz/cql.h>

int main(int argc, char **argv)
{
    cql_transform_t ct;
    int r;
    int i, it = 1;
    CQL_parser cp = cql_parser_create();

    if (argc < 2)
    {
        fprintf (stderr, "usage\n cqltransform <properties> [<query>] [iterations]\n");
        exit (1);
    }
    ct = cql_transform_open_fname(argv[1]);
    if (!ct)
    {
        fprintf (stderr, "failed to read properties %s\n", argv[1]);
        exit (1);
    }
    if (argc >= 4)
        it = atoi(argv[3]);

    for (i = 0; i<it; i++)
    {
    if (argc >= 3)
        r = cql_parser_string(cp, argv[2]);
    else
        r = cql_parser_stdio(cp, stdin);

    if (r)
        fprintf (stderr, "Syntax error\n");
    else
    {
        r = cql_transform_FILE(ct, cql_parser_result(cp), stdout);
	printf("\n");
        if (r)
        {
            const char *addinfo;
            cql_transform_error(ct, &addinfo);
            printf ("Transform error %d %s\n", r, addinfo ? addinfo : "");
        }
    }
    }
    cql_transform_close(ct);
    cql_parser_destroy(cp);
    return 0;
}
