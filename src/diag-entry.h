/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: diag-entry.h,v 1.2 2005-06-25 15:46:04 adam Exp $
 */

struct yaz_diag_entry {
    int code;
    char *msg;
};

const char *yaz_diag_to_str(struct yaz_diag_entry *tab, int code);
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

