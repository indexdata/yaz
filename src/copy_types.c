/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/** \file copy_types.c
    \brief Copies various Z39.50 types
 */

#include <yaz/copy_types.h>

/** macro clone_z_type copies a given ASN.1 type */
#define clone_z_type(x) \
Z_##x *yaz_clone_z_##x(Z_##x *q, NMEM nmem_out) \
{ \
    Z_##x *q1 = 0; \
    ODR enc = odr_createmem(ODR_ENCODE); \
    ODR dec = odr_createmem(ODR_DECODE); \
    if (!z_##x(enc, &q, 0, 0)) \
        return 0; \
    else \
    { \
        int len; \
        char *buf = odr_getbuf(enc, &len, 0); \
        if (buf) \
        { \
            odr_setbuf(dec, buf, len, 0); \
            z_##x(dec, &q1, 0, 0); \
            nmem_transfer(nmem_out, dec->mem);  \
        } \
    } \
    odr_destroy(enc); \
    odr_destroy(dec); \
    return q1; \
}

clone_z_type(NamePlusRecord)
clone_z_type(RPNQuery)
clone_z_type(Query)
clone_z_type(RecordComposition)

Z_RPNQuery *yaz_copy_z_RPNQuery(Z_RPNQuery *q, ODR out)
{
    return yaz_clone_z_RPNQuery(q, out->mem);
}

Z_Query *yaz_copy_Z_Query(Z_Query *q, ODR out)
{
    return yaz_clone_z_Query(q, out->mem);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

