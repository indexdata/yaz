/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: dmalloc.h,v $
 * Revision 1.1  1995-03-30 09:39:40  quinn
 * Moved .h files to include directory
 *
 * Revision 1.1  1995/03/27  08:35:18  quinn
 * Created util library
 * Added memory debugging module. Imported options-manager
 *
 *
 */

#ifndef DMALLOC_H
#define DMALLOC_H

#ifdef DEBUG_MALLOC

#ifdef malloc
#undef malloc
#endif
#ifdef free
#undef free
#endif
#ifdef realloc
#undef realloc
#endif
#define malloc(n) d_malloc(__FILE__, __LINE__, (n))
#define free(p) d_free(__FILE__, __LINE__, (p))
#define realloc(p, n) d_realloc(__FILE__, __LINE__, (p), (n))

void *d_malloc(char *file, int line, int nbytes);
void d_free(char *file, int line, char *ptr);
void *d_realloc(char *file, int line, char *ptr, int nbytes);

#endif

#endif
