/*
 * Copyright (c) 1997-2004, Index Data
 * See the file LICENSE for details.
 *
 * $Id: atoin.c,v 1.2 2004-08-13 11:35:37 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <ctype.h>
#include <yaz/yaz-util.h>

int atoi_n (const char *buf, int len)
{
    int val = 0;

    while (--len >= 0)
    {
        if (isdigit (*(const unsigned char *) buf))
            val = val*10 + (*buf - '0');
	buf++;
    }
    return val;
}

