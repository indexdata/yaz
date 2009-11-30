/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief UTF-16 string utilities for ICU
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if YAZ_HAVE_ICU
#include <yaz/xmalloc.h>

#include <yaz/icu_I18N.h>

#include <yaz/log.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */

struct icu_buf_utf16 * icu_buf_utf16_create(size_t capacity)
{
    struct icu_buf_utf16 * buf16 
        = (struct icu_buf_utf16 *) xmalloc(sizeof(struct icu_buf_utf16));

    buf16->utf16 = 0;
    buf16->utf16_len = 0;
    buf16->utf16_cap = 0;

    if (capacity > 0)
    {
        buf16->utf16 = (UChar *) xmalloc(sizeof(UChar) * capacity);
        buf16->utf16[0] = (UChar) 0;
        buf16->utf16_cap = capacity;
    }
    return buf16;
}

struct icu_buf_utf16 * icu_buf_utf16_clear(struct icu_buf_utf16 * buf16)
{
    if (buf16)
    {
        if (buf16->utf16)
            buf16->utf16[0] = (UChar) 0;
        buf16->utf16_len = 0;
    }
    return buf16;
}

struct icu_buf_utf16 * icu_buf_utf16_resize(struct icu_buf_utf16 * buf16,
                                            size_t capacity)
{
    if (!buf16)
        return 0;
    
    if (capacity >  0)
    {
        if (0 == buf16->utf16)
            buf16->utf16 = (UChar *) xmalloc(sizeof(UChar) * capacity);
        else
            buf16->utf16 
                = (UChar *) xrealloc(buf16->utf16, sizeof(UChar) * capacity);

        icu_buf_utf16_clear(buf16);
        buf16->utf16_cap = capacity;
    } 
    else
    {
        xfree(buf16->utf16);
        buf16->utf16 = 0;
        buf16->utf16_len = 0;
        buf16->utf16_cap = 0;
    }

    return buf16;
}


struct icu_buf_utf16 * icu_buf_utf16_copy(struct icu_buf_utf16 * dest16,
                                          struct icu_buf_utf16 * src16)
{
    if (!dest16 || !src16 || dest16 == src16)
        return 0;

    if (dest16->utf16_cap < src16->utf16_len)
        icu_buf_utf16_resize(dest16, src16->utf16_len * 2);

    u_strncpy(dest16->utf16, src16->utf16, src16->utf16_len);
    dest16->utf16_len = src16->utf16_len;

    return dest16;
}

void icu_buf_utf16_destroy(struct icu_buf_utf16 * buf16)
{
    if (buf16)
        xfree(buf16->utf16);
    xfree(buf16);
}


#endif /* YAZ_HAVE_ICU */

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

