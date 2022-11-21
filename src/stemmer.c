/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file stemmer.c
 * \brief Implements stemmer wrapper
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if YAZ_HAVE_ICU

#include <yaz/stemmer.h>

#include <yaz/xmalloc.h>

#include <libstemmer.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */

enum stemmer_implementation {
    yaz_no_operation,
    yaz_snowball
};
struct yaz_stemmer_t
{
    int implementation;
    char *locale;
    char *rule;
    struct sb_stemmer *sb_stemmer;
};

const char* yaz_stemmer_lookup_charenc(const char *charenc, const char *rule) {
    return "UTF_8";
}

const char* yaz_stemmer_lookup_algorithm(const char *locale, const char *rule) {
    return locale;
}

yaz_stemmer_p yaz_stemmer_snowball_create(const char *locale, const char *rule, UErrorCode *status) {
    const char *charenc = yaz_stemmer_lookup_charenc(locale, rule);
    const char *algorithm = yaz_stemmer_lookup_algorithm(locale,rule);
    struct sb_stemmer *stemmer = sb_stemmer_new(algorithm, charenc);
    yaz_stemmer_p yaz_stemmer;
    if (stemmer == 0) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        yaz_log(YLOG_FATAL, "yaz_stemmer: Failed to create snowball stemmer from locale %srule %s. Showball: charenc %s algorithm %s ",
		locale, rule, charenc, algorithm);
	return 0;
    }
    yaz_log(YLOG_DEBUG, "created snowball stemmer: algorithm %s charenc %s ", algorithm, charenc);
    yaz_stemmer = xmalloc(sizeof(*yaz_stemmer));
    yaz_stemmer->implementation = yaz_snowball;

    yaz_stemmer->locale = xstrdup(locale);
    yaz_stemmer->rule = xstrdup(rule);
    yaz_stemmer->sb_stemmer = stemmer;
    yaz_log(YLOG_DEBUG, "created snowball stemmer: algorithm %s charenc %s ", algorithm, charenc);
    return yaz_stemmer;
}

yaz_stemmer_p yaz_stemmer_create(const char *locale, const char *rule, UErrorCode *status) {
    *status = U_ZERO_ERROR;
    yaz_log(YLOG_DEBUG, "create stemmer: locale %s rule %s ", locale, rule);
    return yaz_stemmer_snowball_create(locale, rule, status);
}

yaz_stemmer_p yaz_stemmer_clone(yaz_stemmer_p stemmer) {
    UErrorCode error = U_ZERO_ERROR;
    if (stemmer == 0)
      return 0;
    return yaz_stemmer_create(stemmer->locale, stemmer->rule, &error);
}

void yaz_stemmer_stem(yaz_stemmer_p stemmer, struct icu_buf_utf16 *dst, struct icu_buf_utf16* src, UErrorCode *status)
{
    switch(stemmer->implementation) {
        case yaz_snowball: {
            struct icu_buf_utf8 *utf8_buf = icu_buf_utf8_create(0);
            icu_utf16_to_utf8(utf8_buf, src, status);
            if (*status == U_ZERO_ERROR) {
                const sb_symbol *cstr = (const sb_symbol*) icu_buf_utf8_to_cstr(utf8_buf);
                const sb_symbol *sb_symbol = sb_stemmer_stem(stemmer->sb_stemmer, cstr, utf8_buf->utf8_len);
                if (sb_symbol == 0) {
                    icu_buf_utf16_copy(dst, src);
                }
                else {

                    const char *cstr2 = (const char *) sb_symbol;
                    icu_utf16_from_utf8_cstr(dst, cstr2 , status);
#if 0
		    yaz_log(YLOG_DEBUG, "stemming %s to %s ", cstr, cstr2);
#endif
                }
            }
            icu_buf_utf8_destroy(utf8_buf);
            return ;
            break;
        }
    case yaz_no_operation:
      yaz_log(YLOG_DEBUG, "Stemmer (No operation) called");
    default: {
            icu_buf_utf16_copy(dst, src);
        }
    }
}

void yaz_stemmer_destroy(yaz_stemmer_p stemmer)
{
  /* Handle no stemmer correctly */
  if (stemmer == 0)
    return ;

  switch (stemmer->implementation) {
  case yaz_snowball:
    sb_stemmer_delete(stemmer->sb_stemmer);
    break;
  }
  xfree(stemmer->locale);
  xfree(stemmer->rule);
  xfree(stemmer);
}

#endif /* YAZ_HAVE_ICU */
