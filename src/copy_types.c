/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/** \file copy_types.c
    \brief Copies various Z39.50 types
 */

#include <yaz/copy_types.h>

Z_RPNQuery *yaz_copy_z_RPNQuery(Z_RPNQuery *q, ODR out)
{
    Z_RPNQuery *q1 = 0;
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    if (!z_RPNQuery(enc, &q, 0, 0))
        return 0;
    else
    {
        int len;
        char *buf = odr_getbuf(enc, &len, 0);
        if (buf)
        {
            odr_setbuf(dec, buf, len, 0);
            z_RPNQuery(dec, &q1, 0, 0);
            nmem_transfer(out->mem, dec->mem);
        }
    }
    odr_destroy(enc);
    odr_destroy(dec);
    return q1;
}

Z_Query *yaz_copy_Z_Query(Z_Query *q, ODR out)
{
    Z_Query *q1 = 0;
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    if (!z_Query(enc, &q, 0, 0))
        return 0;
    else
    {
        int len;
        char *buf = odr_getbuf(enc, &len, 0);
        if (buf)
        {
            odr_setbuf(dec, buf, len, 0);
            z_Query(dec, &q1, 0, 0);
            nmem_transfer(out->mem, dec->mem);
        }
    }
    odr_destroy(enc);
    odr_destroy(dec);
    return q1;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

