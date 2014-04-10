/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <yaz/rpn2cql.h>
#include <yaz/rpn2solr.h>
#include <yaz/pquery.h>
#include <yaz/options.h>

static void usage(void)
{
    fprintf(stderr, "usage\n cql2pqf [-n <n>] [-r] [-s] [-S] <properties> "
            "[<query>]\n");
    fprintf(stderr, " -r  reverse conversion (RPN to Solr/CQL)\n");
    fprintf(stderr, " -s  Solr instead of CQL\n");
    fprintf(stderr, " -S  strict CQL 2.0\n");
    exit(1);
}

int main(int argc, char **argv)
{
    cql_transform_t ct;
    int i, iterations = 1;
    char *query = 0;
    char *fname = 0;
    int reverse = 0;
    int solr = 0;
    int verbose = 0;
    int do_strict = 0;

    int ret;
    char *arg;

    while ((ret = options("n:rsSv", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            if (!fname)
                fname = arg;
            else
                query = arg;
            break;
        case 'n':
            iterations = atoi(arg);
            break;
        case 'r':
            reverse = 1;
            break;
        case 'S':
            do_strict = 1;
            break;
        case 's':
            solr = 1;
            break;
        case 'v':
            verbose = 1;
            break;
        default:
            usage();
        }
    }
    if (fname)
    {
        ct = cql_transform_open_fname(fname);
        if (!ct)
        {
            fprintf(stderr, "failed to read properties %s\n", fname);
            exit(1);
        }
    }
    else
        ct = cql_transform_create();

    if (reverse)
    {
        char buf[1024];

        if (!query)
        {
            if (fgets(buf, sizeof buf, stdin))
            {
                if (*buf && buf[strlen(buf)-1] == '\n')
                    buf[strlen(buf)-1] = '\0';
                query = buf;
            }
        }
        if (query)
        {
            ODR odr = odr_createmem(ODR_ENCODE);
            YAZ_PQF_Parser pp = yaz_pqf_create();
            Z_RPNQuery *rpn = yaz_pqf_parse(pp, odr, query);
            if (!rpn)
            {
                fprintf(stderr, "PQF syntax error\n");
            }
            else
            {
                int ret = 0;
                if (solr)
                    ret = solr_transform_rpn2solr_stream(ct, cql_fputs,
                                                         stdout, rpn);
                else
                    ret = cql_transform_rpn2cql_stream(ct, cql_fputs,
                                                       stdout, rpn);
                if (ret)
                {
                    const char *addinfo;
                    int r = cql_transform_error(ct, &addinfo);
                    printf("Transform error %d %s\n", r, addinfo ? addinfo : "");
                }
                else
                    printf("\n");
            }
            yaz_pqf_destroy(pp);
            odr_destroy(odr);
        }
    }
    else
    {
        CQL_parser cp = cql_parser_create();
        int r = 0;

        cql_parser_strict(cp, do_strict);
        if (query)
        {
            if (verbose)
                printf("Parsing CQL %s\n", query);
            for (i = 0; i<iterations; i++)
                r = cql_parser_string(cp, query);
        }
        else
            r = cql_parser_stdio(cp, stdin);

        if (r)
            fprintf(stderr, "Syntax error\n");
        else if (solr)
        {
            printf("CQL to Solr not supported (supported is reverse -r)\n");
        }
        else
        {
            r = cql_transform_FILE(ct, cql_parser_result(cp), stdout);
            printf("\n");
            if (r)
            {
                const char *addinfo;
                r = cql_transform_error(ct, &addinfo);
                printf("Transform error %d %s\n", r, addinfo ? addinfo : "");
            }
            else
            {
                FILE *null = fopen("/dev/null", "w");
                for (i = 1; i<iterations; i++)
                    cql_transform_FILE(ct, cql_parser_result(cp), null);
                fclose(null);
            }
        }
        cql_parser_destroy(cp);
    }
    cql_transform_close(ct);
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

