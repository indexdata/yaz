/*
 * Copyright (c) 2002-2004, Index Data
 * See the file LICENSE for details.
 *
 * $Id: tstnmem.c,v 1.3 2005-01-05 10:29:42 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <yaz/nmem.h>

int main (int argc, char **argv)
{
    void *cp;
    NMEM n;
    int j;

    nmem_init();
    n = nmem_create();
    if (!n)
        exit (1);
    for (j = 1; j<500; j++)
    {
        cp = nmem_malloc(n, j);
        if (!cp)
            exit(2);
	if (sizeof(long) >= j)
	    *(long*) cp = 123L;
#if HAVE_LONG_LONG
	if (sizeof(long long) >= j)
	    *(long long*) cp = 123L;
#endif
	if (sizeof(double) >= j)
	    *(double*) cp = 12.2;
    }
    
    for (j = 2000; j<20000; j+= 2000)
    {
        cp = nmem_malloc(n, j);
        if (!cp)
            exit(3);
    }
    nmem_destroy(n);
    nmem_exit();
    exit(0);
}
