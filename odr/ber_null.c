/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_null.c,v $
 * Revision 1.1  1995-02-02 16:21:52  quinn
 * First kick.
 *
 */

#include <odr.h>

/*
 * BER-en/decoder for NULL type.
 */
int ber_null(ODR o, int *val)
{
    switch (o->direction)
    {
    	case ODR_ENCODE:
	    *(o->bp++) = 0X00;
	    o->left--;
	    fprintf(stderr, "[NULL]\n");
	    return 1;
	case ODR_DECODE:
	    if (*(o->bp++) != 0X00)
	    	return 0;
	    o->left--;
	    fprintf(stderr, "[NULL]\n");
	    return 1;
	case ODR_PRINT: return 1;
	default: return 0;
    }
}
