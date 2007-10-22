/* $Id: icu_I18N.h,v 1.1 2007-10-22 12:21:39 adam Exp $
   Copyright (c) 2006-2007, Index Data.

   This file is part of Pazpar2.

   Pazpar2 is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Pazpar2 is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Pazpar2; see the file LICENSE.  If not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.
*/

#ifndef ICU_I18NL_H
#define ICU_I18NL_H

#include <yaz/nmem.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <unicode/utypes.h>   /* Basic ICU data types */
#include <unicode/uchar.h>    /* char names           */

//#include <unicode/ustdio.h>
#include <unicode/ucol.h> 
//#include <unicode/ucnv.h>     /* C   Converter API    */
//#include <unicode/ustring.h>  /* some more string fcns*/
//#include <unicode/uloc.h>
#include <unicode/ubrk.h>
//#include <unicode/unistr.h>
#include <unicode/utrans.h>



// declared structs and functions

int icu_check_status (UErrorCode status);

struct icu_buf_utf16
{
  UChar * utf16;
  int32_t utf16_len;
  int32_t utf16_cap;
};

struct icu_buf_utf16 * icu_buf_utf16_create(size_t capacity);
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
  char locale[16];
  char action;
};

struct icu_casemap * icu_casemap_create(const char *locale, char action,
                                            UErrorCode *status);

void icu_casemap_destroy(struct icu_casemap * casemap);

int icu_casemap_casemap(struct icu_casemap * casemap,
                        struct icu_buf_utf16 * dest16,
                        struct icu_buf_utf16 * src16,
                        UErrorCode *status);

int icu_utf16_casemap(struct icu_buf_utf16 * dest16,
                      struct icu_buf_utf16 * src16,
                      const char *locale, char action,
                      UErrorCode *status);

UErrorCode icu_sortkey8_from_utf16(UCollator *coll,
                                   struct icu_buf_utf8 * dest8, 
                                   struct icu_buf_utf16 * src16,
                                   UErrorCode * status);

struct icu_tokenizer
{
  char locale[16];
  char action;
  UBreakIterator* bi;
  struct icu_buf_utf16 * buf16;
  int32_t token_count;
  int32_t token_id;
  int32_t token_start;
  int32_t token_end;
  // keep always invariant
  // 0 <= token_start 
  //   <= token_end 
  //   <= buf16->utf16_len
  // and invariant
  // 0 <= token_id <= token_count
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



struct icu_normalizer
{
  char action;
  struct icu_buf_utf16 * rules16;
  UParseError parse_error[256];
  UTransliterator * trans;
};

struct icu_normalizer * icu_normalizer_create(const char *rules, char action,
                                              UErrorCode *status);


void icu_normalizer_destroy(struct icu_normalizer * normalizer);

int icu_normalizer_normalize(struct icu_normalizer * normalizer,
                             struct icu_buf_utf16 * dest16,
                             struct icu_buf_utf16 * src16,
                             UErrorCode *status);


#if 0
struct icu_token
{
  int32_t token_id;
  uint8_t * display8;
  uint8_t * norm8;
  uint8_t * sort8;
}
#endif


enum icu_chain_step_type {
    ICU_chain_step_type_none,      // 
    ICU_chain_step_type_display,   // convert to utf8 display format 
    ICU_chain_step_type_index,     // convert to utf8 index format 
    ICU_chain_step_type_sortkey,   // convert to utf8 sortkey format 
    ICU_chain_step_type_casemap,   // apply utf16 charmap
    ICU_chain_step_type_normalize, // apply utf16 normalization
    ICU_chain_step_type_tokenize   // apply utf16 tokenization 
};



struct icu_chain_step
{
  // type and action object
  enum icu_chain_step_type type;
  union {
    struct icu_casemap * casemap;
    struct icu_normalizer * normalizer;
    struct icu_tokenizer * tokenizer;  
  } u;
  // temprary post-action utf16 buffer
  struct icu_buf_utf16 * buf16;  
  struct icu_chain_step * previous;
  int more_tokens;
  int need_new_token;
};


struct icu_chain;

struct icu_chain_step * icu_chain_step_create(struct icu_chain * chain,
                                              enum icu_chain_step_type type,
                                              const uint8_t * rule,
                                              struct icu_buf_utf16 * buf16,
                                              UErrorCode *status);


void icu_chain_step_destroy(struct icu_chain_step * step);


struct icu_chain
{
  uint8_t identifier[128];
  uint8_t locale[16];

  // number of tokens returned so far
  int32_t token_count;

  // utf8 output buffers
  struct icu_buf_utf8 * display8;
  struct icu_buf_utf8 * norm8;
  struct icu_buf_utf8 * sort8;

  // utf16 source buffer
  struct icu_buf_utf16 * src16;

  // linked list of chain steps
  struct icu_chain_step * steps;
};

struct icu_chain * icu_chain_create(const uint8_t * identifier, 
                                    const uint8_t * locale);

void icu_chain_destroy(struct icu_chain * chain);

struct icu_chain * icu_chain_xml_config(xmlNode *xml_node, 
                                        UErrorCode * status);


struct icu_chain_step * icu_chain_insert_step(struct icu_chain * chain,
                                              enum icu_chain_step_type type,
                                              const uint8_t * rule,
                                              UErrorCode *status);


int icu_chain_step_next_token(struct icu_chain * chain,
                              struct icu_chain_step * step,
                              UErrorCode *status);

int icu_chain_assign_cstr(struct icu_chain * chain,
                          const char * src8cstr, 
                          UErrorCode *status);

int icu_chain_next_token(struct icu_chain * chain,
                         UErrorCode *status);

int icu_chain_get_token_count(struct icu_chain * chain);

const char * icu_chain_get_display(struct icu_chain * chain);

const char * icu_chain_get_norm(struct icu_chain * chain);

const char * icu_chain_get_sort(struct icu_chain * chain);





#endif // ICU_I18NL_H
