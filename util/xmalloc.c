/*
 * Copyright (C) 1994-1999, Index Data
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: xmalloc.c,v $
 * Revision 1.7  1999-07-13 13:24:53  adam
 * Updated memory debugging memory allocatation routines.
 *
 * Revision 1.6  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.5  1997/10/31 12:20:09  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.4  1996/07/03 13:21:36  adam
 * Function xfree_f checks for NULL pointer.
 *
 * Revision 1.3  1995/12/05  15:08:44  adam
 * Fixed verbose of xrealloc.
 *
 * Revision 1.2  1995/12/05  11:08:37  adam
 * More verbose malloc routines.
 *
 * Revision 1.1  1995/11/01  11:56:53  quinn
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

#define TRACE_XMALLOC 0

#if TRACE_XMALLOC > 1

static const unsigned char head[] = {44, 33, 22, 11};
static const unsigned char tail[] = {11, 22, 33, 44};
static const unsigned char freed[] = {11, 22, 33, 44};

struct dmalloc_info {
    int len;
    char file[16];
    int line;
    struct dmalloc_info *next;
    struct dmalloc_info *prev;
};

struct dmalloc_info *dmalloc_list = 0;

void *xmalloc_d(size_t nbytes, const char *file, int line)
{
    char *res;
    struct dmalloc_info *dinfo;
    
    if (!(res = (char*) malloc(nbytes + sizeof(*dinfo)+8*sizeof(char))))
	return 0;
    dinfo = (struct dmalloc_info *) res;
    strncpy (dinfo->file, file, sizeof(dinfo->file)-1);
    dinfo->file[sizeof(dinfo->file)-1] = '\0';
    dinfo->line = line;
    dinfo->len = nbytes;
    
    dinfo->prev = 0;
    dinfo->next = dmalloc_list;
    if (dinfo->next)
	dinfo->next->prev = dinfo;
    dmalloc_list = dinfo;
    
    memcpy(res + sizeof(*dinfo), head, 4*sizeof(char));
    res += sizeof(*dinfo) + 4*sizeof(char);
    memcpy(res + nbytes, tail, 4*sizeof(char));
    return res;
}

void xfree_d(void *ptr, const char *file, int line)
{
    struct dmalloc_info *dinfo;

    if (!ptr)
	return;
    dinfo = (struct dmalloc_info *)
	((char*)ptr - 4*sizeof(char) - sizeof(*dinfo));
    if (memcmp(head, (char*) ptr - 4*sizeof(char), 4*sizeof(char)))
    {
	logf(LOG_FATAL, "xfree_d bad head, %s:%d, %p", file, line, ptr);
        abort();
    }
    if (memcmp((char*) ptr + dinfo->len, tail, 4*sizeof(char)))
    {
	logf(LOG_FATAL, "xfree_d bad tail, %s:%d, %p", file, line, ptr);
        abort();
    }
    if (dinfo->prev)
	dinfo->prev->next = dinfo->next;
    else
	dmalloc_list = dinfo->next;
    if (dinfo->next)
	dinfo->next->prev = dinfo->prev;
    memcpy ((char*) ptr - 4*sizeof(char), freed, 4*sizeof(char));
    free(dinfo);
    return;
}

void *xrealloc_d(void *p, size_t nbytes, const char *file, int line)
{
    struct dmalloc_info *dinfo;
    char *ptr = (char*) p;
    char *res;
    
    if (!ptr)
    {
	if (!nbytes)
	    return 0;
	res = (char *) malloc(nbytes + sizeof(*dinfo) + 8*sizeof(char));
    }
    else
    {
	if (memcmp(head, ptr - 4*sizeof(char), 4*sizeof(char)))
	{
	    logf(LOG_FATAL, "xrealloc_d bad head, %s:%d, %p", file, line, ptr);
	    abort();
	}
	dinfo = (struct dmalloc_info *) (ptr-4*sizeof(char) - sizeof(*dinfo));
	if (memcmp(ptr + dinfo->len, tail, 4*sizeof(char)))
	{
	    logf(LOG_FATAL, "xrealloc_d bad tail, %s:%d, %p", file, line, ptr);
	    abort();
	}
	if (dinfo->prev)
	    dinfo->prev->next = dinfo->next;
	else
	    dmalloc_list = dinfo->next;
	if (dinfo->next)
	    dinfo->next->prev = dinfo->prev;
	
	if (!nbytes)
	{
	    free (dinfo);
	    return 0;
	}
	res = (char *)
	    realloc(dinfo, nbytes + sizeof(*dinfo) + 8*sizeof(char));
    }
    if (!res)
	return 0;
    dinfo = (struct dmalloc_info *) res;
    strncpy (dinfo->file, file, sizeof(dinfo->file)-1);
    dinfo->file[sizeof(dinfo->file)-1] = '\0';
    dinfo->line = line;
    dinfo->len = nbytes;

    dinfo->prev = 0;
    dinfo->next = dmalloc_list;
    if (dmalloc_list)
	dmalloc_list->prev = dinfo;
    dmalloc_list = dinfo;
    
    memcpy(res + sizeof(*dinfo), head, 4*sizeof(char));
    res += sizeof(*dinfo) + 4*sizeof(char);
    memcpy(res + nbytes, tail, 4*sizeof(char));
    return res;
}

void *xcalloc_d(size_t nmemb, size_t size, const char *file, int line)
{
    char *res;
    struct dmalloc_info *dinfo;
    size_t nbytes = nmemb * size;
    
    if (!(res = (char*) calloc(1, nbytes+sizeof(*dinfo)+8*sizeof(char))))
	return 0;
    dinfo = (struct dmalloc_info *) res;
    strncpy (dinfo->file, file, sizeof(dinfo->file)-1);
    dinfo->file[sizeof(dinfo->file)-1] = '\0';
    dinfo->line = line;
    dinfo->len = nbytes;
    
    dinfo->prev = 0;
    dinfo->next = dmalloc_list;
    if (dinfo->next)
	dinfo->next->prev = dinfo;
    dmalloc_list = dinfo;
    
    memcpy(res + sizeof(*dinfo), head, 4*sizeof(char));
    res += sizeof(*dinfo) + 4*sizeof(char);
    memcpy(res + nbytes, tail, 4*sizeof(char));
    return res;
}

void xmalloc_trav_d(const char *file, int line)
{
    size_t size = 0;
    struct dmalloc_info *dinfo = dmalloc_list;
    
    logf (LOG_LOG, "malloc_trav %s:%d", file, line);
    while (dinfo)
    {
	logf (LOG_LOG, " %20s:%d p=%p size=%d", dinfo->file, dinfo->line,
	      dinfo+sizeof(*dinfo)+4*sizeof(char), dinfo->len);
	size += dinfo->len;
	dinfo = dinfo->next;
    }
    logf (LOG_LOG, "total bytes %ld", (long) size);
}

#else
/* ! TRACE_XMALLOC */
#define xrealloc_d(o, x, f, l) realloc(o, x)
#define xmalloc_d(x, f, l) malloc(x)
#define xcalloc_d(x,y, f, l) calloc(x,y)
#define xfree_d(x, f, l) free(x)
#define xmalloc_trav_d(f, l) 
#endif

void xmalloc_trav_f(const char *s, const char *file, int line)
{
    xmalloc_trav_d(file, line);
}

void *xrealloc_f (void *o, size_t size, const char *file, int line)
{
    void *p = xrealloc_d (o, size, file, line);

#if TRACE_XMALLOC
    logf (LOG_DEBUG,
            "%s:%d: xrealloc(s=%d) %p -> %p", file, line, size, o, p);
#endif
    if (!p)
    {
    	logf (LOG_FATAL|LOG_ERRNO, "Out of memory, realloc (%d bytes)", size);
    	exit(1);
    }
    return p;
}

void *xmalloc_f (size_t size, const char *file, int line)
{
    void *p = xmalloc_d (size, file, line);
    
#if TRACE_XMALLOC
    logf (LOG_DEBUG, "%s:%d: xmalloc(s=%d) %p", file, line, size, p);
#endif
    if (!p)
    {
        logf (LOG_FATAL, "Out of memory - malloc (%d bytes)", size);
        exit (1);
    }
    return p;
}

void *xcalloc_f (size_t nmemb, size_t size, const char *file, int line)
{
    void *p = xcalloc_d (nmemb, size, file, line);
#if TRACE_XMALLOC
    logf (LOG_DEBUG, "%s:%d: xcalloc(s=%d) %p", file, line, size, p);
#endif
    if (!p)
    {
        logf (LOG_FATAL, "Out of memory - calloc (%d, %d)", nmemb, size);
        exit (1);
    }
    return p;
}

char *xstrdup_f (const char *s, const char *file, int line)
{
    char *p = (char *)xmalloc_d (strlen(s)+1, file, line);
#if TRACE_XMALLOC
    logf (LOG_DEBUG, "%s:%d: xstrdup(s=%d) %p", file, line, strlen(s)+1, p);
#endif
    strcpy (p, s);
    return p;
}

void xfree_f(void *p, const char *file, int line)
{
    if (!p)
        return ;
#if TRACE_XMALLOC
    if (p)
        logf (LOG_DEBUG, "%s:%d: xfree %p", file, line, p);
#endif
    xfree_d(p, file, line);
}
