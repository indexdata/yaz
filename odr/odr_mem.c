/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_mem.c,v $
 * Revision 1.1  1995-03-14 10:27:40  quinn
 * Modified makefile to use common lib
 * Beginning to add memory management to odr
 *
 *
 */

#include <stdlib.h>

#include <odr.h>

char *odr_malloc(ODR o, int size)
{
    return malloc(size); /* REPLACE WITH NIBBLE MALLOC!! */
}
