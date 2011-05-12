/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <yaz/srw.h>
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
        
        YAZ_CHECK(compare_solr_req(
                      odr, sr, 0,
                      "GET Default/select? HTTP/1.1\r\n"
                      "User-Agent: YAZ/" YAZ_VERSION "\r\n"
                      "Host: localhost\r\n"
                      "Content-Type: text/xml\r\n\r\n"));
    }

    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(odr, Z_SRW_searchRetrieve_request,
                                        "1.2");
        YAZ_CHECK(compare_solr_req(
                      odr, sr, "utf-8",
                      "GET Default/select? HTTP/1.1\r\n"
                      "User-Agent: YAZ/" YAZ_VERSION "\r\n"
                      "Host: localhost\r\n"
                      "Content-Type: text/xml; charset=utf-8\r\n\r\n"));
    }

    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(odr, Z_SRW_searchRetrieve_request,
                                        "1.2");
        
        sr->u.request->query_type = Z_SRW_query_type_cql;
        sr->u.request->query.cql = "title:solr";
        sr->u.request->startRecord = odr_intdup(odr, 3);
        sr->u.request->maximumRecords = odr_intdup(odr, 10);

        YAZ_CHECK(compare_solr_req(
                      odr, sr, 0,
                      "GET Default/select?q=title%3Asolr&start=2&rows=10"
                      " HTTP/1.1\r\n"
                      "User-Agent: YAZ/" YAZ_VERSION "\r\n"
                      "Host: localhost\r\n"
                      "Content-Type: text/xml\r\n\r\n"));
    }

    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(odr, Z_SRW_searchRetrieve_request,
                                        "1.2");
        
        sr->u.request->query_type = Z_SRW_query_type_cql;
        sr->u.request->query.cql = "title:solr";
        sr->u.request->startRecord = odr_intdup(odr, 3);
        sr->u.request->maximumRecords = odr_intdup(odr, 10);
        sr->u.request->facetList = yaz_pqf_parse_facet_list(
            odr, "@attr 1=date @attr 2=0, @attr 1=title_exact @attr 3=17");

        YAZ_CHECK(compare_solr_req(
                      odr, sr, 0,
                      "GET Default/select?q=title%3Asolr&start=2&rows=10"
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
    YAZ_CHECK_EQ(*response->numberOfRecords, 91);
    YAZ_CHECK_EQ(response->num_records, 0);
    YAZ_CHECK(response->records == 0);
    YAZ_CHECK_EQ(response->num_diagnostics, 0);
    YAZ_CHECK(response->diagnostics == 0);
    YAZ_CHECK(response->nextRecordPosition == 0);

    odr_reset(odr);
    odr_destroy(odr);
#endif
}
int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if YAZ_HAVE_XML2
    LIBXML_TEST_VERSION;
#endif
    tst_encoding();
    tst_decoding();
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

