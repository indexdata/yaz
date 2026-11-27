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

/*
 * Decodes the segments of a constructed octetstring with content length
 * len (negative for indefinite length). The segments are appended to
 * dst, if given, at offset *dlen, and *dlen is advanced past them.
 * Returns 1 on success, 0 on error.
 */
static int ber_oct_segments(ODR o, int len, char *dst, int *dlen, int level)
{
    const char *base = o->op->bp;

    if (o->op->stack_depth + level >= ODR_MAX_STACK)
    {
        odr_seterror(o, OSTACK, 59);
        return 0;
    }
    if (len > odr_max(o))
    {
        odr_seterror(o, OOTHER, 60);
        return 0;
    }
    while (len < 0 || o->op->bp - base < len)
    {
        int res, zclass, tag, cons, slen;

        if (len < 0 && odr_max(o) >= 2 && !o->op->bp[0] && !o->op->bp[1])
        {
            o->op->bp += 2;     /* end of indefinite length content */
            return 1;
        }
        if ((res = ber_dectag(o->op->bp, &zclass, &tag, &cons,
                              odr_max(o))) <= 0
            || zclass != ODR_UNIVERSAL || tag != ODR_OCTETSTRING)
        {
            odr_seterror(o, OPROTO, 61);
            return 0;
        }
        o->op->bp += res;
        if ((res = ber_declen(o->op->bp, &slen, odr_max(o))) < 0)
        {
            odr_seterror(o, OPROTO, 62);
            return 0;
        }
        o->op->bp += res;
        if (cons)
        {
            if (!ber_oct_segments(o, slen, dst, dlen, level + 1))
                return 0;
        }
        else
        {
            if (slen < 0 || slen > odr_max(o))
            {
                odr_seterror(o, OOTHER, 63);
                return 0;
            }
            if (dst)
                memcpy(dst + *dlen, o->op->bp, slen);
            *dlen += slen;
            o->op->bp += slen;
        }
    }
    if (o->op->bp - base != len)
    {
        odr_seterror(o, OCONLEN, 64);
        return 0;
    }
    return 1;
}

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
            int total = 0;
            if (!ber_oct_segments(o, len, 0, &total, 0))
                return 0;
            p->buf = (char *) odr_malloc(o, total + 1);
            p->len = 0;
            o->op->bp = base;
            ber_oct_segments(o, len, p->buf, &p->len, 0);
            p->buf[p->len] = '\0';
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

