/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file ber_oct.c
 * \brief Implements ber_octetstring
 *
 * This source file implements BER encoding and decoding of
 * the OCTETSTRING type.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"
#include <yaz/log.h>
#include <assert.h>

int ber_octetstring(ODR o, Odr_oct *p, int cons)
{
    int res, len;
    const char *base;

    switch (o->direction)
    {
    case ODR_DECODE:
        if ((res = ber_declen(o->op->bp, &len, odr_max(o))) < 0)
        {
            odr_seterror(o, OPROTO, 14);
            return 0;
        }
        o->op->bp += res;
        if (cons)       /* fetch component strings */
        {
            base = o->op->bp;
            while (odp_more_chunks(o, base, len))
                if (!odr_octetstring(o, &p, 0, 0))
                    return 0;
            return 1;
        }
        /* primitive octetstring */
        if (len < 0)
        {
            odr_seterror(o, OOTHER, 15);
            return 0;
        }
        if (len > odr_max(o))
        {
            odr_seterror(o, OOTHER, 16);
            return 0;
        }
        p->len = len;
        p->buf = odr_strdupn(o, o->op->bp, len);
        o->op->bp += len;
        return 1;
    case ODR_ENCODE:
        if ((res = ber_enclen(o, p->len, 5, 0)) < 0)
            return 0;
        if (p->len == 0)
            return 1;
        if (odr_write(o, p->buf, p->len) < 0)
            return 0;
        return 1;
    case ODR_PRINT:
        return 1;
    default:
        odr_seterror(o, OOTHER, 17);
        return 0;
    }
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

