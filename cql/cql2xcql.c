/* $Id: cql2xcql.c,v 1.1 2003-01-06 08:20:27 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include <stdio.h>

#include <yaz/cql.h>

int main(int argc, char **argv)
{
    int r;
    CQL_parser cp = cql_parser_create();
    if (argc == 2)
        r = cql_parser_string(cp, argv[1]);
    else
        r = cql_parser_stdio(cp, stdin);
    if (r)
        fprintf (stderr, "Syntax error\n");
    else
        cql_to_xml_stdio(cql_parser_result(cp), stdout);
    cql_parser_destroy(cp);
    return 0;
}
