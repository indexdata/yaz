/*
 * Copyright (c) 2002-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: srwutil.c,v 1.1 2003-12-20 00:51:19 adam Exp $
 */

#include <yaz/srw.h>

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
            ret = odr_malloc(o, p1 - path + 1);
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

int yaz_check_for_srw(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
		      char **soap_ns, ODR decode)
{
    if (!strcmp(hreq->method, "POST"))
    {
	const char *content_type = z_HTTP_header_lookup(hreq->headers,
							"Content-Type");
	if (content_type && !yaz_strcmp_del("text/xml", content_type, "; "))
	{
	    char *db = "Default";
	    const char *p0 = hreq->path, *p1;
	    Z_SOAP *soap_package = 0;
            int ret = -1;
            int http_code = 500;
            const char *charset_p = 0;
            char *charset = 0;
	    
            static Z_SOAP_Handler soap_handlers[2] = {
#if HAVE_XML2
                {"http://www.loc.gov/zing/srw/", 0,
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

            if ((charset_p = strstr(content_type, "; charset=")))
            {
                int i = 0;
                charset_p += 10;
                while (i < 20 && charset_p[i] &&
                       !strchr("; \n\r", charset_p[i]))
                    i++;
                charset = (char*) odr_malloc(decode, i+1);
                memcpy(charset, charset_p, i);
                charset[i] = '\0';
            }
            ret = z_soap_codec(decode, &soap_package, 
                               &hreq->content_buf, &hreq->content_len,
                               soap_handlers);
	    if (!ret && soap_package->which == Z_SOAP_generic &&
		soap_package->u.generic->no == 0)
	    {
		*srw_pdu = (Z_SRW_PDU*) soap_package->u.generic->p;
		
		if ((*srw_pdu)->which == Z_SRW_searchRetrieve_request &&
		    (*srw_pdu)->u.request->database == 0)
		    (*srw_pdu)->u.request->database = db;

		if ((*srw_pdu)->which == Z_SRW_explain_request &&
		    (*srw_pdu)->u.explain_request->database == 0)
		    (*srw_pdu)->u.explain_request->database = db;

		*soap_ns = odr_strdup(decode, soap_package->ns);
		return 0;
	    }
	    return 1;
	}
    }
    return 2;
}

int yaz_check_for_sru(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
		      char **soap_ns, ODR decode)
{
    if (!strcmp(hreq->method, "GET"))
    {
        char *db = "Default";
        const char *p0 = hreq->path, *p1;
	const char *operation = 0;
#if HAVE_XML2
        int ret = -1;
        char *charset = 0;
        Z_SOAP *soap_package = 0;
        static Z_SOAP_Handler soap_handlers[2] = {
            {"http://www.loc.gov/zing/srw/", 0,
             (Z_SOAP_fun) yaz_srw_codec},
            {0, 0, 0}
        };
#endif
        
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
#if HAVE_XML2
	if (p1)
	    operation = yaz_uri_val(p1, "operation", decode);
	if (!operation)
	    operation = "explain";
        if (p1 && !strcmp(operation, "searchRetrieve"))
        {
            Z_SRW_PDU *sr = yaz_srw_get(decode, Z_SRW_searchRetrieve_request);
            char *query = yaz_uri_val(p1, "query", decode);
            char *pQuery = yaz_uri_val(p1, "pQuery", decode);
            char *sortKeys = yaz_uri_val(p1, "sortKeys", decode);
            
	    *srw_pdu = sr;
            if (query)
            {
                sr->u.request->query_type = Z_SRW_query_type_cql;
                sr->u.request->query.cql = query;
            }
            if (pQuery)
            {
                sr->u.request->query_type = Z_SRW_query_type_pqf;
                sr->u.request->query.pqf = pQuery;
            }
            if (sortKeys)
            {
                sr->u.request->sort_type = Z_SRW_sort_type_sort;
                sr->u.request->sort.sortKeys = sortKeys;
            }
            sr->u.request->recordSchema = yaz_uri_val(p1, "recordSchema", decode);
            sr->u.request->recordPacking = yaz_uri_val(p1, "recordPacking", decode);
            if (!sr->u.request->recordPacking)
                sr->u.request->recordPacking = "xml";
            yaz_uri_val_int(p1, "maximumRecords", decode, 
                        &sr->u.request->maximumRecords);
            yaz_uri_val_int(p1, "startRecord", decode,
                        &sr->u.request->startRecord);

            sr->u.request->database = db;
	    *soap_ns = "SRU";
	    return 0;
        }
	else if (p1 && !strcmp(operation, "explain"))
	{
            Z_SRW_PDU *sr = yaz_srw_get(decode, Z_SRW_explain_request);

	    *srw_pdu = sr;
            sr->u.explain_request->recordPacking =
		yaz_uri_val(p1, "recordPacking", decode);
            if (!sr->u.explain_request->recordPacking)
                sr->u.explain_request->recordPacking = "xml";
	    sr->u.explain_request->database = db;
	    *soap_ns = "SRU";
	    return 0;
	}
#endif
	return 1;
    }
    return 2;
}
