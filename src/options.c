/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: options.c,v $
 * Revision 1.1  2003-10-27 12:21:34  adam
 * Source restructure. yaz-marcdump part of installation
 *
 * Revision 1.8  2000/02/29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.7  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.6  1997/09/01 08:54:13  adam
 * New windows NT/95 port using MSV5.0. Made prefix query handling
 * thread safe. The function options ignores empty arguments when met.
 *
 * Revision 1.5  1995/12/06 13:00:19  adam
 * Minus alone not treated as an option.
 *
 * Revision 1.4  1995/09/29  17:12:35  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:03:03  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/16  08:51:13  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/27  08:35:18  quinn
 * Created util library
 * Added memory debugging module. Imported options-manager
 *
 * Revision 1.2  1994/10/04  17:47:10  adam
 * Function options now returns arg with error option.
 *
 * Revision 1.1  1994/08/16  15:57:22  adam
 * The first utility modules.
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <yaz/options.h>

static int arg_no = 1;
static int arg_off = 0;

int options (const char *desc, char **argv, int argc, char **arg)
{
    int ch, i = 0;
    
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
        arg_off++;
    }
    ch = argv[arg_no][arg_off++];
    while (desc[i])
    {
        int desc_char = desc[i++];
        int type = 0;
        if (desc[i] == ':')
	{	/* string argument */
            type = desc[i++];
	}
        if (desc_char == ch)
	{ /* option with argument */
            if (type)
	    {
                if (argv[arg_no][arg_off])
		{
                    *arg = argv[arg_no]+arg_off;
                    arg_no++;
                    arg_off =  0;
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
                if (argv[arg_no][arg_off])
                    arg_off++;
                else
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
