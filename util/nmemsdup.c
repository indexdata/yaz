/*
 * Copyright (c) 1997-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: nmemsdup.c,v 1.6 2003-03-18 13:34:37 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/nmem.h>

char *nmem_strdup (NMEM mem, const char *src)
{
    char *dst = (char *)nmem_malloc (mem, strlen(src)+1);
    strcpy (dst, src);
    return dst;
}

char *nmem_strdupn (NMEM mem, const char *src, size_t n)
{
    char *dst = (char *)nmem_malloc (mem, n+1);
    memcpy (dst, src, n);
    dst[n] = '\0';
    return dst;
}

int *nmem_intdup(NMEM mem, int v)
{
    int *dst = (int*) nmem_malloc (mem, sizeof(int));
    *dst = v;
    return dst;
}
