/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <yaz/cql.h>
#include <yaz/options.h>

static void usage(const char *prog)
{
    fprintf(stderr, "%s: [-c] [-n iterations] [infile]\n", prog);
    exit(1);
}

int main(int argc, char **argv)
{
    CQL_parser cp;
    int r = 0;
    const char *fname = 0;
    int iterations = 1;
    int ret;
    int convert_to_ccl = 0;
    char *arg;
    char *prog = argv[0];

    while ((ret = options("cn:", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            fname = arg;
            break;
        case 'c':
            convert_to_ccl = 1;
            break;
        case 'n':
            iterations = atoi(arg);
            break;
        default:
            usage(prog);            
        }
    }

    cp = cql_parser_create();
    if (fname)
    {
        int i;
        for (i = 0; i<iterations; i++)
            r = cql_parser_string(cp, fname);
    }
    else
        r = cql_parser_stdio(cp, stdin);
    if (r)
        fprintf (stderr, "Syntax error\n");
    else
    {
        if (convert_to_ccl)
        {
            cql_to_ccl_stdio(cql_parser_result(cp), stdout);
            putchar('\n');
        }
        else
            cql_to_xml_stdio(cql_parser_result(cp), stdout);
    }
    cql_parser_destroy(cp);
    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

