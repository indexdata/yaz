/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: test.c,v 1.3 2006-01-29 21:59:13 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <yaz/test.h>

static FILE *test_fout = 0; /* can't use '= stdout' on some systems */
static int test_total = 0;
static int test_failed = 0;
static int test_verbose = 1;
static char *test_prog = 0;

static FILE *get_file()
{
    if (test_fout)
        return test_fout;
    return stdout;
}

static char *progname(char *argv0)
{
    char *cp = strrchr(argv0, '/');
    if (cp)
        return cp+1;
    cp = strrchr(argv0, '\\');
    if (cp)
        return cp+1;
    return argv0;
}

void yaz_check_init1(int *argc_p, char ***argv_p)
{
    int i = 0;
    int argc = *argc_p;
    char **argv = *argv_p;

    test_prog = progname(argv[0]);

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
                        "--test-file fname     output to fname\n"
                        "--test-verbose level  verbose level\n"
                        "       0=Quiet. Only exit code tells what's wrong\n"
                        "       1=Report+Summary only if tests fail.\n"
                        "       2=Report failures. Print summary always\n"
                        "       3=Report + summary always\n"
                    );
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
}

void yaz_check_term1(void)
{
    /* summary */
    if (test_failed)
    {
        if (test_verbose >= 1)
            fprintf(get_file(), "%d out of %d tests failed for program %s\n",
                    test_failed, test_total, test_prog);
    }
    else
    {
        if (test_verbose >= 2)
            fprintf(get_file(), "%d tests passed for program %s\n",
                    test_total, test_prog);
    }
    if (test_fout)
        fclose(test_fout);
    if (test_failed)
        exit(1);
    exit(0);
}

void yaz_check_print1(int type, const char *file, int line, const char *expr)
{
    const char *msg = "unknown";

    test_total++;
    switch(type)
    {
    case YAZ_TEST_TYPE_FAIL:
        test_failed++;
        msg = "failed";
        if (test_verbose < 1)
            return;
        break;
    case YAZ_TEST_TYPE_OK:
        msg = "OK";
        if (test_verbose < 3)
            return;
        break;
    }
    fprintf(get_file(), "%s:%d %s: %s\n", file, line, msg, expr);
}


/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

