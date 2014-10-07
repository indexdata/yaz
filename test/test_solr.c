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
                      "Host: localhost\r\n"
                      "Content-Type: text/xml\r\n\r\n"));
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
                      "Host: localhost\r\n"
                      "Content-Type: text/xml; charset=utf-8\r\n\r\n"));
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
                      "Host: localhost\r\n"
                      "Content-Type: text/xml\r\n\r\n"));
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
                      "Host: localhost\r\n"
                      "Content-Type: text/xml\r\n\r\n"));
    }

    odr_destroy(odr);
/* YAZ_HAVE_XML2 */
#endif
}


int check_response(ODR o, const char *content, Z_SRW_searchRetrieveResponse **p)
{
    int r;
    Z_GDU *gdu;
    Z_SRW_PDU *sr_p;
    char *http_response = odr_malloc(o, strlen(content) + 300);

    strcpy(http_response,
           "HTTP/1.1 200 OK\r\n"
           "Last-Modified: Wed, 13 Apr 2011 08:30:59 GMT\r\n"
           "ETag: \"MjcyMWE5M2JiNDgwMDAwMFNvbHI=\"\r\n"
           "Content-Type: text/xml; charset=utf-8\r\n");
    sprintf(http_response + strlen(http_response),
            "Content-Length: %d\r\n\r\n", (int) strlen(content));
    strcat(http_response, content);

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
            "<doc><str name=\"author\">Alenius, Hans,</str>"
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
            "</doc></result><lst name=\"facet_counts\">"
            "<lst name=\"facet_queries\"/>"
            "<lst name=\"facet_fields\">"
            "<lst name=\"date\"><int name=\"1978\">5000000000</int><int name=\"1983\">4</int>"
            "<int name=\"1987\">4</int><int name=\"1988\">4</int>"
            "<int name=\"2003\">3</int></lst></lst><lst name=\"facet_dates\"/>"
            "</lst></response>", &response));
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
            "<doc><str name=\"author\">Alenius, Hans,</str>"
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

        YAZ_CHECK(facetList->num == 1);
        if (facetList->num == 1)
        {
            Z_FacetField *facetField = facetList->elements[0];
            int i;

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
//    tst_encoding();
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

