/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: match_glob.c,v 1.1 2007-10-24 13:50:02 adam Exp $
 */

/**
 * \file
 * \brief Glob expression match
 */


#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <yaz/xmalloc.h>
#include <yaz/wrbuf.h>
#include <yaz/match_glob.h>

int yaz_match_glob(const char *glob, const char *text)
{
    if (glob[0] == '\0')
        return *text == '\0';
    if (glob[0] == '*')
    {
        do 
        {
            if (yaz_match_glob(glob+1, text))
                return 1;
        }
        while (*text++);
        return 0;
    }
    if (*text && (glob[0] == '?' || glob[0] == *text))
        return yaz_match_glob(glob+1, text+1);
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

