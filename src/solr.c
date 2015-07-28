/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file solr.c
 * \brief Implements Solr decoding/encoding
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <yaz/srw.h>
#include <yaz/matchstr.h>
#include <yaz/yaz-iconv.h>
#include <yaz/log.h>
#include <yaz/facet.h>
#include <yaz/wrbuf.h>
#include <yaz/proto.h>

#include "sru-p.h"

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>

static void extract_text_node(xmlNodePtr node, WRBUF wrbuf)
{
    xmlNodePtr child;
    for (child = node->children; child ; child = child->next)
    {
        if (child->type == XML_TEXT_NODE)
            wrbuf_puts(wrbuf, (const char *) child->content);
    }
}

static int match_xml_node_attribute(
    xmlNodePtr ptr,
    const char *node_name, const char *attribute_name, const char *value)
{
    const char *attribute_value;
    // check if the node name matches
    if (strcmp((const char*) ptr->name, node_name))
        return 0;
    if (attribute_name)
    {
        attribute_value = yaz_element_attribute_value_get(ptr, node_name,
                                                          attribute_name);
        if (attribute_value && !strcmp(attribute_value, value))
            return 1;
    }
    else /* No attribute to check */
        return 1;
    return 0;
}

static void yaz_solr_decode_result_docs(ODR o, xmlNodePtr ptr,
                                        Odr_int start,
                                        Z_SRW_searchRetrieveResponse *sr)
{
    xmlNodePtr node;
    int offset = 0;
    int i = 0;

    sr->num_records = 0;
    for (node = ptr->children; node; node = node->next)
        if (node->type == XML_ELEMENT_NODE)
            sr->num_records++;

    if (sr->num_records)
        sr->records = odr_malloc(o, sizeof(*sr->records) * sr->num_records);

    for (node = ptr->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            Z_SRW_record *record = sr->records + i;
            xmlBufferPtr buf = xmlBufferCreate();
            xmlNode *tmp = xmlCopyNode(node, 1);

            xmlNodeDump(buf, tmp->doc, tmp, 0, 0);

            xmlFreeNode(tmp);

            record->recordSchema = 0;
            record->recordPacking = Z_SRW_recordPacking_XML;
            record->recordData_len = buf->use;
            record->recordData_buf =
                odr_strdupn(o, (const char *) buf->content, buf->use);
            record->recordPosition = odr_intdup(o, start + offset + 1);

            xmlBufferFree(buf);

            offset++;
            i++;
        }
    }
}

static int yaz_solr_decode_result(ODR o, xmlNodePtr ptr,
                                  Z_SRW_searchRetrieveResponse *sr)
{
    Odr_int start = 0;
    struct _xmlAttr *attr;
    for (attr = ptr->properties; attr; attr = attr->next)
        if (attr->children && attr->children->type == XML_TEXT_NODE)
        {
            if (!strcmp((const char *) attr->name, "numFound"))
            {
                sr->numberOfRecords = odr_intdup(o, odr_atoi(
                        (const char *) attr->children->content));
            }
            else if (!strcmp((const char *) attr->name, "start"))
            {
                start = odr_atoi((const char *) attr->children->content);
            }
        }
    if (sr->numberOfRecords && *sr->numberOfRecords > 0)
        yaz_solr_decode_result_docs(o, ptr, start, sr);
    if (sr->numberOfRecords)
        return 0;
    return -1;
}

static const char *get_facet_term_count(xmlNodePtr node, Odr_int *freq)
{
    const char *term = yaz_element_attribute_value_get(node, "int", "name");
    xmlNodePtr child;
    WRBUF wrbuf = wrbuf_alloc();
    if (!term)
        return term;

    for (child = node->children; child ; child = child->next)
    {
        if (child->type == XML_TEXT_NODE)
            wrbuf_puts(wrbuf, (const char *) child->content);
    }
    *freq = odr_atoi(wrbuf_cstr(wrbuf));
    wrbuf_destroy(wrbuf);
    return term;
}

Z_FacetField *yaz_solr_decode_facet_field(ODR o, xmlNodePtr ptr,
                                          Z_SRW_searchRetrieveResponse *sr)

{
    Z_AttributeList *list;
    Z_FacetField *facet_field;
    int num_terms = 0;
    int index = 0;
    xmlNodePtr node;
    // USE attribute
    const char* name = yaz_element_attribute_value_get(ptr, "lst", "name");
    list = zget_AttributeList_use_string(o, name);
    for (node = ptr->children; node; node = node->next)
        num_terms++;
    facet_field = facet_field_create(o, list, num_terms);
    index = 0;
    for (node = ptr->children; node; node = node->next)
    {
        Odr_int count = 0;
        const char *term = get_facet_term_count(node, &count);
        facet_field_term_set(o, facet_field,
                             facet_term_create_cstr(o, term, count), index);
        index++;
    }
    return facet_field;
}

static int yaz_solr_decode_facet_counts(ODR o, xmlNodePtr root,
                                        Z_SRW_searchRetrieveResponse *sr)
{
    xmlNodePtr ptr;
    for (ptr = root->children; ptr; ptr = ptr->next)
    {
        if (match_xml_node_attribute(ptr, "lst", "name", "facet_fields"))
        {
            xmlNodePtr node;
            Z_FacetList *facet_list;
            int num_facets = 0;
            for (node = ptr->children; node; node= node->next)
            {
                num_facets++;
            }
            facet_list = facet_list_create(o, num_facets);
            num_facets = 0;
            for (node = ptr->children; node; node= node->next)
            {
                facet_list_field_set(o, facet_list,
                                     yaz_solr_decode_facet_field(o, node, sr),
                                     num_facets);
                num_facets++;
            }
            sr->facetList = facet_list;
            break;
        }
    }
    return 0;
}

static void yaz_solr_decode_suggestion_values(xmlNodePtr listPptr, WRBUF wrbuf)
{
    xmlNodePtr node;
    for (node = listPptr; node; node= node->next)
        if (!strcmp((char*) node->name, "lst"))
        {
            xmlNodePtr child;
            for (child = node->children; child; child= child->next)
            {
                if (match_xml_node_attribute(child, "str", "name", "word"))
                {
                    wrbuf_puts(wrbuf, "<suggestion>");
                    extract_text_node(child, wrbuf);
                    wrbuf_puts(wrbuf, "</suggestion>\n");
                }
            }
        }
}

static void yaz_solr_decode_suggestion_lst(xmlNodePtr lstPtr, WRBUF wrbuf)
{
    xmlNodePtr node;
    for (node = lstPtr; node; node= node->next)
        if (match_xml_node_attribute(node, "arr", "name", "suggestion"))
            yaz_solr_decode_suggestion_values(node->children, wrbuf);
}

static void yaz_solr_decode_misspelled(xmlNodePtr lstPtr, WRBUF wrbuf)
{
    xmlNodePtr node;
    for (node = lstPtr; node; node= node->next)
    {
        if (!strcmp((const char*) node->name, "lst"))
        {
            const char *misspelled =
                yaz_element_attribute_value_get(node, "lst", "name");
            if (misspelled)
            {
                wrbuf_printf(wrbuf, "<misspelled term=\"%s\">\n", misspelled);
                yaz_solr_decode_suggestion_lst(node->children, wrbuf);
                wrbuf_puts(wrbuf, "</misspelled>\n");
            }
        }
    }
}

static int yaz_solr_decode_spellcheck(ODR o, xmlNodePtr spellcheckPtr, Z_SRW_searchRetrieveResponse *sr)
{
    xmlNodePtr ptr;
    WRBUF wrbuf = wrbuf_alloc();
    wrbuf_puts(wrbuf, "");
    for (ptr = spellcheckPtr->children; ptr; ptr = ptr->next)
    {
        if (match_xml_node_attribute(ptr, "lst", "name", "suggestions"))
        {
            yaz_solr_decode_misspelled(ptr->children, wrbuf);
        }
    }
    sr->suggestions = odr_strdup(o, wrbuf_cstr(wrbuf));
    return 0;
}

static int yaz_solr_decode_scan_result(ODR o, xmlNodePtr ptr,
                                       Z_SRW_scanResponse *scr)
{
    xmlNodePtr node;
    char *pos;
    int i = 0;

    /* find the actual list */
    for (node = ptr->children; node; node = node->next)
        if (node->type == XML_ELEMENT_NODE)
        {
            ptr = node;
            break;
        }

    scr->num_terms = 0;
    for (node = ptr->children; node; node = node->next)
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((const char *) node->name, "int"))
            scr->num_terms++;

    if (scr->num_terms)
        scr->terms = odr_malloc(o, sizeof(*scr->terms) * scr->num_terms);

    for (node = ptr->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((const char *) node->name, "int"))
        {
            Z_SRW_scanTerm *term = scr->terms + i;

            Odr_int count = 0;
            const char *val = get_facet_term_count(node, &count);

            term->numberOfRecords = odr_intdup(o, count);

            /* if val contains a ^ then it is probably term<^>display term so separate them. This is due to
             * SOLR not being able to encode them into 2 separate attributes.
             */
            pos = strchr(val, '^');
            if (pos != NULL)
            {
            	term->displayTerm = odr_strdup(o, pos + 1);
            	*pos = '\0';
            	term->value = odr_strdup(o, val);
            	*pos = '^';
            }
            else
            {
            	term->value = odr_strdup(o, val);
            	term->displayTerm = NULL;
            }
            term->whereInList = NULL;
            i++;
        }
    }

    if (scr->num_terms)
        return 0;
    return -1;
}
#endif

int yaz_solr_decode_response(ODR o, Z_HTTP_Response *hres, Z_SRW_PDU **pdup)
{
    int ret = -1;
    Z_SRW_PDU *pdu = 0;
#if YAZ_HAVE_XML2
    const char *content_buf = hres->content_buf;
    int content_len = hres->content_len;
    xmlDocPtr doc = xmlParseMemory(content_buf, content_len);

    if (doc)
    {
        Z_SRW_searchRetrieveResponse *sr = NULL;
        Z_SRW_scanResponse *scr = NULL;
        xmlNodePtr ptr;
        xmlNodePtr root = xmlDocGetRootElement(doc);
        if (root && !strcmp((const char *) root->name, "response"))
        {
            for (ptr = root->children; ptr; ptr = ptr->next)
            {
                if (ptr->type == XML_ELEMENT_NODE &&
                    !strcmp((const char *) ptr->name, "result"))
                {
                    pdu = yaz_srw_get(o, Z_SRW_searchRetrieve_response);
                    sr = pdu->u.response;
                    ret = yaz_solr_decode_result(o, ptr, sr);

                }
                if (ptr->type == XML_ELEMENT_NODE &&
                    match_xml_node_attribute(ptr, "lst", "name", "terms"))
                {
                    pdu = yaz_srw_get(o, Z_SRW_scan_response);
                    scr = pdu->u.scan_response;
                    ret = yaz_solr_decode_scan_result(o, ptr, scr);
                }
                /* The check on hits is a work-around to avoid garbled
                   facets on zero results from the SOLR server.
                   The work-around works because the results is before
                   the facets in the xml.
                */
                if (sr && *sr->numberOfRecords > 0 &&
                    match_xml_node_attribute(ptr, "lst", "name",
                                             "facet_counts"))
                    ret = yaz_solr_decode_facet_counts(o, ptr, sr);
                if (sr && *sr->numberOfRecords == 0 &&
                    match_xml_node_attribute(ptr, "lst", "name",
                                             "spellcheck"))
                    ret = yaz_solr_decode_spellcheck(o, ptr, sr);
            }
        }
        xmlFreeDoc(doc);
    }
#endif
    *pdup = pdu;
    return ret;
}

static int yaz_solr_encode_facet_field(
    ODR encode, char **name, char **value, int *i,
    Z_FacetField *facet_field)
{
    Z_AttributeList *attribute_list = facet_field->attributes;
    struct yaz_facet_attr attr_values;
    yaz_facet_attr_init(&attr_values);
    yaz_facet_attr_get_z_attributes(attribute_list, &attr_values);

    if (attr_values.errcode)
        return -1;
    if (attr_values.useattr)
    {
        WRBUF wrbuf = wrbuf_alloc();
        yaz_add_name_value_str(encode, name, value, i,
                               "facet.field",
                               odr_strdup(encode, attr_values.useattr));

        if (attr_values.limit > 0)
        {
            Odr_int v = attr_values.limit;
            wrbuf_rewind(wrbuf);
            wrbuf_printf(wrbuf, "f.%s.facet.limit", attr_values.useattr);
            yaz_add_name_value_int(encode, name, value, i,
                                   odr_strdup(encode, wrbuf_cstr(wrbuf)),
                                   &v);
        }
        if (attr_values.start > 1)
        {
            Odr_int v = attr_values.start - 1;
            wrbuf_rewind(wrbuf);
            wrbuf_printf(wrbuf, "f.%s.facet.offset", attr_values.useattr);
            yaz_add_name_value_int(encode, name, value, i,
                                   odr_strdup(encode, wrbuf_cstr(wrbuf)),
                                   &v);
        }
        if (attr_values.sortorder == 1)
        {
            wrbuf_rewind(wrbuf);
            wrbuf_printf(wrbuf, "f.%s.facet.sort", attr_values.useattr);
            yaz_add_name_value_str(encode, name, value, i,
                                   odr_strdup(encode, wrbuf_cstr(wrbuf)),
                                   "index");
        }
        wrbuf_destroy(wrbuf);
    }
    else
    {
        if (attr_values.limit > 0)
        {
            Odr_int v = attr_values.limit;
            yaz_add_name_value_int(encode, name, value, i, "facet.limit", &v);
        }
        if (attr_values.start > 1)
        {
            Odr_int v = attr_values.start - 1;
            yaz_add_name_value_int(encode, name, value, i, "facet.offset", &v);
        }
        if (attr_values.sortorder == 1)
        {
            yaz_add_name_value_str(encode, name, value, i, "facet.sort",
                                   "index");
        }
    }
    return 0;
}

static int yaz_solr_encode_facet_list(
    ODR encode, char **name, char **value,
    int *i, Z_FacetList *facet_list)
{
    int index;
    for (index = 0; index < facet_list->num; index++)
    {
        int r = yaz_solr_encode_facet_field(encode, name, value, i,
                                            facet_list->elements[index]);
        if (r)
            return -1;

    }
    return 0;
}

int yaz_solr_encode_request(Z_HTTP_Request *hreq, Z_SRW_PDU *srw_pdu,
                            ODR encode, const char *charset)
{
    const char *solr_op = 0;
    char **name, **value;
    char *uri_args;
    char *path;
    char *q;
    char *cp;
    const char *path_args = 0;
    int i = 0;
    int defType_set = 0;
    int no_parms = 20; /* safe upper limit of args without extra_args */
    Z_SRW_extra_arg *ea;

    if (srw_pdu->which == Z_SRW_searchRetrieve_request)
    {   /* to make room for facets in yaz_solr_encode_facet_list later */
        Z_SRW_searchRetrieveRequest *request = srw_pdu->u.request;
        if (request->facetList)
            no_parms += request->facetList->num;
    }
    for (ea = srw_pdu->extra_args; ea; ea = ea->next)
        no_parms++;
    name = (char **) odr_malloc(encode, sizeof(*name) * no_parms);
    value = (char **) odr_malloc(encode, sizeof(*value) * no_parms);

    for (ea = srw_pdu->extra_args; ea; ea = ea->next)
    {
        name[i] = ea->name;
        if (!strcmp(ea->name, "defType"))
            defType_set = 1;
        value[i] = ea->value;
        i++;
    }

    z_HTTP_header_add_basic_auth(encode, &hreq->headers,
                                 srw_pdu->username, srw_pdu->password);
    if (srw_pdu->which == Z_SRW_searchRetrieve_request)
    {
        Z_SRW_searchRetrieveRequest *request = srw_pdu->u.request;
        solr_op = "select";
        if (!srw_pdu->u.request->query)
            return -1;
        if (!defType_set)
            yaz_add_name_value_str(encode, name, value, &i, "defType",
                                   "lucene");
        yaz_add_name_value_str(encode, name, value, &i, "q", request->query);
        if (srw_pdu->u.request->startRecord)
        {
            Odr_int start = *request->startRecord - 1;
            yaz_add_name_value_int(encode, name, value, &i,
                                   "start", &start);
        }
        yaz_add_name_value_int(encode, name, value, &i,
                               "rows", request->maximumRecords);
        yaz_add_name_value_str(encode, name, value, &i,
                               "fl", request->recordSchema);

        switch(srw_pdu->u.request->sort_type)
        {
        case Z_SRW_sort_type_none:
            break;
        case Z_SRW_sort_type_sort:
            yaz_add_name_value_str(encode, name, value, &i, "sort",
                                   srw_pdu->u.request->sort.sortKeys);
            break;
        }
        if (request->facetList)
        {
            Z_FacetList *facet_list = request->facetList;
            yaz_add_name_value_str(encode, name, value, &i, "facet", "true");
            yaz_add_name_value_str(encode, name, value, &i, "facet.mincount", "1");
            if (yaz_solr_encode_facet_list(encode, name, value, &i, facet_list))
                return -1;
        }
    }
    else if (srw_pdu->which == Z_SRW_scan_request)
    {
        Z_SRW_scanRequest *request = srw_pdu->u.scan_request;
        solr_op = "terms";
        if (!srw_pdu->u.scan_request->scanClause)
            return -1;
        if (!strcmp(srw_pdu->u.scan_request->queryType, "pqf"))
        {
            yaz_add_name_value_str(encode, name, value, &i,
                                   "terms.fl", request->scanClause);
            yaz_add_name_value_str(encode, name, value, &i,
                                   "terms.lower", request->scanClause);
        }
        else if (!strcmp(srw_pdu->u.scan_request->queryType, "cql"))
        {
            q = request->scanClause;
            cp = strchr(q, ':');
            if (cp != NULL)
            {
                yaz_add_name_value_str(encode, name, value, &i,
                                       "terms.lower", odr_strdup(encode, cp + 1));
                *cp = '\0';
                yaz_add_name_value_str(encode, name, value, &i,
                                       "terms.fl", odr_strdup(encode, q));
                *cp = ':';
            }
            else
                yaz_add_name_value_str(encode, name, value, &i,
                                       "terms.lower", odr_strdup(encode, q));
        }
        else
            return -1;
        yaz_add_name_value_str(encode, name, value, &i,
                               "terms.sort", "index");
        yaz_add_name_value_int(encode, name, value, &i,
                               "terms.limit", request->maximumTerms);
    }
    else
        return -1;

    name[i++] = 0;

    yaz_array_to_uri(&uri_args, encode, name, value);

    hreq->method = "GET";

    path = (char *)
        odr_malloc(encode, strlen(hreq->path) +
                   strlen(uri_args) + strlen(solr_op) + 5);

    cp = strchr(hreq->path, '#');
    if (cp)
        *cp = '\0';
    cp = strchr(hreq->path, '?');
    if (cp)
    {
        *cp = '\0'; /* args in path */
        path_args = cp + 1;
    }
    strcpy(path, hreq->path);
    cp = strrchr(path, '/');
    if (cp)
    {
        if (!strcmp(cp, "/select") || !strcmp(cp, "/"))
            *cp = '\0';
    }
    strcat(path, "/");
    strcat(path, solr_op);
    strcat(path, "?");
    if (path_args)
    {
        strcat(path, path_args);
        strcat(path, "&");
    }
    strcat(path, uri_args);
    hreq->path = path;
    return 0;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

