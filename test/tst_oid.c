/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#include <stdlib.h>
#include <stdio.h>

#include <yaz/test.h>
#include <yaz/log.h>
#include <yaz/oid_db.h>

static void tst(void)
{
    char oid_buf[OID_STR_MAX];
    const char *n;
    yaz_oid_db_t db;
    const Odr_oid *c_oid;
    Odr_oid *oid;
    NMEM nmem = nmem_create();
    ODR odr = odr_createmem(ODR_ENCODE);

    db = yaz_oid_std();
    YAZ_CHECK(db);

    c_oid = yaz_string_to_oid(db, CLASS_ATTSET, "Bib-1");
    YAZ_CHECK(c_oid && oid_oidcmp(c_oid, yaz_oid_attset_bib_1) == 0);

    c_oid = yaz_string_to_oid(db, CLASS_GENERAL, "Bib-1");
    YAZ_CHECK(c_oid && oid_oidcmp(c_oid, yaz_oid_attset_bib_1) == 0);

    c_oid = yaz_string_to_oid(db, CLASS_ATTSET, "unknown");
    YAZ_CHECK(c_oid == 0);

    oid = yaz_string_to_oid_nmem(db, CLASS_ATTSET, "1.2.840.10003.3.1", nmem);
    YAZ_CHECK(oid && !oid_oidcmp(oid, yaz_oid_attset_bib_1));

    oid = yaz_string_to_oid_nmem(db, CLASS_ATTSET, "unknown", nmem);
    YAZ_CHECK(oid == 0);

    oid = yaz_string_to_oid_odr(db, CLASS_ATTSET, "1.2.840.10003.3.1", odr);
    YAZ_CHECK(oid && !oid_oidcmp(oid, yaz_oid_attset_bib_1));

    oid = yaz_string_to_oid_odr(db, CLASS_ATTSET, "unknown", odr);
    YAZ_CHECK(oid == 0);

    n = yaz_oid_to_string(db, yaz_oid_attset_bib_1, 0);
    YAZ_CHECK(n && !strcmp(n, "Bib-1"));

    n = oid_name_to_dotstring(CLASS_ATTSET, "Bib-1", oid_buf);
    YAZ_CHECK(n && !strcmp(n, "1.2.840.10003.3.1"));

    n = oid_name_to_dotstring(CLASS_DIAGSET, "Bib-1", oid_buf);
    YAZ_CHECK(n && !strcmp(n, "1.2.840.10003.4.1"));

    n = oid_name_to_dotstring(CLASS_DIAGSET, "unknown", oid_buf);
    YAZ_CHECK(!n);

    n = oid_name_to_dotstring(CLASS_DIAGSET, "1.2.840.10003.3.1", oid_buf);
    YAZ_CHECK(!n);

    nmem_destroy(nmem);
    odr_destroy(odr);
}


int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
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

