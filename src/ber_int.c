/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/** 
 * \file ber_int.c
 * \brief Implements BER INTEGER encoding and decoding.
 *
 * This source file implements BER encoding and decoding of
 * the INTEGER type.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef WIN32
#include <winsock.h>
#endif

#include "odr-priv.h"

static int ber_encinteger(ODR o, Odr_int val);
static int ber_decinteger(const unsigned char *buf, Odr_int *val, int max);

int ber_integer(ODR o, Odr_int *val)
{
    int res;

    switch (o->direction)
    {
    case ODR_DECODE:
        if ((res = ber_decinteger(o->bp, val, odr_max(o))) <= 0)
        {
            odr_seterror(o, OPROTO, 50);
            return 0;
        }
        o->bp += res;
        return 1;
    case ODR_ENCODE:
        if ((res = ber_encinteger(o, *val)) < 0)
            return 0;
        return 1;
    case ODR_PRINT:
        return 1;
    default:
        odr_seterror(o, OOTHER, 51);  return 0;
    }
}

/*
 * Returns: number of bytes written or -1 for error (out of bounds).
 */
int ber_encinteger(ODR o, Odr_int val)
{
    unsigned char tmp[sizeof(Odr_int)];
    unsigned long long uval = val;
    int i, len;
    for (i = sizeof(uval); i > 0; )
    {
        tmp[--i] = uval;
        uval >>= 8;
    }
    for (i = 0; i < sizeof(Odr_int)-1; i++)
        if (!((tmp[i] == 0 && !(tmp[i+1] & 0x80))
              ||
              (tmp[i] == 0xFF && (tmp[i+1] & 0x80))))
            break;
    len = sizeof(Odr_int) - i;
    if (ber_enclen(o, len, 1, 1) != 1)
        return -1;
    if (odr_write(o, (unsigned char*) tmp + i, len) < 0)
        return -1;
    return 0;
}

/*
 * Returns: Number of bytes read or 0 if no match, -1 if error.
 */
int ber_decinteger(const unsigned char *buf, Odr_int *val, int max)
{
    unsigned long long uval = 0;
    int i, len;
    int res;
    const unsigned char *b = buf;

    if ((res = ber_declen(b, &len, max)) < 0)
        return -1;
    if (len+res > max || len < 0) /* out of bounds or indefinite encoding */
        return -1;  
    if (len > (int) sizeof(Odr_int))  /* let's be reasonable, here */
        return -1;
    b += res;

    if (*b & 0x80)
        for (; i < sizeof(uval) - len; i++)
            uval = (uval << 8) + 0xFF;
    for (i = 0; i < len; i++)
        uval = (uval << 8) + b[i];
    *val = uval;
    b += len;
    return b - buf;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

