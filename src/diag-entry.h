/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: diag-entry.h,v 1.1 2005-04-22 08:27:58 adam Exp $
 */

struct yaz_diag_entry {
    int code;
    char *msg;
};

const char *yaz_diag_to_str(struct yaz_diag_entry *tab, int code);
