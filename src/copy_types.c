/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/** \file copy_types.c
    \brief Copies various Z39.50 types
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/copy_types.h>

/** macro clone_z_type copies a given ASN.1 type */
#define clone_z_type(x) \
Z_##x *yaz_clone_z_##x(Z_##x *q, NMEM nmem_out) \
{ \
    Z_##x *q1 = 0; \
    ODR enc = odr_createmem(ODR_ENCODE); \
    ODR dec = odr_createmem(ODR_DECODE); \
    if (z_##x(enc, &q, 0, 0)) \
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
} \
int yaz_compare_z_##x(Z_##x *a, Z_##x *b) \
{ \
    int ret = 0; \
    ODR o_a = odr_createmem(ODR_ENCODE); \
    ODR o_b = odr_createmem(ODR_ENCODE); \
    int r_a = z_##x(o_a, &a, 1, 0); \
    int r_b = z_##x(o_b, &b, 1, 0); \
    if (r_a && r_b) \
    { \
        int len_a, len_b; \
        char *buf_a = odr_getbuf(o_a, &len_a, 0); \
        char *buf_b = odr_getbuf(o_b, &len_b, 0); \
        if (buf_a && buf_b && len_a == len_b && !memcmp(buf_a, buf_b, len_a)) \
            ret = 1; \
        else if (!buf_a && !buf_b) \
            ret = 1; \
    } \
    odr_destroy(o_a); \
    odr_destroy(o_b); \
    return ret; \
}

clone_z_type(NamePlusRecord)
clone_z_type(RPNQuery)
clone_z_type(Query)
clone_z_type(RecordComposition)
clone_z_type(OtherInformation)

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

