/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <yaz/srw.h>
#include <yaz/log.h>
#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#endif
#include <yaz/test.h>
#include <yaz/yaz-version.h>
#include <yaz/pquery.h>
#include <yaz/snprintf.h>

#if YAZ_HAVE_XML2
int compare_solr_req(ODR odr, Z_SRW_PDU *sr,
                     const char *charset, const char *expect)
{
    int r;
    Z_GDU *gdu = 0;
    YAZ_CHECK(sr);

    if (!sr)
        return 0;

    gdu = z_get_HTTP_Request_host_path(odr, "localhost", "Default");
    YAZ_CHECK(gdu);
    if (!gdu)
        return 0;

    yaz_solr_encode_request(gdu->u.HTTP_Request, sr, odr, charset);

    r = z_GDU(odr, &gdu, 0, 0);
    YAZ_CHECK(r);
    if (r)
    {
        int len = 0;
        char *buf = odr_getbuf(odr, &len, 0);

        if (buf)
        {
            if (len == strlen(expect) && !memcmp(buf, expect, len))
            {
                odr_reset(odr);
                return 1;
            }
            yaz_log(YLOG_WARN, "Expect:\n%s\n", expect);
            yaz_log(YLOG_WARN, "Got:\n%.*s\n", len, buf);
         }
    }
    odr_reset(odr);
    return 0;
}
#endif

void tst_encoding(void)
{
#if YAZ_HAVE_XML2
    ODR odr = odr_createmem(ODR_ENCODE);

    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(odr, Z_SRW_searchRetrieve_request,
                                        "1.2");

        sr->u.request->query = "title:solr";
        YAZ_CHECK(compare_solr_req(
                      odr, sr, 0,
                      "GET Default/select?defType=lucene&q=title%3Asolr "
                      "HTTP/1.1\r\n"
                      "User-Agent: YAZ/" YAZ_VERSION "\r\n"
                      "Host: localhost\r\n\r\n"));
    }

    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(odr, Z_SRW_searchRetrieve_request,
                                        "1.2");
        sr->u.request->query = "title:solr";
        YAZ_CHECK(compare_solr_req(
                      odr, sr, "utf-8",
                      "GET Default/select?defType=lucene&q=title%3Asolr "
                      "HTTP/1.1\r\n"
                      "User-Agent: YAZ/" YAZ_VERSION "\r\n"
                      "Host: localhost\r\n\r\n"));
    }

    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(odr, Z_SRW_searchRetrieve_request,
                                        "1.2");

        sr->u.request->query = "title:solr";
        sr->u.request->startRecord = odr_intdup(odr, 3);
        sr->u.request->maximumRecords = odr_intdup(odr, 10);

        YAZ_CHECK(compare_solr_req(
                      odr, sr, 0,
                      "GET Default/select?defType=lucene&q=title%3Asolr&"
                      "start=2&rows=10"
                      " HTTP/1.1\r\n"
                      "User-Agent: YAZ/" YAZ_VERSION "\r\n"
                      "Host: localhost\r\n\r\n"));
    }

    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(odr, Z_SRW_searchRetrieve_request,
                                        "1.2");

        sr->u.request->query = "title:solr";
        sr->u.request->startRecord = odr_intdup(odr, 3);
        sr->u.request->maximumRecords = odr_intdup(odr, 10);
        sr->u.request->facetList = yaz_pqf_parse_facet_list(
            odr, "@attr 1=date @attr 2=0, @attr 1=title_exact @attr 3=17");

        YAZ_CHECK(compare_solr_req(
                      odr, sr, 0,
                      "GET Default/select?defType=lucene&q=title%3Asolr&"
                      "start=2&rows=10"
                      "&facet=true&facet.mincount=1&facet.field=date"
                      "&facet.field=title_exact&f.title_exact.facet.limit=17"
                      " HTTP/1.1\r\n"
                      "User-Agent: YAZ/" YAZ_VERSION "\r\n"
                      "Host: localhost\r\n\r\n"));
    }

    odr_destroy(odr);
/* YAZ_HAVE_XML2 */
#endif
}

static int check_response(ODR o, const char *content,
                          Z_SRW_searchRetrieveResponse **p)
{
    int r;
    Z_GDU *gdu;
    Z_SRW_PDU *sr_p;
    char *http_response = odr_malloc(o, strlen(content) + 300);
    const char *start =
           "HTTP/1.1 200 OK\r\n"
           "Last-Modified: Wed, 13 Apr 2011 08:30:59 GMT\r\n"
           "ETag: \"MjcyMWE5M2JiNDgwMDAwMFNvbHI=\"\r\n"
           "Content-Type: text/xml; charset=utf-8\r\n";
    yaz_snprintf(http_response, strlen(content) + 300,
            "%sContent-Length: %d\r\n\r\n%s", start, (int) strlen(content), content);
    odr_setbuf(o, http_response, strlen(http_response), 0);

    *p = 0;
    r = z_GDU(o, &gdu, 0, 0);
    if (!r || gdu->which != Z_GDU_HTTP_Response)
        return 0;
    r = yaz_solr_decode_response(o, gdu->u.HTTP_Response, &sr_p);
    if (r)
        return 0;
    if (sr_p->which != Z_SRW_searchRetrieve_response)
        return 0;
    *p = sr_p->u.response;
    return 1;
}

static int check_srw_response(ODR o, const char *content,
                              Z_SRW_searchRetrieveResponse **p)
{
    int r;
    char *http_response = odr_malloc(o, strlen(content) + 300);

    const char *start =
           "HTTP/1.1 200 OK\r\n"
           "Last-Modified: Wed, 13 Apr 2011 08:30:59 GMT\r\n"
           "ETag: \"MjcyMWE5M2JiNDgwMDAwMFNvbHI=\"\r\n"
           "Content-Type: text/xml; charset=utf-8\r\n";
    yaz_snprintf(http_response, strlen(content) + 300,
            "%sContent-Length: %d\r\n\r\n%s", start, (int) strlen(content), content);

    odr_setbuf(o, http_response, strlen(http_response), 0);

    *p = 0;
    Z_GDU *gdu = 0;
    r = z_GDU(o, &gdu, 0, 0);
    if (!r || gdu->which != Z_GDU_HTTP_Response)
        return 0;
#if YAZ_HAVE_XML2
    Z_HTTP_Response *hres = gdu->u.HTTP_Response;
    Z_SOAP_Handler soap_handlers[4] = {
        {YAZ_XMLNS_SRU_v1_response, 0, (Z_SOAP_fun) yaz_srw_codec},
        {YAZ_XMLNS_SRU_v2_mask, 0, (Z_SOAP_fun) yaz_srw_codec},
        {"searchRetrieveResponse", 0, (Z_SOAP_fun) yaz_srw_codec},
        {0, 0, 0}
    };
    Z_SOAP *soap_package = 0;
    int ret;
    ret = z_soap_codec(o, &soap_package,
                       &hres->content_buf, &hres->content_len,
                       soap_handlers);
    if (ret == 0 && soap_package->which == Z_SOAP_generic)
    {
        Z_SRW_PDU *sr = (Z_SRW_PDU*) soap_package->u.generic->p;
        if (sr->which == Z_SRW_searchRetrieve_response)
            *p = sr->u.response;
        else
        {
            yaz_log(YLOG_WARN, "sr->which=%d", sr->which);
            return 0;
        }
    }
    else
    {
        yaz_log(YLOG_WARN, "z_soap_codec returned ret=%d", ret);
        return 0;
    }
#endif
    return 1;
}


void tst_decoding(void)
{
#if YAZ_HAVE_XML2
    ODR odr = odr_createmem(ODR_DECODE);
    Z_SRW_searchRetrieveResponse *response;

    YAZ_CHECK(check_response(
                  odr,
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                  "<response>\n"
                  "<lst name=\"responseHeader\"><int name=\"status\">0</int>"
                  "<int name=\"QTime\">1</int><lst name=\"params\">"
                  "<str name=\"start\">0</str><str name=\"q\">@attr 1=title solr</str>"
                  "<str name=\"rows\">0</str></lst>"
                  "</lst><result name=\"response\" numFound=\"91\" start=\"0\"/>\n"
                  "</response>\n", &response));
    if (response)
    {
        YAZ_CHECK_EQ(*response->numberOfRecords, 91);
        YAZ_CHECK_EQ(response->num_records, 0);
        YAZ_CHECK(response->records == 0);
        YAZ_CHECK_EQ(response->num_diagnostics, 0);
        YAZ_CHECK(response->diagnostics == 0);
        YAZ_CHECK(response->nextRecordPosition == 0);
        YAZ_CHECK(response->facetList == 0);
    }
    odr_reset(odr);


    YAZ_CHECK(check_response(
                  odr,
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                  "<response>\n"
                  "  <lst name=\"responseHeader\">\n"
                  "    <int name=\"status\">0</int>\n"
                  "    <int name=\"QTime\">1</int>\n"
                  "    <lst name=\"params\">"
                  "       <str name=\"start\">0</str>\n"
                  "       <str name=\"q\">@attr 1=title solr</str>\n"
                  "       <str name=\"rows\">0</str>\n"
                  "    </lst>"
                  "  </lst>\n"
                  "  <result name=\"response\" numFound=\"0\" start=\"0\"/>\n"
                  "  <lst name=\"spellcheck\">\n"
                  "     <lst name=\"suggestions\">\n"
                  "       <lst name=\"author\">\n"
                  "         <int name=\"numFound\">1</int>\n"
                  "         <arr name=\"suggestion\">\n"
                  "           <lst>\n"
                  "              <str name=\"word\">w1</str>\n"
                  "              <int name=\"freq\">1</int>\n"
                  "           </lst>\n"
                  "           <foo/>\n"
                  "           <lst>\n"
                  "              <str name=\"word\">w2</str>\n"
                  "           </lst>\n"
                  "         </arr>\n"
                  "       </lst>\n"
                  "       <lst name=\"The &lt; title\">\n"
                  "         <arr name=\"suggestion\">\n"
                  "           <lst>\n"
                  "              <str name=\"word\">a&amp;b</str>\n"
                  "           </lst>\n"
                  "         </arr>\n"
                  "       </lst>\n"
                  "     </lst>\n"
                  "  </lst>"
                  "</response>\n", &response));
    if (response)
    {
        YAZ_CHECK_EQ(*response->numberOfRecords, 0);
        YAZ_CHECK_EQ(response->num_records, 0);
        YAZ_CHECK(response->records == 0);
        YAZ_CHECK_EQ(response->num_diagnostics, 0);
        YAZ_CHECK(response->diagnostics == 0);
        YAZ_CHECK(response->nextRecordPosition == 0);
        YAZ_CHECK(response->facetList == 0);
        YAZ_CHECK(strcmp(response->suggestions,
                         "<misspelled term=\"author\">\n"
                         "<suggestion>w1</suggestion>\n"
                         "<suggestion>w2</suggestion>\n"
                         "</misspelled>\n"
                         "<misspelled term=\"The &lt; title\">\n"
                         "<suggestion>a&amp;b</suggestion>\n"
                         "</misspelled>\n") == 0);
    }
    odr_reset(odr);




    YAZ_CHECK(
        check_response(
            odr,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<response><lst name=\"responseHeader\">"
            "<int name=\"status\">0</int><int name=\"QTime\">2</int>"
            "<lst name=\"params\"><str name=\"facet\">true</str>"
            "<str name=\"facet.mincount\">1</str><str name=\"start\">0</str>"
            "<str name=\"q\">@attr 1=title solr</str>"
            "<str name=\"f.date.facet.limit\">5</str>"
            "<str name=\"facet.field\">date</str>"
            "<str name=\"rows\">1</str></lst>"
            "</lst><result name=\"response\" numFound=\"91000000000\" start=\"0\">"
            "<doc><str name=\"author\">Alenius, Hans,</str>\n"
            "<str name=\"author-date\">1937-</str>"
            "<str name=\"author-title\"/>"
            "<arr name=\"date\"><str>1969</str></arr>"
            "<str name=\"id\">   73857731 </str>"
            "<arr name=\"lccn\"><str>   73857731 </str></arr>"
            "<arr name=\"medium\"><str>book</str></arr>"
            "<arr name=\"medium_exact\"><str>book</str></arr>"
            "<arr name=\"physical-accomp\"><str/></arr>"
            "<arr name=\"physical-dimensions\"><str>20 cm.</str></arr>"
            "<arr name=\"physical-extent\"><str>140, (1) p.</str></arr>"
            "<arr name=\"physical-format\"><str>illus.</str></arr>"
            "<arr name=\"physical-specified\"><str/></arr>"
            "<arr name=\"physical-unitsize\"><str/></arr>"
            "<arr name=\"physical-unittype\"><str/></arr>"
            "<arr name=\"publication-date\"><str>1969.</str></arr>"
            "<arr name=\"publication-name\"><str>Norstedt,</str></arr>"
            "<arr name=\"publication-place\"><str>Stockholm,</str></arr>"
            "<arr name=\"subject\"><str>Photography</str><str>Artistic</str></arr>"
            "<arr name=\"subject-long\"><str>Photography, Artistic.</str></arr>"
            "<arr name=\"subject_exact\"><str>Photography</str><str>Artistic</str></arr>"
            "<arr name=\"system-control-nr\"><str>(OCoLC)36247690</str></arr>"
            "<str name=\"title\">Solring.</str><str name=\"title-complete\">Solring.</str>"
            "<str name=\"title-dates\"/><str name=\"title-medium\"/>"
            "<str name=\"title-number-section\"/><str name=\"title-remainder\"/>"
            "<str name=\"title-responsibility\"/><str name=\"title_exact\">Solring.</str>"
            "</doc></result>\n"
            "<lst name=\"facet_counts\">\n"
            "  <lst name=\"facet_queries\"/>\n"
            "  <lst name=\"facet_fields\">\n"
            "    <lst name=\"date\">\n"
            "      <int name=\"1978\">5000000000</int>\n"
            "      <int name=\"1983\">4</int>\n"
            "      <int name=\"1987\">4</int>\n"
            "      <int name=\"1988\">4</int>\n"
            "      <int name=\"2003\">3</int>\n"
            "    </lst>\n"
            "    <lst name=\"keyword\">\n"
            "      <int name=\"Soleil\">37</int>\n"
            "      <int name=\"Sun\">26</int>\n"
            "    <lst>\n"
            "      <int name=\"no name property , ignored\">3</int>\n"
            "    </lst>\n"
            "    <foreign>x</foreign>\n"
            "    </lst>\n"
            "    <lst name=\"empty1\">\n"
            "    </lst>\n"
            "    <lst name=\"empty2\"/>"
            "  </lst>\n"
            "<lst name=\"facet_dates\"/>\n"
            "</lst>\n"
            "</response>", &response));
    if (response)
    {
#if HAVE_LONG_LONG
        YAZ_CHECK(*response->numberOfRecords == 91000000000LL);
#endif
        YAZ_CHECK_EQ(response->num_records, 1);
        YAZ_CHECK(response->records);
    }
    if (response && response->records)
    {
        const char *doc =
            "<doc><str name=\"author\">Alenius, Hans,</str>\n"
            "<str name=\"author-date\">1937-</str>"
            "<str name=\"author-title\"/>"
            "<arr name=\"date\"><str>1969</str></arr>"
            "<str name=\"id\">   73857731 </str>"
            "<arr name=\"lccn\"><str>   73857731 </str></arr>"
            "<arr name=\"medium\"><str>book</str></arr>"
            "<arr name=\"medium_exact\"><str>book</str></arr>"
            "<arr name=\"physical-accomp\"><str/></arr>"
            "<arr name=\"physical-dimensions\"><str>20 cm.</str></arr>"
            "<arr name=\"physical-extent\"><str>140, (1) p.</str></arr>"
            "<arr name=\"physical-format\"><str>illus.</str></arr>"
            "<arr name=\"physical-specified\"><str/></arr>"
            "<arr name=\"physical-unitsize\"><str/></arr>"
            "<arr name=\"physical-unittype\"><str/></arr>"
            "<arr name=\"publication-date\"><str>1969.</str></arr>"
            "<arr name=\"publication-name\"><str>Norstedt,</str></arr>"
            "<arr name=\"publication-place\"><str>Stockholm,</str></arr>"
            "<arr name=\"subject\"><str>Photography</str><str>Artistic</str></arr>"
            "<arr name=\"subject-long\"><str>Photography, Artistic.</str></arr>"
            "<arr name=\"subject_exact\"><str>Photography</str><str>Artistic</str></arr>"
            "<arr name=\"system-control-nr\"><str>(OCoLC)36247690</str></arr>"
            "<str name=\"title\">Solring.</str><str name=\"title-complete\">Solring.</str>"
            "<str name=\"title-dates\"/><str name=\"title-medium\"/>"
            "<str name=\"title-number-section\"/><str name=\"title-remainder\"/>"
            "<str name=\"title-responsibility\"/><str name=\"title_exact\">Solring.</str>"
            "</doc>";

        Z_SRW_record *record = response->records;

        YAZ_CHECK(record->recordData_len == strlen(doc) &&
                  !memcmp(record->recordData_buf, doc, record->recordData_len));
    }
    if (response)
    {
        YAZ_CHECK_EQ(response->num_diagnostics, 0);
        YAZ_CHECK(response->diagnostics == 0);
        YAZ_CHECK(response->nextRecordPosition == 0);
        YAZ_CHECK(response->facetList);
    }
    if (response && response->facetList)
    {
        Z_FacetList *facetList = response->facetList;

        YAZ_CHECK(facetList->num == 4);
        if (facetList->num >= 1)
        {
            Z_FacetField *facetField = facetList->elements[0];
            int i;
            YAZ_CHECK(strcmp("date",
                facetField->attributes->attributes[0]->value.complex->list[0]->u.string) == 0);
            YAZ_CHECK(facetField->num_terms == 5);
            if (facetField->num_terms == 5)
            {
                for (i = 0; i < facetField->num_terms; i++)
                {
                    YAZ_CHECK(
                        facetField->terms[i] &&
                        facetField->terms[i]->term &&
                        facetField->terms[i]->term->which == Z_Term_general);
                }
#if HAVE_LONG_LONG
                YAZ_CHECK(*facetField->terms[0]->count == 5000000000LL);
#endif
                YAZ_CHECK(facetField->terms[0]->term->u.general->len == 4
                          && !memcmp(facetField->terms[0]->term->u.general->buf,
                                     "1978", 4));
                YAZ_CHECK(*facetField->terms[1]->count == 4);
                YAZ_CHECK(facetField->terms[1]->term->u.general->len == 4
                          && !memcmp(facetField->terms[1]->term->u.general->buf,
                                     "1983", 4));
                YAZ_CHECK(*facetField->terms[2]->count == 4);
                YAZ_CHECK(facetField->terms[2]->term->u.general->len == 4
                          && !memcmp(facetField->terms[2]->term->u.general->buf,
                                     "1987", 4));
                YAZ_CHECK(*facetField->terms[3]->count == 4);
                YAZ_CHECK(facetField->terms[3]->term->u.general->len == 4
                          && !memcmp(facetField->terms[3]->term->u.general->buf,
                                     "1988", 4));
                YAZ_CHECK(*facetField->terms[4]->count == 3);
                YAZ_CHECK(facetField->terms[4]->term->u.general->len == 4
                          && !memcmp(facetField->terms[4]->term->u.general->buf,
                                     "2003", 4));
            }
        }
        if (facetList->num >= 2)
        {
            Z_FacetField *facetField = facetList->elements[1];
            int i;

            YAZ_CHECK(strcmp("keyword",
                facetField->attributes->attributes[0]->value.complex->list[0]->u.string) == 0);
            YAZ_CHECK(facetField->num_terms == 2);
            if (facetField->num_terms == 2)
            {
                for (i = 0; i < facetField->num_terms; i++)
                {
                    YAZ_CHECK(
                        facetField->terms[i] &&
                        facetField->terms[i]->term &&
                        facetField->terms[i]->term->which == Z_Term_general);
                }
                YAZ_CHECK(*facetField->terms[0]->count == 37);
                YAZ_CHECK(facetField->terms[0]->term->u.general->len == 6
                          && !memcmp(facetField->terms[0]->term->u.general->buf,
                                     "Soleil", 6));
                YAZ_CHECK(*facetField->terms[1]->count == 26);
                YAZ_CHECK(facetField->terms[1]->term->u.general->len == 3
                          && !memcmp(facetField->terms[1]->term->u.general->buf,
                                     "Sun", 3));
            }
        }
        if (facetList->num >= 3)
        {
            Z_FacetField *facetField = facetList->elements[2];

            YAZ_CHECK(strcmp("empty1",
                facetField->attributes->attributes[0]->value.complex->list[0]->u.string) == 0);
            YAZ_CHECK(facetField->num_terms == 0);
        }
        if (facetList->num >= 4)
        {
            Z_FacetField *facetField = facetList->elements[3];

            YAZ_CHECK(strcmp("empty2",
                facetField->attributes->attributes[0]->value.complex->list[0]->u.string) == 0);
            YAZ_CHECK(facetField->num_terms == 0);
        }
    }
    odr_reset(odr);

    YAZ_CHECK(check_srw_response(
                  odr,
"<zs:searchRetrieveResponse xmlns:zs=\"http://www.loc.gov/zing/srw/\">\n"
"  <version>1.1</version>\n"
"  <numberOfRecords>1698</numberOfRecords>\n"
"  <records>\n"
"  </records>\n"
"  <nextRecordPosition>22</nextRecordPosition>\n"
"  <echoedSearchRetrieveRequest>\n"
"    <version>1.1</version>\n"
"    <query>stvo</query>\n"
"    <startRecord>1</startRecord>\n"
"    <maximumRecords>20</maximumRecords>\n"
"    <recordPacking>xml</recordPacking>\n"
"    <recordSchema>dc</recordSchema>\n"
"  </echoedSearchRetrieveRequest>\n"
"  <zs:facetedResults xmlns:fr=\"http://docs.oasis-open.org/ns/search-ws/sru-facetedResults\">\n"
"      <fr:facet>\n"
"        <fr:displayLabel>category</fr:displayLabel>\n"
"        <fr:description>category</fr:description>\n"
"        <fr:index>source</fr:index>\n"
"        <fr:relation>=</fr:relation>\n"
"        <fr:terms>\n"
"          <fr:term>\n"
"            <fr:actualTerm>Zivilrecht</fr:actualTerm>\n"
"            <fr:query>stvo</fr:query>\n"
"            <fr:requestUrl>http://somewhere.de/rest/anwalt/search.ashx?mode=v2&amp;itemsPerPage=20&amp;format=rss+opensearch+mylex-facets&amp;sortOption=relevance&amp;qid=H4sIAAAAAAAEAO29B2AcSZYlJi9tynt%2fSvVK1%2bB0oQiAYBMk2JBAEOzBiM3mkuwdaUcjKasqgcplVmVdZhZAzO2dvPfee%2b%2b999577733ujudTif33%2f8%2fXGZkAWz2zkrayZ4hgKrIHz9%2bfB8%2fIrLlVVa2j5bVLypm%2fw%2bGbBtBDAAAAA%3d%3d&amp;pageNr=0&amp;refine=anwalt:rg_v2.1,,,&amp;search=stvo&amp;useSynonyms=0&amp;useVariations=0</fr:requestUrl>\n"
"            <fr:count>5</fr:count>\n"
"          </fr:term>\n"
"          <fr:term>\n"
"            <fr:actualTerm>Arbeits- und Sozialrecht</fr:actualTerm>\n"
"            <fr:query>stvo</fr:query>\n"
"            <fr:requestUrl>http://somewhere.de/rest/anwalt/search.ashx?mode=v2&amp;itemsPerPage=20&amp;format=rss+opensearch+mylex-facets&amp;sortOption=relevance&amp;qid=H4sIAAAAAAAEAO29B2AcSZYlJi9tynt%2fSvVK1%2bB0oQiAYBMk2JBAEOzBiM3mkuwdaUcjKasqgcplVmVdZhZAzO2dvPfee%2b%2b999577733ujudTif33%2f8%2fXGZkAWz2zkrayZ4hgKrIHz9%2bfB8%2fIrLlVVa2j5bVLypm%2fw%2bGbBtBDAAAAA%3d%3d&amp;pageNr=0&amp;refine=anwalt:rg_v2.8,,,&amp;search=stvo&amp;useSynonyms=0&amp;useVariations=0</fr:requestUrl>\n"
"            <fr:count>4</fr:count>\n"
"          </fr:term>\n"
"        </fr:terms>\n"
"      </fr:facet>\n"
"  </zs:facetedResults>\n"
"</zs:searchRetrieveResponse>"
                  , &response));
    if (response)
    {
        YAZ_CHECK_EQ(*response->numberOfRecords, 1698);
        YAZ_CHECK_EQ(response->num_records, 0);
        YAZ_CHECK(response->records == 0);
        YAZ_CHECK_EQ(response->num_diagnostics, 0);
        YAZ_CHECK(response->diagnostics == 0);
        YAZ_CHECK(response->nextRecordPosition != 0);
        YAZ_CHECK(response->facetList != 0);
    }
    odr_reset(odr);

    odr_destroy(odr);
#endif
}

void tst_yaz_700(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    int r;
    const char *url =
        "http://localhost:9036/XXX/cproxydebug-7/node102/p/105/c=content_connector"
        "a=usr/pw#&? r=cfusr/cfpw p=1.2.3.4:80/www.indexdata.com/staff/";
    int use_full_host = 0;
    Z_GDU *gdu_req = z_get_HTTP_Request_uri(odr, url, 0, use_full_host);
    Z_HTTP_Request *hreq = gdu_req->u.HTTP_Request;
    hreq->method = "GET";

    hreq->content_buf = odr_strdup(odr, "");
    hreq->content_len = 0;

    r = z_GDU(odr, &gdu_req, 0, 0);
    YAZ_CHECK(r);
    if (r)
    {
        int len;
        char *buf = odr_getbuf(odr, &len, 0);
        ODR decode = odr_createmem(ODR_DECODE);
        YAZ_CHECK(buf);
        if (buf)
        {
            odr_setbuf(decode, buf, len, 0);
            r = z_GDU(decode, &gdu_req, 0, 0);
            YAZ_CHECK(r);
        }
        odr_destroy(decode);
    }
    odr_destroy(odr);
}


int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if YAZ_HAVE_XML2
    LIBXML_TEST_VERSION;
#endif
    tst_encoding();
    tst_decoding();
    tst_yaz_700();
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

