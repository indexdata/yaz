/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file odr_oid.c
 * \brief Implements ODR OID codec
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/oid_util.h>
#include "odr-priv.h"

/*
 * Top level oid en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_oid(ODR o, Odr_oid **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
        return 0;
    if (o->op->t_class < 0)
    {
        o->op->t_class = ODR_UNIVERSAL;
        o->op->t_tag = ODR_OID;
    }
    res = ber_tag(o, p, o->op->t_class, o->op->t_tag, &cons, opt, name);
    if (res < 0)
        return 0;
    if (!res)
        return odr_missing(o, opt, name);
    if (cons)
    {
        odr_seterror(o, OPROTO, 46);
        return 0;
    }
    if (o->direction == ODR_PRINT)
    {
        int i;

        odr_prname(o, name);
        odr_printf(o, "OID:");
        for (i = 0; (*p)[i] > -1; i++)
            odr_printf(o, " %d", (*p)[i]);
        odr_printf(o, "\n");
        return 1;
    }
    if (o->direction == ODR_DECODE)
        *p = (Odr_oid *)odr_malloc(o, OID_SIZE * sizeof(**p));
    return ber_oidc(o, *p, OID_SIZE);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

