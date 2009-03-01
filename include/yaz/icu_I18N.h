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
    \brief Internal header for ICU utilities

    These functions, while non-static, are considered unstable and internal
    and may be renamed for each YAZ release.
*/

#ifndef ICU_I18NL_H
#define ICU_I18NL_H

#include <yaz/yconfig.h>

#include <unicode/utypes.h>   /* Basic ICU data types */
#include <unicode/uchar.h>    /* char names           */

#include <unicode/ucol.h> 
#include <unicode/ubrk.h>
#include <unicode/utrans.h>

#include <yaz/icu.h>

/* declared structs and functions */

int icu_check_status (UErrorCode status);

struct icu_buf_utf16
{
    UChar * utf16;
    int32_t utf16_len;
    int32_t utf16_cap;
};

struct icu_buf_utf16 * icu_buf_utf16_create(size_t capacity);

struct icu_buf_utf16 * icu_buf_utf16_clear(struct icu_buf_utf16 * buf16);

struct icu_buf_utf16 * icu_buf_utf16_resize(struct icu_buf_utf16 * buf16,
                                            size_t capacity);

struct icu_buf_utf16 * icu_buf_utf16_copy(struct icu_buf_utf16 * dest16,
                                          struct icu_buf_utf16 * src16);

void icu_buf_utf16_destroy(struct icu_buf_utf16 * buf16);



struct icu_buf_utf8
{
    uint8_t * utf8;
    int32_t utf8_len;
    int32_t utf8_cap;
};

struct icu_buf_utf8 * icu_buf_utf8_create(size_t capacity);

struct icu_buf_utf8 * icu_buf_utf8_clear(struct icu_buf_utf8 * buf8);

struct icu_buf_utf8 * icu_buf_utf8_resize(struct icu_buf_utf8 * buf8,
                                          size_t capacity);

void icu_buf_utf8_destroy(struct icu_buf_utf8 * buf8);


UErrorCode icu_utf16_from_utf8(struct icu_buf_utf16 * dest16,
                               struct icu_buf_utf8 * src8,
                               UErrorCode * status);

UErrorCode icu_utf16_from_utf8_cstr(struct icu_buf_utf16 * dest16,
                                    const char * src8cstr,
                                    UErrorCode * status);


UErrorCode icu_utf16_to_utf8(struct icu_buf_utf8 * dest8,
                             struct icu_buf_utf16 * src16,
                             UErrorCode * status);

struct icu_casemap
{
    char action;
};

struct icu_casemap * icu_casemap_create(char action, UErrorCode *status);

void icu_casemap_destroy(struct icu_casemap * casemap);

int icu_casemap_casemap(struct icu_casemap * casemap,
                        struct icu_buf_utf16 * dest16,
                        struct icu_buf_utf16 * src16,
                        UErrorCode *status,
                        const char *locale);

int icu_utf16_casemap(struct icu_buf_utf16 * dest16,
                      struct icu_buf_utf16 * src16,
                      const char *locale, char action,
                      UErrorCode *status);

void icu_sortkey8_from_utf16(UCollator *coll,
                             struct icu_buf_utf8 * dest8, 
                             struct icu_buf_utf16 * src16,
                             UErrorCode * status);

struct icu_tokenizer
{
    char action;
    UBreakIterator* bi;
    struct icu_buf_utf16 * buf16;
    int32_t token_count;
    int32_t token_id;
    int32_t token_start;
    int32_t token_end;
/*
  keep always invariant
  0 <= token_start 
  <= token_end 
  <= buf16->utf16_len
  and invariant
  0 <= token_id <= token_count
*/
};

struct icu_tokenizer * icu_tokenizer_create(const char *locale, char action,
                                            UErrorCode *status);

void icu_tokenizer_destroy(struct icu_tokenizer * tokenizer);

int icu_tokenizer_attach(struct icu_tokenizer * tokenizer, 
                         struct icu_buf_utf16 * src16, UErrorCode *status);

int32_t icu_tokenizer_next_token(struct icu_tokenizer * tokenizer, 
                                 struct icu_buf_utf16 * tkn16, 
                                 UErrorCode *status);

int32_t icu_tokenizer_token_id(struct icu_tokenizer * tokenizer);
int32_t icu_tokenizer_token_start(struct icu_tokenizer * tokenizer);
int32_t icu_tokenizer_token_end(struct icu_tokenizer * tokenizer);
int32_t icu_tokenizer_token_length(struct icu_tokenizer * tokenizer);
int32_t icu_tokenizer_token_count(struct icu_tokenizer * tokenizer);



struct icu_transform
{
    char action;
    UParseError parse_error;
    UTransliterator * trans;
};

struct icu_transform * icu_transform_create(const char *id, char action,
                                            const char *rules,
                                            UErrorCode *status);

void icu_transform_destroy(struct icu_transform * transform);

int icu_transform_trans(struct icu_transform * transform,
                        struct icu_buf_utf16 * dest16,
                        struct icu_buf_utf16 * src16,
                        UErrorCode *status);

enum icu_chain_step_type {
    ICU_chain_step_type_none,
    ICU_chain_step_type_display,   /* convert to utf8 display format */
    ICU_chain_step_type_casemap,   /* apply utf16 charmap */
    ICU_chain_step_type_transform, /* apply utf16 transform */
    ICU_chain_step_type_tokenize,  /* apply utf16 tokenization */
    ICU_chain_step_type_transliterate  /* apply utf16 tokenization */
};



struct icu_chain_step
{
    /* type and action object */
    enum icu_chain_step_type type;
    union {
	struct icu_casemap * casemap;
	struct icu_transform * transform;
	struct icu_tokenizer * tokenizer;  
    } u;
    /* temprary post-action utf16 buffer */
    struct icu_buf_utf16 * buf16;  
    struct icu_chain_step * previous;
    int more_tokens;
    int need_new_token;
};


struct icu_chain;

struct icu_chain_step * icu_chain_step_create(yaz_icu_chain_t chain,
                                              enum icu_chain_step_type type,
                                              const uint8_t * rule,
                                              struct icu_buf_utf16 * buf16,
                                              UErrorCode *status);


void icu_chain_step_destroy(struct icu_chain_step * step);


struct icu_chain
{
    char *locale;
    int sort;

    const char * src8cstr;

    UCollator * coll;
    
    /* number of tokens returned so far */
    int32_t token_count;
    
    /* utf8 output buffers */
    struct icu_buf_utf8 * display8;
    struct icu_buf_utf8 * norm8;
    struct icu_buf_utf8 * sort8;
    
    /* utf16 source buffer */
    struct icu_buf_utf16 * src16;
    
    /* linked list of chain steps */
    struct icu_chain_step * steps;
};

struct icu_chain_step * icu_chain_insert_step(yaz_icu_chain_t chain,
                                              enum icu_chain_step_type type,
                                              const uint8_t * rule,
                                              UErrorCode *status);

int icu_chain_step_next_token(yaz_icu_chain_t chain,
                              struct icu_chain_step * step,
                              UErrorCode *status);

int icu_chain_token_number(yaz_icu_chain_t chain);

const UCollator * icu_chain_get_coll(yaz_icu_chain_t chain);

yaz_icu_chain_t icu_chain_create(const char * locale,
                                 int sort,
                                 UErrorCode * status);


#endif /* ICU_I18NL_H */

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

