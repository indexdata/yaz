/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/copy_types.h>
#include <yaz/oid_db.h>
#include <yaz/test.h>
#include "../src/zoom-p.h"

/* OPAC record with an octet aligned bibliographic record. A known syntax
   such as GRS-1 is decoded in place when the encoded record is decoded
   again; "x" is not valid GRS-1, so such a record can not be copied. */
static Z_NamePlusRecord *opac_record(ODR o, const Odr_oid *bib_syntax)
{
    Z_NamePlusRecord *npr = (Z_NamePlusRecord *) odr_malloc(o, sizeof(*npr));
    Z_External *ext = (Z_External *) odr_malloc(o, sizeof(*ext));
    Z_External *bib = (Z_External *) odr_malloc(o, sizeof(*bib));
    Z_OPACRecord *opac = (Z_OPACRecord *) odr_malloc(o, sizeof(*opac));

    bib->direct_reference = odr_oiddup(o, bib_syntax);
    bib->indirect_reference = 0;
    bib->descriptor = 0;
    bib->which = Z_External_octet;
    bib->u.octet_aligned = odr_create_Odr_oct(o, "x", 1);

    opac->bibliographicRecord = bib;
    opac->num_holdingsData = 0;
    opac->holdingsData = 0;

    ext->direct_reference = odr_oiddup(o, yaz_oid_recsyn_opac);
    ext->indirect_reference = 0;
    ext->descriptor = 0;
    ext->which = Z_External_OPAC;
    ext->u.opac = opac;

    npr->databaseName = 0;
    npr->which = Z_NamePlusRecord_databaseRecord;
    npr->u.databaseRecord = ext;
    return npr;
}

static void check_clone(const Odr_oid *bib_syntax, int can_copy)
{
    ZOOM_connection c = ZOOM_connection_create(0);
    ZOOM_resultset r;
    ZOOM_record rec, clone;
    Z_NamePlusRecord *npr, *npr2;
    NMEM nmem = nmem_create();

    c->async = 1; /* no target: only queue the search */
    r = ZOOM_connection_search_pqf(c, "computer");
    YAZ_CHECK(r);
    if (!r)
        return;
    npr = opac_record(r->odr, bib_syntax);
    ZOOM_record_cache_add(r, npr, 0, 0, 0, 0, 0);
    rec = ZOOM_resultset_record_immediate(r, 0);
    YAZ_CHECK(rec);

    clone = ZOOM_record_clone(rec);
    npr2 = yaz_clone_z_NamePlusRecord(npr, nmem);
    if (can_copy)
    {
        const char *syntax = ZOOM_record_get(clone, "syntax", 0);
        YAZ_CHECK(clone);
        YAZ_CHECK(syntax && !strcmp(syntax, "OPAC"));
        YAZ_CHECK(ZOOM_record_get(clone, "xml", 0) != 0);
        YAZ_CHECK(npr2);
        YAZ_CHECK(npr2 && yaz_compare_z_NamePlusRecord(npr, npr2));
    }
    else
    {
        /* must fail rather than return a partially decoded record */
        YAZ_CHECK(!clone);
        YAZ_CHECK(!npr2);
    }
    ZOOM_record_destroy(clone);
    nmem_destroy(nmem);
    ZOOM_resultset_destroy(r);
    ZOOM_connection_destroy(c);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_clone(yaz_oid_recsyn_usmarc, 1);
    check_clone(yaz_oid_recsyn_grs_1, 0);
    YAZ_CHECK_TERM;
}
