/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief ICU transforms - using utrans_-functions from ICU
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

#include <unicode/utrans.h>

struct icu_transform
{
    char action;
    UParseError parse_error;
    UTransliterator * trans;
};

struct icu_transform * icu_transform_create(const char *id, char action,
                                            const char *rules, 
                                            UErrorCode *status)
{
    struct icu_buf_utf16 *id16 = icu_buf_utf16_create(0);
    struct icu_buf_utf16 *rules16 = icu_buf_utf16_create(0);

    struct icu_transform *transform
        = (struct icu_transform *) xmalloc(sizeof(struct icu_transform));

    transform->action = action;
    transform->trans = 0;

    if (id)
        icu_utf16_from_utf8_cstr(id16, id, status);
    if (rules)
        icu_utf16_from_utf8_cstr(rules16, rules, status);

    switch (transform->action)
    {
    case 'f':
    case 'F':
        transform->trans
            = utrans_openU(id16->utf16, 
                           id16->utf16_len,
                           UTRANS_FORWARD,
                           rules16->utf16, 
                           rules16->utf16_len,
                           &transform->parse_error, status);
        break;
    case 'r':
    case 'R':
        transform->trans
            = utrans_openU(id16->utf16,
                           id16->utf16_len,
                           UTRANS_REVERSE ,
                           rules16->utf16, 
                           rules16->utf16_len,
                           &transform->parse_error, status);
        break;
    default:
        *status = U_UNSUPPORTED_ERROR;
        break;
    }
    icu_buf_utf16_destroy(rules16);
    icu_buf_utf16_destroy(id16);
    
    if (U_SUCCESS(*status))
        return transform;

    /* freeing if failed */
    icu_transform_destroy(transform);
    return 0;
}

void icu_transform_destroy(struct icu_transform * transform)
{
    if (transform)
    {
        if (transform->trans)
            utrans_close(transform->trans);
        xfree(transform);
    }
}

int icu_transform_trans(struct icu_transform * transform,
                        struct icu_buf_utf16 * dest16,
                        const struct icu_buf_utf16 * src16,
                        UErrorCode *status)
{
    if (!transform || !transform->trans 
        || !src16  || !dest16)
        return 0;

    if (!src16->utf16_len)
    {           /* guarding for empty source string */
        icu_buf_utf16_clear(dest16);
        return 0;
    }

    if (!icu_buf_utf16_copy(dest16, src16))
        return 0;

    utrans_transUChars (transform->trans, 
                        dest16->utf16, &(dest16->utf16_len),
                        dest16->utf16_cap,
                        0, &(dest16->utf16_len), status);

    if (U_FAILURE(*status))
        icu_buf_utf16_clear(dest16);
    
    return dest16->utf16_len;
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

