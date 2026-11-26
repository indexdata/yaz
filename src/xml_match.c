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
#include <yaz/nmem_xml.h>
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
        if (len)
            *len = 0;
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


/* An element that is dumped on its own gets a declaration of each
   namespace that it uses but that is declared further up (bug #740). The
   message has such a declaration once, while every record, and every element
   at the top of a record, gets a copy. Allow those copies
   XML_NS_INHERIT_BASE bytes plus XML_NS_INHERIT_FACTOR times the size of the
   elements themselves. Ordinary records need far less, and the result stays
   in proportion to the message. */
#define XML_NS_INHERIT_BASE 256
#define XML_NS_INHERIT_FACTOR 8

/* Unlike xmlNodeDump, this leaves buf valid if memory runs out */
static int dump_node(xmlBufferPtr buf, xmlNodePtr ptr)
{
    xmlOutputBufferPtr out = xmlOutputBufferCreateBuffer(buf, 0);

    if (!out)
        return -1;
    xmlNodeDumpOutput(out, 0, ptr, 0, 0, 0);
    return xmlOutputBufferClose(out) < 0 ? -1 : 0;
}

/* Adds a declaration of ns to the list that ends at *last. Nothing is done
   if ns is declared inside top or was added before: in both cases its
   _private member, which libxml2 leaves to the application, is top. */
static int inherit_ns(xmlNodePtr top, xmlNsPtr ns, xmlNsPtr **last)
{
    if (ns && ns->_private != top)
    {
        if (!xmlStrEqual(ns->prefix, BAD_CAST "xml")) /* needs none */
        {
            xmlNsPtr n = xmlNewNs(0, ns->href, ns->prefix);
            if (!n)
                return -1;
            n->_private = ns; /* for the reset in yaz_xml_dump_element */
            **last = n;
            *last = &n->next;
        }
        ns->_private = top;
    }
    return 0;
}

/* Makes declarations for the namespaces that top and the elements and
   attributes inside of it use, but that are declared further up: the ones
   that xmlCopyNode adds to a copy of top, in the same order. This is one
   pass over the tree, without searching. */
static int inherit_ns_all(xmlNodePtr top, xmlNsPtr **last)
{
    xmlNodePtr ptr = top;

    while (1)
    {
        if (ptr->type == XML_ELEMENT_NODE)
        {
            xmlNsPtr ns;
            xmlAttrPtr attr;

            for (ns = ptr->nsDef; ns; ns = ns->next)
                ns->_private = top;
            if (inherit_ns(top, ptr->ns, last))
                return -1;
            for (attr = ptr->properties; attr; attr = attr->next)
                if (inherit_ns(top, attr->ns, last))
                    return -1;
            if (ptr->children)
            {
                ptr = ptr->children;
                continue;
            }
        }
        while (ptr != top && !ptr->next)
            ptr = ptr->parent;
        if (ptr == top)
            break;
        ptr = ptr->next;
    }
    return 0;
}

/* Appends element ptr to buf as XML that can be parsed on its own: with
   the namespace declarations that it inherits (bug #740). The result is
   that of dumping xmlCopyNode(ptr, 1), which was done here before. But the
   copy searches the lists of declarations for every element and attribute,
   and it adds a declaration for every attribute with prefix xml. For a
   record from a peer that took time in proportion to the square of its
   size. */
int yaz_xml_dump_element(xmlBufferPtr buf, xmlNodePtr ptr, size_t *own)
{
    xmlNsPtr inherited = 0, *last = &inherited, *end = &ptr->nsDef, ns;

    int start = xmlBufferLength(buf);
    int total, len = -1;
    size_t inherited_size;

    if (inherit_ns_all(ptr, &last) == 0)
    {
        const xmlChar *encoding = 0;

        /* Measure without inherited declarations for the expansion budget. */
        if (inherited)
        {
            xmlBufferPtr org = xmlBufferCreate();
            if (org && dump_node(org, ptr) == 0)
                len = xmlBufferLength(org);
            xmlBufferFree(org);
        }
        /* ptr has the declarations while it is dumped */
        while (*end)
            end = &(*end)->next;
        *end = inherited;
        if (ptr->doc)
        {
            /* as for a copy, which has no document: non-ASCII characters
               in attribute values are dumped as character references */
            encoding = ptr->doc->encoding;
            ptr->doc->encoding = 0;
        }
        if (dump_node(buf, ptr) != 0)
            len = -1;
        else if (!inherited)
            len = xmlBufferLength(buf) - start;
        if (ptr->doc)
            ptr->doc->encoding = encoding;
        *end = 0;
    }
    for (ns = inherited; ns; ns = ns->next)
        ((xmlNsPtr) ns->_private)->_private = 0;
    xmlFreeNsList(inherited);

    total = xmlBufferLength(buf);
    if (len < 0 || total < 0)
        return -1;
    *own += len;
    inherited_size = (size_t) total > *own ? (size_t) total - *own : 0;
    if (inherited_size > XML_NS_INHERIT_BASE &&
        (inherited_size - XML_NS_INHERIT_BASE - 1) / XML_NS_INHERIT_FACTOR >= *own)
        return -1;
    return 0;
}

int yaz_match_xsd_XML_n2(xmlNodePtr ptr, const char *elem, ODR o,
                         char **val, int *len, int fixup_root)
{
    size_t own = 0;
    int no_root_nodes = 0;

    if (!yaz_match_xsd_element(ptr, elem))
        return 0;

    xmlBufferPtr buf = xmlBufferCreate();

    /* Dump each element nodes at top.
       In most cases there is only one root node.. At least one server
       http://www.theeuropeanlibrary.org/sru/sru.pl
       has multiple root nodes in recordData.
    */
    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE)
        {
            if (yaz_xml_dump_element(buf, ptr, &own))
            {
                xmlBufferFree(buf);
                return -1;
            }
            no_root_nodes++;
        }
    }
    if (no_root_nodes != 1 && fixup_root)
    {
        /* does not appear to be an XML document. Make it so */
        xmlBufferAddHead(buf, (const xmlChar *) "<yaz_record>", -1);
        xmlBufferAdd(buf, (const xmlChar *) "</yaz_record>", -1);
    }
    *val = nmem_from_xml_buffer(odr_getmem(o), buf, len);
    xmlBufferFree(buf);

    return *val ? 1 : -1;
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

