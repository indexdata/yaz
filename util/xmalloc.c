/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: xmalloc.c,v $
 * Revision 1.1  1995-11-01 11:56:53  quinn
 * Added Xmalloc.
 *
 * Revision 1.6  1995/10/16  14:03:11  quinn
 * Changes to support element set names and espec1
 *
 * Revision 1.5  1995/09/04  12:34:06  adam
 * Various cleanup. YAZ util used instead.
 *
 * Revision 1.4  1994/10/05  10:16:16  quinn
 * Added xrealloc. Fixed bug in log.
 *
 * Revision 1.3  1994/09/26  16:31:37  adam
 * Added xcalloc_f.
 *
 * Revision 1.2  1994/08/18  08:23:26  adam
 * Res.c now use handles. xmalloc defines xstrdup.
 *
 * Revision 1.1  1994/08/17  13:37:54  adam
 * xmalloc.c added to util.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log.h>
#include <xmalloc.h>
#include <dmalloc.h>

void *xrealloc_f (void *o, size_t size, char *file, int line)
{
    void *p;

#ifdef TRACE_XMALLOC
    fprintf(stderr, "%s:%d: xrealloc(s=%d)\n", file, line, size);
#endif
    p = realloc (o, size);
    if (!p)
    {
    	logf (LOG_FATAL|LOG_ERRNO, "Out of memory, realloc (%d bytes)", size);
    	exit(1);
    }
    return p;
}

void *xmalloc_f (size_t size, char *file, int line)
{
    void *p;

#ifdef TRACE_XMALLOC
    fprintf(stderr, "%s:%d: xmalloc(s=%d)\n", file, line, size);
#endif
    p = malloc (size);
    if (!p)
    {
        logf (LOG_FATAL, "Out of memory - malloc (%d bytes)", size);
        exit (1);
    }
    return p;
}

void *xcalloc_f (size_t nmemb, size_t size, char *file, int line)
{
    void *p;
#ifdef TRACE_XMALLOC
    fprintf(stderr, "%s:%d: xcalloc(s=%d)\n", file, line, size);
#endif
    p = calloc (nmemb, size);
    if (!p)
    {
        logf (LOG_FATAL, "Out of memory - calloc (%d, %d)", nmemb, size);
        exit (1);
    }
    return p;
}

char *xstrdup_f (const char *s, char *file, int line)
{
    char *p = xmalloc (strlen(s)+1);
#ifdef TRACE_XMALLOC
    fprintf(stderr, "%s:%d: xstrdup(s=%d)\n", file, line, strlen(s)+1);
#endif
    strcpy (p, s);
    return p;
}


void xfree_f(void *p, char *file, int line)
{
#ifdef TRACE_XMALLOC
    fprintf(stderr, "%s:%d: xfree\n", file, line);
#endif
    free(p);
}
