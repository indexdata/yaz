/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file
    \brief ICU utilities
*/


#ifndef YAZ_ICU_H
#define YAZ_ICU_H

#include <yaz/yconfig.h>

#include <yaz/xmltypes.h>

#include <unicode/utypes.h>

YAZ_BEGIN_CDECL

typedef struct icu_chain *yaz_icu_chain_t;

YAZ_EXPORT yaz_icu_chain_t icu_chain_create(const char * locale,
                                 int sort,
                                 UErrorCode * status);

YAZ_EXPORT void icu_chain_destroy(yaz_icu_chain_t chain);

YAZ_EXPORT yaz_icu_chain_t icu_chain_xml_config(const xmlNode *xml_node,
                                     int sort,
                                     UErrorCode * status);

YAZ_EXPORT int icu_chain_assign_cstr(yaz_icu_chain_t chain,
                          const char * src8cstr, 
                          UErrorCode *status);

YAZ_EXPORT int icu_chain_next_token(yaz_icu_chain_t chain,
                         UErrorCode *status);

YAZ_EXPORT int icu_chain_token_number(yaz_icu_chain_t chain);

YAZ_EXPORT const char * icu_chain_token_display(yaz_icu_chain_t chain);

YAZ_EXPORT const char * icu_chain_token_norm(yaz_icu_chain_t chain);

YAZ_EXPORT const char * icu_chain_token_sortkey(yaz_icu_chain_t chain);

YAZ_END_CDECL

#endif /* YAZ_ICU_H */

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
