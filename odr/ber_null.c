/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: ber_null.c,v 1.14 2003-03-11 11:03:31 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

/*
 * BER-en/decoder for NULL type.
 */
int ber_null(ODR o)
{
    switch (o->direction)
    {
    case ODR_ENCODE:
        if (odr_putc(o, 0X00) < 0)
            return 0;
#ifdef ODR_DEBUG
        fprintf(stderr, "[NULL]\n");
#endif
        return 1;
    case ODR_DECODE:
        if (odr_max(o) < 1)
        {
            odr_seterror(o, OPROTO, 39);
            return 0;
        }
        if (*(o->bp++) != 0X00)
        {
            odr_seterror(o, OPROTO, 12);
            return 0;
        }
#ifdef ODR_DEBUG
        fprintf(stderr, "[NULL]\n");
#endif
        return 1;
    case ODR_PRINT: return 1;
    default: odr_seterror(o, OOTHER, 13); return 0;
    }
}
