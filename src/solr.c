/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file srwutil.c
 * \brief Implements SRW/SRU utilities.
 */

#include <stdlib.h>
#include <assert.h>
#include <yaz/srw.h>
#include <yaz/matchstr.h>
#include <yaz/yaz-iconv.h>
#include <yaz/log.h>
#include <yaz/facet.h>

#include "sru-p.h"

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>


const char *xml_node_attribute_value_get(xmlNodePtr ptr, const char *node_name, const char *attribute_name) {

    struct _xmlAttr *attr;
    // check if the node name matches
    if (strcmp((const char*) ptr->name, node_name))
        return 0;
    // check if the attribute name and return the value
    for (attr = ptr->properties; attr; attr = attr->next)
        if (attr->children && attr->children->type == XML_TEXT_NODE) {
            if (!strcmp((const char *) attr->name, attribute_name))
                return (const char *) attr->children->content;
        }
    return 0;
}


static int match_xml_node_attribute(xmlNodePtr ptr, const char *node_name, const char *attribute_name, const char *value)
{
    const char *attribute_value;
    // check if the node name matches
    if (strcmp((const char*) ptr->name, node_name))
        return 0;
    attribute_value = xml_node_attribute_value_get(ptr, node_name, attribute_name);
    if (attribute_value && !strcmp(attribute_value, value))
        return 1;
    return 0;
}

static void yaz_solr_decode_result_docs(ODR o, xmlNodePtr ptr, Odr_int start, Z_SRW_searchRetrieveResponse *sr) {
    xmlNodePtr node;
    int offset = 0;
    int i = 0;

    sr->num_records = 0;
    for (node = ptr->children; node; node = node->next)
        if (node->type == XML_ELEMENT_NODE)
            sr->num_records++;

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
            record->recordData_buf = odr_malloc(o, buf->use + 1);
            memcpy(record->recordData_buf, buf->content, buf->use);
            record->recordData_buf[buf->use] = '\0';
            // TODO Solve the real problem in zoom-sru, that doesnt work with 0-based indexes.
            // Work-around: Making the recordPosition 1-based.
            record->recordPosition = odr_intdup(o, start + offset + 1);

            xmlBufferFree(buf);

            offset++;
            i++;
        }
    }
}

static void yaz_solr_decode_result(ODR o, xmlNodePtr ptr, Z_SRW_searchRetrieveResponse *sr) {
    Odr_int start = 0;
    struct _xmlAttr *attr;
    for (attr = ptr->properties; attr; attr = attr->next)
        if (attr->children && attr->children->type == XML_TEXT_NODE) {
            if (!strcmp((const char *) attr->name, "numFound")) {
                sr->numberOfRecords = odr_intdup(o, odr_atoi(
                        (const char *) attr->children->content));
            } else if (!strcmp((const char *) attr->name, "start")) {
                start = odr_atoi((const char *) attr->children->content);
            }
        }
    yaz_solr_decode_result_docs(o, ptr, start, sr);
}

static Z_AttributeList *yaz_solr_use_atttribute_create(ODR o, const char *name) {
    // TODO IMPLEMENT
    return 0;
}


static const char *get_facet_term_count(xmlNodePtr node, int *freq) {
    // TODO implement
    return 0;
}

Z_FacetField *yaz_solr_decode_facet_field(ODR o, xmlNodePtr ptr, Z_SRW_searchRetrieveResponse *sr)
{
    // USE attribute
    const char* name = xml_node_attribute_value_get(ptr, "lst", "name");
    Z_AttributeList *list = yaz_solr_use_atttribute_create(o, name);
    Z_FacetField *facet_field;
    int num_terms = 0;
    int index = 0;
    xmlNodePtr node;
    for (node = ptr->children; node; node = node->next) {
        num_terms++;
    }
    facet_field = facet_field_create(o, list, num_terms);
    index = 0;
    for (node = ptr->children; node; node = node->next) {
        int count = 0;
        const char *term = get_facet_term_count(node, &count);
        facet_field_term_set(o, facet_field, facet_term_create(o, term_create(o, term), count), index);
        index++;
    }
    return facet_field;
}

static void yaz_solr_decode_facet_counts(ODR o, xmlNodePtr root, Z_SRW_searchRetrieveResponse *sr) {
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
                facet_list_field_set(o, facet_list, yaz_solr_decode_facet_field(o, node, sr), num_facets);
                num_facets++;
            }
            sr->facet_list = facet_list;
            break;
        }
    }
}

static void yaz_solr_decode_facets(ODR o, xmlNodePtr ptr, Z_SRW_searchRetrieveResponse *sr) {
    if (match_xml_node_attribute(ptr, "lst", "name", "facet_counts"))
        yaz_solr_decode_facet_counts(o, ptr->children, sr);
}
#endif

int yaz_solr_decode_response(ODR o, Z_HTTP_Response *hres, Z_SRW_PDU **pdup)
{
#if YAZ_HAVE_XML2
    const char *content_buf = hres->content_buf;
    int content_len = hres->content_len;
    xmlDocPtr doc = xmlParseMemory(content_buf, content_len);
    int ret = 0;
    xmlNodePtr ptr = 0;
    Z_SRW_PDU *pdu = yaz_srw_get(o, Z_SRW_searchRetrieve_response);
    Z_SRW_searchRetrieveResponse *sr = pdu->u.response;

    if (!doc)
    {
        ret = -1;
    }
    if (doc)
    {
        xmlNodePtr root = xmlDocGetRootElement(doc);
        if (!root)
        {
            ret = -1;
        }
        else if (strcmp((const char *) root->name, "response"))
        {
            ret = -1;
        }
        else
        {
            /** look for result node */
            for (ptr = root->children; ptr; ptr = ptr->next)
            {
                if (ptr->type == XML_ELEMENT_NODE &&
                    !strcmp((const char *) ptr->name, "result"))
                        yaz_solr_decode_result(o, ptr, sr);
                if (match_xml_node_attribute(ptr, "lst", "name", "facet_counts"))
                        yaz_solr_decode_facet_counts(o, ptr, sr);
            }
            if (!ptr)
            {
                ret = -1;
            }
        }
    }
    if (doc)
        xmlFreeDoc(doc);
    if (ret == 0)
        *pdup = pdu;
    return ret;
#else
    return -1;
#endif
}

void solr_encode_facet_field(ODR encode, Z_FacetField *facet_field) {

}

int yaz_solr_encode_request(Z_HTTP_Request *hreq, Z_SRW_PDU *srw_pdu,
                            ODR encode, const char *charset)
{
    const char *solr_op = 0;
    //TODO Change. not a nice hard coded, unchecked limit.
    int max = 100;
    char *name[100], *value[100];
    char *uri_args;
    char *path;
    int i = 0;

    z_HTTP_header_add_basic_auth(encode, &hreq->headers, 
                                 srw_pdu->username, srw_pdu->password);

    switch (srw_pdu->which)
    {
    case Z_SRW_searchRetrieve_request: {
        Z_SRW_searchRetrieveRequest *request = srw_pdu->u.request;
        solr_op = "select";
        switch(srw_pdu->u.request->query_type)
        {
        case Z_SRW_query_type_pqf:
            yaz_add_name_value_str(encode, name, value, &i,
                                   "q", request->query.pqf);
            break;
        case Z_SRW_query_type_cql:
            yaz_add_name_value_str(encode, name, value, &i,
                                   "q", request->query.cql);
            break;
        default:
            return -1;
        }
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

        if (request->facet_list) {
            int index;
            Z_FacetList *facet_list = request->facet_list;
            yaz_add_name_value_str(encode, name, value, &i, "facet", "true");
            for (index = 0; index < facet_list->num; index++) {
                //TODO impl
                //solr_encode_facet_field(encode, name, value, &i, facet_list->elements[index]);
            }
        }
        break;
    }
    default:
        return -1;
    }
    name[i] = 0;
    yaz_array_to_uri(&uri_args, encode, name, value);
    
    hreq->method = "GET";
    
    path = (char *)
        odr_malloc(encode, strlen(hreq->path) +
                   strlen(uri_args) + strlen(solr_op) + 4);

    sprintf(path, "%s/%s?%s", hreq->path, solr_op, uri_args);
    hreq->path = path;

    z_HTTP_header_add_content_type(encode, &hreq->headers,
                                   "text/xml", charset);
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

