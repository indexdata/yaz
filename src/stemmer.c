

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if YAZ_HAVE_ICU

#include <yaz/yconfig.h>

#include <yaz/stemmer.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */

struct yaz_stemmer_t
{
    int implementation;
    union {
        struct sb_stemmer *snowballer;
    };
};

yaz_stemmer_p yaz_stemmer_create(const char *locale, const char *rule, UErrorCode *status) {
    return 0;
}

yaz_stemmer_p yaz_stemmer_clone(yaz_stemmer_p stemmer) {
    return 0;
}

void yaz_stemmer_stem(yaz_stemmer_p stemmer, struct icu_buf_utf16 *dst, struct icu_buf_utf16* src, UErrorCode *status) {

}

void yaz_stemmer_destroy(yaz_stemmer_p stemmer) {


}

#endif /* YAZ_HAVE_ICU */
