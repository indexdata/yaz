/*
 * Copyright (c) 1997, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: nmemsdup.c,v $
 * Revision 1.2  1998-02-11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.1  1997/09/17 12:10:42  adam
 * YAZ version 1.4.
 *
 */

#include <string.h>
#include <nmem.h>

char *nmem_strdup (NMEM mem, const char *src)
{
    char *dst = (char *)nmem_malloc (mem, strlen(src)+1);
    strcpy (dst, src);
    return dst;
}
