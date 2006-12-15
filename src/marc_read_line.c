/*
 * Copyright (C) 1995-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: marc_read_line.c,v 1.1 2006-12-15 12:37:18 adam Exp $
 */

/**
 * \file marc_read_iso2709.c
 * \brief Implements reading of MARC as ISO2709
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <yaz/marcdisp.h>
#include <yaz/wrbuf.h>
#include <yaz/yaz-util.h>

int yaz_marc_read_line(yaz_marc_t mt,
                       int (*getbyte)(void *client_data),
                       void (*ungetbyte)(int b, void *client_data),
                       void *client_data)
{
    yaz_marc_reset(mt);

    return -1;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

