/*
 * Copyright (c) 2002-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: srw.c,v 1.2 2003-02-14 18:49:24 adam Exp $
 */

#include <yaz/srw.h>

#if HAVE_XSLT
#include <libxml/parser.h>
#include <libxml/tree.h>

static void add_xsd_string_n(xmlNodePtr ptr, const char *elem, char *val,
                             int len)
{
    if (val)
    {
        xmlNodePtr c = xmlNewChild(ptr, 0, elem, 0);
        xmlNodePtr t = xmlNewTextLen(val, len);
        xmlAddChild(c, t);
    }
}

static void add_xsd_string(xmlNodePtr ptr, const char *elem, char *val)
{
    if (val)
        xmlNewChild(ptr, 0, elem, val);
}

static void add_xsd_integer(xmlNodePtr ptr, const char *elem, int *val)
{
    if (val)
    {
        char str[30];
        sprintf(str, "%d", *val);
        xmlNewChild(ptr, 0, elem, str);
    }
}

static int match_element(xmlNodePtr ptr, const char *elem)
{
    if (ptr->type == XML_ELEMENT_NODE && !strcmp(ptr->name, elem))
        return 1;
    return 0;
}

static int match_xsd_string_n(xmlNodePtr ptr, const char *elem, ODR o,
                              char **val, int *len)
{
    struct _xmlAttr *attr;
    if (!match_element(ptr, elem))
        return 0;
    for (attr = ptr->properties; attr; attr = attr->next)
        if (!strcmp(attr->name, "type") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
        {
            const char *t = strchr(attr->children->content, ':');
            if (t)
                t = t + 1;
            else
                t = attr->children->content;
            if (!strcmp(t, "string"))
                break;
        }
    if (!attr)
        return 0;
    ptr = ptr->children;
    if (!ptr || ptr->type != XML_TEXT_NODE)
        return 0;
    *val = odr_strdup(o, ptr->content);
    if (len)
        *len = strlen(ptr->content);
    return 1;
}


static int match_xsd_string(xmlNodePtr ptr, const char *elem, ODR o,
                            char **val)
{
    return match_xsd_string_n(ptr, elem, o, val, 0);
}
                     
static int match_xsd_integer(xmlNodePtr ptr, const char *elem, ODR o, int **val)
{
    struct _xmlAttr *attr;
    if (!match_element(ptr, elem))
        return 0;
    for (attr = ptr->properties; attr; attr = attr->next)
        if (!strcmp(attr->name, "type") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
        {
            const char *t = strchr(attr->children->content, ':');
            if (t)
                t = t + 1;
            else
                t = attr->children->content;
            if (!strcmp(t, "integer"))
                break;
        }
    if (!attr)
        return 0;
    ptr = ptr->children;
    if (!ptr || ptr->type != XML_TEXT_NODE)
        return 0;
    *val = odr_intdup(o, atoi(ptr->content));
    return 1;
}

static int yaz_srw_records(ODR o, xmlNodePtr pptr, Z_SRW_record **recs,
                           int *num, void *client_data, const char *ns)
{
    if (o->direction == ODR_DECODE)
    {
        int i;
        xmlNodePtr ptr;
        *num = 0;
        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "record"))
                (*num)++;
        }
        if (!*num)
            return 1;
        *recs = odr_malloc(o, *num * sizeof(**recs));
        for (i = 0, ptr = pptr->children; ptr; ptr = ptr->next, i++)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "record"))
            {
                xmlNodePtr rptr;
                (*recs)[i].recordSchema = 0;
                (*recs)[i].recordData_buf = 0;
                (*recs)[i].recordData_len = 0;
                (*recs)[i].recordPosition = 0;
                for (rptr = ptr->children; rptr; rptr = rptr->next)
                {
                    if (match_xsd_string(rptr, "recordSchema", o, 
                                         &(*recs)[i].recordSchema))
                        ;
                    else if (match_xsd_string_n(rptr, "recordData", o, 
                                                &(*recs)[i].recordData_buf,
                                                &(*recs)[i].recordData_len))
                        ;
                    else if (match_xsd_integer(rptr, "recordPosition", o, 
                                               &(*recs)[i].recordPosition))
                        ;
                }
            }
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        for (i = 0; i < *num; i++)
        {
            xmlNodePtr rptr = xmlNewChild(pptr, 0, "record", 0);
            add_xsd_string(rptr, "recordSchema", (*recs)[i].recordSchema);
            add_xsd_string_n(rptr, "recordData", (*recs)[i].recordData_buf,
                             (*recs)[i].recordData_len);
            add_xsd_integer(rptr, "recordPosition", (*recs)[i].recordPosition);
        }
    }
    return 0;
}

static int yaz_srw_diagnostics(ODR o, xmlNodePtr pptr, Z_SRW_diagnostic **recs,
                               int *num, void *client_data, const char *ns)
{
    if (o->direction == ODR_DECODE)
    {
        int i;
        xmlNodePtr ptr;
        *num = 0;
        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "diagnostic"))
                (*num)++;
        }
        if (!*num)
            return 1;
        *recs = odr_malloc(o, *num * sizeof(**recs));
        for (i = 0, ptr = pptr->children; ptr; ptr = ptr->next, i++)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "diagnostic"))
            {
                xmlNodePtr rptr;
                (*recs)[i].code = 0;
                (*recs)[i].details = 0;
                for (rptr = ptr->children; rptr; rptr = rptr->next)
                {
                    if (match_xsd_integer(rptr, "code", o, 
                                               &(*recs)[i].code))
                        ;
                    else if (match_xsd_string(rptr, "details", o, 
                                              &(*recs)[i].details))
                        ;
                }
                i++;
            }
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        for (i = 0; i < *num; i++)
        {
            xmlNodePtr rptr = xmlNewChild(pptr, 0, "diagnostic", 0);
            add_xsd_integer(rptr, "code", (*recs)[i].code);
            add_xsd_string(rptr, "details", (*recs)[i].details);
        }
    }
    return 0;
}


int yaz_srw_codec(ODR o, void * vptr, Z_SRW_searchRetrieve **handler_data,
                  void *client_data, const char *ns)
{
    xmlNodePtr pptr = vptr;
    if (o->direction == ODR_DECODE)
    {
        xmlNodePtr method = pptr->children;

        while (method && method->type == XML_TEXT_NODE)
            method = method->next;
        
        if (method->type != XML_ELEMENT_NODE)
            return -1;
        if (method && !strcmp(method->name, "searchRetrieveRequest"))
        {
            Z_SRW_searchRetrieve **p = handler_data;
            xmlNodePtr ptr = method->children;
            Z_SRW_searchRetrieveRequest *req;

            *p = odr_malloc(o, sizeof(**p));
            (*p)->which = Z_SRW_searchRetrieve_request;
            req = (*p)->u.request = odr_malloc(o, sizeof(*req));
            req->query = 0;
            req->xQuery = 0;
            req->sortKeys = 0;
            req->xSortKeys = 0;
            req->startRecord = 0;
            req->maximumRecords = 0;
            req->recordSchema = 0;
            req->recordPacking = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (match_xsd_string(ptr, "query", o, 
                                     &req->query))
                    ;
                else if (match_xsd_string(ptr, "pQuery", o, 
                                     &req->pQuery))
                    ;
                else if (match_xsd_string(ptr, "sortKeys", o, 
                                          &req->sortKeys))
                    ;
                else if (match_xsd_string(ptr, "recordSchema", o, 
                                          &req->recordSchema))
                    ;
                else if (match_xsd_string(ptr, "recordPacking", o,
                                          &req->recordPacking))
                    ;
                else if (match_xsd_integer(ptr, "startRecord", o,
                                           &req->startRecord))
                    ;
                else if (match_xsd_integer(ptr, "maximumRecords", o,
                                           &req->maximumRecords))
                    ;
                /* missing is xQuery, xSortKeys .. */
            }
        }
        else if (method && !strcmp(method->name, "searchRetrieveResponse"))
        {
            Z_SRW_searchRetrieve **p = handler_data;
            xmlNodePtr ptr = method->children;
            Z_SRW_searchRetrieveResponse *res;

            *p = odr_malloc(o, sizeof(**p));
            (*p)->which = Z_SRW_searchRetrieve_response;
            res = (*p)->u.response = odr_malloc(o, sizeof(*res));

            res->numberOfRecords = 0;
            res->resultSetId = 0;
            res->resultSetIdleTime = 0;
            res->records = 0;
            res->num_records = 0;
            res->diagnostics = 0;
            res->num_diagnostics = 0;
            res->nextRecordPosition = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (match_xsd_integer(ptr, "numberOfRecords", o, 
                                      &res->numberOfRecords))
                    ;
                else if (match_xsd_string(ptr, "resultSetId", o, 
                                          &res->resultSetId))
                    ;
                else if (match_xsd_integer(ptr, "resultSetIdleTime", o, 
                                           &res->resultSetIdleTime))
                    ;
                else if (match_element(ptr, "records"))
                    yaz_srw_records(o, ptr, &res->records,
                                    &res->num_records, client_data,
                                    ns);
                else if (match_element(ptr, "diagnostics"))
                    yaz_srw_diagnostics(o, ptr, &res->diagnostics,
                                        &res->num_diagnostics,
                                        client_data, ns);
                else if (match_xsd_integer(ptr, "nextRecordPosition", o,
                                           &res->nextRecordPosition))
                    ;
            }

        }
        else
            return -1;

    }
    else if (o->direction == ODR_ENCODE)
    {
        Z_SRW_searchRetrieve **p = handler_data;
        if ((*p)->which == Z_SRW_searchRetrieve_request)
        {
            Z_SRW_searchRetrieveRequest *req = (*p)->u.request;
            xmlNsPtr ns_srw = xmlNewNs(pptr, ns, "zs");
            xmlNodePtr ptr = xmlNewChild(pptr, ns_srw,
                                         "searchRetrieveRequest", 0);

            add_xsd_string(ptr, "query", req->query);
            add_xsd_string(ptr, "pQuery", req->pQuery);
            add_xsd_string(ptr, "sortKeys", req->sortKeys);
            add_xsd_integer(ptr, "startRecord", req->startRecord);
            add_xsd_integer(ptr, "maximumRecords", req->maximumRecords);
            add_xsd_string(ptr, "recordSchema", req->recordSchema);
            add_xsd_string(ptr, "recordPacking", req->recordPacking);
        }
        else if ((*p)->which == Z_SRW_searchRetrieve_response)
        {
            Z_SRW_searchRetrieveResponse *res = (*p)->u.response;
            xmlNsPtr ns_srw = xmlNewNs(pptr, ns, "zs");
            xmlNodePtr ptr = xmlNewChild(pptr, ns_srw,
                                         "searchRetrieveResponse", 0);

            add_xsd_integer(ptr, "numberOfRecords", res->numberOfRecords);
            add_xsd_string(ptr, "resultSetId", res->resultSetId);
            add_xsd_integer(ptr, "resultSetIdleTime", res->resultSetIdleTime);
            if (res->num_records)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, "records", 0);
                yaz_srw_records(o, rptr, &res->records, &res->num_records,
                                client_data, ns);
            }
            if (res->num_diagnostics)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, "diagnostics", 0);
                yaz_srw_diagnostics(o, rptr, &res->diagnostics,
                                    &res->num_diagnostics, client_data, ns);
            }
            add_xsd_integer(ptr, "nextRecordPosition", res->nextRecordPosition);
        }
        else
            return -1;

    }
    return 0;
}

Z_SRW_searchRetrieve *yaz_srw_get(ODR o, int which)
{
    Z_SRW_searchRetrieve *sr = odr_malloc(o, sizeof(*o));
    sr->which = which;
    switch(which)
    {
    case Z_SRW_searchRetrieve_request:
        sr->u.request = odr_malloc(o, sizeof(*sr->u.request));
        sr->u.request->query = 0;
        sr->u.request->xQuery = 0;
        sr->u.request->pQuery = 0;
        sr->u.request->sortKeys = 0;
        sr->u.request->xSortKeys = 0;
        sr->u.request->startRecord = 0;
        sr->u.request->maximumRecords = 0;
        sr->u.request->recordSchema = 0;
        sr->u.request->recordPacking = 0;
        break;
    case Z_SRW_searchRetrieve_response:
        sr->u.response = odr_malloc(o, sizeof(*sr->u.response));
        sr->u.response->numberOfRecords = 0;
        sr->u.response->resultSetId = 0;
        sr->u.response->resultSetIdleTime = 0;
        sr->u.response->records = 0;
        sr->u.response->num_records = 0;
        sr->u.response->diagnostics = 0;
        sr->u.response->num_diagnostics = 0;
        sr->u.response->nextRecordPosition = 0;
    }
    return sr;
}
#endif
