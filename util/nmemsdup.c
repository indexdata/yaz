/*
 * Copyright (c) 1997-2001, Index Data.
 * See the file LICENSE for details.
 *
 * $Log: nmemsdup.c,v $
 * Revision 1.5  2001-03-25 21:55:13  adam
 * Added odr_intdup. Ztest server returns TaskPackage for ItemUpdate.
 *
 * Revision 1.4  2000/02/29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.3  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.2  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.1  1997/09/17 12:10:42  adam
 * YAZ version 1.4.
 *
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

int *nmem_intdup(NMEM mem, int v)
{
    int *dst = (int*) nmem_malloc (mem, sizeof(int));
    *dst = v;
    return dst;
}
