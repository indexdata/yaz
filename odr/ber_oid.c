/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: ber_oid.c,v 1.14 2003-01-06 08:20:27 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

int ber_oidc(ODR o, Odr_oid *p)
{
    int len, lenp, end;
    int pos, n, res, id;
    unsigned char octs[8];

    switch (o->direction)
    {
    case ODR_DECODE:
        if ((res = ber_declen(o->bp, &len)) < 1)
        {
            o->error = OPROTO;
            return 0;
        }
        if (len < 0)
        {
            o->error = OPROTO;
            return 0;
        }
        o->bp += res;
        if (len == 0)
        {
            *p = -1;
            return 1;
        }
        p[0] = *o->bp / 40;
        if (p[0] > 2)
            p[0] = 2;
        p[1] = *o->bp - p[0] * 40;
        o->bp++;
        pos = 2;
        len--;
        while (len)
        {
            p[pos] = 0;
            do
            {
                if (!len)
                {
                    o->error = OPROTO;
                    return 0;
                }
                p[pos] <<= 7;
                p[pos] |= *o->bp & 0X7F;
                len--;
            }
            while (*(o->bp++) & 0X80);
            pos++;
        }
        p[pos] = -1;
        return 1;
    case ODR_ENCODE:
        /* we'll allow ourselves the quiet luxury of only doing encodings
           shorter than 127 */
        lenp = odr_tell(o);
        if (odr_putc(o, 0) < 0)   /* dummy */
            return 0;
        if (p[0] < 0 && p[1] <= 0)
        {
            o->error = ODATA;
            return 0;
        }
        for (pos = 1; p[pos] >= 0; pos++)
        {
            id = pos > 1 ? p[pos] : p[0] * 40 + p[1];
            n = 0;
            do
            {
                octs[n++] = id & 0X7F;
                id >>= 7;
            }
            while (id);
            while (n--)
            {
                unsigned char p;

                p = octs[n] | ((n > 0) << 7);
                if (odr_putc(o, p) < 0)
                    return 0;
            }
        }
        end = odr_tell(o);
        odr_seek(o, ODR_S_SET, lenp);
        if (ber_enclen(o, (end - lenp) - 1, 1, 1) != 1)
        {
            o->error = OOTHER;
            return 0;
        }
        odr_seek(o, ODR_S_END, 0);
        return 1;
    default: 
        o->error = OOTHER; return 0;
    }
}
