/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/test.h>
#include <yaz/proto.h>
#include "../src/zoom-p.h"
#include "../src/sru-p.h"

/* Z39.50 SearchResponse with userFacets: one facet, no use attribute. */
static void check_z3950_no_use_attribute(void)
{
    static const unsigned char response[] = {
        0xb7, 0x28, /* SearchResponse */
        0x97, 0x01, 0x01, /* resultCount */
        0x98, 0x01, 0x00, /* numberOfRecordsReturned */
        0x99, 0x01, 0x01, /* nextResultSetPosition */
        0x96, 0x01, 0xff, /* searchStatus */
        0xbf, 0x81, 0x4b, 0x18, /* additionalSearchInfo */
        0x30, 0x16,
        0xa4, 0x14, /* externallyDefinedInfo */
        0x06, 0x0a, /* 1.2.840.10003.10.1000.81.5 (userFacets) */
        0x2a, 0x86, 0x48, 0xce, 0x13, 0x0a, 0x87, 0x68, 0x51, 0x05,
        0xa0, 0x06,
        0x30, 0x04, /* FacetList */
        0x30, 0x02, /* FacetField */
        0xa1, 0x00  /* attributes: empty */
    };
    ODR odr = odr_createmem(ODR_DECODE);
    struct ZOOM_resultset_p r;
    Z_APDU *apdu = 0;

    memset(&r, 0, sizeof(r));
    r.odr = odr_createmem(ODR_DECODE);
    odr_setbuf(odr, (char *) response, sizeof(response), 0);
    YAZ_CHECK(z_APDU(odr, &apdu, 0, 0));
    YAZ_CHECK(apdu && apdu->which == Z_APDU_searchResponse);
    if (apdu && apdu->which == Z_APDU_searchResponse)
    {
        ZOOM_facet_field f;

        ZOOM_handle_facet_result(0, &r,
                                 apdu->u.searchResponse->additionalSearchInfo);
        YAZ_CHECK_EQ(ZOOM_resultset_facets_size(&r), 1);
        f = ZOOM_resultset_get_facet_field_by_index(&r, 0);
        YAZ_CHECK(f);
        if (f)
        {
            const char *name = ZOOM_facet_field_name(f);
            YAZ_CHECK(name && !strcmp(name, ""));
            YAZ_CHECK_EQ(ZOOM_facet_field_term_count(f), 0);
        }
        YAZ_CHECK(ZOOM_resultset_get_facet_field(&r, "title") == 0);
    }
    odr_destroy(r.odr);
    odr_destroy(odr);
}

#if YAZ_HAVE_XML2
/* SRU facetedResults: the first facet has no index. */
static void check_sru_no_index(void)
{
    const char *response =
        "<facetedResults xmlns=\""
        "http://docs.oasis-open.org/ns/search-ws/facetedResults\">"
        "<facet><terms><term><actualTerm>water</actualTerm>"
        "<count>1</count></term></terms></facet>"
        "<facet><index>title</index><terms><term><actualTerm>a</actualTerm>"
        "<count>2</count></term></terms></facet>"
        "</facetedResults>";
    ODR odr = odr_createmem(ODR_DECODE);
    struct ZOOM_resultset_p r;
    xmlDocPtr doc = xmlParseMemory(response, (int) strlen(response));
    Z_FacetList *fl = 0;

    memset(&r, 0, sizeof(r));
    r.odr = odr_createmem(ODR_DECODE);
    YAZ_CHECK(doc);
    if (doc)
        yaz_sru_facet_response(odr, &fl, xmlDocGetRootElement(doc));
    YAZ_CHECK(fl && fl->num == 2);
    if (fl && fl->num == 2)
    {
        ZOOM_facet_field f;

        ZOOM_handle_facet_list(&r, fl);
        YAZ_CHECK_EQ(ZOOM_resultset_facets_size(&r), 2);

        f = ZOOM_resultset_get_facet_field_by_index(&r, 0);
        YAZ_CHECK(f);
        if (f)
        {
            int freq = 0;
            const char *name = ZOOM_facet_field_name(f);
            const char *term;
            YAZ_CHECK(name && !strcmp(name, ""));
            YAZ_CHECK_EQ(ZOOM_facet_field_term_count(f), 1);
            term = ZOOM_facet_field_get_term(f, 0, &freq);
            YAZ_CHECK(term && !strcmp(term, "water"));
            YAZ_CHECK_EQ(freq, 1);
        }
        f = ZOOM_resultset_get_facet_field(&r, "title");
        YAZ_CHECK(f);
        YAZ_CHECK(f == ZOOM_resultset_get_facet_field_by_index(&r, 1));
        if (f)
        {
            const char *name = ZOOM_facet_field_name(f);
            YAZ_CHECK(name && !strcmp(name, "title"));
            YAZ_CHECK_EQ(ZOOM_facet_field_term_count(f), 1);
        }
    }
    if (doc)
        xmlFreeDoc(doc);
    odr_destroy(r.odr);
    odr_destroy(odr);
}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_z3950_no_use_attribute();
#if YAZ_HAVE_XML2
    check_sru_no_index();
#endif
    YAZ_CHECK_TERM;
}
