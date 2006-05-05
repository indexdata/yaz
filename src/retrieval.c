/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: retrieval.c,v 1.2 2006-05-05 18:37:08 adam Exp $
 */
/**
 * \file retrieval.c
 * \brief Retrieval utility
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/retrieval.h>
#include <yaz/wrbuf.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/tpath.h>

#if HAVE_XSLT
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>

/** \brief The internal structure for yaz_retrieval_t */
struct yaz_retrieval_struct {
    /** \brief memory for configuration */
    NMEM nmem;

    /** \brief string buffer for error messages */
    WRBUF wr_error;

    /** \brief path for opening files  */
    char *path;

    /** \brief retrieval list */
    struct yaz_retrieval_elem *list;

    /** \brief last pointer in retrieval list */
    struct yaz_retrieval_elem **list_p;
};

/** \brief information per 'retrieval' construct */
struct yaz_retrieval_elem {
    /** \brief schema identifier */
    const char *identifier;
    /** \brief schema short-hand (such sa "dc") */
    const char *schema;
    /** \brief record syntax */
    const char *syntax;
    /** \brief backend schema */
    const char *backend_schema;
    /** \brief backend syntax */
    const char *backend_syntax;

    /** \brief record conversion */
    yaz_record_conv_t record_conv;

    /** \breif next element in list */
    struct yaz_retrieval_elem *next;
};

static void yaz_retrieval_reset(yaz_retrieval_t p);

yaz_retrieval_t yaz_retrieval_create()
{
    yaz_retrieval_t p = xmalloc(sizeof(*p));
    p->nmem = nmem_create();
    p->wr_error = wrbuf_alloc();
    p->list = 0;
    p->path = 0;
    yaz_retrieval_reset(p);
    return p;
}

void yaz_retrieval_destroy(yaz_retrieval_t p)
{
    if (p)
    {
        yaz_retrieval_reset(p);
        nmem_destroy(p->nmem);
        wrbuf_free(p->wr_error, 1);
        xfree(p->path);
        xfree(p);
    }
}

void yaz_retrieval_reset(yaz_retrieval_t p)
{
    struct yaz_retrieval_elem *el = p->list;
    for(; el; el = el->next)
        yaz_record_conv_destroy(el->record_conv);

    wrbuf_rewind(p->wr_error);
    nmem_reset(p->nmem);

    p->list = 0;
    p->list_p = &p->list;
}

/** \brief parse retrieval XML config */
static int conf_retrieval(yaz_retrieval_t p, const xmlNode *ptr)
{

    struct _xmlAttr *attr;
    struct yaz_retrieval_elem *el = nmem_malloc(p->nmem, sizeof(*el));

    el->syntax = 0;
    el->identifier = 0;
    el->schema = 0;
    el->backend_schema = 0;
    el->backend_syntax = 0;

    el->next = 0;

    for (attr = ptr->properties; attr; attr = attr->next)
    {
        if (!xmlStrcmp(attr->name, BAD_CAST "syntax") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            el->syntax = 
                nmem_strdup(p->nmem, (const char *) attr->children->content);
        else if (!xmlStrcmp(attr->name, BAD_CAST "identifier") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            el->identifier =
                nmem_strdup(p->nmem, (const char *) attr->children->content);
        else if (!xmlStrcmp(attr->name, BAD_CAST "schema") &&
                 attr->children && attr->children->type == XML_TEXT_NODE)
            el->schema = 
                nmem_strdup(p->nmem, (const char *) attr->children->content);
        else if (!xmlStrcmp(attr->name, BAD_CAST "backendschema") &&
                 attr->children && attr->children->type == XML_TEXT_NODE)
            el->backend_schema = 
                nmem_strdup(p->nmem, (const char *) attr->children->content);
        else if (!xmlStrcmp(attr->name, BAD_CAST "backendsyntax") &&
                 attr->children && attr->children->type == XML_TEXT_NODE)
            el->backend_syntax = 
                nmem_strdup(p->nmem, (const char *) attr->children->content);
        else
        {
            wrbuf_printf(p->wr_error, "Bad attribute '%s'.", attr->name);
            return -1;
        }
    }
    el->record_conv = yaz_record_conv_create();

    yaz_record_conv_set_path(el->record_conv, p->path);
    
    if (yaz_record_conv_configure(el->record_conv, ptr->children))
    {
        wrbuf_printf(p->wr_error, "%s",
                     yaz_record_conv_get_error(el->record_conv));
        yaz_record_conv_destroy(el->record_conv);
        return -1;
    }
    
    *p->list_p = el;
    p->list_p = &el->next;
    return 0;
}

int yaz_retrieval_configure(yaz_retrieval_t p, const void *ptr_v)
{
    const xmlNode *ptr = ptr_v; 

    yaz_retrieval_reset(p);

    if (ptr && ptr->type == XML_ELEMENT_NODE &&
        !strcmp((const char *) ptr->name, "retrievalinfo"))
    {
        for (ptr = ptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type != XML_ELEMENT_NODE)
                continue;
            if (!strcmp((const char *) ptr->name, "retrieval"))
            {
                if (conf_retrieval(p, ptr))
                    return -1;
            }
            else
            {
                wrbuf_printf(p->wr_error, "Bad element '%s'."
                             " Expected 'retrieval'", ptr->name);
                return -1;
            }
        }
    }
    else
    {
        wrbuf_printf(p->wr_error, "Missing 'retrievalinfo' element");
        return -1;
    }
    return 0;
}

int yaz_retrieval_request(yaz_retrieval_t p, const char *schema,
                          const char *syntax, yaz_record_conv_t *rc)
{
    wrbuf_rewind(p->wr_error);
    wrbuf_printf(p->wr_error, "yaz_retrieval_request: not implemented");
    return -1;
}

const char *yaz_retrieval_get_error(yaz_retrieval_t p)
{
    return wrbuf_buf(p->wr_error);
}

void yaz_retrieval_set_path(yaz_retrieval_t p, const char *path)
{
    xfree(p->path);
    p->path = 0;
    if (path)
        p->path = xstrdup(path);
}

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

