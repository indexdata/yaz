/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: record_conv.c,v 1.5 2006-05-07 14:48:25 adam Exp $
 */
/**
 * \file record_conv.c
 * \brief Record Conversions utility
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/yaz-iconv.h>
#include <yaz/marcdisp.h>
#include <yaz/record_conv.h>
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

/** \brief The internal structure for yaz_record_conv_t */
struct yaz_record_conv_struct {
    /** \brief memory for configuration */
    NMEM nmem;

    /** \brief conversion rules (allocated using NMEM) */
    struct yaz_record_conv_rule *rules;

    /** \brief pointer to last conversion rule pointer in chain */
    struct yaz_record_conv_rule **rules_p;

    /** \brief string buffer for error messages */
    WRBUF wr_error;

    /** \brief path for opening files  */
    char *path;
};

/** \brief tranformation types (rule types) */
enum YAZ_RECORD_CONV_RULE 
{
    YAZ_RECORD_CONV_RULE_XSLT,
    YAZ_RECORD_CONV_RULE_MARC
};


/** \brief tranformation info (rule info) */
struct yaz_record_conv_rule {
    enum YAZ_RECORD_CONV_RULE which;
    union {
        struct {
            xsltStylesheetPtr xsp;
            int dummy;
        } xslt;
        struct {
            yaz_iconv_t iconv_t;
            int input_format;
            int output_format;
        } marc;
    } u;
    struct yaz_record_conv_rule *next;
};

/** \brief reset rules+configuration */
static void yaz_record_conv_reset(yaz_record_conv_t p)
{
    struct yaz_record_conv_rule *r;
    for (r = p->rules; r; r = r->next)
    {
        if (r->which == YAZ_RECORD_CONV_RULE_MARC)
        {
            if (r->u.marc.iconv_t)
                yaz_iconv_close(r->u.marc.iconv_t);
        }
        else if (r->which == YAZ_RECORD_CONV_RULE_XSLT)
        {
            xsltFreeStylesheet(r->u.xslt.xsp);
        }
    }
    wrbuf_rewind(p->wr_error);
    nmem_reset(p->nmem);

    p->rules = 0;

    p->rules_p = &p->rules;
}

yaz_record_conv_t yaz_record_conv_create()
{
    yaz_record_conv_t p = xmalloc(sizeof(*p));
    p->nmem = nmem_create();
    p->wr_error = wrbuf_alloc();
    p->rules = 0;
    p->path = 0;

    yaz_record_conv_reset(p);
    return p;
}

void yaz_record_conv_destroy(yaz_record_conv_t p)
{
    if (p)
    {
        yaz_record_conv_reset(p);
        nmem_destroy(p->nmem);
        wrbuf_free(p->wr_error, 1);
        xfree(p->path);
        xfree(p);
    }
}

/** \brief adds a rule */
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

/** \brief parse 'xslt' conversion node */
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
    if (!stylesheet)
    {
        wrbuf_printf(p->wr_error, "Missing attribute 'stylesheet'");
        return -1;
    }
    else
    {
        char fullpath[1024];
        xsltStylesheetPtr xsp;
        if (!yaz_filepath_resolve(stylesheet, p->path, 0, fullpath))
        {
            wrbuf_printf(p->wr_error, "could not locate '%s'. Path=%s",
                         stylesheet, p->path);
            return -1;
        }
        xsp = xsltParseStylesheetFile((xmlChar*) fullpath);
        if (!xsp)
        {
            wrbuf_printf(p->wr_error, "xsltParseStylesheetFile failed'");
            return -1;
        }
        else
        {
            struct yaz_record_conv_rule *r = 
                add_rule(p, YAZ_RECORD_CONV_RULE_XSLT);
            r->u.xslt.xsp = xsp;
        }
    }
    return 0;
}

/** \brief parse 'marc' conversion node */
static int conv_marc(yaz_record_conv_t p, const xmlNode *ptr)
{
    struct _xmlAttr *attr;
    const char *input_charset = 0;
    const char *output_charset = 0;
    const char *input_format = 0;
    const char *output_format = 0;
    int input_format_mode = 0;
    int output_format_mode = 0;
    struct yaz_record_conv_rule *r;
    yaz_iconv_t cd = 0;

    for (attr = ptr->properties; attr; attr = attr->next)
    {
        if (!xmlStrcmp(attr->name, BAD_CAST "inputcharset") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            input_charset = (const char *) attr->children->content;
        else if (!xmlStrcmp(attr->name, BAD_CAST "outputcharset") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            output_charset = (const char *) attr->children->content;
        else if (!xmlStrcmp(attr->name, BAD_CAST "inputformat") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            input_format = (const char *) attr->children->content;
        else if (!xmlStrcmp(attr->name, BAD_CAST "outputformat") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            output_format = (const char *) attr->children->content;
        else
        {
            wrbuf_printf(p->wr_error, "Bad attribute '%s'.", attr->name);
            return -1;
        }
    }
    if (!input_format)
    {
        wrbuf_printf(p->wr_error, "Attribute 'inputformat' required");
        return -1;
    }
    else if (!strcmp(input_format, "marc"))
    {
        input_format_mode = YAZ_MARC_ISO2709;
    }
    else if (!strcmp(input_format, "xml"))
    {
        input_format_mode = YAZ_MARC_MARCXML;
        /** Libxml2 generates UTF-8 encoding by default .
            So we convert from UTF-8 to outputcharset (if defined) 
        */
        if (!input_charset && output_charset)
            input_charset = "utf-8";
    }
    else
    {
        wrbuf_printf(p->wr_error, "Bad inputformat: '%s'", input_format);
        return -1;
    }
    
    if (!output_format)
    {
        wrbuf_printf(p->wr_error, "Attribute 'outputformat' required");
        return -1;
    }
    else if (!strcmp(output_format, "line"))
    {
        output_format_mode = YAZ_MARC_LINE;
    }
    else if (!strcmp(output_format, "marcxml"))
    {
        output_format_mode = YAZ_MARC_MARCXML;
        if (input_charset && !output_charset)
            output_charset = "utf-8";
    }
    else if (!strcmp(output_format, "marc"))
    {
        output_format_mode = YAZ_MARC_ISO2709;
    }
    else if (!strcmp(output_format, "marcxchange"))
    {
        output_format_mode = YAZ_MARC_XCHANGE;
        if (input_charset && !output_charset)
            output_charset = "utf-8";
    }
    else
    {
        wrbuf_printf(p->wr_error, "Bad outputformat: '%s'", input_format);
        return -1;
    }
    if (input_charset && output_charset)
    {
        cd = yaz_iconv_open(output_charset, input_charset);
        if (!cd)
        {
            wrbuf_printf(p->wr_error, "Unsupported character set mamping"
                         " inputcharset=%s outputcharset=%s",
                         input_charset, output_charset);
            return -1;
        }
    }
    else if (input_charset)
    {
        wrbuf_printf(p->wr_error, "Attribute 'outputcharset' missing");
        return -1;
    }
    else if (output_charset)
    {
        wrbuf_printf(p->wr_error, "Attribute 'inputcharset' missing");
        return -1;
    }
    r = add_rule(p, YAZ_RECORD_CONV_RULE_MARC);
    r->u.marc.iconv_t = cd;

    r->u.marc.input_format = input_format_mode;
    r->u.marc.output_format = output_format_mode;
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
            else if (!strcmp((const char *) ptr->name, "marc"))
            {
                if (conv_marc(p, ptr))
                    return -1;
            }
            else
            {
                wrbuf_printf(p->wr_error, "Bad element '%s'."
                             "Expected marc, xslt, ..", ptr->name);
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

int yaz_record_conv_record(yaz_record_conv_t p,
                           const char *input_record_buf,
                           size_t input_record_len,
                           WRBUF output_record)
{
    int ret = 0;
    WRBUF record = output_record; /* pointer transfer */
    struct yaz_record_conv_rule *r = p->rules;
    wrbuf_rewind(p->wr_error);
    
    wrbuf_write(record, input_record_buf, input_record_len);
    for (; ret == 0 && r; r = r->next)
    {
        if (r->which == YAZ_RECORD_CONV_RULE_XSLT)
        {
            xmlDocPtr doc = xmlParseMemory(wrbuf_buf(record),
                                           wrbuf_len(record));
            if (!doc)
            {
                wrbuf_printf(p->wr_error, "xmlParseMemory failed");
                ret = -1;
            }
            else
            {
                xmlDocPtr res = xsltApplyStylesheet(r->u.xslt.xsp, doc, 0);
                if (res)
                {
                    xmlChar *out_buf;
                    int out_len;
                    xmlDocDumpFormatMemory (res, &out_buf, &out_len, 1);

                    wrbuf_rewind(record);
                    wrbuf_write(record, (const char *) out_buf, out_len);

                    xmlFree(out_buf);
                    xmlFreeDoc(res);
                }
                else
                {
                    wrbuf_printf(p->wr_error, "xsltApplyStylesheet faailed");
                    ret = -1;
                }
                xmlFreeDoc(doc);
            }
        }
        else if (r->which == YAZ_RECORD_CONV_RULE_MARC)
        {
            yaz_marc_t mt = yaz_marc_create();

            yaz_marc_xml(mt, r->u.marc.output_format);

            if (r->u.marc.iconv_t)
                yaz_marc_iconv(mt, r->u.marc.iconv_t);
            if (r->u.marc.input_format == YAZ_MARC_ISO2709)
            {
                int sz = yaz_marc_read_iso2709(mt, wrbuf_buf(record),
                                               wrbuf_len(record));
                if (sz > 0)
                    ret = 0;
                else
                    ret = -1;
            }
            else if (r->u.marc.input_format == YAZ_MARC_MARCXML)
            {
                xmlDocPtr doc = xmlParseMemory(wrbuf_buf(record),
                                               wrbuf_len(record));
                if (!doc)
                {
                    wrbuf_printf(p->wr_error, "xmlParseMemory failed");
                    ret = -1;
                }
                else
                {
                    ret = yaz_marc_read_xml(mt, xmlDocGetRootElement(doc));
                    if (ret)
                        wrbuf_printf(p->wr_error, "yaz_marc_read_xml failed");
                }
                xmlFreeDoc(doc);
            }
            else
            {
                wrbuf_printf(p->wr_error, "unsupported input format");
                ret = -1;
            }
            if (ret == 0)
            {
                wrbuf_rewind(record);
                ret = yaz_marc_write_mode(mt, record);
                if (ret)
                    wrbuf_printf(p->wr_error, "yaz_marc_write_mode failed");
            }
            yaz_marc_destroy(mt);
        }
    }
    return ret;
}

const char *yaz_record_conv_get_error(yaz_record_conv_t p)
{
    return wrbuf_buf(p->wr_error);
}

void yaz_record_conv_set_path(yaz_record_conv_t p, const char *path)
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

