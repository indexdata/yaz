/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: dmalloc.c,v $
 * Revision 1.2  1995-04-10 10:23:50  quinn
 * Fixes.
 *
 * Revision 1.1  1995/03/27  08:35:17  quinn
 * Created util library
 * Added memory debugging module. Imported options-manager
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const unsigned long head = 0xaabbccdd;
static const unsigned long tail = 0x11223344;
static const unsigned long freed = 0xffeeffee;

void *d_malloc(char *file, int line, int nbytes)
{
    char *res;
    int long len;

    if (!(res = malloc(nbytes + 3 * sizeof(long))))
    	return 0;
    fprintf(stderr, "---d_malloc, '%s':%d, %d->%p\n",
    	file, line, nbytes, res + 2 * sizeof(long));
    len = nbytes;
    memcpy(res, &head, sizeof(long));
    memcpy(res + sizeof(long), &len, sizeof(long));
    res += 2 * sizeof(long);
    memcpy(res + nbytes, &tail, sizeof(long));
    return res;
}

void d_free(char *file, int line, char *ptr)
{
    long len;

    if (memcmp(&head, ptr - 2 * sizeof(long), sizeof(long)))
    	abort();
    memcpy(ptr, &freed, sizeof(long));
    memcpy(&len, ptr - sizeof(long), sizeof(long));
    if (memcmp(ptr + len, &tail, sizeof(long)))
    	abort();
    fprintf(stderr, "---d_free, '%s':%d, %p (%d)\n",
    	file, line, ptr, len);
    free(ptr - 2 * sizeof(long));
    return;
}

void *d_realloc(char *file, int line, char *ptr, int nbytes)
{
    long len, nlen = nbytes;
    char *p = ptr;
    char *r;

    if (memcmp(&head, ptr - 2 * sizeof(long), sizeof(long)))
    	abort();
    memcpy(&len, ptr - sizeof(long), sizeof(long));
    if (memcmp(ptr + len, &tail, sizeof(long)))
    	abort();
    if (!(r = realloc(ptr, nbytes + 3 * sizeof(long))))
    	return 0;
    fprintf(stderr, "---d_realloc, '%s':%d, %d->%d, %p->%p\n",
    	file, line, len, nbytes, p, r + 2 * sizeof(long));
    memcpy(r, &head, sizeof(long));
    memcpy(r + sizeof(long), &nlen, sizeof(long));
    if (r != ptr - 2 * sizeof(long))
    {
    	memcpy(r + 2 * sizeof(long), ptr, len);
	memcpy(ptr - 2 * sizeof(long), &freed, sizeof(long));
    }
    r += 2 * sizeof(long);
    memcpy(r + nbytes, &tail, sizeof(long));
    return r;
}
