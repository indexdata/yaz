/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: marcdump.c,v $
 * Revision 1.11  2000-07-04 08:53:22  adam
 * Fixed bug.
 *
 * Revision 1.10  2000/02/29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.9  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.8  1999/05/26 07:49:35  adam
 * C++ compilation.
 *
 * Revision 1.7  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.6  1997/12/12 06:32:33  adam
 * Added include of string.h.
 *
 * Revision 1.5  1997/09/24 13:29:40  adam
 * Added verbose option -v to marcdump utility.
 *
 * Revision 1.4  1995/11/01 13:55:05  quinn
 * Minor adjustments
 *
 * Revision 1.3  1995/05/16  08:51:12  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.2  1995/05/15  11:56:56  quinn
 * Debuggng & adjustments.
 *
 * Revision 1.1  1995/04/10  10:28:47  quinn
 * Added copy of CCL and MARC display
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <yaz/marcdisp.h>
#include <yaz/xmalloc.h>
#include <yaz/options.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
 
int main (int argc, char **argv)
{
    int ret;
    char *arg;
    int verbose = 0;
    FILE *inf;
    long file_size;
    char *buf, *p;
    char *prog = *argv;
    int count = 0;
    int no = 0;

    while ((ret = options("v", argv, argc, &arg)) != -2)
    {
	no++;
        switch (ret)
        {
        case 0:
	    inf = fopen (arg, "r");
	    if (!inf)
	    {
		fprintf (stderr, "%s: cannot open %s:%s\n",
			 prog, arg, strerror (errno));
		exit (1);
	    }
	    if (fseek (inf, 0L, SEEK_END))
	    {
		fprintf (stderr, "%s: cannot seek in %s:%s\n",
			 prog, arg, strerror (errno));
		exit (1);
	    }
	    file_size = ftell (inf);    
	    if (fseek (inf, 0L, SEEK_SET))
	    {
		fprintf (stderr, "%s: cannot seek in %s:%s\n",
			 prog, arg, strerror (errno));
		exit (1);
	    }
	    buf = (char *)xmalloc (file_size);
	    if (!buf)
	    {
		fprintf (stderr, "%s: cannot xmalloc: %s\n",
			 prog, strerror (errno));
		exit (1);
	    }
	    if ((long) fread (buf, 1, file_size, inf) != file_size)
	    {
		fprintf (stderr, "%s: cannot read %s: %s\n",
			 prog, arg, strerror (errno));
		exit (1);
	    }
	    for (p = buf; (ret = marc_display_ex (p, stdout, verbose)) > 0;)
	    {
		p += ret;
		count++;
	    }
	    fclose (inf);
	    xfree (buf);
            break;
        case 'v':
	    verbose++;
            break;
        default:
            fprintf (stderr, "Usage: %s [-v] file...\n", prog);
            exit (1);
        }
    }
    if (!no)
    {
	fprintf (stderr, "Usage: %s [-v] file...\n", prog);
	exit (1);
    }
    exit (0);
}
