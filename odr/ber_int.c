/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_int.c,v $
 * Revision 1.3  1995-02-09 15:51:46  quinn
 * Works better now.
 *
 * Revision 1.2  1995/02/07  17:52:58  quinn
 * A damn mess, but now things work, I think.
 *
 * Revision 1.1  1995/02/02  16:21:52  quinn
 * First kick.
 *
 */

#include <odr.h>
#include <netinet/in.h>
#include <string.h>

static int ber_encinteger(unsigned char *buf, int val, int maxlen);
static int ber_decinteger(unsigned char *buf, int *val);

int ber_integer(ODR o, int *val)
{
    int res;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = ber_decinteger(o->bp, val)) <= 0)
	    	return 0;
	    o->bp += res;
	    o->left -= res;
	    return 1;
    	case ODR_ENCODE:
	    if ((res = ber_encinteger(o->bp, *val, o->left)) <= 0)
	    	return 0;
	    o->bp += res;
	    o->left -= res;
	    return 1;
    	case ODR_PRINT: return 1;
    	default:  return 0;
    }
}

/*
 * Returns: number of bytes written or -1 for error (out of bounds).
 */
int ber_encinteger(unsigned char *buf, int val, int maxlen)
{
    unsigned char *b = buf, *lenpos;
    int a, len;
    union { int i; unsigned char c[sizeof(int)]; } tmp;

    lenpos = b;
    maxlen--;
    b++;

    tmp.i = htonl(val);   /* ensure that that we're big-endian */
    for (a = 0; a < sizeof(int) - 1; a++)  /* skip superfluous octets */
    	if (!((tmp.c[a] == 0 && !(tmp.c[a+1] & 0X80)) ||
	    (tmp.c[a] == 0XFF && (tmp.c[a+1] & 0X80))))
	    break;
    if ((len = sizeof(int) - a) > maxlen)
    	return -1;
    memcpy(b, tmp.c + a, len);
    b += len;
    if (ber_enclen(lenpos, len, 1, 1) != 1)
    	return -1;
#ifdef ODR_DEBUG
    fprintf(stderr, "[val=%d]", val);
#endif
    return b - buf;
}

/*
 * Returns: Number of bytes read or 0 if no match, -1 if error.
 */
int ber_decinteger(unsigned char *buf, int *val)
{
    unsigned char *b = buf, fill;
    int res, len, remains;
    union { int i; unsigned char c[sizeof(int)]; } tmp;

    if ((res = ber_declen(b, &len)) < 0)
    	return -1;
    if (len > sizeof(int))    /* let's be reasonable, here */
    	return -1;
    b+= res;

    remains = sizeof(int) - len;
    memcpy(tmp.c + remains, b, len);
    if (*b & 0X80)
    	fill = 0XFF;
    else
    	fill = 0X00;
    memset(tmp.c, fill, remains);
    *val = ntohl(tmp.i);

    b += len;
#ifdef ODR_DEBUG
    fprintf(stderr, "[val=%d]", *val);
#endif
    return b - buf;
}
