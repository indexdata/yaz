/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: ber_bool.c,v 1.1 2003-10-27 12:21:30 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "odr-priv.h"

int ber_boolean(ODR o, int *val)
{
    int res, len;

    switch (o->direction)
    {
    case ODR_ENCODE:
        if (ber_enclen(o, 1, 1, 1) != 1)
            return 0;
        if (odr_putc(o, *val) < 0)
            return 0;
#ifdef ODR_DEBUG
        fprintf(stderr, "[val=%d]\n", *val);
#endif
        return 1;
    case ODR_DECODE:
        if ((res = ber_declen(o->bp, &len, odr_max(o))) < 0)
        {
            odr_seterror(o, OPROTO, 9);
            return 0;
        }
        o->bp+= res;
        if (len != 1 || odr_max(o) < len)
        {
            odr_seterror(o, OPROTO, 10);
            return 0;
        }
        *val = *o->bp;
        o->bp++;
#ifdef ODR_DEBUG
        fprintf(stderr, "[val=%d]\n", *val);
#endif
        return 1;
    case ODR_PRINT:
        return 1;
    default: odr_seterror(o, OOTHER, 11); return 0;
    }
}
