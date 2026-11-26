/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/test.h>
#include <yaz/pquery.h>
#include "../src/zoom-p.h"

/* Scan set with one termInfo entry, as a Z39.50 target could return it */
static void check_term(Z_Term *term, char *display_term, ODR odr,
                       const char *expect_term, const char *expect_display)
{
    struct ZOOM_scanset_p scan;
    Z_APDU *apdu = zget_APDU(odr, Z_APDU_scanResponse);
    Z_ScanResponse *res = apdu->u.scanResponse;
    Z_Entry *e = (Z_Entry *) odr_malloc(odr, sizeof(*e));
    Z_TermInfo *t = (Z_TermInfo *) odr_malloc(odr, sizeof(*t));
    const char *value;
    size_t occ, len;

    memset(t, 0, sizeof(*t));
    t->term = term;
    t->displayTerm = display_term;
    t->globalOccurrences = odr_intdup(odr, 7);
    e->which = Z_Entry_termInfo;
    e->u.termInfo = t;
    res->entries = (Z_ListEntries *) odr_malloc(odr, sizeof(*res->entries));
    memset(res->entries, 0, sizeof(*res->entries));
    res->entries->num_entries = 1;
    res->entries->entries = &e;

    memset(&scan, 0, sizeof(scan));
    scan.scan_response = res;
    YAZ_CHECK_EQ(ZOOM_scanset_size(&scan), 1);

    occ = 0;
    len = 99;
    value = ZOOM_scanset_term(&scan, 0, &occ, &len);
    YAZ_CHECK_EQ(occ, 7);
    if (expect_term)
    {
        YAZ_CHECK_EQ(len, strlen(expect_term));
        YAZ_CHECK(value && len == strlen(expect_term) &&
                  !memcmp(value, expect_term, len));
    }
    else
    {
        YAZ_CHECK(!value);
        YAZ_CHECK_EQ(len, 0);
    }

    occ = 0;
    len = 99;
    value = ZOOM_scanset_display_term(&scan, 0, &occ, &len);
    YAZ_CHECK_EQ(occ, 7);
    if (expect_display)
    {
        YAZ_CHECK_EQ(len, strlen(expect_display));
        YAZ_CHECK(value && len == strlen(expect_display) &&
                  !memcmp(value, expect_display, len));
    }
    else
    {
        YAZ_CHECK(!value);
        YAZ_CHECK_EQ(len, 0);
    }
}

static void check_scan_terms(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_Term *term;

    term = z_Term_create(odr, Z_Term_general, "apple", 5);
    check_term(term, 0, odr, "apple", "apple");
    check_term(term, "Apple", odr, "apple", "Apple");

    term = z_Term_create(odr, Z_Term_characterString, "apple", 5);
    check_term(term, 0, odr, "apple", "apple");
    check_term(term, "Apple", odr, "apple", "Apple");

    /* Alternatives that are not strings must not be read as one */
    term = z_Term_create(odr, Z_Term_numeric, "1234", 4);
    check_term(term, 0, odr, 0, 0);
    check_term(term, "1234", odr, 0, "1234");

    term = z_Term_create(odr, Z_Term_null, "", 0);
    check_term(term, 0, odr, 0, 0);

    term = (Z_Term *) odr_malloc(odr, sizeof(*term));
    term->which = Z_Term_integerAndUnit;
    term->u.integerAndUnit = (Z_IntUnit *)
        odr_malloc(odr, sizeof(*term->u.integerAndUnit));
    term->u.integerAndUnit->value = odr_intdup(odr, 1234);
    term->u.integerAndUnit->unitUsed = 0;
    check_term(term, 0, odr, 0, 0);
    check_term(term, "1234", odr, 0, "1234");

    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_scan_terms();
    YAZ_CHECK_TERM;
}
