/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief ICU character case (u_strToUpper, etc)
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

#include <unicode/ustring.h>

struct icu_casemap
{
    char action;
};

struct icu_casemap * icu_casemap_create(char action, UErrorCode *status)
{    
    struct icu_casemap * casemap
        = (struct icu_casemap *) xmalloc(sizeof(struct icu_casemap));
    casemap->action = action;

    switch (casemap->action)
    {
    case 'l':   
    case 'L':   
    case 'u':   
    case 'U':   
    case 't':  
    case 'T':  
    case 'f':  
    case 'F':  
        break;
    default:
        icu_casemap_destroy(casemap);
        return 0;
    }
    return casemap;
}

struct icu_casemap *icu_casemap_clone(struct icu_casemap *old)
{
    struct icu_casemap * casemap
        = (struct icu_casemap *) xmalloc(sizeof(struct icu_casemap));
    casemap->action = old->action;
    return casemap;
}

void icu_casemap_destroy(struct icu_casemap * casemap)
{
    xfree(casemap);
}

int icu_casemap_casemap(struct icu_casemap * casemap,
                        struct icu_buf_utf16 * dest16,
                        struct icu_buf_utf16 * src16,
                        UErrorCode *status,
                        const char *locale)
{
    if(!casemap)
        return 0;
    
    return icu_utf16_casemap(dest16, src16, locale,
                             casemap->action, status);
}

int icu_utf16_casemap(struct icu_buf_utf16 * dest16,
                      struct icu_buf_utf16 * src16,
                      const char *locale, char action,
                      UErrorCode *status)
{
    int32_t dest16_len = 0;

    if (!src16->utf16_len)
    {           /* guarding for empty source string */
        if (dest16->utf16)
            dest16->utf16[0] = (UChar) 0;
        dest16->utf16_len = 0;
        return U_ZERO_ERROR;
    }
    
    switch (action)
    {
    case 'l':    
    case 'L':    
        dest16_len = u_strToLower(dest16->utf16, dest16->utf16_cap,
                                  src16->utf16, src16->utf16_len, 
                                  locale, status);
        break;
    case 'u':    
    case 'U':    
        dest16_len = u_strToUpper(dest16->utf16, dest16->utf16_cap,
                                  src16->utf16, src16->utf16_len, 
                                  locale, status);
        break;
    case 't':    
    case 'T':    
        dest16_len = u_strToTitle(dest16->utf16, dest16->utf16_cap,
                                  src16->utf16, src16->utf16_len,
                                  0, locale, status);
        break;
    case 'f':    
    case 'F':    
        dest16_len = u_strFoldCase(dest16->utf16, dest16->utf16_cap,
                                   src16->utf16, src16->utf16_len,
                                   U_FOLD_CASE_DEFAULT, status);
        break;
        
    default:
        return U_UNSUPPORTED_ERROR;
        break;
    }

    /* check for buffer overflow, resize and retry */
    if (*status == U_BUFFER_OVERFLOW_ERROR
        && dest16 != src16        /* do not resize if in-place conversion */
        )
    {
        icu_buf_utf16_resize(dest16, dest16_len * 2);
        *status = U_ZERO_ERROR;

        switch (action) {    
        case 'l':    
        case 'L':    
            dest16_len = u_strToLower(dest16->utf16, dest16->utf16_cap,
                                      src16->utf16, src16->utf16_len, 
                                      locale, status);
            break;
        case 'u':    
        case 'U':    
            dest16_len = u_strToUpper(dest16->utf16, dest16->utf16_cap,
                                      src16->utf16, src16->utf16_len, 
                                      locale, status);
            break;
        case 't':    
        case 'T':    
            dest16_len = u_strToTitle(dest16->utf16, dest16->utf16_cap,
                                      src16->utf16, src16->utf16_len,
                                      0, locale, status);
            break;
        case 'f':    
        case 'F':    
            dest16_len = u_strFoldCase(dest16->utf16, dest16->utf16_cap,
                                       src16->utf16, src16->utf16_len,
                                       U_FOLD_CASE_DEFAULT, status);
            break;
        
        default:
            return U_UNSUPPORTED_ERROR;
            break;
        }
    }
    
    if (U_SUCCESS(*status)
        && dest16_len <= dest16->utf16_cap)
        dest16->utf16_len = dest16_len;
    else
    {
        if (dest16->utf16)
            dest16->utf16[0] = (UChar) 0;
        dest16->utf16_len = 0;
    }
  
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

