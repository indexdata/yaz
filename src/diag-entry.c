/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: diag-entry.c,v 1.2 2005-06-25 15:46:04 adam Exp $
 */

#include "diag-entry.h"

const char *yaz_diag_to_str(struct yaz_diag_entry *tab, int code)
{
    int i;
    for (i=0; tab[i].msg; i++)
        if (tab[i].code == code)
            return tab[i].msg;
    return "Unknown error";
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

