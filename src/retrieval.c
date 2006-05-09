/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: retrieval.c,v 1.9 2006-05-09 11:35:28 adam Exp $
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
#include <yaz/proto.h>

#if HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>

/** \brief The internal structure for yaz_retrieval_t */
struct yaz_retrieval_struct {
    /** \brief ODR memory for configuration */
    ODR odr;

    /** \brief odr's NMEM memory (odr->mem) */
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
    int *syntax;
    /** \brief backend schema */
    const char *backend_schema;
    /** \brief backend syntax */
    int *backend_syntax;

    /** \brief record conversion */
    yaz_record_conv_t record_conv;

    /** \brief next element in list */
    struct yaz_retrieval_elem *next;
};

static void yaz_retrieval_reset(yaz_retrieval_t p);

yaz_retrieval_t yaz_retrieval_create()
{
    yaz_retrieval_t p = xmalloc(sizeof(*p));
    p->odr = odr_createmem(ODR_ENCODE);
    p->nmem = p->odr->mem;
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
        odr_destroy(p->odr);
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
    odr_reset(p->odr);

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
        {
            el->syntax = yaz_str_to_z3950oid(
                p->odr, CLASS_RECSYN,
                (const char *) attr->children->content);
            if (!el->syntax)
            {
                wrbuf_printf(p->wr_error, "Bad syntax '%s'",
                             (const char *) attr->children->content);
                return -1;
            }
        }
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
        {
            el->backend_syntax = yaz_str_to_z3950oid(
                p->odr, CLASS_RECSYN,
                (const char *) attr->children->content);
            if (!el->backend_syntax)
            {
                wrbuf_printf(p->wr_error, "Bad backendsyntax '%s'",
                             (const char *) attr->children->content);
                return -1;
            }
        }
        else
        {
            wrbuf_printf(p->wr_error, "Bad attribute '%s'.", attr->name);
            return -1;
        }
    }
    if (!el->syntax)
    {
        wrbuf_printf(p->wr_error, "Missing 'syntax' attribute");
        return -1;
    }

    el->record_conv = 0; /* OK to have no 'convert' sub content */
    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE)
        {
            el->record_conv = yaz_record_conv_create();
            
            yaz_record_conv_set_path(el->record_conv, p->path);
        
            if (yaz_record_conv_configure(el->record_conv, ptr))
            {
                wrbuf_printf(p->wr_error, "%s",
                             yaz_record_conv_get_error(el->record_conv));
                yaz_record_conv_destroy(el->record_conv);
                return -1;
            }
        }
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

int yaz_retrieval_request(yaz_retrieval_t p,
                          const char *schema, int *syntax,
                          const char **match_schema, int **match_syntax,
                          yaz_record_conv_t *rc,
                          const char **backend_schema,
                          int **backend_syntax)
{
    struct yaz_retrieval_elem *el = p->list;
    int syntax_matches = 0;
    int schema_matches = 0;

    wrbuf_rewind(p->wr_error);
    if (!el)
        return 0;
    for(; el; el = el->next)
    {
        int schema_ok = 0;
        int syntax_ok = 0;

        if (schema && el->schema && !strcmp(schema, el->schema))
            schema_ok = 1;
        if (schema && el->identifier && !strcmp(schema, el->identifier))
            schema_ok = 1;
        if (!schema)
            schema_ok = 1;
        if (schema && !el->schema)
            schema_ok = 1;
        
        if (syntax && el->syntax && !oid_oidcmp(syntax, el->syntax))
            syntax_ok = 1;
        if (!syntax)
            syntax_ok = 1;

        if (syntax_ok)
            syntax_matches++;
        if (schema_ok)
            schema_matches++;
        if (syntax_ok && schema_ok)
        {
            *match_syntax = el->syntax;
            *match_schema = el->schema;
            if (backend_schema)
                *backend_schema = el->backend_schema;
            if (backend_syntax)
                *backend_syntax = el->backend_syntax;
            if (rc)
                *rc = el->record_conv;
            return 0;
        }
    }
    if (!syntax_matches && syntax)
    {
        char buf[OID_STR_MAX];
        wrbuf_printf(p->wr_error, "%s", oid_to_dotstring(syntax, buf));
        return 2;
    }
    if (schema)
        wrbuf_printf(p->wr_error, "%s", schema);
    if (!schema_matches)
        return 1;
    return 3;
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

