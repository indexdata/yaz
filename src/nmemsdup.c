/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: nmemsdup.c,v 1.3 2005-01-15 19:47:14 adam Exp $
 */

/**
 * \file nmemsdup.c
 * \brief Implements NMEM dup utilities
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
