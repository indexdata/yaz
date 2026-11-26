/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/charneg.h>
#include <yaz/test.h>

static Z_External *make_proposal(ODR odr)
{
    const char *charsets[] = {"UTF-8", "ISO-8859-1"};
    const char *langs[] = {"eng", "dan", "ger"};

    return yaz_set_proposal_charneg(odr, charsets, 2, langs, 3, 1);
}

static void tst_proposal(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    NMEM nmem = nmem_create();
    Z_External *ext = make_proposal(odr);
    char **charsets = 0, **langs = 0;
    int num_charsets = -1, num_langs = -1, selected = -1;

    YAZ_CHECK(ext);
    if (ext)
    {
        yaz_get_proposal_charneg(nmem, ext->u.charNeg3,
                                 &charsets, &num_charsets,
                                 &langs, &num_langs, &selected);
        YAZ_CHECK_EQ(num_charsets, 2);
        YAZ_CHECK(num_charsets == 2 && charsets &&
                  !strcmp(charsets[0], "UTF-8") &&
                  !strcmp(charsets[1], "ISO-8859-1"));
        YAZ_CHECK_EQ(num_langs, 3);
        YAZ_CHECK(num_langs == 3 && langs &&
                  !strcmp(langs[0], "eng") && !strcmp(langs[2], "ger"));
        YAZ_CHECK_EQ(selected, 1);
    }
    nmem_destroy(nmem);
    odr_destroy(odr);
}

static void tst_response(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    NMEM nmem = nmem_create();
    Z_External *ext = yaz_set_response_charneg(odr, "UTF-8", "eng", 1);
    char *charset = 0, *lang = 0;
    int selected = -1;

    YAZ_CHECK(ext);
    if (ext)
    {
        yaz_get_response_charneg(nmem, ext->u.charNeg3,
                                 &charset, &lang, &selected);
        YAZ_CHECK(charset && !strcmp(charset, "UTF-8"));
        YAZ_CHECK(lang && !strcmp(lang, "eng"));
        YAZ_CHECK_EQ(selected, 1);
    }
    nmem_destroy(nmem);
    odr_destroy(odr);
}

/* A peer decides which arm of the CHOICE it sends. A proposal must not be
   read as a response, nor a response as a proposal. */
static void tst_proposal_as_response(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    NMEM nmem = nmem_create();
    Z_External *ext = make_proposal(odr);
    char *charset = 0, *lang = 0;
    int selected = -1;

    YAZ_CHECK(ext);
    if (ext)
    {
        YAZ_CHECK_EQ(ext->u.charNeg3->which,
                     Z_CharSetandLanguageNegotiation_proposal);
        yaz_get_response_charneg(nmem, ext->u.charNeg3,
                                 &charset, &lang, &selected);
        YAZ_CHECK(charset == 0);
        YAZ_CHECK(lang == 0);
        YAZ_CHECK_EQ(selected, -1);
    }
    nmem_destroy(nmem);
    odr_destroy(odr);
}

static void tst_response_as_proposal(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    NMEM nmem = nmem_create();
    Z_External *ext = yaz_set_response_charneg(odr, "UTF-8", "eng", 1);
    char **charsets = 0, **langs = 0;
    int num_charsets = -1, num_langs = -1, selected = -1;

    YAZ_CHECK(ext);
    if (ext)
    {
        YAZ_CHECK_EQ(ext->u.charNeg3->which,
                     Z_CharSetandLanguageNegotiation_response);
        yaz_get_proposal_charneg(nmem, ext->u.charNeg3,
                                 &charsets, &num_charsets,
                                 &langs, &num_langs, &selected);
        YAZ_CHECK_EQ(num_charsets, 0);
        YAZ_CHECK(charsets == 0);
        YAZ_CHECK_EQ(num_langs, 0);
        YAZ_CHECK(langs == 0);
        YAZ_CHECK_EQ(selected, -1);
    }
    nmem_destroy(nmem);
    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_proposal();
    tst_response();
    tst_proposal_as_response();
    tst_response_as_proposal();
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
