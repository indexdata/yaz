/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file odr_util.c
 * \brief Implements various ODR utilities
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include "odr-priv.h"
#include <yaz/oid_util.h>

void odr_prname(ODR o, const char *name)
{
    if (o->op->indent < 16)
        odr_printf(o, "%*s", o->op->indent * 2, "");
    else
        odr_printf(o, "level=%-7d%*s", o->op->indent,
                   2 * (o->op->indent % 8) , "");
    if (name)
        odr_printf(o, "%s ", name);
}

int odp_more_chunks(ODR o, const char *base, int len)
{
    if (!len)
        return 0;
    if (len < 0) /* indefinite length */
    {
        if (*o->op->bp == 0 && *(o->op->bp + 1) == 0)
        {
            o->op->bp += 2;
            return 0;
        }
        else
            return 1;
    }
    else
        return o->op->bp - base < len;
}

Odr_oid *odr_oiddup_nmem(NMEM nmem, const Odr_oid *o)
{
    Odr_oid *r;

    if (!o)
        return 0;
    if (!(r = (Odr_oid *)
          nmem_malloc(nmem, (oid_oidlen(o) + 1) * sizeof(Odr_oid))))
        return 0;
    oid_oidcpy(r, o);
    return r;
}

Odr_oid *odr_oiddup(ODR odr, const Odr_oid *o)
{
    return odr_oiddup_nmem(odr_getmem(odr), o);
}

Odr_oid *odr_getoidbystr_nmem(NMEM nmem, const char *str)
{
    Odr_oid oid[OID_SIZE];
    Odr_oid *ret;

    if (oid_dotstring_to_oid(str, oid))
        return 0;
    ret = (Odr_oid *)nmem_malloc(nmem, sizeof(*ret)*(oid_oidlen(oid) + 1));
    oid_oidcpy(ret, oid);
    return ret;
}

Odr_oid *odr_getoidbystr(ODR o, const char *str)
{
    return odr_getoidbystr_nmem(odr_getmem(o), str);
}

int odr_missing(ODR o, int opt, const char *name)
{
    if (o->error)
        return 0;
    if (!opt)
    {
        odr_seterror(o, OREQUIRED, 53);
        odr_setelement(o, name);
    }
    return opt;
}

/*
 * Reallocate the buffer `old', using the ODR memory pool `o' to be
 * big enough to hold its existing value (if any) plus `prefix' (if
 * any) and a separator character.  Copy `prefix', a forward slash and
 * the old value into the new area and return its address.  Can be
 * used as follows:
 *      initRequest->implementationName = odr_prepend(o,
 *              initRequest->implementationName, "ZOOM-C");
 */
char *odr_prepend(ODR o, const char *prefix, const char *old)
{
    int plen = (prefix == 0) ? 0 : strlen(prefix);
    int olen = (old == 0) ? 0 : strlen(old);
    char *res = (char*) odr_malloc (o, olen + plen + 2);

    *res = '\0';
    if (plen > 0)
        strcpy(res, prefix);
    if (plen > 0 && old != 0)
        strcat(res, "/");
    if (old != 0)
        strcat(res, old);
    return res;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

