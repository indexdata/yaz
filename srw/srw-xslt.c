/* $Id: srw-xslt.c,v 1.1 2003-01-06 08:20:28 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#if HAVE_XSLT
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>
#endif

#include <yaz/srw-util.h>

struct xslt_maps_info {
#if HAVE_XSLT
    xmlDocPtr doc;
#else
    int dummy;
#endif
};

struct xslt_map_result_info {
#if HAVE_XSLT
    xmlChar *buf;
#else
    char *buf;
#endif
    int len;
    char *schema;
};

xslt_maps xslt_maps_create()
{
    xslt_maps m = malloc(sizeof(*m));
#if HAVE_XSLT
    m->doc = 0;
#endif
    return m;
}

void xslt_maps_free(xslt_maps m)
{
#if HAVE_XSLT
    xmlFreeDoc(m->doc);
#endif
    free (m);
}

int xslt_maps_file(xslt_maps m, const char *f)
{
#if HAVE_XSLT
    if (m->doc)
        xmlFreeDoc(m->doc);
    m->doc = xmlParseFile(f);
    if (!m->doc)
        return -1;
    return 0;
#else
    return -2;
#endif
}

void xslt_map_free (xslt_map_result res)
{
    if (res)
    {
        free (res->schema);
#if HAVE_XSLT
        xmlFree(res->buf);
#endif
        free (res);
    }
}

xslt_map_result xslt_map (xslt_maps m, const char *schema_source,
                          const char *schema_target,
                          const char *in_buf, int in_len)
{
#if HAVE_XSLT
    const char *map_ns = "http://indexdata.dk/srw/schema-mappings/v1.0/";
    xmlNodePtr ptr;

    if (!m)
        return 0;
    ptr = xmlDocGetRootElement(m->doc);
    while (ptr && ptr->type == XML_ELEMENT_NODE)
    {
        if (!strcmp(ptr->name, "schema-mappings"))
        {
            ptr = ptr->children;
            break;
        }
    }
    for (; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE &&  !strcmp(ptr->name, "map")
            && !strcmp(ptr->ns->href, map_ns) && ptr->children)
        {
            xmlNodePtr src = ptr->children;
            int source_ok = 0;
            int target_ok = 0;
            const char *full_target = 0;
            const char *filename = 0;
            
            for (; src; src = src->next)
            {
                if (src->type == XML_ELEMENT_NODE &&
                    !strcmp(src->name, "schema") &&
                    !strcmp(src->ns->href, map_ns))
                {
                    struct _xmlAttr *attr = src->properties;
                    for (; attr; attr = attr->next)
                        if (!strcmp(attr->name, "target") &&
                            attr->children &&
                            attr->children->type == XML_TEXT_NODE)
                        {
                            full_target = attr->children->content;
                            if (!strcmp(attr->children->content,
                                        schema_target))
                                target_ok = 1;
                        }
                        else if (!strcmp(attr->name, "source")
                                 && attr->children
                                 && attr->children->type == XML_TEXT_NODE)
                            
                        {
                            if (!strcmp(schema_source,
                                        attr->children->content))
                                source_ok = 1;
                        }
                        else if (!strcmp(attr->name, "alias")
                                 && attr->children
                                 && attr->children->type == XML_TEXT_NODE)
                        {
                            if (!strcmp(attr->children->content, schema_target))
                                target_ok = 1;
                        }
                }
                if (src->type == XML_ELEMENT_NODE &&
                    !strcmp(src->name, "stylesheet") &&
                    !strcmp(src->ns->href, map_ns))
                {
                    struct _xmlAttr *attr = src->properties;
                    for (; attr; attr = attr->next)
                        if (!strcmp(attr->name, "filename") &&
                            attr->children &&
                            attr->children->type == XML_TEXT_NODE)
                        {
                            filename = attr->children->content;
                        }
                }
            }
            if (source_ok && target_ok)
            {
                if (filename)
                {
                    xslt_map_result out = malloc(sizeof(*out));
                    xmlDocPtr res, doc = xmlParseMemory(in_buf, in_len);
                    xmlDocPtr xslt_doc = xmlParseFile(filename);
                    xsltStylesheetPtr xsp;
                    
                    xsp = xsltParseStylesheetDoc(xslt_doc);
                    
                    res = xsltApplyStylesheet(xsp, doc, 0);
                    
                    xmlDocDumpMemory (res, &out->buf, &out->len);
                
                    xsltFreeStylesheet(xsp);
                    
                    xmlFreeDoc(doc);
                    xmlFreeDoc(res);
                    
                    out->schema = strdup(full_target);
                    return out;
                }
                else
                {
                    xslt_map_result out = malloc(sizeof(*out));
                    out->buf = xmlMalloc(in_len);
                    memcpy (out->buf, in_buf, in_len);
                    out->len = in_len;
                    out->schema = strdup(full_target);
                    return out; 
                }
            }
        }
    }
#endif
    return 0;
}

char *xslt_map_result_buf(xslt_map_result res)
{
    return res->buf;
}
int xslt_map_result_len(xslt_map_result res)
{
    return res->len;
}

char *xslt_map_result_schema(xslt_map_result res)
{
    return res->schema;
}
