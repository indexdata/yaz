/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: marcdump.c,v $
 * Revision 1.4  1995-11-01 13:55:05  quinn
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <marcdisp.h>
#include <xmalloc.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
 
int main (int argc, char **argv)
{
    FILE *inf;
    long file_size;
    char *buf;
    int r;

    if (argc < 2)
    {
        fprintf (stderr, "usage\n%s <file>\n", *argv);
	exit (1);
    }
    inf = fopen (argv[1], "r");
    if (!inf)
    {
        fprintf (stderr, "%s: cannot open %s:%s\n",
		 *argv, argv[1], strerror (errno));
        exit (1);
    }
    if (fseek (inf, 0L, SEEK_END))
    {
        fprintf (stderr, "%s: cannot seek in %s:%s\n",
		 *argv, argv[1], strerror (errno));
        exit (1);
    }
    file_size = ftell (inf);    
    if (fseek (inf, 0L, SEEK_SET))
    {
        fprintf (stderr, "%s: cannot seek in %s:%s\n",
		 *argv, argv[1], strerror (errno));
        exit (1);
    }
    buf = xmalloc (file_size);
    if (!buf)
    {
        fprintf (stderr, "%s: cannot xmalloc: %s\n",
		 *argv, strerror (errno));
        exit (1);
    }
    if (fread (buf, 1, file_size, inf) != file_size)
    {
        fprintf (stderr, "%s: cannot read %s: %s\n",
		 *argv, argv[1], strerror (errno));
        exit (1);
    }
    while ((r = marc_display (buf, stdout)) > 0)
        buf += r;
    exit (0);
}
