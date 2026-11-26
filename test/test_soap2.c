/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <yaz/test.h>
#include <yaz/srw.h>
#include <yaz/soap.h>
#include <yaz/wrbuf.h>

#if YAZ_HAVE_XML2
#include <libxml/parser.h>

static void check_soap_namespace(const char *xml, const char *error)
{
    ODR o = odr_createmem(ODR_DECODE);
    Z_SOAP_Handler handlers[] = {
        { YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec },
        { 0, 0, 0 }
    };
    Z_SOAP *soap = 0;
    char *buf = (char *) xml;
    int len = strlen(xml);
    Z_SRW_PDU *sr = 0;
    char *charset = 0;

    YAZ_CHECK_EQ(z_soap_codec(o, &soap, &buf, &len, handlers), error ? -1 : 0);
    YAZ_CHECK(soap != 0);
    if (soap)
    {
        YAZ_CHECK_EQ(soap->which, error ? Z_SOAP_error : Z_SOAP_generic);
        if (error && soap->which == Z_SOAP_error)
        {
            YAZ_CHECK(!strcmp(soap->u.soap_error->fault_code, "SOAP-ENV:Client"));
            YAZ_CHECK(!strcmp(soap->u.soap_error->fault_string, error));
        }
    }
    odr_reset(o);
    soap = 0;
    Z_HTTP_Request *request = z_get_HTTP_Request(o)->u.HTTP_Request;
    request->method = "POST";
    request->content_buf = (char *) xml;
    request->content_len = strlen(xml);
    z_HTTP_header_add(o, &request->headers, "Content-Type", "text/xml");
    YAZ_CHECK_EQ(yaz_srw_decode(request, &sr, &soap, o, &charset), error ? 1 : 0);
    YAZ_CHECK(error ? sr == 0 : sr != 0);
    YAZ_CHECK(soap && soap->which == (error ? Z_SOAP_error : Z_SOAP_generic));
    odr_destroy(o);
}

static void tst_soap_namespaces(void)
{
    const char *ns[] = {
        "http://schemas.xmlsoap.org/soap/envelope/",
        "http://www.w3.org/2001/06/soap-envelope"
    };
    const char *payload = "<searchRetrieveRequest xmlns=\""
        YAZ_XMLNS_SRU_v1_1 "\"><version>1.1</version>"
        "<query>test</query></searchRetrieveRequest>";
    const struct {
        const char *header;
        const char *body;
        const char *error;
    } cases[] = {
        { "<Header/>", "s:Body", "SOAP Body element not found" },
        { "", "Body", "SOAP bad NS for Body element" },
        { "<s:Header/>", "Body", "SOAP bad NS for Body element" },
        { "<x:Header xmlns:x=\"urn:wrong\"/>", "s:Body",
          "SOAP Body element not found" },
        { "", "x:Body", "SOAP bad NS for Body element" },
        { "", "s:Body", 0 },
        { "<s:Header/>", "s:Body", 0 }
    };
    size_t i, j;
    WRBUF xml = wrbuf_alloc();
    for (i = 0; i < sizeof(ns) / sizeof(*ns); i++)
    {
        for (j = 0; j < sizeof(cases) / sizeof(*cases); j++)
        {
            wrbuf_rewind(xml);
            wrbuf_printf(xml, "<s:Envelope xmlns:s=\"%s\" xmlns:x=\"urn:wrong\">"
                         "%s<%s>%s</%s></s:Envelope>", ns[i], cases[j].header,
                         cases[j].body, payload, cases[j].body);
            check_soap_namespace(wrbuf_cstr(xml), cases[j].error);
        }
        /* Default namespaces qualify elements; xmlns="" removes them. */
        wrbuf_rewind(xml);
        wrbuf_printf(xml, "<Envelope xmlns=\"%s\"><Header/><Body>%s</Body>"
                     "</Envelope>", ns[i], payload);
        check_soap_namespace(wrbuf_cstr(xml), 0);
        wrbuf_rewind(xml);
        wrbuf_printf(xml, "<Envelope xmlns=\"%s\"><Header xmlns=\"\"/>"
                     "<Body>%s</Body></Envelope>", ns[i], payload);
        check_soap_namespace(wrbuf_cstr(xml), "SOAP Body element not found");
        wrbuf_rewind(xml);
        wrbuf_printf(xml, "<Envelope xmlns=\"%s\"><Header/>"
                     "<Body xmlns=\"\">%s</Body></Envelope>", ns[i], payload);
        check_soap_namespace(wrbuf_cstr(xml), "SOAP bad NS for Body element");
    }
    wrbuf_destroy(xml);
}

static void tst_scan_response_terms(void)
{
    /* only term elements count; other nodes must not advance the index */
    const char *xml = "<scanResponse xmlns=\"" YAZ_XMLNS_SRU_v1_1 "\">\n"
        "<version>1.1</version>\n"
        "<terms>\n"
        " <!-- first -->\n"
        " <term><value>a</value><numberOfRecords>1</numberOfRecords></term>\n"
        " <other/>\n"
        " <term><value>b</value><displayTerm>B</displayTerm></term>\n"
        " <term><value>c</value></term>\n"
        "</terms>\n"
        "</scanResponse>";
    ODR o = odr_createmem(ODR_DECODE);
    Z_SOAP_Handler handlers[] = {
        { YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec },
        { 0, 0, 0 }
    };
    Z_SOAP *soap = 0;
    char *buf = (char *) xml;
    int len = strlen(xml);

    YAZ_CHECK_EQ(z_soap_codec(o, &soap, &buf, &len, handlers), 0);
    YAZ_CHECK(soap && soap->which == Z_SOAP_generic);
    if (soap && soap->which == Z_SOAP_generic)
    {
        Z_SRW_PDU *sr = (Z_SRW_PDU *) soap->u.generic->p;

        YAZ_CHECK(sr && sr->which == Z_SRW_scan_response);
        if (sr && sr->which == Z_SRW_scan_response)
        {
            Z_SRW_scanResponse *res = sr->u.scan_response;

            YAZ_CHECK_EQ(res->num_terms, 3);
            if (res->num_terms == 3)
            {
                Z_SRW_scanTerm *t = res->terms;

                YAZ_CHECK(t[0].value && !strcmp(t[0].value, "a"));
                YAZ_CHECK(t[0].numberOfRecords &&
                          *t[0].numberOfRecords == 1);
                YAZ_CHECK(t[0].displayTerm == 0);
                YAZ_CHECK(t[1].value && !strcmp(t[1].value, "b"));
                YAZ_CHECK(t[1].numberOfRecords == 0);
                YAZ_CHECK(t[1].displayTerm && !strcmp(t[1].displayTerm, "B"));
                YAZ_CHECK(t[2].value && !strcmp(t[2].value, "c"));
                YAZ_CHECK(t[2].whereInList == 0);
            }
        }
    }
    odr_destroy(o);
}

static void tst_ucp_methods(void)
{
    const struct {
        const char *method;
        int which;
    } cases[] = {
        { "updateRequest", Z_SRW_update_request },
        { "updateResponse", Z_SRW_update_response },
        /* no PDU type for these: must be rejected */
        { "explainUpdateRequest", 0 },
        { "explainUpdateResponse", 0 }
    };
    size_t i;
    WRBUF xml = wrbuf_alloc();
    for (i = 0; i < sizeof(cases) / sizeof(*cases); i++)
    {
        ODR o = odr_createmem(ODR_DECODE);
        Z_SOAP *soap = 0;
        Z_SRW_PDU *sr = 0;
        char *charset = 0;
        Z_HTTP_Request *request = z_get_HTTP_Request(o)->u.HTTP_Request;

        wrbuf_rewind(xml);
        wrbuf_printf(xml, "<u:%s xmlns:u=\"" YAZ_XMLNS_UPDATE_v0_9 "\"/>",
                     cases[i].method);
        request->method = "POST";
        request->content_buf = (char *) wrbuf_cstr(xml);
        request->content_len = wrbuf_len(xml);
        z_HTTP_header_add(o, &request->headers, "Content-Type", "text/xml");
        YAZ_CHECK_EQ(yaz_srw_decode(request, &sr, &soap, o, &charset),
                     cases[i].which ? 0 : 1);
        YAZ_CHECK(soap && soap->which ==
                  (cases[i].which ? Z_SOAP_generic : Z_SOAP_error));
        if (cases[i].which)
        {
            YAZ_CHECK(sr && sr->which == cases[i].which);
        }
        else
        {
            YAZ_CHECK(sr == 0);
        }
        odr_destroy(o);
    }
    wrbuf_destroy(xml);
}

static void tst_srw_facet_analysis(void)
{
    /* Only facet elements become entries in the decoded facet list. */
    const char *xml = "<searchRetrieveResponse xmlns=\""
        YAZ_XMLNS_SRU_v1_1 "\"><version>1.1</version>"
        "<numberOfRecords>1</numberOfRecords>"
        "<facet_analysis><facets><other/>"
        "<facet code=\"author\">"
        "<facetvalue est_representation=\"3\">Smith</facetvalue>"
        "<other/></facet>"
        "<other/></facets></facet_analysis>"
        "</searchRetrieveResponse>";
    ODR o = odr_createmem(ODR_DECODE);
    Z_SOAP_Handler handlers[] = {
        { YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec },
        { 0, 0, 0 }
    };
    Z_SOAP *soap = 0;
    char *buf = (char *) xml;
    int len = strlen(xml);
    Z_SRW_PDU *sr = 0;

    YAZ_CHECK_EQ(z_soap_codec(o, &soap, &buf, &len, handlers), 0);
    YAZ_CHECK(soap && soap->which == Z_SOAP_generic);
    if (soap && soap->which == Z_SOAP_generic)
        sr = (Z_SRW_PDU *) soap->u.generic->p;
    YAZ_CHECK(sr && sr->which == Z_SRW_searchRetrieve_response);
    if (sr && sr->which == Z_SRW_searchRetrieve_response)
    {
        Z_FacetList *fl = sr->u.response->facetList;

        YAZ_CHECK(fl);
        if (fl)
        {
            YAZ_CHECK_EQ(fl->num, 1);
            if (fl->num == 1)
            {
                YAZ_CHECK(fl->elements[0]);
                if (fl->elements[0])
                    YAZ_CHECK_EQ(fl->elements[0]->num_terms, 1);
            }
        }
    }
    odr_destroy(o);
}

static void tst_srw(void)
{
    const char *charset = 0;
    char *content_buf = 0;
    int content_len;
    ODR o = odr_createmem(ODR_ENCODE);
    Z_SOAP_Handler h[2] = {
        {"http://www.loc.gov/zing/srw/", 0, (Z_SOAP_fun) yaz_srw_codec},
        {0, 0, 0}
    };
    Z_SRW_PDU *sr = yaz_srw_get(o, Z_SRW_searchRetrieve_request);
    Z_SOAP *p = (Z_SOAP *) odr_malloc(o, sizeof(*p));

    YAZ_CHECK(o);
    YAZ_CHECK(sr);
    YAZ_CHECK(p);
#if 0
    sr->u.request->query = "jordb" "\xe6" "r";
#else
    sr->u.request->query = "jordbaer";
#endif

    p->which = Z_SOAP_generic;
    p->u.generic = (Z_SOAP_Generic *) odr_malloc(o, sizeof(*p->u.generic));
    p->u.generic->no = 0;
    p->u.generic->ns = 0;
    p->u.generic->p = sr;
    p->ns = "http://schemas.xmlsoap.org/soap/envelope/";

    int ret = z_soap_codec_enc(o, &p, &content_buf, &content_len, h, charset);
    odr_destroy(o);
    YAZ_CHECK(ret == 0);  /* codec failed ? */
}

/* decodes a response that declares a namespace with a name of uri_len
   bytes, with data in recordData (which 0), extraRecordData (1) or
   extraResponseData (2). Returns the data as decoded, or null if the
   response was not accepted */
static const char *srw_data_ns(ODR o, int uri_len, int which,
                               const char *data)
{
    Z_SOAP_Handler h[2] = {
        { YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec },
        { 0, 0, 0 }
    };
    Z_SOAP *soap = 0;
    WRBUF w = wrbuf_alloc();
    char *buf;
    int len, i;
    const char *res = 0;

    wrbuf_puts(w, "<s:Envelope xmlns:s=\""
               "http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>"
               "<searchRetrieveResponse xmlns=\"" YAZ_XMLNS_SRU_v1_1 "\""
               " xmlns:a=\"");
    for (i = 0; i < uri_len; i++)
        wrbuf_putc(w, 'u');
    wrbuf_puts(w, "\"><version>1.1</version>"
               "<numberOfRecords>1</numberOfRecords><records><record>"
               "<recordData>");
    wrbuf_puts(w, which == 0 ? data : "x");
    wrbuf_puts(w, "</recordData>");
    if (which == 1)
    {
        wrbuf_puts(w, "<extraRecordData>");
        wrbuf_puts(w, data);
        wrbuf_puts(w, "</extraRecordData>");
    }
    wrbuf_puts(w, "</record></records>");
    if (which == 2)
    {
        wrbuf_puts(w, "<extraResponseData>");
        wrbuf_puts(w, data);
        wrbuf_puts(w, "</extraResponseData>");
    }
    wrbuf_puts(w, "</searchRetrieveResponse></s:Body></s:Envelope>");
    buf = (char *) wrbuf_buf(w);
    len = wrbuf_len(w);
    if (z_soap_codec(o, &soap, &buf, &len, h) == 0)
    {
        YAZ_CHECK(soap && soap->which == Z_SOAP_generic);
        if (soap && soap->which == Z_SOAP_generic)
        {
            Z_SRW_PDU *sr = (Z_SRW_PDU *) soap->u.generic->p;
            YAZ_CHECK(sr && sr->which == Z_SRW_searchRetrieve_response &&
                      sr->u.response->num_records == 1);
            if (sr && sr->which == Z_SRW_searchRetrieve_response &&
                sr->u.response->num_records == 1)
            {
                Z_SRW_searchRetrieveResponse *r = sr->u.response;
                if (which == 0)
                {
                    res = r->records[0].recordData_buf;
                    len = r->records[0].recordData_len;
                }
                else if (which == 1 && r->extra_records[0])
                {
                    res = r->extra_records[0]->extraRecordData_buf;
                    len = r->extra_records[0]->extraRecordData_len;
                }
                else if (which == 2)
                {
                    res = sr->extraResponseData_buf;
                    len = sr->extraResponseData_len;
                }
                YAZ_CHECK(res && len == (int) strlen(res));
            }
        }
    }
    else
    {
        YAZ_CHECK(soap && soap->which == Z_SOAP_error &&
                  !strcmp(soap->u.soap_error->fault_string,
                          "SOAP Handler returned error"));
    }
    wrbuf_destroy(w);
    return res;
}

/* a record gets a declaration of each namespace that it inherits. Every
   record, and every element at the top of it, gets a copy: they are limited
   to 256 bytes and 8 times the size of the record as it is in the message */
static void tst_srw_record_ns(void)
{
    ODR o = odr_createmem(ODR_DECODE);
    WRBUF w = wrbuf_alloc();
    const char *res;
    int i;

    res = srw_data_ns(o, 5, 0, "<a:r/>");
    YAZ_CHECK(res && !strcmp(res, "<a:r xmlns:a=\"uuuuu\"/>"));

    res = srw_data_ns(o, 5, 0, "<a:r/><a:r/>");
    YAZ_CHECK(res && !strcmp(res, "<yaz_record><a:r xmlns:a=\"uuuuu\"/>"
                             "<a:r xmlns:a=\"uuuuu\"/></yaz_record>"));

    /* 6 bytes get 11 + 293 = 256 + 8 * 6 more */
    res = srw_data_ns(o, 293, 0, "<a:r/>");
    YAZ_CHECK(res && strlen(res) == 6 + 11 + 293);
    YAZ_CHECK(srw_data_ns(o, 294, 0, "<a:r/>") == 0);
    YAZ_CHECK(srw_data_ns(o, 1000, 0, "<r xmlns=\"urn:r\"><r a:b=\"\"/></r>")
              == 0);

    /* a long name is fine for a record that is not small .. */
    res = srw_data_ns(o, 1000, 0, "<a:r>012345678901234567890123456789012345"
                      "6789012345678901234567890123456789012345678901234567"
                      "89</a:r>");
    YAZ_CHECK(res && strlen(res) == 101 + 11 + 1000);

    /* .. or that does not use the namespace */
    res = srw_data_ns(o, 1000, 0, "<r xmlns=\"urn:r\"/>");
    YAZ_CHECK(res && !strcmp(res, "<r xmlns=\"urn:r\"/>"));

    /* many elements at the top of a record */
    for (i = 0; i < 200; i++)
        wrbuf_puts(w, "<a:title>The title</a:title>");
    res = srw_data_ns(o, 60, 0, wrbuf_cstr(w));
    YAZ_CHECK(res && strlen(res) == 200 * (28 + 11 + 60) + 25);

    wrbuf_rewind(w);
    for (i = 0; i < 200; i++)
        wrbuf_puts(w, "<a:r/>");
    YAZ_CHECK(srw_data_ns(o, 30, 0, wrbuf_cstr(w)) != 0);
    YAZ_CHECK(srw_data_ns(o, 60, 0, wrbuf_cstr(w)) == 0);

    /* the same for extraRecordData and extraResponseData */
    for (i = 1; i <= 2; i++)
    {
        res = srw_data_ns(o, 5, i, "<a:r/><a:r/>");
        YAZ_CHECK(res && !strcmp(res, "<a:r xmlns:a=\"uuuuu\"/>"
                                 "<a:r xmlns:a=\"uuuuu\"/>"));
        YAZ_CHECK(srw_data_ns(o, 1000, i, "<a:r/><a:r/>") == 0);
    }

    wrbuf_destroy(w);
    odr_destroy(o);
}

/* recordData_len must always match recordData_buf, also when an empty
   recordData follows one with content. */
static void check_srw_record_data(const char *record_data, const char *expect)
{
    ODR o = odr_createmem(ODR_DECODE);
    Z_SOAP_Handler h[2] = {
        {YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec},
        {0, 0, 0}
    };
    Z_SOAP *soap = 0;
    WRBUF xml = wrbuf_alloc();
    char *buf;
    int len;

    wrbuf_printf(xml, "<s:Envelope xmlns:s=\""
                 "http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>"
                 "<searchRetrieveResponse xmlns=\"" YAZ_XMLNS_SRU_v1_1 "\">"
                 "<version>1.1</version><numberOfRecords>1</numberOfRecords>"
                 "<records><record><recordSchema>dc</recordSchema>%s"
                 "<recordPosition>1</recordPosition></record></records>"
                 "</searchRetrieveResponse></s:Body></s:Envelope>",
                 record_data);
    buf = (char *) wrbuf_cstr(xml);
    len = (int) wrbuf_len(xml);
    YAZ_CHECK_EQ(z_soap_codec(o, &soap, &buf, &len, h), 0);
    YAZ_CHECK(soap && soap->which == Z_SOAP_generic);
    if (soap && soap->which == Z_SOAP_generic)
    {
        Z_SRW_PDU *sr = (Z_SRW_PDU *) soap->u.generic->p;
        YAZ_CHECK(sr && sr->which == Z_SRW_searchRetrieve_response);
        if (sr && sr->which == Z_SRW_searchRetrieve_response)
        {
            Z_SRW_searchRetrieveResponse *res = sr->u.response;
            YAZ_CHECK_EQ(res->num_records, 1);
            if (res->num_records == 1)
            {
                Z_SRW_record *rec = res->records;
                YAZ_CHECK(rec->recordData_buf != 0);
                YAZ_CHECK_EQ(rec->recordData_len, (int) strlen(expect));
                YAZ_CHECK(rec->recordData_buf &&
                          rec->recordData_len == (int) strlen(expect) &&
                          !memcmp(rec->recordData_buf, expect,
                                  rec->recordData_len));
            }
        }
    }
    wrbuf_destroy(xml);
    odr_destroy(o);
}

static void tst_srw_record_data(void)
{
    check_srw_record_data("<recordData>payload</recordData>", "payload");
    check_srw_record_data("<recordData/>", "");
    check_srw_record_data("<recordData></recordData>", "");
    check_srw_record_data("<recordData>payload</recordData><recordData/>", "");
    check_srw_record_data("<recordData><a/></recordData><recordData/>", "");
    check_srw_record_data("<recordData/><recordData>payload</recordData>",
                          "payload");
}

static void check_sru_basic_auth(const char *authorization,
                                 const char *username, const char *password)
{
    ODR o = odr_createmem(ODR_DECODE);
    Z_HTTP_Request *request = z_get_HTTP_Request(o)->u.HTTP_Request;
    Z_SRW_PDU *sr = 0;
    Z_SOAP *soap = 0;
    char *charset = 0;
    Z_SRW_diagnostic *diag = 0;
    int num_diag = 0;

    request->method = "GET";
    request->path = "/Default?version=1.1&operation=searchRetrieve&query=a";
    z_HTTP_header_add(o, &request->headers, "Authorization", authorization);
    YAZ_CHECK_EQ(yaz_sru_decode(request, &sr, &soap, o, &charset,
                                &diag, &num_diag), 0);
    YAZ_CHECK(sr != 0);
    if (sr)
    {
        YAZ_CHECK(username ?
                  sr->username && !strcmp(sr->username, username) :
                  sr->username == 0);
        YAZ_CHECK(password ?
                  sr->password && !strcmp(sr->password, password) :
                  sr->password == 0);
    }
    odr_destroy(o);
}

static void tst_sru_basic_auth(void)
{
    check_sru_basic_auth("Basic dXNlcjpwYXNz", "user", "pass");
    check_sru_basic_auth("Basic dXNlcg==", "user", 0);
    /* not Base64: must be ignored rather than partially decoded */
    check_sru_basic_auth("Basic dXNlcjpwYXNz!!!!", 0, 0);
    check_sru_basic_auth("Basic !!!!", 0, 0);
    check_sru_basic_auth("Basic dXNlcjpwYXN!", 0, 0);
}
#endif

static void tst_array_to_uri(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    char **names;
    char **values;
    char *query_string;

    int r = yaz_uri_to_array("&", odr, &names, &values);
    YAZ_CHECK_EQ(r, 0);
    if (r == 0)
    {
        YAZ_CHECK(names[0] == 0);
        YAZ_CHECK(values[0] == 0);
    }
    r = yaz_uri_to_array("&&", odr, &names, &values);
    YAZ_CHECK_EQ(r, 0);
    if (r == 0)
    {
        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, ""));

        YAZ_CHECK(names[0] == 0);
        YAZ_CHECK(values[0] == 0);
    }
    r = yaz_uri_to_array("a=AA&bb=%42B", odr, &names, &values);
    YAZ_CHECK_EQ(r, 2);
    if (r == 2)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], "BB"));

        YAZ_CHECK(names[2] == 0);
        YAZ_CHECK(values[2] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=BB"));
    }
    r = yaz_uri_to_array("a=AA&bb&x", odr, &names, &values);
    YAZ_CHECK_EQ(r, 3);
    if (r == 3)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], ""));

        YAZ_CHECK(names[2] && !strcmp(names[2], "x"));
        YAZ_CHECK(values[2] && !strcmp(values[1], ""));

        YAZ_CHECK(names[3] == 0);
        YAZ_CHECK(values[3] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=&x="));
    }

    r = yaz_uri_to_array("a=AA&bb=&x=", odr, &names, &values);
    YAZ_CHECK_EQ(r, 3);
    if (r == 3)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], ""));

        YAZ_CHECK(names[2] && !strcmp(names[2], "x"));
        YAZ_CHECK(values[2] && !strcmp(values[1], ""));

        YAZ_CHECK(names[3] == 0);
        YAZ_CHECK(values[3] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=&x="));
    }

    r = yaz_uri_to_array("a=AA&&&bb&x&&", odr, &names, &values);
    YAZ_CHECK_EQ(r, 3);
    if (r == 3)
    {
        YAZ_CHECK(names[0] && !strcmp(names[0], "a"));
        YAZ_CHECK(values[0] && !strcmp(values[0], "AA"));

        YAZ_CHECK(names[1] && !strcmp(names[1], "bb"));
        YAZ_CHECK(values[1] && !strcmp(values[1], ""));

        YAZ_CHECK(names[2] && !strcmp(names[2], "x"));
        YAZ_CHECK(values[2] && !strcmp(values[2], ""));

        YAZ_CHECK(names[3] == 0);
        YAZ_CHECK(values[3] == 0);

        yaz_array_to_uri(&query_string, odr, names, values);
        YAZ_CHECK(!strcmp(query_string, "a=AA&bb=&x="));
    }

    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
#if YAZ_HAVE_XML2
    LIBXML_TEST_VERSION;
    tst_srw();
    tst_soap_namespaces();
    tst_scan_response_terms();
    tst_ucp_methods();
    tst_srw_facet_analysis();
    tst_srw_record_ns();
    tst_srw_record_data();
    tst_sru_basic_auth();
#endif
    tst_array_to_uri();
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

