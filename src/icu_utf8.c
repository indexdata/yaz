/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief UTF-8 string utilities for ICU
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

struct icu_buf_utf8 *icu_buf_utf8_create(size_t capacity)
{
    struct icu_buf_utf8 * buf8 
        = (struct icu_buf_utf8 *) xmalloc(sizeof(struct icu_buf_utf8));

    buf8->utf8 = 0;
    buf8->utf8_len = 0;
    buf8->utf8_cap = 0;

    if (capacity > 0)
    {
        buf8->utf8 = (uint8_t *) xmalloc(sizeof(uint8_t) * capacity);
        buf8->utf8[0] = (uint8_t) 0;
        buf8->utf8_cap = capacity;
    }
    return buf8;
}

struct icu_buf_utf8 * icu_buf_utf8_clear(struct icu_buf_utf8 * buf8)
{
    if (buf8)
    {
        if (buf8->utf8)
            buf8->utf8[0] = (uint8_t) 0;
        buf8->utf8_len = 0;
    }
    return buf8;
}

struct icu_buf_utf8 * icu_buf_utf8_resize(struct icu_buf_utf8 * buf8,
                                          size_t capacity)
{
    if (!buf8)
        return 0;

    if (capacity >  0)
    {
        if (0 == buf8->utf8)
            buf8->utf8 = (uint8_t *) xmalloc(sizeof(uint8_t) * capacity);
        else
            buf8->utf8 
                = (uint8_t *) xrealloc(buf8->utf8, sizeof(uint8_t) * capacity);
        
        buf8->utf8_cap = capacity;
    } 
    else { 
        xfree(buf8->utf8);
        buf8->utf8 = 0;
        buf8->utf8_len = 0;
        buf8->utf8_cap = 0;
    }
    
    return buf8;
}

const char *icu_buf_utf8_to_cstr(struct icu_buf_utf8 *src8)
{
    if (!src8 || src8->utf8_len == 0)
        return "";

    if (src8->utf8_len == src8->utf8_cap)
        src8 = icu_buf_utf8_resize(src8, src8->utf8_len * 2 + 1);

    src8->utf8[src8->utf8_len] = '\0';

    return (const char *) src8->utf8;
}

void icu_buf_utf8_destroy(struct icu_buf_utf8 * buf8)
{
    if (buf8)
        xfree(buf8->utf8);
    xfree(buf8);
}

UErrorCode icu_utf16_from_utf8_cstr(struct icu_buf_utf16 * dest16,
                                    const char * src8cstr,
                                    UErrorCode * status)
{
    size_t src8cstr_len = 0;
    int32_t utf16_len = 0;

    *status = U_ZERO_ERROR;
    src8cstr_len = strlen(src8cstr);
  
    u_strFromUTF8(dest16->utf16, dest16->utf16_cap,
                  &utf16_len,
                  src8cstr, src8cstr_len, status);
  
    /* check for buffer overflow, resize and retry */
    if (*status == U_BUFFER_OVERFLOW_ERROR)
    {
        icu_buf_utf16_resize(dest16, utf16_len * 2);
        *status = U_ZERO_ERROR;
        u_strFromUTF8(dest16->utf16, dest16->utf16_cap,
                      &utf16_len,
                      src8cstr, src8cstr_len, status);
    }

    if (U_SUCCESS(*status)  
        && utf16_len <= dest16->utf16_cap)
        dest16->utf16_len = utf16_len;
    else 
        icu_buf_utf16_clear(dest16);
  
    return *status;
}

UErrorCode icu_utf16_to_utf8(struct icu_buf_utf8 *dest8,
                             const struct icu_buf_utf16 *src16,
                             UErrorCode * status)
{
    int32_t utf8_len = 0;
  
    u_strToUTF8((char *) dest8->utf8, dest8->utf8_cap,
                &utf8_len,
                src16->utf16, src16->utf16_len, status);
  
    /* check for buffer overflow, resize and retry */
    if (*status == U_BUFFER_OVERFLOW_ERROR)
    {
        icu_buf_utf8_resize(dest8, utf8_len * 2);
        *status = U_ZERO_ERROR;
        u_strToUTF8((char *) dest8->utf8, dest8->utf8_cap,
                    &utf8_len,
                    src16->utf16, src16->utf16_len, status);
    }

    if (U_SUCCESS(*status)  
        && utf8_len <= dest8->utf8_cap)
        dest8->utf8_len = utf8_len;
    else 
        icu_buf_utf8_clear(dest8);
  
    return *status;
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

