/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file options.c
 * \brief Implements command line options parsing
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <yaz/options.h>

static int arg_no = 1;
static int arg_off = 0;

int options (const char *desc, char **argv, int argc, char **arg)
{
    const char *opt_buf = 0;
    size_t i = 0;
    int ch = 0;
    
    if (arg_no >= argc)
        return -2;
    if (arg_off == 0)
    {
        while (argv[arg_no][0] == '\0')
        {
            arg_no++;
            if (arg_no >= argc)
                return -2;
        }
        if (argv[arg_no][0] != '-' || argv[arg_no][1] == '\0')
        {
            *arg = argv[arg_no++];
            return 0;
        }
        arg_off++; /* skip - */
    }
    if (argv[arg_no][1] == '-')
    {   /* long opt */
        opt_buf = argv[arg_no]+2;
        arg_off = strlen(argv[arg_no]);
    }
    else
    {   /* single char opt */
        ch = argv[arg_no][arg_off++];
    }
    while (desc[i])
    {
        int desc_char = desc[i++];
        int type = 0;
        while (desc[i] == '{')
        {
            size_t i0 = ++i;
            while (desc[i] && desc[i] != '}')
                i++;
            if (opt_buf && (i - i0) == strlen(opt_buf) &&
                memcmp(opt_buf, desc+i0, i - i0) == 0)
                ch = desc_char;
            if (desc[i])
                i++;
        }
        if (desc[i] == ':')
        {       /* option with string argument */
            type = desc[i++];
        }
        if (desc_char == ch)
        { 
            if (type) /* option with argument */
            {
                if (argv[arg_no][arg_off])
                {
                    *arg = argv[arg_no]+arg_off;
                    arg_no++;
                    arg_off = 0;
                }
                else
                {
                    arg_no++;
                    arg_off = 0;
                    if (arg_no < argc)
                        *arg = argv[arg_no++];
                    else
                        *arg = "";
                }
            }
            else /* option with no argument */
            {
                if (!argv[arg_no][arg_off])
                {
                    arg_off = 0;
                    arg_no++;
                }
            }
            return ch;
        }               
    }
    *arg = argv[arg_no]+arg_off-1;
    arg_no = arg_no + 1;
    arg_off = 0;
    return -1;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

