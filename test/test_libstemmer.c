/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <yaz/stemmer.h>
#include <yaz/test.h>

int test_stemmer_stem(yaz_stemmer_p stemmer, const char* to_stem, const char *expected) 
{
    struct icu_buf_utf16 *src  = icu_buf_utf16_create(0);
    struct icu_buf_utf16 *dst  = icu_buf_utf16_create(0);
    struct icu_buf_utf8  *dst8 = icu_buf_utf8_create(0);

    UErrorCode status; 
    const char *result;
    icu_utf16_from_utf8_cstr(src, to_stem, &status);
    yaz_stemmer_stem(stemmer, dst, src, &status); 
    /* Assume fail */
    int rc = 0;
    if (status == U_ZERO_ERROR) {
        icu_utf16_to_utf8(dst8, dst, &status);
        result = icu_buf_utf8_to_cstr(dst8);
        rc = strcmp(result, expected) == 0;
    }
    icu_buf_utf8_destroy(dst8);
    icu_buf_utf16_destroy(src);
    icu_buf_utf16_destroy(dst);
    return rc;
}



static void tst(void)
{
    UErrorCode status;
    //== U_ZERO_ERROR; 
    yaz_stemmer_p stemmer = yaz_stemmer_create("en", "porter", &status);
    YAZ_CHECK(stemmer); 

    /* fail  */
    YAZ_CHECK(test_stemmer_stem(stemmer, "beer", "water") == 0 ); 

    /* Same */
    YAZ_CHECK(test_stemmer_stem(stemmer, "adadwwr", "adadwwr")); 

    /* Remove S */
    YAZ_CHECK(test_stemmer_stem(stemmer, "beers", "beer")); 
    YAZ_CHECK(test_stemmer_stem(stemmer, "persons", "person")); 

    /* Remove s and ing  */
    YAZ_CHECK(test_stemmer_stem(stemmer, "runs", "run")); 
    YAZ_CHECK(test_stemmer_stem(stemmer, "running", "run")); 

    yaz_stemmer_destroy(stemmer);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

