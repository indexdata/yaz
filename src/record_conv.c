/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: record_conv.c,v 1.1 2006-05-02 20:47:45 adam Exp $
 */
/**
 * \file record_conv.c
 * \brief Record Conversions utility
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

#include <string.h>

#include <yaz/record_conv.h>
#include <yaz/wrbuf.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>

/** \brief The internal structure for yaz_record_conv_t */
struct yaz_record_conv_struct {
    /** memory for configuration */
    NMEM nmem;

    /** conversion rules (allocated using NMEM) */
    struct yaz_record_conv_rule *rules;

    /** pointer to last conversion rule pointer in chain */
    struct yaz_record_conv_rule **rules_p;

    /** string buffer for error messages */
    WRBUF wr_error;
};

/** \brief tranformation types (rule types) */
enum YAZ_RECORD_CONV_RULE 
{
    YAZ_RECORD_CONV_RULE_XSLT,
    YAZ_RECORD_CONV_RULE_MARC_TO_XML,
    YAZ_RECORD_CONV_RULE_XML_TO_MARC
};

/** \brief tranformation info (rule info) */
struct yaz_record_conv_rule {
    enum YAZ_RECORD_CONV_RULE which;
    union {
        struct {
            const char *stylesheet;
        } xslt;
        struct {
            const char *charset;
        } marc_to_xml;
        struct {
            const char *charset;
        } xml_to_marc;
    } u;
    struct yaz_record_conv_rule *next;
};

yaz_record_conv_t yaz_record_conv_create()
{
    yaz_record_conv_t p = xmalloc(sizeof(*p));
    p->nmem = nmem_create();
    p->wr_error = wrbuf_alloc();
    return p;
}

void yaz_record_conv_destroy(yaz_record_conv_t p)
{
    if (p)
    {
        nmem_destroy(p->nmem);
        wrbuf_free(p->wr_error, 1);
        xfree(p);
    }
}

#if HAVE_XML2
static struct yaz_record_conv_rule *add_rule(yaz_record_conv_t p,
                                             enum YAZ_RECORD_CONV_RULE type)
{
    struct yaz_record_conv_rule *r = nmem_malloc(p->nmem, sizeof(*r));
    r->which = type;
    r->next = 0;
    *p->rules_p = r;
    p->rules_p = &r->next;
    return r;
}

static void yaz_record_conv_reset(yaz_record_conv_t p)
{
    wrbuf_rewind(p->wr_error);
    nmem_reset(p->nmem);
    p->rules = 0;
    p->rules_p = &p->rules;
}

static int conv_xslt(yaz_record_conv_t p, const xmlNode *ptr)
{
    struct _xmlAttr *attr;
    const char *stylesheet = 0;

    for (attr = ptr->properties; attr; attr = attr->next)
    {
        if (!xmlStrcmp(attr->name, BAD_CAST "stylesheet") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            stylesheet = (const char *) attr->children->content;
        else
        {
            wrbuf_printf(p->wr_error, "Bad attribute '%s'."
                         "Expected stylesheet.", attr->name);
            return -1;
        }
    }
    if (stylesheet)
    {
        struct yaz_record_conv_rule *r =
            add_rule(p, YAZ_RECORD_CONV_RULE_XSLT);
        r->u.xslt.stylesheet = nmem_strdup(p->nmem, stylesheet);
        return 0;
    }
    wrbuf_printf(p->wr_error, "Missing attribute 'stylesheet'");
    return -1;
}

static int conv_marc_to_xml(yaz_record_conv_t p, const xmlNode *ptr)
{
    struct _xmlAttr *attr;
    const char *charset = 0;
    struct yaz_record_conv_rule *r;

    for (attr = ptr->properties; attr; attr = attr->next)
    {
        if (!xmlStrcmp(attr->name, BAD_CAST "charset") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            charset = (const char *) attr->children->content;
        else
        {
            wrbuf_printf(p->wr_error, "Bad attribute '%s'."
                         "Expected charset.", attr->name);
            return -1;
        }
    }
    r = add_rule(p, YAZ_RECORD_CONV_RULE_MARC_TO_XML);
    if (charset)
        r->u.marc_to_xml.charset = nmem_strdup(p->nmem, charset);
    else
        r->u.marc_to_xml.charset = 0;
    return 0;
}

static int conv_xml_to_marc(yaz_record_conv_t p, const xmlNode *ptr)
{
    struct _xmlAttr *attr;
    const char *charset = 0;
    struct yaz_record_conv_rule *r;

    for (attr = ptr->properties; attr; attr = attr->next)
    {
        if (!xmlStrcmp(attr->name, BAD_CAST "charset") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            charset = (const char *) attr->children->content;
        else
        {
            wrbuf_printf(p->wr_error, "Bad attribute '%s'."
                         "Expected charset.", attr->name);
            return -1;
        }
    }
    r = add_rule(p, YAZ_RECORD_CONV_RULE_XML_TO_MARC);
    if (charset)
        r->u.xml_to_marc.charset = nmem_strdup(p->nmem, charset);
    else
        r->u.xml_to_marc.charset = 0;
    return 0;
}


int yaz_record_conv_configure(yaz_record_conv_t p, const void *ptr_v)
{
    const xmlNode *ptr = ptr_v; 

    yaz_record_conv_reset(p);

    if (ptr && ptr->type == XML_ELEMENT_NODE &&
        !strcmp((const char *) ptr->name, "convert"))
    {
        for (ptr = ptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type != XML_ELEMENT_NODE)
                continue;
            if (!strcmp((const char *) ptr->name, "xslt"))
            {
                if (conv_xslt(p, ptr))
                    return -1;
            }
            else if (!strcmp((const char *) ptr->name, "marc_to_xml"))
            {
                if (conv_marc_to_xml(p, ptr))
                    return -1;
            }
            else if (!strcmp((const char *) ptr->name, "xml_to_marc"))
            {
                if (conv_xml_to_marc(p, ptr))
                    return -1;
            }
            else
            {
                wrbuf_printf(p->wr_error, "Bad element '%s'."
                             "Expected xslt, marc_to_xml,...", ptr->name);
                return -1;
            }
        }
    }
    else
    {
        wrbuf_printf(p->wr_error, "Missing 'convert' element");
        return -1;
    }
    return 0;
}

#else
/* HAVE_XML2 */
int yaz_record_conv_configure(yaz_record_conv_t p, const void *ptr_v)
{
    wrbuf_rewind(p->wr_error);
    wrbuf_printf(p->wr_error, "No XML support for yaz_record_conv");
    return -1;
}

#endif

const char *yaz_record_conv_get_error(yaz_record_conv_t p)
{
    return wrbuf_buf(p->wr_error);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

