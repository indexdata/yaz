/*
 * Copyright (c) 1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: atoin.c,v $
 * Revision 1.3  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.2  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.1  1997/09/04 07:52:27  adam
 * Moved atoi_n function to separate source file.
 *
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
        if (isdigit (*buf))
            val = val*10 + (*buf - '0');
	buf++;
    }
    return val;
}
