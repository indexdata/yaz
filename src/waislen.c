/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: waislen.c,v $
 * Revision 1.1  2003-10-27 12:21:36  adam
 * Source restructure. yaz-marcdump part of installation
 *
 * Revision 1.5  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.4  1999/01/08 11:23:15  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.3  1996/02/29 14:23:13  adam
 * Bug fix.
 *
 * Revision 1.2  1996/02/26  18:34:44  adam
 * Bug fix.
 *
 * Revision 1.1  1996/02/20  13:02:58  quinn
 * Wais length.
 *
 *
 */

#include <stdio.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>
/*
 * Return length of WAIS package or 0
 */
int completeWAIS(const unsigned char *buf, int len)
{
    int i, lval = 0;

    if (len < 25)
	return 0;
    if (*buf != '0')
	return 0;
    /* calculate length */
    for (i = 0; i < 10; i++)
	lval = lval * 10 + (buf[i] - '0');
    lval += 25;
    if (len >= lval)
	return lval;
    return 0;
}
