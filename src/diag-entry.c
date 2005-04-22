/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: diag-entry.c,v 1.1 2005-04-22 08:27:58 adam Exp $
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
