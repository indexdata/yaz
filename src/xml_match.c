/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file xml_match.c
 * \brief XML node inspection utilities
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/srw.h>
#if YAZ_HAVE_XML2
#include "sru-p.h"

int yaz_match_xsd_element(xmlNodePtr ptr, const char *elem)
{
    if (ptr && ptr->type == XML_ELEMENT_NODE &&
        !xmlStrcmp(ptr->name, BAD_CAST elem))
    {
        return 1;
    }
    return 0;
}

#define CHECK_TYPE 0

int yaz_match_xsd_string_n_nmem(xmlNodePtr ptr, const char *elem, NMEM nmem,
                                char **val, int *len)
{
#if CHECK_TYPE
    struct _xmlAttr *attr;
#endif
    if (!yaz_match_xsd_element(ptr, elem))
        return 0;
#if CHECK_TYPE
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
#endif
    ptr = ptr->children;
    if (!ptr || ptr->type != XML_TEXT_NODE)
    {
        *val = "";
        return 1;
    }
    *val = nmem_strdup(nmem, (const char *) ptr->content);
    if (len)
        *len = xmlStrlen(ptr->content);
    return 1;
}

int yaz_match_xsd_string_n(xmlNodePtr ptr, const char *elem, ODR o,
                           char **val, int *len)
{
    return yaz_match_xsd_string_n_nmem(ptr, elem, o->mem, val, len);
}

int yaz_match_xsd_string(xmlNodePtr ptr, const char *elem, ODR o, char **val)
{
    return yaz_match_xsd_string_n(ptr, elem, o, val, 0);
}

int yaz_match_xsd_XML_n2(xmlNodePtr ptr, const char *elem, ODR o,
                         char **val, int *len, int fixup_root)
{
    xmlBufferPtr buf;
    int no_root_nodes = 0;

    if (!yaz_match_xsd_element(ptr, elem))
        return 0;

    buf = xmlBufferCreate();

    /* Copy each element nodes at top.
       In most cases there is only one root node.. At least one server
       http://www.theeuropeanlibrary.org/sru/sru.pl
       has multiple root nodes in recordData.
    */
    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE)
        {
            /* copy node to get NS right (bug #740). */
            xmlNode *tmp = xmlCopyNode(ptr, 1);

            xmlNodeDump(buf, tmp->doc, tmp, 0, 0);

            xmlFreeNode(tmp);
            no_root_nodes++;
        }
    }
    if (no_root_nodes != 1 && fixup_root)
    {
        /* does not appear to be an XML document. Make it so */
        xmlBufferAddHead(buf, (const xmlChar *) "<yaz_record>", -1);
        xmlBufferAdd(buf, (const xmlChar *) "</yaz_record>", -1);
    }
    *val = odr_strdupn(o, (const char *) buf->content, buf->use);
    if (len)
        *len = buf->use;

    xmlBufferFree(buf);

    return 1;
}

int yaz_match_xsd_XML_n(xmlNodePtr ptr, const char *elem, ODR o,
                        char **val, int *len)
{
    return yaz_match_xsd_XML_n2(ptr, elem, o, val, len, 0);
}

int yaz_match_xsd_integer(xmlNodePtr ptr, const char *elem, ODR o,
                          Odr_int **val)
{
#if CHECK_TYPE
    struct _xmlAttr *attr;
#endif
    if (!yaz_match_xsd_element(ptr, elem))
        return 0;
#if CHECK_TYPE
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
#endif
    ptr = ptr->children;
    if (!ptr || ptr->type != XML_TEXT_NODE)
        return 0;
    *val = odr_intdup(o, odr_atoi((const char *) ptr->content));
    return 1;
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

