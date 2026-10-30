/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
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

#include "icu_I18N.h"

#include <yaz/log.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */

struct icu_buf_utf16 *icu_buf_utf16_create(size_t required)
{
    struct icu_buf_utf16 *buf16;
    size_t capacity;

    if (required > INT32_MAX)
        return NULL;

    capacity = required > INT32_MAX / 2 ? INT32_MAX : required * 2;
    buf16 = (struct icu_buf_utf16 *) xmalloc(sizeof(struct icu_buf_utf16));
    buf16->utf16_len = 0;
    buf16->utf16_cap = (int32_t) capacity;
    if (capacity > 0)
    {
        buf16->utf16 = (UChar *) xmalloc(sizeof(UChar) * capacity);
        buf16->utf16[0] = (UChar) 0;
    }
    else
        buf16->utf16 = 0;
    return buf16;
}

struct icu_buf_utf16 *icu_buf_utf16_clear(struct icu_buf_utf16 *buf16)
{
    assert(buf16);
    if (buf16->utf16)
        buf16->utf16[0] = (UChar) 0;
    buf16->utf16_len = 0;
    return buf16;
}

struct icu_buf_utf16 *icu_buf_utf16_resize(struct icu_buf_utf16 *buf16,
                                           size_t required)
{
    size_t capacity;

    assert(buf16);
    if (required > INT32_MAX)
        return NULL;
    capacity = required > INT32_MAX / 2 ? INT32_MAX : required * 2;
    if (capacity > 0)
    {
        if (0 == buf16->utf16)
            buf16->utf16 = (UChar *) xmalloc(sizeof(UChar) * capacity);
        else
            buf16->utf16
                = (UChar *) xrealloc(buf16->utf16, sizeof(UChar) * capacity);
        buf16->utf16_cap = (int32_t) capacity;
    }
    return buf16;
}


struct icu_buf_utf16 *icu_buf_utf16_copy(struct icu_buf_utf16 *dest16,
                                         const struct icu_buf_utf16 *src16)
{
    if (!dest16 || !src16 || dest16 == src16)
        return 0;

    if (dest16->utf16_cap < src16->utf16_len &&
        !icu_buf_utf16_resize(dest16, (size_t) src16->utf16_len))
        return NULL;

    u_strncpy(dest16->utf16, src16->utf16, src16->utf16_len);
    dest16->utf16_len = src16->utf16_len;

    return dest16;
}


struct icu_buf_utf16 *icu_buf_utf16_append(struct icu_buf_utf16 *dest16,
                                           const struct icu_buf_utf16 *src16)
{
    size_t required;

    assert(dest16);
    if (!src16)
        return dest16;
    if (dest16 == src16)
        return 0;

    if (src16->utf16_len == 0)
        return dest16;

    required = (size_t) dest16->utf16_len + (size_t) src16->utf16_len;
    if (required > INT32_MAX)
        return NULL;
    if ((size_t) dest16->utf16_cap < required &&
        !icu_buf_utf16_resize(dest16, required))
        return NULL;

    u_strncpy(dest16->utf16 + dest16->utf16_len,
              src16->utf16, src16->utf16_len);
    dest16->utf16_len = (int32_t) required;

    return dest16;
}


void icu_buf_utf16_destroy(struct icu_buf_utf16 *buf16)
{
    if (buf16)
        xfree(buf16->utf16);
    xfree(buf16);
}

void icu_buf_utf16_log(const char *lead, struct icu_buf_utf16 *src16)
{
    if (src16)
    {
        struct icu_buf_utf8 *dst8 = icu_buf_utf8_create(0);
        UErrorCode status = U_ZERO_ERROR;
        icu_utf16_to_utf8(dst8, src16, &status);
        yaz_log(YLOG_LOG, "%s=%s", lead, dst8->utf8);
        icu_buf_utf8_destroy(dst8);
    }
    else
    {
        yaz_log(YLOG_LOG, "%s=NULL", lead);
    }
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

