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

#include "sru-p.h"

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

int yaz_solr_decode(ODR o, Z_HTTP_Response *hres, Z_SRW_PDU **pdup)
{
#if YAZ_HAVE_XML2
    const char *content_buf = hres->content_buf;
    int content_len = hres->content_len;
    xmlDocPtr doc = xmlParseMemory(content_buf, content_len);
    int ret = 0;
    xmlNodePtr ptr = 0;
    Odr_int start = 0;
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
                    break;
            }
            if (!ptr)
            {
                ret = -1;
            }
        }
    }
    if (ptr)
    {   /* got result node */
        struct _xmlAttr *attr;

        for (attr = ptr->properties; attr; attr = attr->next)
            if (attr->children && attr->children->type == XML_TEXT_NODE)
            {
                if (!strcmp((const char *) attr->name, "numFound"))
                {
                    sr->numberOfRecords =
                        odr_intdup(o, 
                                   odr_atoi(
                                       (const char *) attr->children->content));
                }
                else if (!strcmp((const char *) attr->name, "start"))
                {
                    start = odr_atoi((const char *) attr->children->content);
                }
            }
    }
    if (ptr)
    {
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
                record->recordPosition = odr_intdup(o, start + offset);

                xmlBufferFree(buf);

                offset++;
                i++;
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

int yaz_solr_encode(Z_HTTP_Request *hreq, Z_SRW_PDU *srw_pdu,
                    ODR encode, const char *charset)
{
    const char *solr_op = 0;
    char *name[30], *value[30];
    char *uri_args;
    char *path;
    int i = 0;

    z_HTTP_header_add_basic_auth(encode, &hreq->headers, 
                                 srw_pdu->username, srw_pdu->password);

    switch (srw_pdu->which)
    {
    case Z_SRW_searchRetrieve_request:
        solr_op = "select";
        
        switch(srw_pdu->u.request->query_type)
        {
        case Z_SRW_query_type_pqf:
            yaz_add_name_value_str(encode, name, value, &i,
                                   "q", srw_pdu->u.request->query.pqf);
            break;
        case Z_SRW_query_type_cql:
            yaz_add_name_value_str(encode, name, value, &i,
                                   "q", srw_pdu->u.request->query.cql);
            break;
        default:
            return -1;
        }
        if (srw_pdu->u.request->startRecord)
        {
            Odr_int start = *srw_pdu->u.request->startRecord - 1;
            yaz_add_name_value_int(encode, name, value, &i,
                                   "start", &start);
        }
        yaz_add_name_value_int(encode, name, value, &i,
                               "rows", srw_pdu->u.request->maximumRecords);
        break;
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

