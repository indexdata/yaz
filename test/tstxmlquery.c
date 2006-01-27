/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstxmlquery.c,v 1.3 2006-01-27 19:04:15 adam Exp $
 */

#include <stdlib.h>
#include <stdio.h>

#include <yaz/wrbuf.h>
#include <yaz/xmlquery.h>
#include <yaz/pquery.h>
#include <yaz/test.h>

static void pqf2xml_text(const char *pqf)
{
    YAZ_PQF_Parser parser = yaz_pqf_create();
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_RPNQuery *rpn;

    YAZ_CHECK(parser);

    YAZ_CHECK(odr);

    rpn = yaz_pqf_parse(parser, odr, pqf);

    YAZ_CHECK(rpn);

    yaz_pqf_destroy(parser);

    Z_Query *query = odr_malloc(odr, sizeof(*query));
    query->which = Z_Query_type_1;
    query->u.type_1 = rpn;

    odr_destroy(odr);
#if HAVE_XML2

#endif
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);

    pqf2xml_text("@attr 1=4 computer");

    exit(0);
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

