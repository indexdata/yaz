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
#include <yaz/wrbuf.h>

#include "sru-p.h"

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>

#define SOLR_MAX_PARAMETERS  100

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

static int  yaz_solr_decode_result(ODR o, xmlNodePtr ptr, Z_SRW_searchRetrieveResponse *sr) {
    Odr_int start = 0;
    struct _xmlAttr *attr;
    for (attr = ptr->properties; attr; attr = attr->next)
        if (attr->children && attr->children->type == XML_TEXT_NODE) {
            if (!strcmp((const char *) attr->name, "numFound")) {
                sr->numberOfRecords = odr_intdup(o, odr_atoi(
                        (const char *) attr->children->content));
            } 
            else if (!strcmp((const char *) attr->name, "start")) {
                start = odr_atoi((const char *) attr->children->content);
            }
        }
    if (sr->numberOfRecords && *sr->numberOfRecords > 0)
        yaz_solr_decode_result_docs(o, ptr, start, sr);
    if (sr->numberOfRecords)
        return 0;
    return -1;
}

static Z_AttributeList *yaz_solr_use_atttribute_create(ODR o, const char *name) {
    Z_AttributeList *attributes= (Z_AttributeList *) odr_malloc(o, sizeof(*attributes));
    Z_AttributeElement ** elements;
    attributes->num_attributes = 1;
    /* TODO check on name instead
    if (!attributes->num_attributes) {
        attributes->attributes = (Z_AttributeElement**)odr_nullval();
        return attributes;
    }
    */
    elements = (Z_AttributeElement**) odr_malloc (o, attributes->num_attributes * sizeof(*elements));
    elements[0] = (Z_AttributeElement*)odr_malloc(o,sizeof(**elements));
    elements[0]->attributeType = odr_malloc(o, sizeof(*elements[0]->attributeType));
    *elements[0]->attributeType = 1;
    elements[0]->attributeSet = odr_nullval();
    elements[0]->which = Z_AttributeValue_complex;
    elements[0]->value.complex = (Z_ComplexAttribute *) odr_malloc(o, sizeof(Z_ComplexAttribute));
    elements[0]->value.complex->num_list = 1;
    elements[0]->value.complex->list = (Z_StringOrNumeric **) odr_malloc(o, 1 * sizeof(Z_StringOrNumeric *));
    elements[0]->value.complex->list[0] = (Z_StringOrNumeric *) odr_malloc(o, sizeof(Z_StringOrNumeric));
    elements[0]->value.complex->list[0]->which = Z_StringOrNumeric_string;
    elements[0]->value.complex->list[0]->u.string = (Z_InternationalString *) odr_strdup(o, name);
    elements[0]->value.complex->semanticAction = 0;
    elements[0]->value.complex->num_semanticAction = 0;
    attributes->attributes = elements;
    return attributes;
}


static const char *get_facet_term_count(xmlNodePtr node, int *freq) {

    const char *term = xml_node_attribute_value_get(node, "int", "name");
    xmlNodePtr child;
    WRBUF wrbuf = wrbuf_alloc();
    if (!term)
        return term;

    for (child = node->children; child ; child = child->next) {
        if (child->type == XML_TEXT_NODE)
        wrbuf_puts(wrbuf, (const char *) child->content);
    }
    *freq = atoi(wrbuf_cstr(wrbuf));
    wrbuf_destroy(wrbuf);
    return term;
}

Z_FacetField *yaz_solr_decode_facet_field(ODR o, xmlNodePtr ptr, Z_SRW_searchRetrieveResponse *sr)

{
    Z_AttributeList *list;
    Z_FacetField *facet_field;
    int num_terms = 0;
    int index = 0;
    xmlNodePtr node;
    // USE attribute
    const char* name = xml_node_attribute_value_get(ptr, "lst", "name");
    char *pos = strstr(name, "_exact");
    /* HACK */
    if (pos) {
        pos[0] = 0;
    }
    list = yaz_solr_use_atttribute_create(o, name);
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

static int yaz_solr_decode_facet_counts(ODR o, xmlNodePtr root, Z_SRW_searchRetrieveResponse *sr) {
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
            sr->facetList = facet_list;
            break;
        }
    }
    return 0;
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
            /** look for result (required) and facets node (optional) */
            int rc_result = -1;
            int rc_facets = 0;
            for (ptr = root->children; ptr; ptr = ptr->next)
            {
                if (ptr->type == XML_ELEMENT_NODE &&
                    !strcmp((const char *) ptr->name, "result"))
                        rc_result = yaz_solr_decode_result(o, ptr, sr);
                /* TODO The check on hits is a work-around to avoid garbled facets on zero results from the SOLR server.
                 * The work-around works because the results is before the facets in the xml. */
                if (rc_result == 0 &&  *sr->numberOfRecords > 0 && match_xml_node_attribute(ptr, "lst", "name", "facet_counts"))
                    rc_facets =  yaz_solr_decode_facet_counts(o, ptr, sr);
            }
            ret = rc_result + rc_facets;
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

static void yaz_solr_encode_facet_field(ODR encode, char **name, char **value, int *i, Z_FacetField *facet_field, int *limit) {
      Z_AttributeList *attribute_list = facet_field->attributes;
      struct yaz_facet_attr attr_values;
      yaz_facet_attr_init(&attr_values);
      yaz_facet_attr_get_z_attributes(attribute_list, &attr_values);
      // TODO do we want to support server decided
      if (!attr_values.errcode && attr_values.useattr) {
          WRBUF wrbuf = wrbuf_alloc();
          wrbuf_puts(wrbuf, (char *) attr_values.useattr);
          /* Skip date field */
          if (strcmp("date", attr_values.useattr) != 0)
              wrbuf_puts(wrbuf, "_exact");
          yaz_add_name_value_str(encode, name, value, i, "facet.field", odr_strdup(encode, wrbuf_cstr(wrbuf)));
          if (attr_values.limit > 0) {
              WRBUF wrbuf2 = wrbuf_alloc();
              Odr_int olimit;
              wrbuf_puts(wrbuf2, "f.");
              wrbuf_puts(wrbuf2, wrbuf_cstr(wrbuf));
              wrbuf_puts(wrbuf2, ".facet.limit");
              olimit = attr_values.limit;
              yaz_add_name_value_int(encode, name, value, i, odr_strdup(encode, wrbuf_cstr(wrbuf2)), &olimit);
              wrbuf_destroy(wrbuf2);
          }
          wrbuf_destroy(wrbuf);
      }
}

static void yaz_solr_encode_facet_list(ODR encode, char **name, char **value, int *i, Z_FacetList *facet_list, int *limit) {

    int index;
    for (index = 0; index < facet_list->num; index++)  {
        yaz_solr_encode_facet_field(encode, name, value, i, facet_list->elements[index], limit);

    }
}


int yaz_solr_encode_request(Z_HTTP_Request *hreq, Z_SRW_PDU *srw_pdu,
                            ODR encode, const char *charset)
{
    const char *solr_op = 0;
    //TODO Change. not a nice hard coded, unchecked limit.
    char *name[SOLR_MAX_PARAMETERS], *value[SOLR_MAX_PARAMETERS];
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

        if (request->facetList) {
            Z_FacetList *facet_list = request->facetList;
            int limit = 0;
            yaz_add_name_value_str(encode, name, value, &i, "facet", "true");
            yaz_add_name_value_str(encode, name, value, &i, "facet.mincount", "1");
            yaz_solr_encode_facet_list(encode, name, value, &i, facet_list, &limit);
            /*
            olimit = limit;
            yaz_add_name_value_int(encode, name, value, &i, "facet.limit", &olimit);
             */

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

