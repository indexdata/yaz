/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief Glob expression match
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <yaz/xmalloc.h>
#include <yaz/wrbuf.h>
#include <yaz/match_glob.h>

int yaz_match_glob(const char *glob, const char *text)
{
    return yaz_match_glob2(glob, text, 0);
}

int yaz_match_glob2(const char *glob, const char *text, int case_insensitive)
{
    while (1)
    {
        if (*glob == '\0')
            return *text == '\0';
        if (*glob == '*')
        {
            do
            {
                if (yaz_match_glob2(glob+1, text, case_insensitive))
                    return 1;
            }
            while (*text++);
            return 0;
        }
        if (!*text)
            return 0;
        if (*glob != '?')
        {
            if (case_insensitive)
            {
                if (tolower(*glob) != tolower(*text))
                    return 0;
            }
            else if (*glob != *text)
                return 0;
        }
        glob++;
        text++;
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

