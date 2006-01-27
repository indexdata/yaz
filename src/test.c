/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: test.c,v 1.1 2006-01-27 18:58:58 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include <yaz/test.h>

static FILE *test_fout;
static int test_number = 0;
static int test_verbose = 0;

void yaz_check_init1(int *argc_p, char ***argv_p)
{
    int i = 0;
    int argc = *argc_p;
    char **argv = *argv_p;

    test_fout = 0;
    for (i = 1; i<argc; i++)
    {
        if (strlen(argv[i]) >= 7 && !memcmp(argv[i], "--test-", 7))
        {
            const char *suf = argv[i]+7;
            if (i < argc-1 && !strcmp(suf, "file"))
            {
                i++;
                if (test_fout)
                    fclose(test_fout);
                test_fout = fopen(argv[i], "w");
                continue;
            }
            else if (i < argc-1 && !strcmp(suf, "verbose"))
            {
                i++;
                test_verbose = atoi(argv[i]);
                continue;
            }
            else if (!strcmp(suf, "help"))
            {
                fprintf(stderr, 
                        "--test-help           help\n"
                        "--test-verbose level  verbose; 0=quiet; 1=normal; 2=more\n"
                        "--test-file fname     output to fname\n");
                exit(0);
            }
            else
            {
                fprintf(stderr, "Unrecognized option for YAZ test: %s\n",
                        argv[i]);
                fprintf(stderr, "Use --test-help for more info\n");
                exit(1);
            }
            
        }
        break;
    }
    /* remove --test- options from argc, argv so that they disappear */
    (*argv_p)[i-1] = **argv_p;  /* program name */
    --i;
    *argc_p -= i;
    *argv_p += i;
    if (!test_fout)
        test_fout = stdout;
}

void yaz_check_print1(int type, const char *file, int line, const char *expr)
{
    const char *msg = "unknown";

    test_number++;
    switch(type)
    {
    case YAZ_TEST_TYPE_FAIL:
        msg = "failed";
        if (test_verbose < 1)
            return;
        break;
    case YAZ_TEST_TYPE_OK:
        msg = "OK";
        if (test_verbose < 2)
            return;
        break;
    }
    fprintf(test_fout, "%s:%d %s: %s\n", file, line, msg, expr);
}


/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

