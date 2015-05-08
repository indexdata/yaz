/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file srwutil.c
 * \brief Implements SRW/SRU utilities.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <yaz/srw.h>
#include <yaz/matchstr.h>
#include <yaz/base64.h>
#include <yaz/yaz-iconv.h>
#include "sru-p.h"

#define MAX_SRU_PARAMETERS 30

static Z_SRW_extra_arg **append_extra_arg(ODR odr, Z_SRW_extra_arg **l,
                                          const char *n, const char *v)
{
    if (n && v && *v != '\0')
    {
        while (*l)
            l = &(*l)->next;
        *l = (Z_SRW_extra_arg *) odr_malloc(odr, sizeof(**l));
        (*l)->name = odr_strdup(odr, n);
        (*l)->value = odr_strdup(odr, v);
        (*l)->next = 0;
        l = &(*l)->next;
    }
    return l;
}

static Z_SRW_extra_arg **append_extra_arg_int(ODR odr, Z_SRW_extra_arg **l,
                                              const char *n, Odr_int *v)
{
    if (v)
    {
        char str[32];
        sprintf(str, ODR_INT_PRINTF, *v);
        l = append_extra_arg(odr, l, n, str);
    }
    return l;
}

static char *yaz_decode_sru_dbpath_odr(ODR n, const char *uri, size_t len)
{
    return odr_strdupn(n, uri, len);
}

void yaz_encode_sru_dbpath_buf(char *dst, const char *db)
{
    assert(db);
    *dst = '/';
    strcpy(dst+1, db);
}

char *yaz_encode_sru_dbpath_odr(ODR out, const char *db)
{
    char *dst = odr_malloc(out, 3 * strlen(db) + 2);
    yaz_encode_sru_dbpath_buf(dst, db);
    return dst;
}

#if YAZ_HAVE_XML2
const char *yaz_element_attribute_value_get(xmlNodePtr ptr,
                                            const char *node_name,
                                            const char *attribute_name)
{
    struct _xmlAttr *attr;
    // check if the node name matches
    if (strcmp((const char*) ptr->name, node_name))
        return 0;
    // check if the attribute name and return the value
    for (attr = ptr->properties; attr; attr = attr->next)
        if (attr->children && attr->children->type == XML_TEXT_NODE)
        {
            if (!strcmp((const char *) attr->name, attribute_name))
                return (const char *) attr->children->content;
        }
    return 0;
}
#endif

int yaz_srw_check_content_type(Z_HTTP_Response *hres)
{
    const char *content_type = z_HTTP_header_lookup(hres->headers,
                                                    "Content-Type");
    if (content_type)
    {
        if (!yaz_strcmp_del("text/xml", content_type, "; "))
            return 1;
        if (!yaz_strcmp_del("application/xml", content_type, "; "))
            return 1;
        if (!yaz_strcmp_del("application/sru+xml", content_type, "; "))
            return 1;
    }
    return 0;
}

/**
 * Look for authentication tokens in HTTP Basic parameters or in x-username/x-password
 * parameters. Added by SH.
 */
static void yaz_srw_decodeauth(Z_SRW_PDU *sr, Z_HTTP_Request *hreq,
                               char *username, char *password, ODR decode)
{
    const char *basic = z_HTTP_header_lookup(hreq->headers, "Authorization");

    if (username)
        sr->username = username;
    if (password)
        sr->password = password;

    if (basic)
    {
        int len;
        char out[256];
        char ubuf[256] = "", pbuf[256] = "", *p;
        if (strncmp(basic, "Basic ", 6))
            return;
        basic += 6;
        len = strlen(basic);
        if (!len || len > 256)
            return;
        yaz_base64decode(basic, out);
        /* Format of out should be username:password at this point */
        strcpy(ubuf, out);
        if ((p = strchr(ubuf, ':')))
        {
            *(p++) = '\0';
            if (*p)
                strcpy(pbuf, p);
        }
        if (*ubuf)
            sr->username = odr_strdup(decode, ubuf);
        if (*pbuf)
            sr->password = odr_strdup(decode, pbuf);
    }
}

void yaz_mk_srw_diagnostic(ODR o, Z_SRW_diagnostic *d,
                           const char *uri, const char *message,
                           const char *details)
{
    d->uri = odr_strdup(o, uri);
    if (message)
        d->message = odr_strdup(o, message);
    else
        d->message = 0;
    if (details)
        d->details = odr_strdup(o, details);
    else
        d->details = 0;
}

void yaz_mk_std_diagnostic(ODR o, Z_SRW_diagnostic *d,
                           int code, const char *details)
{
    char uri[40];

    sprintf(uri, "info:srw/diagnostic/1/%d", code);
    yaz_mk_srw_diagnostic(o, d, uri, 0, details);
}

void yaz_add_srw_diagnostic_uri(ODR o, Z_SRW_diagnostic **d,
                                int *num, const char *uri,
                                const char *message, const char *details)
{
    Z_SRW_diagnostic *d_new;
    d_new = (Z_SRW_diagnostic *) odr_malloc(o, (*num + 1)* sizeof(**d));
    if (*num)
        memcpy(d_new, *d, *num *sizeof(**d));
    *d = d_new;

    yaz_mk_srw_diagnostic(o, *d + *num, uri, message, details);
    (*num)++;
}

void yaz_add_srw_diagnostic(ODR o, Z_SRW_diagnostic **d,
                            int *num, int code, const char *addinfo)
{
    char uri[40];

    sprintf(uri, "info:srw/diagnostic/1/%d", code);
    yaz_add_srw_diagnostic_uri(o, d, num, uri, 0, addinfo);
}


void yaz_add_sru_update_diagnostic(ODR o, Z_SRW_diagnostic **d,
                                   int *num, int code, const char *addinfo)
{
    char uri[40];

    sprintf(uri, "info:srw/diagnostic/12/%d", code);
    yaz_add_srw_diagnostic_uri(o, d, num, uri, 0, addinfo);
}


void yaz_mk_sru_surrogate(ODR o, Z_SRW_record *record, int pos,
                          int code, const char *details)
{
    const char *message = yaz_diag_srw_str(code);
    int len = 200;
    if (message)
        len += strlen(message);
    if (details)
        len += strlen(details);

    record->recordData_buf = (char *) odr_malloc(o, len);

    sprintf(record->recordData_buf, "<diagnostic "
            "xmlns=\"http://www.loc.gov/zing/srw/diagnostic/\">\n"
            " <uri>info:srw/diagnostic/1/%d</uri>\n", code);
    if (details)
        sprintf(record->recordData_buf + strlen(record->recordData_buf),
                " <details>%s</details>\n", details);
    if (message)
        sprintf(record->recordData_buf + strlen(record->recordData_buf),
                " <message>%s</message>\n", message);
    sprintf(record->recordData_buf + strlen(record->recordData_buf),
            "</diagnostic>\n");
    record->recordData_len = strlen(record->recordData_buf);
    record->recordPosition = odr_intdup(o, pos);
    record->recordSchema = "info:srw/schema/1/diagnostics-v1.1";
}

static void grab_charset(ODR o, const char *content_type, char **charset)
{
    if (charset)
    {
        const char *charset_p = 0;
        if (content_type && (charset_p = strstr(content_type, "; charset=")))
        {
            int j = 0, i = 0;
            int sep = 0;
            charset_p += 10; /* skip ; charset=  */
            if (charset_p[i] == '"' || charset_p[i] == '\'')
                sep = charset_p[i++];
            *charset = odr_strdup(o, charset_p);
            while (charset_p[i] && charset_p[i] != sep)
            {
                if (!sep && strchr("; \n\r", charset_p[i]))
                    break;
                if (charset_p[i] == '\\' && charset_p[i+1])
                    i++;
                (*charset)[j++] = charset_p[i++];
            }
            (*charset)[j] = '\0';
        }
    }
}

int yaz_srw_decode(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
                   Z_SOAP **soap_package, ODR decode, char **charset)
{
    if (!strcmp(hreq->method, "POST"))
    {
        const char *content_type = z_HTTP_header_lookup(hreq->headers,
                                                        "Content-Type");
        if (content_type &&
            (!yaz_strcmp_del("text/xml", content_type, "; ") ||
             !yaz_strcmp_del("application/soap+xml", content_type, "; ") ||
             !yaz_strcmp_del("text/plain", content_type, "; ")))
        {
            char *db = "Default";
            const char *p0 = hreq->path, *p1;
            int ret = -1;

            static Z_SOAP_Handler soap_handlers[5] = {
#if YAZ_HAVE_XML2
                { YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec },
                { YAZ_XMLNS_SRU_v1_0, 0, (Z_SOAP_fun) yaz_srw_codec },
                { YAZ_XMLNS_UPDATE_v0_9, 0, (Z_SOAP_fun) yaz_ucp_codec },
                { YAZ_XMLNS_SRU_v2_mask, 0, (Z_SOAP_fun) yaz_srw_codec },
#endif
                {0, 0, 0}
            };

            if (*p0 == '/')
                p0++;
            p1 = strchr(p0, '?');
            if (!p1)
                p1 = p0 + strlen(p0);
            if (p1 != p0)
                db = yaz_decode_sru_dbpath_odr(decode, p0, p1 - p0);

            ret = z_soap_codec(decode, soap_package,
                               &hreq->content_buf, &hreq->content_len,
                               soap_handlers);
            if (ret == 0 && (*soap_package)->which == Z_SOAP_generic)
            {
                *srw_pdu = (Z_SRW_PDU*) (*soap_package)->u.generic->p;
                yaz_srw_decodeauth(*srw_pdu, hreq, 0, 0, decode);

                /* last entry in handlers - SRU 2.0 - is turned into
                   offset 0.. due to other pieces relying on it */
                if ((*soap_package)->u.generic->no == 3)
                    (*soap_package)->u.generic->no = 0;
                if ((*srw_pdu)->which == Z_SRW_searchRetrieve_request &&
                    (*srw_pdu)->u.request->database == 0)
                    (*srw_pdu)->u.request->database = db;

                if ((*srw_pdu)->which == Z_SRW_explain_request &&
                    (*srw_pdu)->u.explain_request->database == 0)
                    (*srw_pdu)->u.explain_request->database = db;

                if ((*srw_pdu)->which == Z_SRW_scan_request &&
                    (*srw_pdu)->u.scan_request->database == 0)
                    (*srw_pdu)->u.scan_request->database = db;

                if ((*srw_pdu)->which == Z_SRW_update_request &&
                    (*srw_pdu)->u.update_request->database == 0)
                    (*srw_pdu)->u.update_request->database = db;

                return 0;
            }
            return 1;
        }
    }
    return 2;
}

#if YAZ_HAVE_XML2
static int yaz_sru_decode_integer(ODR odr, const char *pname,
                                  const char *valstr, Odr_int **valp,
                                  Z_SRW_diagnostic **diag, int *num_diag,
                                  int min_value)
{
    int ival;
    if (!valstr)
        return 0;
    if (sscanf(valstr, "%d", &ival) != 1)
    {
        yaz_add_srw_diagnostic(odr, diag, num_diag,
                               YAZ_SRW_UNSUPP_PARAMETER_VALUE, pname);
        return 0;
    }
    if (min_value >= 0 && ival < min_value)
    {
        yaz_add_srw_diagnostic(odr, diag, num_diag,
                               YAZ_SRW_UNSUPP_PARAMETER_VALUE, pname);
        return 0;
    }
    *valp = odr_intdup(odr, ival);
    return 1;
}
#endif

/**
   http://www.loc.gov/z3950/agency/zing/srw/service.html
*/
int yaz_sru_decode(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
                   Z_SOAP **soap_package, ODR decode, char **charset,
                   Z_SRW_diagnostic **diag, int *num_diag)
{
#if YAZ_HAVE_XML2
    static Z_SOAP_Handler soap_handlers[2] = {
        {YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec},
        {0, 0, 0}
    };
#endif
    const char *content_type = z_HTTP_header_lookup(hreq->headers,
                                                    "Content-Type");

    /*
      SRU GET: ignore content type.
      SRU POST: we support "application/x-www-form-urlencoded";
      not  "multipart/form-data" .
    */
    if (!strcmp(hreq->method, "GET")
        ||
        (!strcmp(hreq->method, "POST") && content_type &&
         !yaz_strcmp_del("application/x-www-form-urlencoded",
                         content_type, "; ")))
    {
        char *db = "Default";
        const char *p0 = hreq->path, *p1;
#if YAZ_HAVE_XML2
        const char *operation = 0;
        char *version = 0;
        char *query = 0;
        char *queryType = "cql";
        char *username = 0;
        char *password = 0;
        char *sortKeys = 0;
        char *stylesheet = 0;
        char *scanClause = 0;
        char *recordXPath = 0;
        char *recordSchema = 0;
        char *recordXMLEscaping = 0;
        char *recordPacking = 0;
        char *maximumRecords = 0;
        char *startRecord = 0;
        char *maximumTerms = 0;
        char *responsePosition = 0;
        const char *facetLimit = 0;
        const char *facetStart = 0;
        const char *facetSort = 0;
        Z_SRW_extra_arg *extra_args = 0;
#endif
        char **uri_name;
        char **uri_val;

        grab_charset(decode, content_type, charset);
        if (charset && *charset == 0 && !strcmp(hreq->method, "GET"))
            *charset = "UTF-8";

        if (*p0 == '/')
            p0++;
        p1 = strchr(p0, '?');
        if (!p1)
            p1 = p0 + strlen(p0);
        if (p1 != p0)
            db = yaz_decode_sru_dbpath_odr(decode, p0, p1 - p0);
        if (!strcmp(hreq->method, "POST") && hreq->content_buf)
            p1 = hreq->content_buf;
        yaz_uri_to_array(p1, decode, &uri_name, &uri_val);
#if YAZ_HAVE_XML2
        if (uri_name)
        {
            int i;
            for (i = 0; uri_name[i]; i++)
            {
                char *n = uri_name[i];
                char *v = uri_val[i];
                if (!strcmp(n, "query"))
                    query = v;
                else if (!strcmp(n, "x-pquery"))
                {
                    query = v;
                    queryType = "pqf";
                }
                else if (!strcmp(n, "queryType"))
                    queryType = v;
                else if (!strcmp(n, "x-username"))
                    username = v;
                else if (!strcmp(n, "x-password"))
                    password = v;
                else if (!strcmp(n, "operation"))
                    operation = v;
                else if (!strcmp(n, "stylesheet"))
                    stylesheet = v;
                else if (!strcmp(n, "sortKeys"))
                    sortKeys = v;
                else if (!strcmp(n, "recordXPath"))
                    recordXPath = v;
                else if (!strcmp(n, "recordSchema"))
                    recordSchema = v;
                else if (!strcmp(n, "recordPacking"))
                    recordPacking = v;
                else if (!strcmp(n, "recordXMLEscaping"))
                    recordXMLEscaping = v;
                else if (!strcmp(n, "version"))
                    version = v;
                else if (!strcmp(n, "scanClause"))
                    scanClause = v;
                else if (!strcmp(n, "x-pScanClause"))
                {
                    scanClause = v;
                    queryType = "pqf";
                }
                else if (!strcmp(n, "maximumRecords"))
                    maximumRecords = v;
                else if (!strcmp(n, "startRecord"))
                    startRecord = v;
                else if (!strcmp(n, "maximumTerms"))
                    maximumTerms = v;
                else if (!strcmp(n, "responsePosition"))
                    responsePosition = v;
                else if (!strcmp(n, "facetLimit"))
                    facetLimit = v;
                else if (!strcmp(n, "facetStart"))
                    facetStart = v;
                else if (!strcmp(n, "facetSort"))
                    facetSort = v;
                else if (!strcmp(n, "extraRequestData"))
                    ; /* ignoring extraRequestData */
                else if (n[0] == 'x' && n[1] == '-')
                {
                    append_extra_arg(decode, &extra_args, n, v);
                }
                else
                {
                    if (*num_diag < 10)
                        yaz_add_srw_diagnostic(decode, diag, num_diag,
                                               YAZ_SRW_UNSUPP_PARAMETER, n);
                }
            }
        }
        if (!operation)
        {
            if (query)
                operation = "searchRetrieve";
            else if (scanClause)
                operation = "scan";
            else
                operation = "explain";
        }
        version = yaz_negotiate_sru_version(version);

        if (!version)
        {   /* negotiation failed. */
            yaz_add_srw_diagnostic(decode, diag, num_diag,
                                   YAZ_SRW_UNSUPP_VERSION, "2.0");
            version = "2.0";
        }
        if (!operation)
        {
            if (uri_name)
                yaz_add_srw_diagnostic(
                    decode, diag, num_diag,
                    YAZ_SRW_MANDATORY_PARAMETER_NOT_SUPPLIED, "operation");
            operation = "explain";
        }
        if (strcmp(version, "2.0"))
        {
            if (recordXMLEscaping)
            {
                yaz_add_srw_diagnostic(decode, diag, num_diag,
                                       YAZ_SRW_UNSUPP_PARAMETER,
                                       "recordXMLEscaping");

            }
            recordXMLEscaping = recordPacking;
            recordPacking = "packed";
        }
        if (!recordXMLEscaping)
            recordXMLEscaping = "xml";
        if (!strcmp(operation, "searchRetrieve"))
        {
            Z_SRW_PDU *sr = yaz_srw_get(decode, Z_SRW_searchRetrieve_request);

            sr->srw_version = version;
            sr->extra_args = extra_args;
            *srw_pdu = sr;
            yaz_srw_decodeauth(sr, hreq, username, password, decode);

            sr->u.request->queryType = queryType;
            sr->u.request->query = query;

            if (!query)
                yaz_add_srw_diagnostic(
                    decode, diag, num_diag,
                    YAZ_SRW_MANDATORY_PARAMETER_NOT_SUPPLIED, "query");

            if (sortKeys)
            {
                sr->u.request->sort_type = Z_SRW_sort_type_sort;
                sr->u.request->sort.sortKeys = sortKeys;
            }
            sr->u.request->recordXPath = recordXPath;
            sr->u.request->recordSchema = recordSchema;
            sr->u.request->recordPacking = recordXMLEscaping;
            sr->u.request->packing = recordPacking;
            sr->u.request->stylesheet = stylesheet;
            yaz_sru_facet_request(decode , &sr->u.request->facetList,
                                  &facetLimit, &facetStart, &facetSort);

            yaz_sru_decode_integer(decode, "maximumRecords", maximumRecords,
                                   &sr->u.request->maximumRecords,
                                   diag, num_diag, 0);

            yaz_sru_decode_integer(decode, "startRecord", startRecord,
                                   &sr->u.request->startRecord,
                                   diag, num_diag, 1);

            sr->u.request->database = db;

            (*soap_package) = (Z_SOAP *)
                odr_malloc(decode, sizeof(**soap_package));
            (*soap_package)->which = Z_SOAP_generic;

            (*soap_package)->u.generic = (Z_SOAP_Generic *)
                odr_malloc(decode, sizeof(*(*soap_package)->u.generic));

            (*soap_package)->u.generic->p = sr;
            (*soap_package)->u.generic->ns = soap_handlers[0].ns;
            (*soap_package)->u.generic->no = 0;

            (*soap_package)->ns = "SRU";

            return 0;
        }
        else if (!strcmp(operation, "explain"))
        {
            /* Transfer SRU explain parameters to common struct */
            /* http://www.loc.gov/z3950/agency/zing/srw/explain.html */
            Z_SRW_PDU *sr = yaz_srw_get(decode, Z_SRW_explain_request);

            sr->srw_version = version;
            sr->extra_args = extra_args;
            yaz_srw_decodeauth(sr, hreq, username, password, decode);
            *srw_pdu = sr;
            sr->u.explain_request->recordPacking = recordXMLEscaping;
            sr->u.explain_request->packing = recordPacking;
            sr->u.explain_request->database = db;

            sr->u.explain_request->stylesheet = stylesheet;

            (*soap_package) = (Z_SOAP *)
                odr_malloc(decode, sizeof(**soap_package));
            (*soap_package)->which = Z_SOAP_generic;

            (*soap_package)->u.generic = (Z_SOAP_Generic *)
                odr_malloc(decode, sizeof(*(*soap_package)->u.generic));

            (*soap_package)->u.generic->p = sr;
            (*soap_package)->u.generic->ns = soap_handlers[0].ns;
            (*soap_package)->u.generic->no = 0;

            (*soap_package)->ns = "SRU";

            return 0;
        }
        else if (!strcmp(operation, "scan"))
        {
            /* Transfer SRU scan parameters to common struct */
            /* http://www.loc.gov/z3950/agency/zing/srw/scan.html */
            Z_SRW_PDU *sr = yaz_srw_get(decode, Z_SRW_scan_request);

            sr->srw_version = version;
            sr->extra_args = extra_args;
            *srw_pdu = sr;
            yaz_srw_decodeauth(sr, hreq, username, password, decode);

            sr->u.scan_request->queryType = queryType;
            sr->u.scan_request->scanClause = scanClause;

            if (!scanClause)
                yaz_add_srw_diagnostic(
                    decode, diag, num_diag,
                    YAZ_SRW_MANDATORY_PARAMETER_NOT_SUPPLIED, "scanClause");
            sr->u.scan_request->database = db;

            yaz_sru_decode_integer(decode, "maximumTerms",
                                   maximumTerms,
                                   &sr->u.scan_request->maximumTerms,
                                   diag, num_diag, 0);

            yaz_sru_decode_integer(decode, "responsePosition",
                                   responsePosition,
                                   &sr->u.scan_request->responsePosition,
                                   diag, num_diag, 0);

            sr->u.scan_request->stylesheet = stylesheet;

            (*soap_package) = (Z_SOAP *)
                odr_malloc(decode, sizeof(**soap_package));
            (*soap_package)->which = Z_SOAP_generic;

            (*soap_package)->u.generic = (Z_SOAP_Generic *)
                odr_malloc(decode, sizeof(*(*soap_package)->u.generic));

            (*soap_package)->u.generic->p = sr;
            (*soap_package)->u.generic->ns = soap_handlers[0].ns;
            (*soap_package)->u.generic->no = 0;

            (*soap_package)->ns = "SRU";

            return 0;
        }
        else
        {
            /* unsupported operation ... */
            /* Act as if we received a explain request and throw diagnostic. */

            Z_SRW_PDU *sr = yaz_srw_get(decode, Z_SRW_explain_request);

            sr->srw_version = version;
            *srw_pdu = sr;
            sr->u.explain_request->recordPacking = recordPacking;
            sr->u.explain_request->database = db;

            sr->u.explain_request->stylesheet = stylesheet;

            (*soap_package) = (Z_SOAP *)
                odr_malloc(decode, sizeof(**soap_package));
            (*soap_package)->which = Z_SOAP_generic;

            (*soap_package)->u.generic = (Z_SOAP_Generic *)
                odr_malloc(decode, sizeof(*(*soap_package)->u.generic));

            (*soap_package)->u.generic->p = sr;
            (*soap_package)->u.generic->ns = soap_handlers[0].ns;
            (*soap_package)->u.generic->no = 0;

            (*soap_package)->ns = "SRU";

            yaz_add_srw_diagnostic(decode, diag, num_diag,
                                   YAZ_SRW_UNSUPP_OPERATION, operation);
            return 0;
        }
#else
        return 1;
#endif
    }
    return 2;
}

Z_SRW_extra_record *yaz_srw_get_extra_record(ODR o)
{
    Z_SRW_extra_record *res = (Z_SRW_extra_record *)
        odr_malloc(o, sizeof(*res));

    res->extraRecordData_buf = 0;
    res->extraRecordData_len = 0;
    res->recordIdentifier = 0;
    return res;
}


Z_SRW_record *yaz_srw_get_records(ODR o, int n)
{
    Z_SRW_record *res = (Z_SRW_record *) odr_malloc(o, n * sizeof(*res));
    int i;

    for (i = 0; i<n; i++)
    {
        res[i].recordSchema = 0;
        res[i].recordPacking = Z_SRW_recordPacking_string;
        res[i].recordData_buf = 0;
        res[i].recordData_len = 0;
        res[i].recordPosition = 0;
    }
    return res;
}

Z_SRW_record *yaz_srw_get_record(ODR o)
{
    return yaz_srw_get_records(o, 1);
}

static Z_SRW_PDU *yaz_srw_get_core_ver(ODR o, const char *version)
{
    Z_SRW_PDU *p = (Z_SRW_PDU *) odr_malloc(o, sizeof(*p));
    p->srw_version = odr_strdup(o, version);
    p->username = 0;
    p->password = 0;
    p->extra_args = 0;
    p->extraResponseData_buf = 0;
    p->extraResponseData_len = 0;
    return p;
}

Z_SRW_PDU *yaz_srw_get_core_v_2_0(ODR o)
{
    return yaz_srw_get_core_ver(o, "2.0");
}

Z_SRW_PDU *yaz_srw_get(ODR o, int which)
{
    return yaz_srw_get_pdu(o, which, "2.0");
}

/* http://docs.oasis-open.org/search-ws/searchRetrieve/v1.0/os/schemas/sruResponse.xsd */
Z_SRW_PDU *yaz_srw_get_pdu_e(ODR o, int which, Z_SRW_PDU *req)
{
    int version2 = !req->srw_version || strcmp(req->srw_version, "2.") > 0;
    Z_SRW_PDU *res = yaz_srw_get_pdu(o, which, req->srw_version);
    Z_SRW_extra_arg **l = &res->extra_args, *ea;
    l = append_extra_arg(o, l, "version", req->srw_version);
    if (req->which == Z_SRW_searchRetrieve_request &&
        which == Z_SRW_searchRetrieve_response)
    {
        if (req->u.request->queryType &&
            strcmp(req->u.request->queryType, "cql"))
            l = append_extra_arg(o, l, "queryType", req->u.request->queryType);
        l = append_extra_arg(o, l, "query", req->u.request->query);
        l = append_extra_arg_int(o, l, "startRecord",
                                 req->u.request->startRecord);
        l = append_extra_arg_int(o, l, "maximumRecords",
                                 req->u.request->maximumRecords);
        if (version2)
        {
            l = append_extra_arg(o, l, "recordXMLEscaping",
                                 req->u.request->recordPacking);
            l = append_extra_arg(o, l, "recordPacking",
                                 req->u.request->packing);
        }
        else
            l = append_extra_arg(o, l, "recordPacking",
                                 req->u.request->recordPacking);
        l = append_extra_arg(o, l, "recordSchema",
                             req->u.request->recordSchema);
        if (req->u.request->sort_type == Z_SRW_sort_type_sort)
            l = append_extra_arg(o, l, "sortKeys",
                                 req->u.request->sort.sortKeys);
        l = append_extra_arg(o, l, "stylesheet", req->u.request->stylesheet);
    }
    if (req->which == Z_SRW_explain_request &&
        which == Z_SRW_explain_response)
    {
        if (version2)
        {
            l = append_extra_arg(o, l, "recordXMLEscaping",
                                 req->u.explain_request->recordPacking);
            l = append_extra_arg(o, l, "recordPacking",
                                 req->u.explain_request->packing);
        }
        else
            l = append_extra_arg(o, l, "recordPacking",
                                 req->u.explain_request->recordPacking);
        l = append_extra_arg(o, l, "stylesheet",
                             req->u.explain_request->stylesheet);
    }
    for (ea = req->extra_args; ea; ea = ea->next)
        l = append_extra_arg(o, l, ea->name, ea->value);
    return res;
}

Z_SRW_PDU *yaz_srw_get_pdu(ODR o, int which, const char *version)
{
    Z_SRW_PDU *sr = yaz_srw_get_core_ver(o, version);

    sr->which = which;
    switch (which)
    {
    case Z_SRW_searchRetrieve_request:
        sr->u.request = (Z_SRW_searchRetrieveRequest *)
            odr_malloc(o, sizeof(*sr->u.request));
        sr->u.request->queryType = "cql";
        sr->u.request->query = 0;
        sr->u.request->sort_type = Z_SRW_sort_type_none;
        sr->u.request->sort.none = 0;
        sr->u.request->startRecord = 0;
        sr->u.request->maximumRecords = 0;
        sr->u.request->recordSchema = 0;
        sr->u.request->recordPacking = 0;
        sr->u.request->packing = 0;
        sr->u.request->recordXPath = 0;
        sr->u.request->database = 0;
        sr->u.request->resultSetTTL = 0;
        sr->u.request->stylesheet = 0;
        sr->u.request->facetList = 0;
        break;
    case Z_SRW_searchRetrieve_response:
        sr->u.response = (Z_SRW_searchRetrieveResponse *)
            odr_malloc(o, sizeof(*sr->u.response));
        sr->u.response->numberOfRecords = 0;
        sr->u.response->resultCountPrecision = 0;
        sr->u.response->resultSetId = 0;
        sr->u.response->resultSetIdleTime = 0;
        sr->u.response->records = 0;
        sr->u.response->num_records = 0;
        sr->u.response->diagnostics = 0;
        sr->u.response->num_diagnostics = 0;
        sr->u.response->nextRecordPosition = 0;
        sr->u.response->extra_records = 0;
        sr->u.response->facetList = 0;
        sr->u.response->suggestions = 0;
        break;
    case Z_SRW_explain_request:
        sr->u.explain_request = (Z_SRW_explainRequest *)
            odr_malloc(o, sizeof(*sr->u.explain_request));
        sr->u.explain_request->recordPacking = 0;
        sr->u.explain_request->packing = 0;
        sr->u.explain_request->database = 0;
        sr->u.explain_request->stylesheet = 0;
        break;
    case Z_SRW_explain_response:
        sr->u.explain_response = (Z_SRW_explainResponse *)
            odr_malloc(o, sizeof(*sr->u.explain_response));
        sr->u.explain_response->record.recordData_buf = 0;
        sr->u.explain_response->record.recordData_len = 0;
        sr->u.explain_response->record.recordSchema = 0;
        sr->u.explain_response->record.recordPosition = 0;
        sr->u.explain_response->record.recordPacking =
            Z_SRW_recordPacking_string;
        sr->u.explain_response->diagnostics = 0;
        sr->u.explain_response->num_diagnostics = 0;
        sr->u.explain_response->extra_record = 0;
        break;
    case Z_SRW_scan_request:
        sr->u.scan_request = (Z_SRW_scanRequest *)
            odr_malloc(o, sizeof(*sr->u.scan_request));
        sr->u.scan_request->database = 0;
        sr->u.scan_request->stylesheet = 0;
        sr->u.scan_request->maximumTerms = 0;
        sr->u.scan_request->responsePosition = 0;
        sr->u.scan_request->queryType = "cql";
        sr->u.scan_request->scanClause = 0;
        break;
    case Z_SRW_scan_response:
        sr->u.scan_response = (Z_SRW_scanResponse *)
            odr_malloc(o, sizeof(*sr->u.scan_response));
	sr->u.scan_response->terms = 0;
	sr->u.scan_response->num_terms = 0;
	sr->u.scan_response->diagnostics = 0;
	sr->u.scan_response->num_diagnostics = 0;
        break;
    case Z_SRW_update_request:
        sr->u.update_request = (Z_SRW_updateRequest *)
            odr_malloc(o, sizeof(*sr->u.update_request));
	sr->u.update_request->database = 0;
	sr->u.update_request->stylesheet = 0;
        sr->u.update_request->record = 0;
	sr->u.update_request->recordId = 0;
	sr->u.update_request->recordVersions = 0;
	sr->u.update_request->num_recordVersions = 0;
        sr->u.update_request->extra_record = 0;
        sr->u.update_request->extraRequestData_buf = 0;
        sr->u.update_request->extraRequestData_len = 0;
	sr->u.request->database = 0;
        break;
    case Z_SRW_update_response:
        sr->u.update_response = (Z_SRW_updateResponse *)
            odr_malloc(o, sizeof(*sr->u.update_response));
	sr->u.update_response->operationStatus = 0;
	sr->u.update_response->recordId = 0;
	sr->u.update_response->recordVersions = 0;
	sr->u.update_response->num_recordVersions = 0;
	sr->u.update_response->record = 0;
        sr->u.update_response->extra_record = 0;
        sr->u.update_response->extraResponseData_buf = 0;
        sr->u.update_response->extraResponseData_len = 0;
	sr->u.update_response->diagnostics = 0;
	sr->u.update_response->num_diagnostics = 0;
    }
    return sr;
}

void yaz_add_name_value_int(ODR o, char **name, char **value, int *i,
                            char *a_name, Odr_int *val)
{
    if (val)
    {
        name[*i] = a_name;
        value[*i] = (char *) odr_malloc(o, 40);
        sprintf(value[*i], ODR_INT_PRINTF, *val);
        (*i)++;
    }
}

void yaz_add_name_value_str(ODR o, char **name, char **value,  int *i,
                            char *a_name, char *val)
{
    if (val)
    {
        name[*i] = a_name;
        value[*i] = val;
        (*i)++;
    }
}

static int yaz_get_sru_parms(const Z_SRW_PDU *srw_pdu, ODR encode,
                             char **name, char **value, int max_names)
{
    int version2 = strcmp(srw_pdu->srw_version, "2.") > 0;
    int i = 0;
    char *queryType;
    yaz_add_name_value_str(encode, name, value, &i, "version",
                           srw_pdu->srw_version);
    name[i] = "operation";
    switch (srw_pdu->which)
    {
    case Z_SRW_searchRetrieve_request:
        value[i++] = "searchRetrieve";
        queryType = srw_pdu->u.request->queryType;
        if (version2)
        {
            if (queryType && strcmp(queryType, "cql"))
                yaz_add_name_value_str(encode, name, value, &i, "queryType",
                                       queryType);
            yaz_add_name_value_str(encode, name, value, &i, "query",
                                   srw_pdu->u.request->query);
        }
        else
        {
            if (!strcmp(queryType, "cql"))
            {
                yaz_add_name_value_str(encode, name, value, &i, "query",
                                       srw_pdu->u.request->query);
            }
            else if (!strcmp(queryType, "pqf"))
            {
                yaz_add_name_value_str(encode, name, value, &i, "x-pquery",
                                       srw_pdu->u.request->query);
            }
            else if (!strcmp(queryType, "xcql"))
            {
                yaz_add_name_value_str(encode, name, value, &i, "x-cql",
                                       srw_pdu->u.request->query);
            }
        }
        switch (srw_pdu->u.request->sort_type)
        {
        case Z_SRW_sort_type_none:
            break;
        case Z_SRW_sort_type_sort:
            yaz_add_name_value_str(encode, name, value, &i, "sortKeys",
                                   srw_pdu->u.request->sort.sortKeys);
            break;
        }
        yaz_add_name_value_int(encode, name, value, &i, "startRecord",
                               srw_pdu->u.request->startRecord);
        yaz_add_name_value_int(encode, name, value, &i, "maximumRecords",
                               srw_pdu->u.request->maximumRecords);
        yaz_add_name_value_str(encode, name, value, &i, "recordSchema",
                               srw_pdu->u.request->recordSchema);
        if (version2)
        {
            yaz_add_name_value_str(encode, name, value, &i, "recordXMLEscaping",
                                   srw_pdu->u.request->recordPacking);
            yaz_add_name_value_str(encode, name, value, &i, "recordPacking",
                                   srw_pdu->u.request->packing);
        }
        else
            yaz_add_name_value_str(encode, name, value, &i, "recordPacking",
                                   srw_pdu->u.request->recordPacking);
        yaz_add_name_value_str(encode, name, value, &i, "recordXPath",
                               srw_pdu->u.request->recordXPath);
        yaz_add_name_value_str(encode, name, value, &i, "stylesheet",
                               srw_pdu->u.request->stylesheet);
        yaz_add_name_value_int(encode, name, value, &i, "resultSetTTL",
                               srw_pdu->u.request->resultSetTTL);
        {
            const char *facetLimit = 0;
            const char *facetStart = 0;
            const char *facetSort = 0;
            yaz_sru_facet_request(encode, &srw_pdu->u.request->facetList,
                                  &facetLimit, &facetStart, &facetSort);
            yaz_add_name_value_str(encode, name, value, &i, "facetLimit",
                                   (char *) facetLimit);
            yaz_add_name_value_str(encode, name, value, &i, "facetStart",
                                   (char *) facetStart);
            yaz_add_name_value_str(encode, name, value, &i, "facetSort",
                                   (char *) facetSort);
        }
        break;
    case Z_SRW_explain_request:
        value[i++] = "explain";

        if (version2)
        {
            yaz_add_name_value_str(encode, name, value, &i, "recordXMLEscaping",
                                   srw_pdu->u.explain_request->recordPacking);
            yaz_add_name_value_str(encode, name, value, &i, "recordPacking",
                                   srw_pdu->u.explain_request->packing);
        }
        else
            yaz_add_name_value_str(encode, name, value, &i, "recordPacking",
                                   srw_pdu->u.explain_request->recordPacking);
        yaz_add_name_value_str(encode, name, value, &i, "stylesheet",
                               srw_pdu->u.explain_request->stylesheet);
        break;
    case Z_SRW_scan_request:
        value[i++] = "scan";
        queryType = srw_pdu->u.scan_request->queryType;
        if (version2)
        {
            if (queryType && strcmp(queryType, "cql"))
                yaz_add_name_value_str(encode, name, value, &i, "queryType",
                                       queryType);
            yaz_add_name_value_str(encode, name, value, &i, "scanClause",
                                   srw_pdu->u.scan_request->scanClause);
        }
        else
        {
            if (!queryType || !strcmp(queryType, "cql"))
                yaz_add_name_value_str(encode, name, value, &i, "scanClause",
                                       srw_pdu->u.scan_request->scanClause);
            else if (!strcmp(queryType, "pqf"))
                yaz_add_name_value_str(encode, name, value, &i, "x-pScanClause",
                                       srw_pdu->u.scan_request->scanClause);
            else if (!strcmp(queryType, "xcql"))
                yaz_add_name_value_str(encode, name, value, &i,
                                       "x-cqlScanClause",
                                       srw_pdu->u.scan_request->scanClause);
        }
        yaz_add_name_value_int(encode, name, value, &i, "responsePosition",
                               srw_pdu->u.scan_request->responsePosition);
        yaz_add_name_value_int(encode, name, value, &i, "maximumTerms",
                               srw_pdu->u.scan_request->maximumTerms);
        yaz_add_name_value_str(encode, name, value, &i, "stylesheet",
                               srw_pdu->u.scan_request->stylesheet);
        break;
    case Z_SRW_update_request:
        value[i++] = "update";
        break;
    default:
        return -1;
    }
    if (srw_pdu->extra_args)
    {
        Z_SRW_extra_arg *ea = srw_pdu->extra_args;
        for (; ea && i < max_names-1; ea = ea->next)
        {
            name[i] = ea->name;
            value[i] = ea->value;
            i++;
        }
    }
    name[i++] = 0;

    return 0;
}

int yaz_sru_get_encode(Z_HTTP_Request *hreq, Z_SRW_PDU *srw_pdu,
                       ODR encode, const char *charset)
{
    char *name[MAX_SRU_PARAMETERS], *value[MAX_SRU_PARAMETERS]; /* definite upper limit for SRU params */
    char *uri_args;
    char *path;
    char *cp;

    z_HTTP_header_add_basic_auth(encode, &hreq->headers,
                                 srw_pdu->username, srw_pdu->password);
    if (yaz_get_sru_parms(srw_pdu, encode, name, value, MAX_SRU_PARAMETERS))
        return -1;
    yaz_array_to_uri(&uri_args, encode, name, value);

    hreq->method = "GET";

    cp = strchr(hreq->path, '#');
    if (cp)
        *cp = '\0';

    path = (char *)
        odr_malloc(encode, strlen(hreq->path) + strlen(uri_args) + 4);

    sprintf(path, "%s%c%s", hreq->path, strchr(hreq->path, '?') ? '&' : '?', 
            uri_args);
    hreq->path = path;

    z_HTTP_header_add_content_type(encode, &hreq->headers,
                                   "text/xml", charset);
    return 0;
}

int yaz_sru_post_encode(Z_HTTP_Request *hreq, Z_SRW_PDU *srw_pdu,
                        ODR encode, const char *charset)
{
    char *name[MAX_SRU_PARAMETERS], *value[MAX_SRU_PARAMETERS]; /* definite upper limit for SRU params */
    char *uri_args;

    z_HTTP_header_add_basic_auth(encode, &hreq->headers,
                                 srw_pdu->username, srw_pdu->password);
    if (yaz_get_sru_parms(srw_pdu, encode, name, value, MAX_SRU_PARAMETERS))
        return -1;

    yaz_array_to_uri(&uri_args, encode, name, value);

    hreq->method = "POST";

    hreq->content_buf = uri_args;
    hreq->content_len = strlen(uri_args);

    z_HTTP_header_add_content_type(encode, &hreq->headers,
                                   "application/x-www-form-urlencoded",
                                   charset);
    return 0;
}

int yaz_sru_soap_encode(Z_HTTP_Request *hreq, Z_SRW_PDU *srw_pdu,
                        ODR odr, const char *charset)
{
    Z_SOAP_Handler handlers[3] = {
#if YAZ_HAVE_XML2
        {YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec},
        {YAZ_XMLNS_UPDATE_v0_9, 0, (Z_SOAP_fun) yaz_ucp_codec},
#endif
        {0, 0, 0}
    };
    Z_SOAP *p = (Z_SOAP*) odr_malloc(odr, sizeof(*p));

    z_HTTP_header_add_basic_auth(odr, &hreq->headers,
                                 srw_pdu->username, srw_pdu->password);
    z_HTTP_header_add_content_type(odr,
                                   &hreq->headers,
                                   "text/xml", 0 /* no charset in MIME */);

    z_HTTP_header_add(odr, &hreq->headers,
                      "SOAPAction", "\"\"");
    p->which = Z_SOAP_generic;
    p->u.generic = (Z_SOAP_Generic *) odr_malloc(odr, sizeof(*p->u.generic));
    p->u.generic->no = 0;
    p->u.generic->ns = 0;
    p->u.generic->p = srw_pdu;
    p->ns = "http://schemas.xmlsoap.org/soap/envelope/";

#if YAZ_HAVE_XML2
    if (srw_pdu->which == Z_SRW_update_request ||
        srw_pdu->which == Z_SRW_update_response)
        p->u.generic->no = 1; /* second handler */
#endif
    return z_soap_codec_enc(odr, &p,
                            &hreq->content_buf,
                            &hreq->content_len, handlers,
                            charset);
}

Z_SRW_recordVersion *yaz_srw_get_record_versions(ODR odr, int num)
{
    Z_SRW_recordVersion *ver
        = (Z_SRW_recordVersion *) odr_malloc(odr,num * sizeof(*ver));
    int i;
    for (i = 0; i < num; ++i)
    {
        ver[i].versionType = 0;
        ver[i].versionValue = 0;
    }
    return ver;
}

const char *yaz_srw_pack_to_str(int pack)
{
    switch(pack)
    {
    case Z_SRW_recordPacking_string:
        return "string";
    case Z_SRW_recordPacking_XML:
        return "xml";
    case Z_SRW_recordPacking_URL:
        return "url";
    }
    return 0;
}

int yaz_srw_str_to_pack(const char *str)
{
    if (!yaz_matchstr(str, "string"))
        return Z_SRW_recordPacking_string;
    if (!yaz_matchstr(str, "xml"))
        return Z_SRW_recordPacking_XML;
    if (!yaz_matchstr(str, "url"))
        return Z_SRW_recordPacking_URL;
    return -1;
}

void yaz_encode_sru_extra(Z_SRW_PDU *sr, ODR odr, const char *extra_args)
{
    if (extra_args)
    {
        char **name;
        char **val;
        Z_SRW_extra_arg **ea = &sr->extra_args;
        yaz_uri_to_array(extra_args, odr, &name, &val);

        while (*name)
        {
            ea = append_extra_arg(odr, ea, *name, *val);
            val++;
            name++;
        }
    }
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

