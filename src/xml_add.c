/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file xml_add.c
 * \brief XML node creation utilities
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <yaz/srw.h>
#if YAZ_HAVE_XML2
#include "sru-p.h"

void add_XML_n(xmlNodePtr ptr, const char *elem, char *val, int len,
               xmlNsPtr ns_ptr)
{
    if (val)
    {
        xmlDocPtr doc = xmlParseMemory(val,len);
        if (doc)
        {
            xmlNodePtr c = xmlNewChild(ptr, ns_ptr, BAD_CAST elem, 0);
            xmlNodePtr t = xmlDocGetRootElement(doc);
            xmlAddChild(c, xmlCopyNode(t,1));
            xmlFreeDoc(doc);
        }
    }
}

xmlNodePtr add_xsd_string_n(xmlNodePtr ptr, const char *elem, const char *val,
                            int len)
{
    if (val)
    {
        xmlNodePtr c = xmlNewChild(ptr, 0, BAD_CAST elem, 0);
        xmlNodePtr t = xmlNewTextLen(BAD_CAST val, len);
        xmlAddChild(c, t);
        return t;
    }
    return 0;
}

xmlNodePtr add_xsd_string_ns(xmlNodePtr ptr, const char *elem, const char *val,
                             xmlNsPtr ns_ptr)
{
    if (val)
    {
        xmlNodePtr c = xmlNewChild(ptr, ns_ptr, BAD_CAST elem, 0);
        xmlNodePtr t = xmlNewText(BAD_CAST val);
        xmlAddChild(c, t);
        return t;
    }
    return 0;
}

xmlNodePtr add_xsd_string(xmlNodePtr ptr, const char *elem, const char *val)
{
    return add_xsd_string_ns(ptr, elem, val, 0);
}

void add_xsd_integer(xmlNodePtr ptr, const char *elem,
                            const Odr_int *val)
{
    if (val)
    {
        char str[40];
        sprintf(str, ODR_INT_PRINTF, *val);
        xmlNewTextChild(ptr, 0, BAD_CAST elem, BAD_CAST str);
    }
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

