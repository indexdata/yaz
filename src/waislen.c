/*
 * Copyright (c) 1995-2004, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: waislen.c,v 1.2 2004-10-15 00:19:01 adam Exp $
 */
/**
 * \file waislen.c
 * \brief Implements WAIS package handling
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
