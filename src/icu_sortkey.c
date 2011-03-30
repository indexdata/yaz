/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief sortkey utility based on ICU Collator
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

void icu_sortkey8_from_utf16(UCollator *coll,
                             struct icu_buf_utf8 * dest8, 
                             struct icu_buf_utf16 * src16,
                             UErrorCode * status)
{ 
    int32_t sortkey_len = 0;

    sortkey_len = ucol_getSortKey(coll, src16->utf16, src16->utf16_len,
                                  dest8->utf8, dest8->utf8_cap);

    /* check for buffer overflow, resize and retry */
    if (sortkey_len > dest8->utf8_cap)
    {
        icu_buf_utf8_resize(dest8, sortkey_len * 2);
        sortkey_len = ucol_getSortKey(coll, src16->utf16, src16->utf16_len,
                                      dest8->utf8, dest8->utf8_cap);
    }

    if (U_SUCCESS(*status)
        && sortkey_len > 0)
        dest8->utf8_len = sortkey_len;
    else 
        icu_buf_utf8_clear(dest8);
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

