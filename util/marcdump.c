/*
 * Copyright (c) 1995-2001, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: marcdump.c,v $
 * Revision 1.13  2001-02-10 01:21:59  adam
 * Dumper only keeps one record at a time in memory.
 *
 * Revision 1.12  2000/10/02 11:07:45  adam
 * Added peer_name member for bend_init handler. Changed the YAZ
 * client so that tcp: can be avoided in target spec.
 *
 * Revision 1.11  2000/07/04 08:53:22  adam
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
#include <yaz/yaz-util.h>
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
    int r;
    char *arg;
    int verbose = 0;
    FILE *inf;
    char buf[100001];
    char *prog = *argv;
    int no = 0;
    FILE *cfile = 0;

    while ((r = options("vc:", argv, argc, &arg)) != -2)
    {
	int count;
	no++;
        switch (r)
        {
	case 'c':
	    if (cfile)
		fclose (cfile);
	    cfile = fopen (arg, "w");
	    break;
        case 0:
	    inf = fopen (arg, "r");
	    count = 0;
	    if (!inf)
	    {
		fprintf (stderr, "%s: cannot open %s:%s\n",
			 prog, arg, strerror (errno));
		exit (1);
	    }
	    if (cfile)
		fprintf (cfile, "char *marc_records[] = {\n");
	    while (1)
	    {
		int len;
		
		r = fread (buf, 1, 5, inf);
		if (r < 5)
		    break;
		len = atoi_n(buf, 5);
		if (len < 25 || len > 100000)
		    break;
		len = len - 5;
		r = fread (buf + 5, 1, len, inf);
		if (r < len)
		    break;
		r = marc_display_ex (buf, stdout, verbose);
		if (r <= 0)
		    break;
		if (cfile)
		{
		    char *p = buf;
		    int i;
		    if (count)
			fprintf (cfile, ",");
		    fprintf (cfile, "{\n");
		    for (i = 0; i < r; i++)
		    {
			if ((i & 15) == 0)
			    fprintf (cfile, "  \"");
			fprintf (cfile, "\\x%02X", p[i] & 255);
			
			if (i < r - 1 && (i & 15) == 15)
			    fprintf (cfile, "\"\n");
			
			}
		    fprintf (cfile, "\"\n}");
		}
		count++;
	    }
	    if (cfile)
		fprintf (cfile, "};\n");
            break;
        case 'v':
	    verbose++;
            break;
        default:
            fprintf (stderr, "Usage: %s [-c cfile] [-v] file...\n", prog);
            exit (1);
        }
    }
    if (cfile)
	fclose (cfile);
    if (!no)
    {
	fprintf (stderr, "Usage: %s [-v] file...\n", prog);
	exit (1);
    }
    exit (0);
}
