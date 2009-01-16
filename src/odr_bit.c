/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file odr_bit.c
 * \brief Implements ODR BITSTRING codec
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include "odr-priv.h"

/*
 * Top level bitstring string en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_bitstring(ODR o, Odr_bitmask **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
        return 0;
    if (o->op->t_class < 0)
    {
        o->op->t_class = ODR_UNIVERSAL;
        o->op->t_tag = ODR_BITSTRING;
    }
    res = ber_tag(o, p, o->op->t_class, o->op->t_tag, &cons, opt, name);
    if (res < 0)
        return 0;
    if (!res)
        return odr_missing(o, opt, name);
    if (o->direction == ODR_PRINT)
    {
        int i = ODR_BITMASK_SIZE;
        int j;
        odr_prname(o, name);
        odr_printf(o, "BITSTRING(len=%d) ",(*p)->top + 1);
        while (--i > 0)
            if (ODR_MASK_GET(*p, i))
                break;
        for (j = 0; j <= i; j++)
        {
            odr_printf(o, "%c", ODR_MASK_GET(*p, j) ? '1' : '0');
            if (j && ((j+1)&7) == 0)
                odr_printf(o, "-");
        }
        odr_printf(o, "\n");
        return 1;
    }
    if (o->direction == ODR_DECODE)
    {
        *p = (Odr_bitmask *)odr_malloc(o, sizeof(Odr_bitmask));
        memset((*p)->bits, 0, ODR_BITMASK_SIZE);
        (*p)->top = -1;
    }
#if 0
    /* ignoring the cons helps with at least one target. 
     * http://bugzilla.indexdata.dk/cgi-bin/bugzilla/show_bug.cgi?id=24
     */
    return ber_bitstring(o, *p, 0);
#else
    return ber_bitstring(o, *p, cons);
#endif
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

