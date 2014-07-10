/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file srw.c
 * \brief Implements SRW/SRU package encoding and decoding
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <yaz/srw.h>
#include <yaz/wrbuf.h>
#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <assert.h>
#include <yaz/facet.h>
#include <yaz/proto.h>
#include "sru-p.h"

char *yaz_negotiate_sru_version(char *input_ver)
{
    if (!input_ver)
        return "2.0";
    if (!strcmp(input_ver, "1.1"))
        return "1.1";
    if (!strncmp(input_ver, "1.", 2))
        return "1.2";
    if (!strncmp(input_ver, "2.", 2))
        return "2.0";
    return 0;
}

static int yaz_srw_record(ODR o, xmlNodePtr pptr, Z_SRW_record *rec,
                          Z_SRW_extra_record **extra,
                          void *client_data, int version2)
{
    if (o->direction == ODR_DECODE)
    {
        Z_SRW_extra_record ex;

        char *spack = 0;
        xmlNodePtr ptr;
#ifdef Z_SRW_packed
        rec->packing = 0;
#endif
        rec->recordSchema = 0;
        rec->recordData_buf = 0;
        rec->recordData_len = 0;
        rec->recordPosition = 0;
        *extra = 0;

        ex.extraRecordData_buf = 0;
        ex.extraRecordData_len = 0;
        ex.recordIdentifier = 0;

        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {

            if (yaz_match_xsd_string(ptr, "recordSchema", o,
                                     &rec->recordSchema))
                ;
            else if (yaz_match_xsd_string(ptr, "recordPacking", o, &spack))
                ;  /* can't rely on it: in SRU 2.0 it's different semantics */
            else if (yaz_match_xsd_integer(ptr, "recordPosition", o,
                                           &rec->recordPosition))
                ;
            else if (yaz_match_xsd_element(ptr, "recordData"))
            {
                /* we assume XML packing, if any element nodes exist below
                   recordData. Unfortunately, in SRU 2.0 recordPacking
                   means something different */
                xmlNode *p = ptr->children;
                for (; p; p = p->next)
                    if (p->type == XML_ELEMENT_NODE)
                        break;
                if (p)
                {
                    yaz_match_xsd_XML_n2(
                        ptr, "recordData", o,
                        &rec->recordData_buf, &rec->recordData_len, 1);
                    rec->recordPacking = Z_SRW_recordPacking_XML;
                }
                else
                {
                    yaz_match_xsd_string_n(
                        ptr, "recordData", o,
                        &rec->recordData_buf, &rec->recordData_len);
                    rec->recordPacking = Z_SRW_recordPacking_string;
                }
            }
            else if (yaz_match_xsd_XML_n(ptr, "extraRecordData", o,
                                         &ex.extraRecordData_buf,
                                         &ex.extraRecordData_len) )
                ;
            else
                yaz_match_xsd_string(ptr, "recordIdentifier", o,
                                     &ex.recordIdentifier);
        }
        if (ex.extraRecordData_buf || ex.recordIdentifier)
        {
            *extra = (Z_SRW_extra_record *)
                odr_malloc(o, sizeof(Z_SRW_extra_record));
            memcpy(*extra, &ex, sizeof(Z_SRW_extra_record));
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        xmlNodePtr ptr = pptr;
        int pack = rec->recordPacking;
        const char *spack = yaz_srw_pack_to_str(pack);

        /* recordSchema and recordData are required */
        if (!rec->recordSchema)
            xmlNewChild(ptr, 0, BAD_CAST "recordSchema", 0);
        else
            add_xsd_string(ptr, "recordSchema", rec->recordSchema);
        if (spack)
        {
            if (version2)
                add_xsd_string(ptr, "recordXMLEscaping", spack);
            else
                add_xsd_string(ptr, "recordPacking", spack);
        }
        if (!rec->recordData_buf)
            xmlNewChild(ptr, 0, BAD_CAST "recordData", 0);
        else
        {
            switch (pack)
            {
            case Z_SRW_recordPacking_string:
                add_xsd_string_n(ptr, "recordData", rec->recordData_buf,
                                 rec->recordData_len);
                break;
            case Z_SRW_recordPacking_XML:
                add_XML_n(ptr, "recordData", rec->recordData_buf,
                          rec->recordData_len, 0);
                break;
            case Z_SRW_recordPacking_URL:
                add_xsd_string_n(ptr, "recordData", rec->recordData_buf,
                                 rec->recordData_len);
                break;
            }
        }
        if (rec->recordPosition)
            add_xsd_integer(ptr, "recordPosition", rec->recordPosition );
        if (extra && *extra)
        {
            if ((*extra)->recordIdentifier)
                add_xsd_string(ptr, "recordIdentifier",
                               (*extra)->recordIdentifier);
            if ((*extra)->extraRecordData_buf)
                add_XML_n(ptr, "extraRecordData",
                          (*extra)->extraRecordData_buf,
                          (*extra)->extraRecordData_len, 0);
        }
    }
    return 0;
}

static int yaz_srw_records(ODR o, xmlNodePtr pptr, Z_SRW_record **recs,
                           Z_SRW_extra_record ***extra,
                           int *num, void *client_data, int version2)
{
    if (o->direction == ODR_DECODE)
    {
        int i;
        xmlNodePtr ptr;
        *num = 0;
        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !xmlStrcmp(ptr->name, BAD_CAST "record"))
                (*num)++;
        }
        if (!*num)
            return 1;
        *recs = (Z_SRW_record *) odr_malloc(o, *num * sizeof(**recs));
        *extra = (Z_SRW_extra_record **) odr_malloc(o, *num * sizeof(**extra));
        for (i = 0, ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !xmlStrcmp(ptr->name, BAD_CAST "record"))
            {
                yaz_srw_record(o, ptr, *recs + i, *extra + i, client_data, 0);
                i++;
            }
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        for (i = 0; i < *num; i++)
        {
            xmlNodePtr rptr = xmlNewChild(pptr, 0, BAD_CAST "record",
                                          0);
            yaz_srw_record(o, rptr, (*recs)+i, (*extra ? *extra + i : 0),
                           client_data, version2);
        }
    }
    return 0;
}

static int yaz_srw_version(ODR o, xmlNodePtr pptr, Z_SRW_recordVersion *rec,
                           void *client_data, const char *ns)
{
    if (o->direction == ODR_DECODE)
    {
        xmlNodePtr ptr;
        rec->versionType = 0;
        rec->versionValue = 0;
        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {

            if (yaz_match_xsd_string(ptr, "versionType", o,
                                     &rec->versionType))
                ;
            else
                yaz_match_xsd_string(ptr, "versionValue", o,
                                     &rec->versionValue);
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        xmlNodePtr ptr = pptr;
        add_xsd_string(ptr, "versionType", rec->versionType);
        add_xsd_string(ptr, "versionValue", rec->versionValue);
    }
    return 0;
}

static int yaz_srw_versions(ODR o, xmlNodePtr pptr,
                            Z_SRW_recordVersion **vers,
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
                !xmlStrcmp(ptr->name, BAD_CAST "recordVersion"))
                (*num)++;
        }
        if (!*num)
            return 1;
        *vers = (Z_SRW_recordVersion *) odr_malloc(o, *num * sizeof(**vers));
        for (i = 0, ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !xmlStrcmp(ptr->name, BAD_CAST "recordVersion"))
            {
                yaz_srw_version(o, ptr, *vers + i, client_data, ns);
                i++;
            }
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        for (i = 0; i < *num; i++)
        {
            xmlNodePtr rptr = xmlNewChild(pptr, 0, BAD_CAST "version",
                                          0);
            yaz_srw_version(o, rptr, (*vers)+i, client_data, ns);
        }
    }
    return 0;
}

Z_FacetTerm *yaz_sru_proxy_get_facet_term_count(ODR odr, xmlNodePtr node)
{
    Odr_int freq;
    xmlNodePtr child;
    WRBUF wrbuf = wrbuf_alloc();
    Z_FacetTerm *facet_term;
    const char *freq_string = yaz_element_attribute_value_get(
        node, "facetvalue", "est_representation");
    if (freq_string)
        freq = odr_atoi(freq_string);
    else
        freq = -1;

    for (child = node->children; child ; child = child->next)
    {
        if (child->type == XML_TEXT_NODE)
            wrbuf_puts(wrbuf, (const char *) child->content);
    }
    facet_term = facet_term_create_cstr(odr, wrbuf_cstr(wrbuf), freq);
    wrbuf_destroy(wrbuf);
    return facet_term;
}

static Z_FacetField *yaz_sru_proxy_decode_facet_field(ODR odr, xmlNodePtr ptr)
{
    Z_AttributeList *list;
    Z_FacetField *facet_field;
    int num_terms = 0;
    int index = 0;
    xmlNodePtr node;
    /* USE attribute */
    const char* name = yaz_element_attribute_value_get(ptr, "facet", "code");
    yaz_log(YLOG_DEBUG, "sru-proxy facet type: %s", name);

    list = zget_AttributeList_use_string(odr, name);
    for (node = ptr->children; node; node = node->next) {
        if (yaz_match_xsd_element(node, "facetvalue"))
            num_terms++;
    }
    facet_field = facet_field_create(odr, list, num_terms);
    index = 0;
    for (node = ptr->children; node; node = node->next)
    {
        if (yaz_match_xsd_element(node, "facetvalue"))
        {
            facet_field_term_set(odr, facet_field,
                                 yaz_sru_proxy_get_facet_term_count(odr, node),
                                 index);
            index++;
        }
    }
    return facet_field;
}

static int yaz_sru_proxy_decode_facets(ODR o, xmlNodePtr root,
                                       Z_FacetList **facetList)
{
    xmlNodePtr ptr;

    for (ptr = root->children; ptr; ptr = ptr->next)
    {
        if (yaz_match_xsd_element(ptr, "facets"))
        {
            xmlNodePtr node;
            Z_FacetList *facet_list;
            int num_facets = 0;
            for (node = ptr->children; node; node= node->next)
            {
                if (node->type == XML_ELEMENT_NODE)
                    num_facets++;
            }
            facet_list = facet_list_create(o, num_facets);
            num_facets = 0;
            for (node = ptr->children; node; node= node->next)
            {
                if (yaz_match_xsd_element(node, "facet"))
                {
                    facet_list_field_set(
                        o, facet_list,
                        yaz_sru_proxy_decode_facet_field(o, node), num_facets);
                    num_facets++;
                }
            }
            *facetList = facet_list;
            break;
        }
    }
    return 0;
}



static int yaz_srw_decode_diagnostics(ODR o, xmlNodePtr pptr,
                                      Z_SRW_diagnostic **recs, int *num,
                                      void *client_data, const char *ns)
{
    int i;
    xmlNodePtr ptr;
    *num = 0;
    for (ptr = pptr; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE &&
            !xmlStrcmp(ptr->name, BAD_CAST "diagnostic"))
            (*num)++;
    }
    if (!*num)
        return 1;
    *recs = (Z_SRW_diagnostic *) odr_malloc(o, *num * sizeof(**recs));
    for (i = 0; i < *num; i++)
    {
        (*recs)[i].uri = 0;
        (*recs)[i].details = 0;
        (*recs)[i].message = 0;
    }
    for (i = 0, ptr = pptr; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE &&
            !xmlStrcmp(ptr->name, BAD_CAST "diagnostic"))
        {
            xmlNodePtr rptr;
            (*recs)[i].uri = 0;
            (*recs)[i].details = 0;
            (*recs)[i].message = 0;
            for (rptr = ptr->children; rptr; rptr = rptr->next)
            {
                if (yaz_match_xsd_string(rptr, "uri", o, &(*recs)[i].uri))
                    ;
                else if (yaz_match_xsd_string(rptr, "details", o,
                                              &(*recs)[i].details))
                    ;
                else
                    yaz_match_xsd_string(rptr, "message", o,
                                         &(*recs)[i].message);
            }
            i++;
        }
    }
    return 0;
}

int sru_decode_surrogate_diagnostics(const char *buf, size_t len,
                                     Z_SRW_diagnostic **diag,
                                     int *num, ODR odr)
{
    int ret = 0;
    xmlDocPtr doc = xmlParseMemory(buf, len);
    if (doc)
    {
        xmlNodePtr ptr = xmlDocGetRootElement(doc);
        while (ptr && ptr->type != XML_ELEMENT_NODE)
            ptr = ptr->next;
        if (ptr && ptr->ns
            && !xmlStrcmp(ptr->ns->href,
                          BAD_CAST "http://www.loc.gov/zing/srw/diagnostic/"))
        {
            ret = yaz_srw_decode_diagnostics(odr, ptr, diag, num, 0, 0);
        }
        xmlFreeDoc(doc);
    }
    return ret;
}

static int yaz_srw_diagnostics(ODR o, xmlNodePtr pptr, Z_SRW_diagnostic **recs,
                               int *num, void *client_data, const char *ns,
                               int version2)
{
    if (o->direction == ODR_DECODE)
    {
        return yaz_srw_decode_diagnostics(o, pptr->children, recs, num, client_data, ns);
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        xmlNsPtr ns_diag =
            xmlNewNs(pptr, BAD_CAST (version2 ?
                                     YAZ_XMLNS_DIAG_v2 : YAZ_XMLNS_DIAG_v1_1),
                     BAD_CAST "diag" );
        for (i = 0; i < *num; i++)
        {
            const char *std_diag = "info:srw/diagnostic/1/";
            const char *ucp_diag = "info:srw/diagnostic/12/";
            xmlNodePtr rptr = xmlNewChild(pptr, ns_diag,
                                          BAD_CAST "diagnostic", 0);
            add_xsd_string(rptr, "uri", (*recs)[i].uri);
            add_xsd_string(rptr, "details", (*recs)[i].details);
            if ((*recs)[i].message)
                add_xsd_string(rptr, "message", (*recs)[i].message);
            else if ((*recs)[i].uri )
            {
                if (!strncmp((*recs)[i].uri, std_diag, strlen(std_diag)))
                {
                    int no = atoi((*recs)[i].uri + strlen(std_diag));
                    const char *message = yaz_diag_srw_str(no);
                    if (message)
                        add_xsd_string(rptr, "message", message);
                }
                else if (!strncmp((*recs)[i].uri, ucp_diag, strlen(ucp_diag)))
                {
                    int no = atoi((*recs)[i].uri + strlen(ucp_diag));
                    const char *message = yaz_diag_sru_update_str(no);
                    if (message)
                        add_xsd_string(rptr, "message", message);
                }
            }
        }
    }
    return 0;
}

static int yaz_srw_term(ODR o, xmlNodePtr pptr, Z_SRW_scanTerm *term,
                        void *client_data, const char *ns)
{
    if (o->direction == ODR_DECODE)
    {
        xmlNodePtr ptr;
        term->value = 0;
        term->numberOfRecords = 0;
        term->displayTerm = 0;
        term->whereInList = 0;
        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (yaz_match_xsd_string(ptr, "value", o,  &term->value))
                ;
            else if (yaz_match_xsd_integer(ptr, "numberOfRecords", o,
                                           &term->numberOfRecords))
                ;
            else if (yaz_match_xsd_string(ptr, "displayTerm", o,
                                          &term->displayTerm))
                ;
            else
                yaz_match_xsd_string(ptr, "whereInList", o, &term->whereInList);
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        xmlNodePtr ptr = pptr;
        add_xsd_string(ptr, "value", term->value);
        add_xsd_integer(ptr, "numberOfRecords", term->numberOfRecords);
        add_xsd_string(ptr, "displayTerm", term->displayTerm);
        add_xsd_string(ptr, "whereInList", term->whereInList);
    }
    return 0;
}

static int yaz_srw_terms(ODR o, xmlNodePtr pptr, Z_SRW_scanTerm **terms,
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
                !xmlStrcmp(ptr->name, BAD_CAST "term"))
                (*num)++;
        }
        if (!*num)
            return 1;
        *terms = (Z_SRW_scanTerm *) odr_malloc(o, *num * sizeof(**terms));
        for (i = 0, ptr = pptr->children; ptr; ptr = ptr->next, i++)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !xmlStrcmp(ptr->name, BAD_CAST "term"))
                yaz_srw_term(o, ptr, (*terms)+i, client_data, ns);
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        for (i = 0; i < *num; i++)
        {
            xmlNodePtr rptr = xmlNewChild(pptr, 0, BAD_CAST "term", 0);
            yaz_srw_term(o, rptr, (*terms)+i, client_data, ns);
        }
    }
    return 0;
}

static void encode_echoed_args(xmlNodePtr ptr, Z_SRW_PDU *p, const char *name)
{
    Z_SRW_extra_arg *ea = p->extra_args;
    if (ea)
    {
        xmlNode *p1 = xmlNewChild(ptr, 0, BAD_CAST name, 0);
        xmlNode *p2 = 0;
        for (; ea; ea = ea->next)
        {
            if (ea->name && ea->name[0] == 'x' && ea->name[1] == '-')
            {
                /* not really according to XSD as of July 2014 */
                if (!p2)
                    p2 = xmlNewChild(p1, 0,
                                     BAD_CAST "extraRequestData", 0);
                /* skip +2: "x-" in element */
                add_xsd_string(p2, ea->name + 2, ea->value);
            }
            else
                add_xsd_string(p1, ea->name, ea->value);
        }
    }
}

int yaz_srw_codec(ODR o, void * vptr, Z_SRW_PDU **handler_data,
                  void *client_data, const char *ns)
{
    xmlNodePtr pptr = (xmlNodePtr) vptr;
    if (o->direction == ODR_DECODE)
    {
        Z_SRW_PDU **p = handler_data;
        xmlNodePtr method = pptr->children;
        char *neg_version;

        while (method && method->type == XML_TEXT_NODE)
            method = method->next;

        if (!method)
            return -1;
        if (method->type != XML_ELEMENT_NODE)
            return -1;

        *p = yaz_srw_get_core_v_2_0(o);

        if (!xmlStrcmp(method->name, BAD_CAST "searchRetrieveRequest"))
        {
            xmlNodePtr ptr = method->children;
            Z_SRW_searchRetrieveRequest *req;
            char *recordPacking = 0;
            char *recordXMLEscaping = 0;
            const char *facetLimit = 0;
            const char *facetStart = 0;
            const char *facetSort = 0;

            (*p)->which = Z_SRW_searchRetrieve_request;
            req = (*p)->u.request = (Z_SRW_searchRetrieveRequest *)
                odr_malloc(o, sizeof(*req));
            req->queryType = "cql";
            req->query = 0;
            req->sort_type = Z_SRW_sort_type_none;
            req->sort.none = 0;
            req->startRecord = 0;
            req->maximumRecords = 0;
            req->recordSchema = 0;
            req->recordPacking = 0;
            req->packing = 0;
            req->recordXPath = 0;
            req->resultSetTTL = 0;
            req->stylesheet = 0;
            req->database = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_string(ptr, "queryType", o,
                                          &req->queryType))
                    ;
                else if (yaz_match_xsd_string(ptr, "query", o,
                                          &req->query))
                    ;
                else if (yaz_match_xsd_string(ptr, "pQuery", o,
                                          &req->query))
                    req->queryType = "pqf";
                else if (yaz_match_xsd_string(ptr, "xQuery", o,
                                          &req->query))
                    req->queryType = "xcql";
                else if (yaz_match_xsd_integer(ptr, "startRecord", o,
                                           &req->startRecord))
                    ;
                else if (yaz_match_xsd_integer(ptr, "maximumRecords", o,
                                           &req->maximumRecords))
                    ;
                else if (yaz_match_xsd_string(ptr, "recordPacking", o,
                                          &recordPacking))
                    ;
                else if (yaz_match_xsd_string(ptr, "recordXMLEscaping", o,
                                          &recordXMLEscaping))
                    ;
                else if (yaz_match_xsd_string(ptr, "recordSchema", o,
                                          &req->recordSchema))
                    ;
                else if (yaz_match_xsd_string(ptr, "recordXPath", o,
                                          &req->recordXPath))
                    ;
                else if (yaz_match_xsd_integer(ptr, "resultSetTTL", o,
                                           &req->resultSetTTL))
                    ;
                else if (yaz_match_xsd_string(ptr, "sortKeys", o,
                                          &req->sort.sortKeys))
                    req->sort_type = Z_SRW_sort_type_sort;
                else if (yaz_match_xsd_string(ptr, "stylesheet", o,
                                          &req->stylesheet))
                    ;
                else if (yaz_match_xsd_string(ptr, "database", o,
                                              &req->database))
                    ;
                else if (yaz_match_xsd_string(ptr, "facetLimit", o,
                                              (char**) &facetLimit))
                    ;
                else if (yaz_match_xsd_string(ptr, "facetStart", o,
                                              (char**) &facetStart))
                    ;
                else if (yaz_match_xsd_string(ptr, "facetSort", o,
                                              (char**) &facetSort))
                    ;
                else
                    ;
            }
            if (!req->query)
            {
                /* should put proper diagnostic here */
                return -1;
            }
            if (!strcmp((*p)->srw_version, "2.0"))
            {
                req->recordPacking = recordXMLEscaping;
                req->packing = recordPacking;
            }
            else
            {
                req->recordPacking = recordPacking;
            }
            yaz_sru_facet_request(o, &req->facetList, &facetLimit, &facetStart,
                                  &facetSort);
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "searchRetrieveResponse"))
        {
            xmlNodePtr ptr = method->children;
            Z_SRW_searchRetrieveResponse *res;

            (*p)->which = Z_SRW_searchRetrieve_response;
            res = (*p)->u.response = (Z_SRW_searchRetrieveResponse *)
                odr_malloc(o, sizeof(*res));

            res->numberOfRecords = 0;
            res->resultCountPrecision = 0;
            res->resultSetId = 0;
            res->resultSetIdleTime = 0;
            res->records = 0;
            res->num_records = 0;
            res->diagnostics = 0;
            res->num_diagnostics = 0;
            res->nextRecordPosition = 0;
            res->facetList = 0;
            res->suggestions = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_XML_n(ptr, "extraResponseData", o,
                                         &(*p)->extraResponseData_buf,
                                         &(*p)->extraResponseData_len))
                    ;
                else if (yaz_match_xsd_integer(ptr, "numberOfRecords", o,
                                           &res->numberOfRecords))
                    ;
                else if (yaz_match_xsd_string(ptr, "resultCountPrecision", o,
                                           &res->resultCountPrecision))
                    ;
                else if (yaz_match_xsd_string(ptr, "resultSetId", o,
                                          &res->resultSetId))
                    ;
                else if (yaz_match_xsd_integer(ptr, "resultSetIdleTime", o,
                                           &res->resultSetIdleTime))
                    ;
                else if (yaz_match_xsd_integer(ptr, "resultSetTTL", o,
                                           &res->resultSetIdleTime))
                    ;
                else if (yaz_match_xsd_element(ptr, "records"))
                    yaz_srw_records(o, ptr, &res->records,
                                    &res->extra_records,
                                    &res->num_records, client_data, 0);
                else if (yaz_match_xsd_integer(ptr, "nextRecordPosition", o,
                                           &res->nextRecordPosition))
                    ;
                else if (yaz_match_xsd_element(ptr, "diagnostics"))
                    yaz_srw_diagnostics(o, ptr, &res->diagnostics,
                                        &res->num_diagnostics,
                                        client_data, ns, 0);
                else if (yaz_match_xsd_element(ptr, "facet_analysis"))
                    yaz_sru_proxy_decode_facets(o, ptr, &res->facetList);
                else if (yaz_match_xsd_element(ptr, "facetedResults"))
                    yaz_sru_facet_response(o, &res->facetList, ptr);
            }
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "explainRequest"))
        {
            Z_SRW_explainRequest *req;
            xmlNodePtr ptr = method->children;

            (*p)->which = Z_SRW_explain_request;
            req = (*p)->u.explain_request = (Z_SRW_explainRequest *)
                odr_malloc(o, sizeof(*req));
            req->recordPacking = 0;
            req->packing = 0;
            req->database = 0;
            req->stylesheet = 0;
            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_XML_n(ptr, "extraResponseData", o,
                                         &(*p)->extraResponseData_buf,
                                         &(*p)->extraResponseData_len))
                    ;
                else if (yaz_match_xsd_string(ptr, "stylesheet", o,
                                          &req->stylesheet))
                    ;
                else if (yaz_match_xsd_string(ptr, "recordPacking", o,
                                          &req->recordPacking))
                    ;
                else
                    yaz_match_xsd_string(ptr, "database", o, &req->database);
            }
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "explainResponse"))
        {
            Z_SRW_explainResponse *res;
            xmlNodePtr ptr = method->children;

            (*p)->which = Z_SRW_explain_response;
            res = (*p)->u.explain_response = (Z_SRW_explainResponse*)
                odr_malloc(o, sizeof(*res));
            res->diagnostics = 0;
            res->num_diagnostics = 0;
            res->record.recordSchema = 0;
            res->record.recordData_buf = 0;
            res->record.recordData_len = 0;
            res->record.recordPosition = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_XML_n(ptr, "extraResponseData", o,
                                         &(*p)->extraResponseData_buf,
                                         &(*p)->extraResponseData_len))
                    ;
                else if (yaz_match_xsd_element(ptr, "record"))
                    yaz_srw_record(o, ptr, &res->record, &res->extra_record,
                                   client_data, 0);
                else if (yaz_match_xsd_element(ptr, "diagnostics"))
                    yaz_srw_diagnostics(o, ptr, &res->diagnostics,
                                        &res->num_diagnostics,
                                        client_data, ns, 0);
                ;
            }
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "scanRequest"))
        {
            Z_SRW_scanRequest *req;
            xmlNodePtr ptr = method->children;

            (*p)->which = Z_SRW_scan_request;
            req = (*p)->u.scan_request = (Z_SRW_scanRequest *)
                odr_malloc(o, sizeof(*req));
            req->queryType = "cql";
            req->scanClause = 0;
            req->responsePosition = 0;
            req->maximumTerms = 0;
            req->stylesheet = 0;
            req->database = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_XML_n(ptr, "extraResponseData", o,
                                         &(*p)->extraResponseData_buf,
                                         &(*p)->extraResponseData_len))
                    ;
                else if (yaz_match_xsd_string(ptr, "scanClause", o,
                                          &req->scanClause))
                    ;
                else if (yaz_match_xsd_string(ptr, "pScanClause", o,
                                          &req->scanClause))
                {
                    req->queryType = "pqf";
                }
                else if (yaz_match_xsd_integer(ptr, "responsePosition", o,
                                           &req->responsePosition))
                    ;
                else if (yaz_match_xsd_integer(ptr, "maximumTerms", o,
                                           &req->maximumTerms))
                    ;
                else if (yaz_match_xsd_string(ptr, "stylesheet", o,
                                          &req->stylesheet))
                    ;
                else
                    yaz_match_xsd_string(ptr, "database", o, &req->database);
            }
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "scanResponse"))
        {
            Z_SRW_scanResponse *res;
            xmlNodePtr ptr = method->children;

            (*p)->which = Z_SRW_scan_response;
            res = (*p)->u.scan_response = (Z_SRW_scanResponse *)
                odr_malloc(o, sizeof(*res));
            res->terms = 0;
            res->num_terms = 0;
            res->diagnostics = 0;
            res->num_diagnostics = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_XML_n(ptr, "extraResponseData", o,
                                         &(*p)->extraResponseData_buf,
                                         &(*p)->extraResponseData_len))
                    ;
                else if (yaz_match_xsd_element(ptr, "terms"))
                    yaz_srw_terms(o, ptr, &res->terms,
                                  &res->num_terms, client_data,
                                  ns);
                else if (yaz_match_xsd_element(ptr, "diagnostics"))
                    yaz_srw_diagnostics(o, ptr, &res->diagnostics,
                                        &res->num_diagnostics,
                                        client_data, ns, 0);
            }
        }
        else
        {
            *p = 0;
            return -1;
        }
        neg_version = yaz_negotiate_sru_version((*p)->srw_version);
        if (neg_version)
            (*p)->srw_version = neg_version;
    }
    else if (o->direction == ODR_ENCODE)
    {
        Z_SRW_PDU **p = handler_data;
        xmlNsPtr ns_srw;
        xmlNodePtr ptr = 0;
        int version2 = !(*p)->srw_version ||
            strcmp((*p)->srw_version, "2.") > 0;
        if ((*p)->which == Z_SRW_searchRetrieve_request)
        {
            Z_SRW_searchRetrieveRequest *req = (*p)->u.request;
            const char *queryType = req->queryType;
            if (version2)
                ns = "http://docs.oasis-open.org/ns/search-ws/sruRequest";
            ptr = xmlNewChild(pptr, 0, BAD_CAST "searchRetrieveRequest", 0);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns, BAD_CAST "zs");
            xmlSetNs(ptr, ns_srw);

            if (!version2)
                add_xsd_string(ptr, "version", (*p)->srw_version);
            if (version2)
            {
                add_xsd_string(ptr, "query", req->query);
            }
            else
            {
                if (!queryType || !strcmp(queryType, "cql"))
                    add_xsd_string(ptr, "query", req->query);
                else if (!strcmp(queryType, "xcql"))
                    add_xsd_string(ptr, "xQuery", req->query);
                else if (!strcmp(queryType, "pqf"))
                    add_xsd_string(ptr, "pQuery", req->query);
                queryType = 0;
            }
            add_xsd_integer(ptr, "startRecord", req->startRecord);
            add_xsd_integer(ptr, "maximumRecords", req->maximumRecords);
            if (version2)
                add_xsd_string(ptr, "recordXMLEscaping", req->recordPacking);
            else
                add_xsd_string(ptr, "recordPacking", req->recordPacking);
            add_xsd_string(ptr, "recordSchema", req->recordSchema);
            add_xsd_string(ptr, "recordXPath", req->recordXPath);
            add_xsd_integer(ptr, "resultSetTTL", req->resultSetTTL);
            add_xsd_string(ptr, "stylesheet", req->stylesheet);
            add_xsd_string(ptr, "queryType", queryType);
            switch (req->sort_type)
            {
            case Z_SRW_sort_type_none:
                break;
            case Z_SRW_sort_type_sort:
                add_xsd_string(ptr, "sortKeys", req->sort.sortKeys);
                break;
            case Z_SRW_sort_type_xSort:
                add_xsd_string(ptr, "xSortKeys", req->sort.xSortKeys);
                break;
            }
            /* still unsupported are: renderedBy, httpAccept, responseType */
            add_xsd_string(ptr, "database", req->database);
            if (version2)
                add_xsd_string(ptr, "recordPacking", req->packing);
            {
                const char *limit = 0;
                const char *start = 0;
                const char *sort = 0;
                yaz_sru_facet_request(o, &req->facetList, &limit, &start,
                                      &sort);
                add_xsd_string(ptr, "facetLimit", limit);
                add_xsd_string(ptr, "facetStart", start);
                add_xsd_string(ptr, "facetSort", sort);
            }
        }
        else if ((*p)->which == Z_SRW_searchRetrieve_response)
        {
            Z_SRW_searchRetrieveResponse *res = (*p)->u.response;
            if (version2)
                ns = "http://docs.oasis-open.org/ns/search-ws/sruResponse";
            ptr = xmlNewChild(pptr, 0, BAD_CAST "searchRetrieveResponse", 0);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns, BAD_CAST "zs");
            xmlSetNs(ptr, ns_srw);

            if (!version2)
                add_xsd_string(ptr, "version", (*p)->srw_version);
            add_xsd_integer(ptr, "numberOfRecords", res->numberOfRecords);
            add_xsd_string(ptr, "resultSetId", res->resultSetId);
            if (!version2)
                add_xsd_integer(ptr, "resultSetIdleTime",
                                res->resultSetIdleTime);
            if (res->num_records)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, BAD_CAST "records", 0);
                yaz_srw_records(o, rptr, &res->records, &res->extra_records,
                                &res->num_records,
                                client_data, version2);
            }
            add_xsd_integer(ptr, "nextRecordPosition",
                            res->nextRecordPosition);
            encode_echoed_args(ptr, *p, "echoedSearchRetrieveRequest");
            if (res->num_diagnostics)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, BAD_CAST "diagnostics",
                                              0);
                yaz_srw_diagnostics(o, rptr, &res->diagnostics,
                                    &res->num_diagnostics, client_data, ns,
                                    version2);
            }
            if (version2)
            {
                if (ptr && (*p)->extraResponseData_len)
                    add_XML_n(ptr, "extraResponseData",
                              (*p)->extraResponseData_buf,
                              (*p)->extraResponseData_len, ns_srw);
                add_xsd_integer(ptr, "resultSetTTL", res->resultSetIdleTime);
                add_xsd_string(ptr, "resultCountPrecision",
                               res->resultCountPrecision);
                yaz_sru_facet_response(o, &res->facetList, ptr);
                return 0; /* so we don't make extra response data twice */
            }
        }
        else if ((*p)->which == Z_SRW_explain_request)
        {
            Z_SRW_explainRequest *req = (*p)->u.explain_request;
            if (version2)
                ns = "http://docs.oasis-open.org/ns/search-ws/sruRequest";
            ptr = xmlNewChild(pptr, 0, BAD_CAST "explainRequest", 0);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns, BAD_CAST "zs");
            xmlSetNs(ptr, ns_srw);

            if (!version2)
                add_xsd_string(ptr, "version", (*p)->srw_version);
            if (version2)
            {
                add_xsd_string(ptr, "recordXMLEscaping", req->recordPacking);
                add_xsd_string(ptr, "recordPacking", req->packing);
            }
            else
                add_xsd_string(ptr, "recordPacking", req->recordPacking);
            add_xsd_string(ptr, "stylesheet", req->stylesheet);
            add_xsd_string(ptr, "database", req->database);
        }
        else if ((*p)->which == Z_SRW_explain_response)
        {
            Z_SRW_explainResponse *res = (*p)->u.explain_response;
            if (version2)
                ns = "http://docs.oasis-open.org/ns/search-ws/sruResponse";
            ptr = xmlNewChild(pptr, 0, BAD_CAST "explainResponse", 0);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns, BAD_CAST "zs");
            xmlSetNs(ptr, ns_srw);

            add_xsd_string(ptr, "version", (*p)->srw_version);
            if (1)
            {
                xmlNodePtr ptr1 = xmlNewChild(ptr, 0, BAD_CAST "record", 0);
                yaz_srw_record(o, ptr1, &res->record, &res->extra_record,
                               client_data, version2);
            }
            encode_echoed_args(ptr, *p, "echoedExplainRequest");
            if (res->num_diagnostics)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, BAD_CAST "diagnostics",
                                              0);
                yaz_srw_diagnostics(o, rptr, &res->diagnostics,
                                    &res->num_diagnostics, client_data, ns,
                                    version2);
            }
        }
        else if ((*p)->which == Z_SRW_scan_request)
        {
            Z_SRW_scanRequest *req = (*p)->u.scan_request;
            const char *queryType = req->queryType;
            if (version2)
                ns = "http://docs.oasis-open.org/ns/search-ws/scan";
            ptr = xmlNewChild(pptr, 0, BAD_CAST "scanRequest", 0);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns, BAD_CAST "zs");
            xmlSetNs(ptr, ns_srw);

            if (!version2)
                add_xsd_string(ptr, "version", (*p)->srw_version);

            if (version2)
            {
                if (queryType && strcmp(queryType, "cql"))
                    add_xsd_string(ptr, "queryType", queryType);
                add_xsd_string(ptr, "scanClause", req->scanClause);
            }
            else
            {
                if (!queryType || !strcmp(queryType, "cql"))
                    add_xsd_string(ptr, "scanClause", req->scanClause);
                else if (!strcmp(queryType, "pqf"))
                    add_xsd_string(ptr, "pScanClause", req->scanClause);
            }
            add_xsd_integer(ptr, "responsePosition", req->responsePosition);
            add_xsd_integer(ptr, "maximumTerms", req->maximumTerms);
            add_xsd_string(ptr, "stylesheet", req->stylesheet);
            add_xsd_string(ptr, "database", req->database);
        }
        else if ((*p)->which == Z_SRW_scan_response)
        {
            Z_SRW_scanResponse *res = (*p)->u.scan_response;
            if (version2)
                ns = "http://docs.oasis-open.org/ns/search-ws/scan";
            ptr = xmlNewChild(pptr, 0, BAD_CAST "scanResponse", 0);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns, BAD_CAST "zs");
            xmlSetNs(ptr, ns_srw);

            if (!version2)
                add_xsd_string(ptr, "version", (*p)->srw_version);
            if (res->num_terms)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, BAD_CAST "terms", 0);
                yaz_srw_terms(o, rptr, &res->terms, &res->num_terms,
                              client_data, ns);
            }
            if (res->num_diagnostics)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, BAD_CAST "diagnostics",
                                              0);
                yaz_srw_diagnostics(o, rptr, &res->diagnostics,
                                    &res->num_diagnostics, client_data, ns,
                                    version2);
            }
        }
        else
            return -1;
        if (ptr && (*p)->extraResponseData_len)
            add_XML_n(ptr, "extraResponseData",
                      (*p)->extraResponseData_buf,
                      (*p)->extraResponseData_len, ns_srw);


    }
    return 0;
}

int yaz_ucp_codec(ODR o, void * vptr, Z_SRW_PDU **handler_data,
                  void *client_data, const char *ns_ucp_str)
{
    xmlNodePtr pptr = (xmlNodePtr) vptr;
    const char *ns_srw_str = YAZ_XMLNS_SRU_v1_1;
    if (o->direction == ODR_DECODE)
    {
        Z_SRW_PDU **p = handler_data;
        xmlNodePtr method = pptr->children;

        while (method && method->type == XML_TEXT_NODE)
            method = method->next;

        if (!method)
            return -1;
        if (method->type != XML_ELEMENT_NODE)
            return -1;

        *p = yaz_srw_get_core_v_2_0(o);

        if (!xmlStrcmp(method->name, BAD_CAST "updateRequest"))
        {
            xmlNodePtr ptr = method->children;
            Z_SRW_updateRequest *req;
            char *oper = 0;

            (*p)->which = Z_SRW_update_request;
            req = (*p)->u.update_request = (Z_SRW_updateRequest *)
                odr_malloc(o, sizeof(*req));
            req->database = 0;
            req->operation = 0;
            req->recordId = 0;
            req->recordVersions = 0;
            req->num_recordVersions = 0;
            req->record = 0;
            req->extra_record = 0;
            req->extraRequestData_buf = 0;
            req->extraRequestData_len = 0;
            req->stylesheet = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_string(ptr, "action", o,
                                          &oper)){
                    if (oper)
                    {
                        if (!strcmp(oper, "info:srw/action/1/delete"))
                            req->operation = "delete";
                        else if (!strcmp(oper,"info:srw/action/1/replace" ))
                            req->operation = "replace";
                        else if (!strcmp(oper, "info:srw/action/1/create"))
                            req->operation = "insert";
                    }
                }
                else if (yaz_match_xsd_string(ptr, "recordIdentifier", o,
                                          &req->recordId))
                    ;
                else if (yaz_match_xsd_element(ptr, "recordVersions" ) )
                    yaz_srw_versions( o, ptr, &req->recordVersions,
                                      &req->num_recordVersions, client_data,
                                      ns_ucp_str);
                else if (yaz_match_xsd_element(ptr, "record"))
                {
                    req->record = yaz_srw_get_record(o);
                    yaz_srw_record(o, ptr, req->record, &req->extra_record,
                                   client_data, 0);
                }
                else if (yaz_match_xsd_string(ptr, "stylesheet", o,
                                          &req->stylesheet))
                    ;
                else
                    yaz_match_xsd_string(ptr, "database", o, &req->database);
            }
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "updateResponse"))
        {
            xmlNodePtr ptr = method->children;
            Z_SRW_updateResponse *res;

            (*p)->which = Z_SRW_update_response;
            res = (*p)->u.update_response = (Z_SRW_updateResponse *)
                odr_malloc(o, sizeof(*res));

            res->operationStatus = 0;
            res->recordId = 0;
            res->recordVersions = 0;
            res->num_recordVersions = 0;
            res->diagnostics = 0;
            res->num_diagnostics = 0;
            res->record = 0;
            res->extra_record = 0;
            res->extraResponseData_buf = 0;
            res->extraResponseData_len = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (yaz_match_xsd_string(ptr, "version", o,
                                     &(*p)->srw_version))
                    ;
                else if (yaz_match_xsd_string(ptr, "operationStatus", o,
                                          &res->operationStatus ))
                    ;
                else if (yaz_match_xsd_string(ptr, "recordIdentifier", o,
                                          &res->recordId))
                    ;
                else if (yaz_match_xsd_element(ptr, "recordVersions" ))
                    yaz_srw_versions(o, ptr, &res->recordVersions,
                                     &res->num_recordVersions,
                                     client_data, ns_ucp_str);
                else if (yaz_match_xsd_element(ptr, "record"))
                {
                    res->record = yaz_srw_get_record(o);
                    yaz_srw_record(o, ptr, res->record, &res->extra_record,
                                   client_data, 0);
                }
                else if (yaz_match_xsd_element(ptr, "diagnostics"))
                    yaz_srw_diagnostics(o, ptr, &res->diagnostics,
                                        &res->num_diagnostics,
                                        client_data, ns_ucp_str, 0);
            }
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "explainUpdateRequest"))
        {
        }
        else if (!xmlStrcmp(method->name, BAD_CAST "explainUpdateResponse"))
        {
        }
        else
        {
            *p = 0;
            return -1;
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        Z_SRW_PDU **p = handler_data;
        xmlNsPtr ns_ucp, ns_srw;

        if ((*p)->which == Z_SRW_update_request)
        {
            Z_SRW_updateRequest *req = (*p)->u.update_request;
            xmlNodePtr ptr = xmlNewChild(pptr, 0, BAD_CAST "updateRequest", 0);
	    ns_ucp = xmlNewNs(ptr, BAD_CAST ns_ucp_str, BAD_CAST "zu");
	    xmlSetNs(ptr, ns_ucp);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns_srw_str, BAD_CAST "zs");

	    add_xsd_string_ns(ptr, "version", (*p)->srw_version, ns_srw);
	    add_xsd_string(ptr, "action", req->operation);
            add_xsd_string(ptr, "recordIdentifier", req->recordId );
	    if (req->recordVersions)
                yaz_srw_versions( o, ptr, &req->recordVersions,
                                  &req->num_recordVersions,
                                  client_data, ns_ucp_str);
	    if (req->record && req->record->recordData_len)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, BAD_CAST "record", 0);
                xmlSetNs(rptr, ns_srw);
                yaz_srw_record(o, rptr, req->record, &req->extra_record,
                               client_data, 0);
	    }
	    if (req->extraRequestData_len)
            {
                add_XML_n(ptr, "extraRequestData",
                          req->extraRequestData_buf,
                          req->extraRequestData_len, ns_srw);
            }
	    add_xsd_string(ptr, "stylesheet", req->stylesheet);
            add_xsd_string(ptr, "database", req->database);
        }
        else if ((*p)->which == Z_SRW_update_response)
        {
            Z_SRW_updateResponse *res = (*p)->u.update_response;
            xmlNodePtr ptr = xmlNewChild(pptr, 0, (xmlChar *)
                                         "updateResponse", 0);
	    ns_ucp = xmlNewNs(ptr, BAD_CAST ns_ucp_str, BAD_CAST "zu");
	    xmlSetNs(ptr, ns_ucp);
            ns_srw = xmlNewNs(ptr, BAD_CAST ns_srw_str, BAD_CAST "zs");

	    add_xsd_string_ns(ptr, "version", (*p)->srw_version, ns_srw);
            add_xsd_string(ptr, "operationStatus", res->operationStatus );
            add_xsd_string(ptr, "recordIdentifier", res->recordId );
	    if (res->recordVersions)
                yaz_srw_versions(o, ptr, &res->recordVersions,
                                 &res->num_recordVersions,
                                 client_data, ns_ucp_str);
	    if (res->record && res->record->recordData_len)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, BAD_CAST "record", 0);
                xmlSetNs(rptr, ns_srw);
                yaz_srw_record(o, rptr, res->record, &res->extra_record,
                               client_data, 0);
	    }
	    if (res->num_diagnostics)
	    {
                xmlNsPtr ns_diag =
                    xmlNewNs(pptr, BAD_CAST YAZ_XMLNS_DIAG_v1_1,
                             BAD_CAST "diag" );

		xmlNodePtr rptr = xmlNewChild(ptr, ns_diag, BAD_CAST "diagnostics", 0);
		yaz_srw_diagnostics(o, rptr, &res->diagnostics,
                                    &res->num_diagnostics, client_data,
                                    ns_ucp_str, 0);
            }
	    if (res->extraResponseData_len)
                add_XML_n(ptr, "extraResponseData",
                          res->extraResponseData_buf,
                          res->extraResponseData_len, ns_srw);
        }
        else
            return -1;
    }
    return 0;
}

#endif


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

