/*
 * Copyright (c) 2002-2005, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: srwutil.c,v 1.22 2005-01-11 10:50:06 adam Exp $
 */
/**
 * \file srwutil.c
 * \brief Implements SRW/SRU utilities.
 */

#include <yaz/srw.h>
#include <yaz/yaz-iconv.h>

static int hex_digit (int ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a'+10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A'+10;
    return 0;
}

int yaz_uri_array(const char *path, ODR o, char ***name, char ***val)
{
    int no = 2;
    const char *cp;
    *name = 0;
    if (*path != '?')
	return no;
    path++;
    cp = path;
    while ((cp = strchr(cp, '&')))
    {
	cp++;
	no++;
    }
    *name = odr_malloc(o, no * sizeof(char**));
    *val = odr_malloc(o, no * sizeof(char**));

    for (no = 0; *path; no++)
    {
        const char *p1 = strchr(path, '=');
	size_t i = 0;
	char *ret;
        if (!p1)
            break;

	(*name)[no] = odr_malloc(o, (p1-path)+1);
	memcpy((*name)[no], path, p1-path);
	(*name)[no][p1-path] = '\0';

	path = p1 + 1;
	p1 = strchr(path, '&');
	if (!p1)
	    p1 = strlen(path) + path;
	(*val)[no] = ret = odr_malloc(o, p1 - path + 1);
	while (*path && *path != '&')
	{
	    if (*path == '+')
	    {
		ret[i++] = ' ';
		path++;
	    }
	    else if (*path == '%' && path[1] && path[2])
	    {
		ret[i++] = hex_digit (path[1])*16 + hex_digit (path[2]);
		path = path + 3;
	    }
	    else
		ret[i++] = *path++;
	}
	ret[i] = '\0';

	if (*path)
	    path++;
    }
    (*name)[no] = 0;
    (*val)[no] = 0;
    return no;
}

char *yaz_uri_val(const char *path, const char *name, ODR o)
{
    size_t nlen = strlen(name);
    if (*path != '?')
        return 0;
    path++;
    while (path && *path)
    {
        const char *p1 = strchr(path, '=');
        if (!p1)
            break;
        if ((size_t)(p1 - path) == nlen && !memcmp(path, name, nlen))
        {
            size_t i = 0;
            char *ret;
            
            path = p1 + 1;
            p1 = strchr(path, '&');
            if (!p1)
                p1 = strlen(path) + path;
            ret = (char *) odr_malloc(o, p1 - path + 1);
            while (*path && *path != '&')
            {
                if (*path == '+')
                {
                    ret[i++] = ' ';
                    path++;
                }
                else if (*path == '%' && path[1] && path[2])
                {
                    ret[i++] = hex_digit (path[1])*16 + hex_digit (path[2]);
                    path = path + 3;
                }
                else
                    ret[i++] = *path++;
            }
            ret[i] = '\0';
            return ret;
        }
        path = strchr(p1, '&');
        if (path)
            path++;
    }
    return 0;
}

void yaz_uri_val_int(const char *path, const char *name, ODR o, int **intp)
{
    const char *v = yaz_uri_val(path, name, o);
    if (v)
        *intp = odr_intdup(o, atoi(v));
}

void yaz_mk_std_diagnostic(ODR o, Z_SRW_diagnostic *d, 
			   int code, const char *details)
{
    d->uri = (char *) odr_malloc(o, 50);
    sprintf(d->uri, "info:srw/diagnostic/1/%d", code);
    d->message = 0;
    if (details)
	d->details = odr_strdup(o, details);
    else
	d->details = 0;
}

void yaz_add_srw_diagnostic(ODR o, Z_SRW_diagnostic **d,
			    int *num, int code, const char *addinfo)
{
    Z_SRW_diagnostic *d_new;
    d_new = (Z_SRW_diagnostic *) odr_malloc (o, (*num + 1)* sizeof(**d));
    if (*num)
	memcpy (d_new, *d, *num *sizeof(**d));
    *d = d_new;

    yaz_mk_std_diagnostic(o, *d + *num, code, addinfo);
    (*num)++;
}

int yaz_srw_decode(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
		   Z_SOAP **soap_package, ODR decode, char **charset)
{
    if (!strcmp(hreq->method, "POST"))
    {
	const char *content_type = z_HTTP_header_lookup(hreq->headers,
							"Content-Type");
	if (content_type && !yaz_strcmp_del("text/xml", content_type, "; "))
	{
	    char *db = "Default";
	    const char *p0 = hreq->path, *p1;
            int ret = -1;
            const char *charset_p = 0;
	    
            static Z_SOAP_Handler soap_handlers[3] = {
#if HAVE_XML2
                {"http://www.loc.gov/zing/srw/", 0,
                 (Z_SOAP_fun) yaz_srw_codec},
                {"http://www.loc.gov/zing/srw/v1.0/", 0,
                 (Z_SOAP_fun) yaz_srw_codec},
#endif
                {0, 0, 0}
            };
	    
	    if (*p0 == '/')
		p0++;
	    p1 = strchr(p0, '?');
	    if (!p1)
		p1 = p0 + strlen(p0);
	    if (p1 != p0)
	    {
		db = (char*) odr_malloc(decode, p1 - p0 + 1);
		memcpy (db, p0, p1 - p0);
		db[p1 - p0] = '\0';
	    }

            if (charset && (charset_p = strstr(content_type, "; charset=")))
            {
                int i = 0;
                charset_p += 10;
                while (i < 20 && charset_p[i] &&
                       !strchr("; \n\r", charset_p[i]))
                    i++;
                *charset = (char*) odr_malloc(decode, i+1);
                memcpy(*charset, charset_p, i);
                (*charset)[i] = '\0';
            }
            ret = z_soap_codec(decode, soap_package, 
                               &hreq->content_buf, &hreq->content_len,
                               soap_handlers);
	    if (ret == 0 && (*soap_package)->which == Z_SOAP_generic)
	    {
		*srw_pdu = (Z_SRW_PDU*) (*soap_package)->u.generic->p;
		
		if ((*srw_pdu)->which == Z_SRW_searchRetrieve_request &&
		    (*srw_pdu)->u.request->database == 0)
		    (*srw_pdu)->u.request->database = db;

		if ((*srw_pdu)->which == Z_SRW_explain_request &&
		    (*srw_pdu)->u.explain_request->database == 0)
		    (*srw_pdu)->u.explain_request->database = db;

		if ((*srw_pdu)->which == Z_SRW_scan_request &&
		    (*srw_pdu)->u.scan_request->database == 0)
		    (*srw_pdu)->u.scan_request->database = db;

		return 0;
	    }
	    return 1;
	}
    }
    return 2;
}

/**
  http://www.loc.gov/z3950/agency/zing/srw/service.html
*/ 
int yaz_sru_decode(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
		   Z_SOAP **soap_package, ODR decode, char **charset,
		   Z_SRW_diagnostic **diag, int *num_diag)
{
#if HAVE_XML2
    static Z_SOAP_Handler soap_handlers[2] = {
	{"http://www.loc.gov/zing/srw/", 0,
	 (Z_SOAP_fun) yaz_srw_codec},
	{0, 0, 0}
    };
#endif
    if (!strcmp(hreq->method, "GET"))
    {
        char *db = "Default";
        const char *p0 = hreq->path, *p1;
	const char *operation = 0;
	char *version = 0;
	char *query = 0;
	char *pQuery = 0;
	char *sortKeys = 0;
	char *stylesheet = 0;
	char *scanClause = 0;
	char *pScanClause = 0;
	char *recordXPath = 0;
	char *recordSchema = 0;
	char *recordPacking = "xml";  /* xml packing is default for SRU */
	char *maximumRecords = 0;
	char *startRecord = 0;
	char *maximumTerms = 0;
	char *responsePosition = 0;
	char *extraRequestData = 0;
	char **uri_name;
	char **uri_val;

	if (charset)
	    *charset = 0;
        if (*p0 == '/')
            p0++;
        p1 = strchr(p0, '?');
        if (!p1)
            p1 = p0 + strlen(p0);
        if (p1 != p0)
        {
            db = (char*) odr_malloc(decode, p1 - p0 + 1);
            memcpy (db, p0, p1 - p0);
            db[p1 - p0] = '\0';
        }
	yaz_uri_array(p1, decode, &uri_name, &uri_val);
#if HAVE_XML2
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
		    pQuery = v;
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
		else if (!strcmp(n, "version"))
		    version = v;
		else if (!strcmp(n, "scanClause"))
		    scanClause = v;
		else if (!strcmp(n, "x-pScanClause"))
		    pScanClause = v;
		else if (!strcmp(n, "maximumRecords"))
		    maximumRecords = v;
		else if (!strcmp(n, "startRecord"))
		    startRecord = v;
		else if (!strcmp(n, "maximumTerms"))
		    maximumTerms = v;
		else if (!strcmp(n, "responsePosition"))
		    responsePosition = v;
		else if (!strcmp(n, "extraRequestData"))
		    extraRequestData = v;
		else
		    yaz_add_srw_diagnostic(decode, diag, num_diag, 8, n);
	    }
	}
	if (!version)
	{
	    if (uri_name)
		yaz_add_srw_diagnostic(decode, diag, num_diag, 7, "version");
	    version = "1.1";
	}
	if (strcmp(version, "1.1"))
	    yaz_add_srw_diagnostic(decode, diag, num_diag, 5, "1.1");
	if (!operation)
	{
	    if (uri_name)
		yaz_add_srw_diagnostic(decode, diag, num_diag, 7, "operation");
	    operation = "explain";
	}
        if (!strcmp(operation, "searchRetrieve"))
        {
            Z_SRW_PDU *sr = yaz_srw_get(decode, Z_SRW_searchRetrieve_request);

	    sr->srw_version = version;
	    *srw_pdu = sr;
            if (query)
            {
                sr->u.request->query_type = Z_SRW_query_type_cql;
                sr->u.request->query.cql = query;
            }
            else if (pQuery)
            {
                sr->u.request->query_type = Z_SRW_query_type_pqf;
                sr->u.request->query.pqf = pQuery;
            }
	    else
		yaz_add_srw_diagnostic(decode, diag, num_diag, 7, "query");

            if (sortKeys)
            {
                sr->u.request->sort_type = Z_SRW_sort_type_sort;
                sr->u.request->sort.sortKeys = sortKeys;
            }
            sr->u.request->recordXPath = recordXPath;
            sr->u.request->recordSchema = recordSchema;
            sr->u.request->recordPacking = recordPacking;
            sr->u.request->stylesheet = stylesheet;

	    if (maximumRecords)
		sr->u.request->maximumRecords =
		    odr_intdup(decode, atoi(maximumRecords));
	    if (startRecord)
		sr->u.request->startRecord =
		    odr_intdup(decode, atoi(startRecord));

            sr->u.request->database = db;

	    (*soap_package) = odr_malloc(decode, sizeof(**soap_package));
	    (*soap_package)->which = Z_SOAP_generic;
	    
	    (*soap_package)->u.generic =
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
	    *srw_pdu = sr;
            sr->u.explain_request->recordPacking = recordPacking;
	    sr->u.explain_request->database = db;

            sr->u.explain_request->stylesheet = stylesheet;

	    (*soap_package) = odr_malloc(decode, sizeof(**soap_package));
	    (*soap_package)->which = Z_SOAP_generic;
	    
	    (*soap_package)->u.generic =
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
	    *srw_pdu = sr;

            if (scanClause)
            {
                sr->u.scan_request->query_type = Z_SRW_query_type_cql;
		sr->u.scan_request->scanClause.cql = scanClause;
            }
            else if (pScanClause)
            {
                sr->u.scan_request->query_type = Z_SRW_query_type_pqf;
                sr->u.scan_request->scanClause.pqf = pScanClause;
            }
	    else
		yaz_add_srw_diagnostic(decode, diag, num_diag, 7,
				       "scanClause");
	    sr->u.scan_request->database = db;

	    if (maximumTerms)
		sr->u.scan_request->maximumTerms =
		    odr_intdup(decode, atoi(maximumTerms));
	    if (responsePosition)
		sr->u.scan_request->responsePosition =
		    odr_intdup(decode, atoi(responsePosition));

            sr->u.scan_request->stylesheet = stylesheet;

	    (*soap_package) = odr_malloc(decode, sizeof(**soap_package));
	    (*soap_package)->which = Z_SOAP_generic;
	    
	    (*soap_package)->u.generic =
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

	    (*soap_package) = odr_malloc(decode, sizeof(**soap_package));
	    (*soap_package)->which = Z_SOAP_generic;
	    
	    (*soap_package)->u.generic =
		odr_malloc(decode, sizeof(*(*soap_package)->u.generic));
	    
	    (*soap_package)->u.generic->p = sr;
	    (*soap_package)->u.generic->ns = soap_handlers[0].ns;
	    (*soap_package)->u.generic->no = 0;
	    
	    (*soap_package)->ns = "SRU";

	    yaz_add_srw_diagnostic(decode, diag, num_diag, 4, operation);
	    return 0;
	}
#endif
	return 1;
    }
    return 2;
}

Z_SRW_PDU *yaz_srw_get(ODR o, int which)
{
    Z_SRW_PDU *sr = (Z_SRW_PDU *) odr_malloc(o, sizeof(*o));

    sr->srw_version = odr_strdup(o, "1.1");
    sr->which = which;
    switch(which)
    {
    case Z_SRW_searchRetrieve_request:
        sr->u.request = (Z_SRW_searchRetrieveRequest *)
            odr_malloc(o, sizeof(*sr->u.request));
        sr->u.request->query_type = Z_SRW_query_type_cql;
        sr->u.request->query.cql = 0;
        sr->u.request->sort_type = Z_SRW_sort_type_none;
        sr->u.request->sort.none = 0;
        sr->u.request->startRecord = 0;
        sr->u.request->maximumRecords = 0;
        sr->u.request->recordSchema = 0;
        sr->u.request->recordPacking = 0;
        sr->u.request->recordXPath = 0;
	sr->u.request->database = 0;
	sr->u.request->resultSetTTL = 0;
	sr->u.request->stylesheet = 0;
        break;
    case Z_SRW_searchRetrieve_response:
        sr->u.response = (Z_SRW_searchRetrieveResponse *)
            odr_malloc(o, sizeof(*sr->u.response));
        sr->u.response->numberOfRecords = 0;
        sr->u.response->resultSetId = 0;
        sr->u.response->resultSetIdleTime = 0;
        sr->u.response->records = 0;
        sr->u.response->num_records = 0;
        sr->u.response->diagnostics = 0;
        sr->u.response->num_diagnostics = 0;
        sr->u.response->nextRecordPosition = 0;
        break;
    case Z_SRW_explain_request:
        sr->u.explain_request = (Z_SRW_explainRequest *)
            odr_malloc(o, sizeof(*sr->u.explain_request));
        sr->u.explain_request->recordPacking = 0;
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
	break;
    case Z_SRW_scan_request:
        sr->u.scan_request = (Z_SRW_scanRequest *)
            odr_malloc(o, sizeof(*sr->u.scan_request));
	sr->u.scan_request->database = 0;
	sr->u.scan_request->stylesheet = 0;
	sr->u.scan_request->maximumTerms = 0;
	sr->u.scan_request->responsePosition = 0;
	sr->u.scan_request->query_type = Z_SRW_query_type_cql;
	sr->u.scan_request->scanClause.cql = 0;
        break;
    case Z_SRW_scan_response:
        sr->u.scan_response = (Z_SRW_scanResponse *)
            odr_malloc(o, sizeof(*sr->u.scan_response));
	sr->u.scan_response->terms = 0;
	sr->u.scan_response->num_terms = 0;
	sr->u.scan_response->diagnostics = 0;
	sr->u.scan_response->num_diagnostics = 0;
    }
    return sr;
}



/* bib1:srw */
static int srw_bib1_map[] = {
    1, 1,
    2, 2,
    3, 11,
    4, 35,
    5, 12,
    6, 38,
    7, 30,
    8, 32,
    9, 29,
    108, 10,  /* Malformed query : Syntax error */
    10, 10,
    11, 12,
    11, 23,
    12, 60,
    13, 61,
    13, 62,
    14, 63,
    14, 64,
    14, 65,
    15, 68,
    15, 69,
    16, 70,
    17, 70,
    18, 50,
    19, 55,
    20, 56, 
    21, 52,
    22, 50,
    23, 3,
    24, 66,
    25, 66,
    26, 66,
    27, 51,
    28, 52,
    29, 52,
    30, 51,
    31, 57,
    32, 58,
    33, 59,
    100, 1, /* bad map */
    101, 3,
    102, 3,
    103, 3,
    104, 3,
    105, 3, 
    106, 66,
    107, 11,
    108, 13,
    108, 14,
    108, 25,
    108, 26,
    108, 27,
    108, 45,
        
    109, 2,
    110, 37,
    111, 1,
    112, 58,
    113, 10,
    114, 16,
    115, 16,
    116, 16,
    117, 19,
    117, 20,
    118, 22,
    119, 32,
    119, 31,
    120, 28,
    121, 15,
    122, 32,
    123, 22,
    123, 17,
    123, 18,
    124, 24,
    125, 36,
    126, 36, 
    127, 36,
    128, 51,
    129, 39,
    130, 43,
    131, 40,
    132, 42,
    201, 44,
    201, 33,
    201, 34,
    202, 41,
    203, 43,
    205, 1,  /* bad map */
    206, 1,  /* bad map */
    207, 89,
    208, 1,  /* bad map */
    209, 80,
    210, 80,
    210, 81,
    211, 84,
    212, 85,
    213, 92,
    214, 90,
    215, 91,
    216, 92,
    217, 63,
    218, 1,  /* bad map */
    219, 1,  /* bad map */
    220, 1,  /* bad map */
    221, 1,  /* bad map */
    222, 1,  /* bad map */
    223, 1,  /* bad map */
    224, 1,  /* bad map */
    225, 1,  /* bad map */
    226, 1,  /* bad map */
    227, 66,
    228, 1,  /* bad map */
    229, 36,
    230, 83,
    231, 89,
    232, 1,
    233, 1, /* bad map */
    234, 1, /* bad map */
    235, 2,
    236, 3, 
    237, 82,
    238, 67,
    239, 66,
    240, 1, /* bad map */
    241, 1, /* bad map */
    242, 70,
    243, 1, /* bad map */
    244, 66,
    245, 10,
    246, 10,
    247, 10,
    1001, 1, /* bad map */
    1002, 1, /* bad map */
    1003, 1, /* bad map */
    1004, 1, /* bad map */
    1005, 1, /* bad map */
    1006, 1, /* bad map */
    1007, 100,
    1008, 1, 
    1009, 1,
    1010, 3,
    1011, 3,
    1012, 3,
    1013, 3,
    1014, 3,
    1015, 3,
    1015, 3,
    1016, 3,
    1017, 3,
    1018, 2,
    1019, 2,
    1020, 2,
    1021, 3,
    1022, 3,
    1023, 3,
    1024, 16,
    1025, 3,
    1026, 64,
    1027, 1,
    1028, 65,
    1029, 1,
    1040, 1,
    /* 1041-1065 */
    1066, 66,
    1066, 67,
    0
};

int yaz_diag_bib1_to_srw (int code)
{
    const int *p = srw_bib1_map;
    while (*p)
    {
        if (code == p[0])
            return p[1];
        p += 2;
    }
    return 1;
}

int yaz_diag_srw_to_bib1(int code)
{
    const int *p = srw_bib1_map;
    while (*p)
    {
        if (code == p[1])
            return p[0];
        p += 2;
    }
    return 1;
}

