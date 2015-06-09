/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
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
#include <yaz/z-opac.h>
#include <yaz/xml_get.h>

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#if YAZ_HAVE_XSLT
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>
#endif
#if YAZ_HAVE_EXSLT
#include <libexslt/exslt.h>
#endif

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

struct marc_info {
    NMEM nmem;
    const char *input_charset;
    const char *output_charset;
    int input_format_mode;
    int output_format_mode;
    const char *leader_spec;
};

/** \brief tranformation info (rule info) */
struct yaz_record_conv_rule {
    struct yaz_record_conv_type *type;
    void *info;
    struct yaz_record_conv_rule *next;
};

/** \brief reset rules+configuration */
static void yaz_record_conv_reset(yaz_record_conv_t p)
{

    struct yaz_record_conv_rule *r;
    for (r = p->rules; r; r = r->next)
    {
        r->type->destroy(r->info);
    }
    wrbuf_rewind(p->wr_error);
    nmem_reset(p->nmem);

    p->rules = 0;

    p->rules_p = &p->rules;
}

void yaz_record_conv_destroy(yaz_record_conv_t p)
{
    if (p)
    {
        yaz_record_conv_reset(p);
        nmem_destroy(p->nmem);
        wrbuf_destroy(p->wr_error);

        xfree(p->path);
        xfree(p);
    }
}

#if YAZ_HAVE_XSLT
struct xslt_info {
    NMEM nmem;
    xmlDocPtr xsp_doc;
    const char **xsl_parms;
};

static void *construct_xslt(const xmlNode *ptr,
                            const char *path, WRBUF wr_error)
{
    struct _xmlAttr *attr;
    const char *stylesheet = 0;
    struct xslt_info *info = 0;
    NMEM nmem = 0;
    int max_parms = 10;
    int no_parms = 0;

    if (strcmp((const char *) ptr->name, "xslt"))
        return 0;

    for (attr = ptr->properties; attr; attr = attr->next)
    {
        if (!xmlStrcmp(attr->name, BAD_CAST "stylesheet") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            stylesheet = (const char *) attr->children->content;
        else
        {
            wrbuf_printf(wr_error, "Bad attribute '%s'"
                         "Expected stylesheet.", attr->name);
            return 0;
        }
    }
    nmem = nmem_create();
    info = nmem_malloc(nmem, sizeof(*info));
    info->nmem = nmem;
    info->xsl_parms = nmem_malloc(
        nmem, (2 * max_parms + 1) * sizeof(*info->xsl_parms));

    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
        const char *name = 0;
        const char *value = 0;
        char *qvalue = 0;
        if (ptr->type != XML_ELEMENT_NODE)
            continue;
        if (strcmp((const char *) ptr->name, "param"))
        {
            wrbuf_printf(wr_error, "Bad element '%s'"
                         "Expected param.", ptr->name);
            nmem_destroy(nmem);
            return 0;
        }
        for (attr = ptr->properties; attr; attr = attr->next)
        {
            if (!xmlStrcmp(attr->name, BAD_CAST "name") &&
                attr->children && attr->children->type == XML_TEXT_NODE)
                name = (const char *) attr->children->content;
            else if (!xmlStrcmp(attr->name, BAD_CAST "value") &&
                attr->children && attr->children->type == XML_TEXT_NODE)
                value = (const char *) attr->children->content;
            else
            {
                wrbuf_printf(wr_error, "Bad attribute '%s'"
                             "Expected name or value.", attr->name);
                nmem_destroy(nmem);
                return 0;
            }
        }
        if (!name || !value)
        {
            wrbuf_printf(wr_error, "Missing attributes name or value");
            nmem_destroy(nmem);
            return 0;
        }
        if (no_parms >= max_parms)
        {
            wrbuf_printf(wr_error, "Too many parameters given");
            nmem_destroy(nmem);
            return 0;
        }

        qvalue = nmem_malloc(nmem, strlen(value) + 3);
        strcpy(qvalue, "\'");
        strcat(qvalue, value);
        strcat(qvalue, "\'");

        info->xsl_parms[2 * no_parms] = nmem_strdup(nmem, name);
        info->xsl_parms[2 * no_parms + 1] = qvalue;
        no_parms++;
    }
    info->xsl_parms[2 * no_parms] = 0;

    if (!stylesheet)
    {
        wrbuf_printf(wr_error, "Element <xslt>: "
                     "attribute 'stylesheet' expected");
        nmem_destroy(nmem);
    }
    else
    {
        char fullpath[1024];
        xsltStylesheetPtr xsp;
        if (!yaz_filepath_resolve(stylesheet, path, 0, fullpath))
        {
            wrbuf_printf(wr_error, "Element <xslt stylesheet=\"%s\"/>:"
                         " could not locate stylesheet '%s'",
                         stylesheet, stylesheet);
            if (path)
                wrbuf_printf(wr_error, " with path '%s'", path);

            nmem_destroy(nmem);
            return 0;
        }
        info->xsp_doc = xmlParseFile(fullpath);
        if (!info->xsp_doc)
        {
            wrbuf_printf(wr_error, "Element: <xslt stylesheet=\"%s\"/>:"
                         " xml parse failed: %s", stylesheet, fullpath);
            if (path)
                wrbuf_printf(wr_error, " with path '%s'", path);
            nmem_destroy(nmem);
            return 0;
        }
        /* need to copy this before passing it to the processor. It will
           be encapsulated in the xsp and destroyed by xsltFreeStylesheet */
        xsp = xsltParseStylesheetDoc(xmlCopyDoc(info->xsp_doc, 1));
        if (!xsp)
        {
            wrbuf_printf(wr_error, "Element: <xslt stylesheet=\"%s\"/>:"
                         " xslt parse failed: %s", stylesheet, fullpath);
            if (path)
                wrbuf_printf(wr_error, " with path '%s'", path);
            wrbuf_printf(wr_error, " ("
#if YAZ_HAVE_EXSLT

                         "EXSLT enabled"
#else
                         "EXSLT not supported"
#endif
                         ")");
            xmlFreeDoc(info->xsp_doc);
            nmem_destroy(info->nmem);
        }
        else
        {
            xsltFreeStylesheet(xsp);
            return info;
        }
    }
    return 0;
}

static int convert_xslt(void *vinfo, WRBUF record, WRBUF wr_error)
{
    int ret = 0;
    struct xslt_info *info = vinfo;

    xmlDocPtr doc = xmlParseMemory(wrbuf_buf(record),
                                   wrbuf_len(record));
    if (!doc)
    {
        wrbuf_printf(wr_error, "xmlParseMemory failed");
        ret = -1;
    }
    else
    {
        xmlDocPtr xsp_doc = xmlCopyDoc(info->xsp_doc, 1);
        xsltStylesheetPtr xsp = xsltParseStylesheetDoc(xsp_doc);
        xmlDocPtr res = xsltApplyStylesheet(xsp, doc, info->xsl_parms);
        if (res)
        {
            xmlChar *out_buf = 0;
            int out_len;

#if HAVE_XSLTSAVERESULTTOSTRING
            xsltSaveResultToString(&out_buf, &out_len, res, xsp);
#else
            xmlDocDumpFormatMemory (res, &out_buf, &out_len, 1);
#endif
            if (!out_buf)
            {
                wrbuf_printf(wr_error,
                             "xsltSaveResultToString failed");
                ret = -1;
            }
            else
            {
                wrbuf_rewind(record);
                wrbuf_write(record, (const char *) out_buf, out_len);

                xmlFree(out_buf);
            }
            xmlFreeDoc(res);
        }
        else
        {
            wrbuf_printf(wr_error, "xsltApplyStylesheet failed");
            ret = -1;
        }
        xmlFreeDoc(doc);
        xsltFreeStylesheet(xsp); /* frees xsp_doc too */
    }
    return ret;
}

static void destroy_xslt(void *vinfo)
{
    struct xslt_info *info = vinfo;

    if (info)
    {
        xmlFreeDoc(info->xsp_doc);
        nmem_destroy(info->nmem);
    }
}

/* YAZ_HAVE_XSLT */
#endif

struct select_info {
    NMEM nmem;
    char *xpath_expr;
};

static void *construct_select(const xmlNode *ptr,
                              const char *path, WRBUF wr_error)
{
    if (strcmp((const char *) ptr->name, "select"))
        return 0;
    else
    {
        NMEM nmem = nmem_create();
        struct select_info *info = nmem_malloc(nmem, sizeof(*info));
        const char *attr_str;
        const char *xpath = 0;

        info->nmem = nmem;
        info->xpath_expr = 0;
        attr_str = yaz_xml_get_prop(ptr, "path%s", &xpath);
        if (attr_str)
        {
            wrbuf_printf(wr_error, "Bad attribute '%s'"
                         "Expected xpath.", attr_str);
            nmem_destroy(nmem);
                return 0;
        }
        if (xpath)
            info->xpath_expr = nmem_strdup(nmem, xpath);
        return info;
    }
}

static int convert_select(void *vinfo, WRBUF record, WRBUF wr_error)
{
    int ret = 0;
    struct select_info *info = vinfo;

    xmlDocPtr doc = xmlParseMemory(wrbuf_buf(record),
                                   wrbuf_len(record));
    if (!doc)
    {
        wrbuf_printf(wr_error, "xmlParseMemory failed");
        ret = -1;
    }
    else
    {
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
        if (xpathCtx && info->xpath_expr)
        {
            xmlXPathObjectPtr xpathObj =
                xmlXPathEvalExpression((const xmlChar *) info->xpath_expr,
                                       xpathCtx);
            if (xpathObj)
            {
                xmlNodeSetPtr nodes = xpathObj->nodesetval;
                if (nodes)
                {
                    int i;
                    if (nodes->nodeNr > 0)
                        wrbuf_rewind(record);
                    for (i = 0; i < nodes->nodeNr; i++)
                    {
                        xmlNode *ptr = nodes->nodeTab[i];
                        if (ptr->type == XML_ELEMENT_NODE)
                            ptr = ptr->children;
                        for (; ptr; ptr = ptr->next)
                            if (ptr->type == XML_TEXT_NODE)
                                wrbuf_puts(record, (const char *) ptr->content);
                    }
                }
                xmlXPathFreeObject(xpathObj);
            }
            xmlXPathFreeContext(xpathCtx);
        }
        xmlFreeDoc(doc);
    }
    return ret;
}

static void destroy_select(void *vinfo)
{
    struct select_info *info = vinfo;

    if (info)
        nmem_destroy(info->nmem);
}


static void *construct_solrmarc(const xmlNode *ptr,
                                const char *path, WRBUF wr_error)
{
    if (strcmp((const char *) ptr->name, "solrmarc"))
        return 0;
    return wr_error; /* any non-null ptr will do; we don't use it later*/
}

static int convert_solrmarc(void *info, WRBUF record, WRBUF wr_error)
{
    WRBUF w = wrbuf_alloc();
    const char *buf = wrbuf_buf(record);
    size_t i, sz = wrbuf_len(record);
    for (i = 0; i < sz; i++)
    {
        int ch;
        if (buf[i] == '#' && i < sz - 3 && buf[i+3] == ';'
            && atoi_n_check(buf+i+1, 2, &ch))
            i += 3;
        else
            ch = buf[i];
        wrbuf_putc(w, ch);
    }
    wrbuf_rewind(record);
    wrbuf_write(record, wrbuf_buf(w), wrbuf_len(w));
    wrbuf_destroy(w);
    return 0;
}

static void destroy_solrmarc(void *info)
{
}

static void *construct_marc(const xmlNode *ptr,
                            const char *path, WRBUF wr_error)
{
    NMEM nmem = nmem_create();
    struct marc_info *info = nmem_malloc(nmem, sizeof(*info));
    struct _xmlAttr *attr;
    const char *input_format = 0;
    const char *output_format = 0;

    if (strcmp((const char *) ptr->name, "marc"))
    {
        nmem_destroy(nmem);
        return 0;
    }
    info->nmem = nmem;
    info->input_charset = 0;
    info->output_charset = 0;
    info->input_format_mode = 0;
    info->output_format_mode = 0;
    info->leader_spec = 0;

    for (attr = ptr->properties; attr; attr = attr->next)
    {
        if (!xmlStrcmp(attr->name, BAD_CAST "inputcharset") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            info->input_charset = (const char *) attr->children->content;
        else if (!xmlStrcmp(attr->name, BAD_CAST "outputcharset") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            info->output_charset = (const char *) attr->children->content;
        else if (!xmlStrcmp(attr->name, BAD_CAST "inputformat") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            input_format = (const char *) attr->children->content;
        else if (!xmlStrcmp(attr->name, BAD_CAST "outputformat") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
            output_format = (const char *) attr->children->content;
        else if (!xmlStrcmp(attr->name, BAD_CAST "leaderspec") &&
                 attr->children && attr->children->type == XML_TEXT_NODE)
            info->leader_spec =
                nmem_strdup(info->nmem,(const char *) attr->children->content);
        else
        {
            wrbuf_printf(wr_error, "Element <marc>: expected attributes"
                         "'inputformat', 'inputcharset', 'outputformat' or"
                         " 'outputcharset', got attribute '%s'",
                         attr->name);
            nmem_destroy(info->nmem);
            return 0;
        }
    }
    if (!input_format)
    {
        wrbuf_printf(wr_error, "Element <marc>: "
                     "attribute 'inputformat' required");
        nmem_destroy(info->nmem);
        return 0;
    }
    else if (!strcmp(input_format, "marc"))
    {
        info->input_format_mode = YAZ_MARC_ISO2709;
    }
    else if (!strcmp(input_format, "xml"))
    {
        info->input_format_mode = YAZ_MARC_MARCXML;
        /** Libxml2 generates UTF-8 encoding by default .
            So we convert from UTF-8 to outputcharset (if defined)
        */
        if (!info->input_charset && info->output_charset)
            info->input_charset = "utf-8";
    }
    else if (!strcmp(input_format, "json"))
    {
        info->input_format_mode = YAZ_MARC_JSON;
    }
    else
    {
        wrbuf_printf(wr_error, "Element <marc inputformat='%s'>: "
                     " Unsupported input format"
                     " defined by attribute value",
                     input_format);
        nmem_destroy(info->nmem);
        return 0;
    }

    if (!output_format)
    {
        wrbuf_printf(wr_error,
                     "Element <marc>: attribute 'outputformat' required");
        nmem_destroy(info->nmem);
        return 0;
    }
    else if (!strcmp(output_format, "line"))
    {
        info->output_format_mode = YAZ_MARC_LINE;
    }
    else if (!strcmp(output_format, "marcxml"))
    {
        info->output_format_mode = YAZ_MARC_MARCXML;
        if (info->input_charset && !info->output_charset)
            info->output_charset = "utf-8";
    }
    else if (!strcmp(output_format, "turbomarc"))
    {
        info->output_format_mode = YAZ_MARC_TURBOMARC;
        if (info->input_charset && !info->output_charset)
            info->output_charset = "utf-8";
    }
    else if (!strcmp(output_format, "marc"))
    {
        info->output_format_mode = YAZ_MARC_ISO2709;
    }
    else if (!strcmp(output_format, "marcxchange"))
    {
        info->output_format_mode = YAZ_MARC_XCHANGE;
        if (info->input_charset && !info->output_charset)
            info->output_charset = "utf-8";
    }
    else if (!strcmp(output_format, "json"))
    {
        info->output_format_mode = YAZ_MARC_JSON;
        if (info->input_charset && !info->output_charset)
            info->output_charset = "utf-8";
    }
    else
    {
        wrbuf_printf(wr_error, "Element <marc outputformat='%s'>: "
                     " Unsupported output format"
                     " defined by attribute value",
                     output_format);
        nmem_destroy(info->nmem);
        return 0;
    }
    if (info->input_charset && info->output_charset)
    {
        yaz_iconv_t cd = yaz_iconv_open(info->output_charset,
                                        info->input_charset);
        if (!cd)
        {
            wrbuf_printf(wr_error,
                         "Element <marc inputcharset='%s' outputcharset='%s'>:"
                         " Unsupported character set mapping"
                         " defined by attribute values",
                         info->input_charset, info->output_charset);
            nmem_destroy(info->nmem);
            return 0;
        }
        yaz_iconv_close(cd);
    }
    else if (!info->output_charset)
    {
        wrbuf_printf(wr_error, "Element <marc>: "
                     "attribute 'outputcharset' missing");
        nmem_destroy(info->nmem);
        return 0;
    }
    else if (!info->input_charset)
    {
        wrbuf_printf(wr_error, "Element <marc>: "
                     "attribute 'inputcharset' missing");
        nmem_destroy(info->nmem);
        return 0;
    }
    info->input_charset = nmem_strdup(info->nmem, info->input_charset);
    info->output_charset = nmem_strdup(info->nmem, info->output_charset);
    return info;
}

static int convert_marc(void *info, WRBUF record, WRBUF wr_error)
{
    struct marc_info *mi = info;
    const char *input_charset = mi->input_charset;
    int ret = 0;
    yaz_marc_t mt = yaz_marc_create();

    yaz_marc_xml(mt, mi->output_format_mode);
    if (mi->leader_spec)
        yaz_marc_leader_spec(mt, mi->leader_spec);

    if (mi->input_format_mode == YAZ_MARC_ISO2709)
    {
        int sz = yaz_marc_read_iso2709(mt, wrbuf_buf(record),
                                       wrbuf_len(record));
        if (sz > 0)
        {
            if (yaz_marc_check_marc21_coding(input_charset, wrbuf_buf(record),
                                             wrbuf_len(record)))
                input_charset = "utf-8";
            ret = 0;
        }
        else
            ret = -1;
    }
    else if (mi->input_format_mode == YAZ_MARC_MARCXML ||
             mi->input_format_mode == YAZ_MARC_TURBOMARC)
    {
        xmlDocPtr doc = xmlParseMemory(wrbuf_buf(record),
                                       wrbuf_len(record));
        if (!doc)
        {
            wrbuf_printf(wr_error, "xmlParseMemory failed");
            ret = -1;
        }
        else
        {
            ret = yaz_marc_read_xml(mt, xmlDocGetRootElement(doc));
            if (ret)
                wrbuf_printf(wr_error, "yaz_marc_read_xml failed");
        }
        xmlFreeDoc(doc);
    }
    else
    {
        wrbuf_printf(wr_error, "unsupported input format");
        ret = -1;
    }
    if (ret == 0)
    {
        yaz_iconv_t cd = yaz_iconv_open(mi->output_charset, input_charset);

        if (cd)
            yaz_marc_iconv(mt, cd);

        wrbuf_rewind(record);
        ret = yaz_marc_write_mode(mt, record);
        if (ret)
            wrbuf_printf(wr_error, "yaz_marc_write_mode failed");
        if (cd)
            yaz_iconv_close(cd);
    }
    yaz_marc_destroy(mt);
    return ret;
}

static void destroy_marc(void *info)
{
    struct marc_info *mi = info;

    nmem_destroy(mi->nmem);
}

int yaz_record_conv_configure_t(yaz_record_conv_t p, const xmlNode *ptr,
                                struct yaz_record_conv_type *types)
{
    struct yaz_record_conv_type bt[4];
    size_t i = 0;

    /* register marc */
    bt[i].construct = construct_marc;
    bt[i].convert = convert_marc;
    bt[i++].destroy = destroy_marc;

    bt[i-1].next = &bt[i];
    bt[i].construct = construct_solrmarc;
    bt[i].convert = convert_solrmarc;
    bt[i++].destroy = destroy_solrmarc;

    bt[i-1].next = &bt[i];
    bt[i].construct = construct_select;
    bt[i].convert = convert_select;
    bt[i++].destroy = destroy_select;

#if YAZ_HAVE_XSLT
    /* register xslt */
    bt[i-1].next = &bt[i];
    bt[i].construct = construct_xslt;
    bt[i].convert = convert_xslt;
    bt[i++].destroy = destroy_xslt;
#endif

    bt[i-1].next = types;
    yaz_record_conv_reset(p);

    /* parsing element children */
    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
        struct yaz_record_conv_type *t;
        struct yaz_record_conv_rule *r;
        void *info = 0;
        if (ptr->type != XML_ELEMENT_NODE)
            continue;
        for (t = &bt[0]; t; t = t->next)
        {
            wrbuf_rewind(p->wr_error);
            info = t->construct(ptr, p->path, p->wr_error);

            if (info || wrbuf_len(p->wr_error))
                break;
            /* info== 0 and no error reported , ie not handled by it */
        }
        if (!info)
        {
            if (wrbuf_len(p->wr_error) == 0)
                wrbuf_printf(p->wr_error, "Element <backend>: expected "
                             "<marc> or <xslt> element, got <%s>"
                             , ptr->name);
            return -1;
        }
        r = (struct yaz_record_conv_rule *) nmem_malloc(p->nmem, sizeof(*r));
        r->next = 0;
        r->info = info;
        r->type = nmem_malloc(p->nmem, sizeof(*t));
        memcpy(r->type, t, sizeof(*t));
        *p->rules_p = r;
        p->rules_p = &r->next;
    }
    return 0;
}

int yaz_record_conv_configure(yaz_record_conv_t p, const xmlNode *ptr)
{
    return yaz_record_conv_configure_t(p, ptr, 0);
}

static int yaz_record_conv_record_rule(yaz_record_conv_t p,
                                       struct yaz_record_conv_rule *r,
                                       const char *input_record_buf,
                                       size_t input_record_len,
                                       WRBUF output_record)
{
    int ret = 0;
    WRBUF record = output_record; /* pointer transfer */
    wrbuf_rewind(p->wr_error);

    wrbuf_write(record, input_record_buf, input_record_len);
    for (; ret == 0 && r; r = r->next)
        ret = r->type->convert(r->info, record, p->wr_error);
    return ret;
}

int yaz_record_conv_opac_record(yaz_record_conv_t p,
                                Z_OPACRecord *input_record,
                                WRBUF output_record)
{
    int ret = 0;
    struct yaz_record_conv_rule *r = p->rules;
    if (!r || r->type->construct != construct_marc)
    {
        wrbuf_puts(p->wr_error, "Expecting MARC rule as first rule for OPAC");
        ret = -1; /* no marc rule so we can't do OPAC */
    }
    else
    {
        struct marc_info *mi = r->info;
        const char *input_charset = mi->input_charset;
        yaz_iconv_t cd;

        WRBUF res = wrbuf_alloc();
        yaz_marc_t mt = yaz_marc_create();

        if (yaz_opac_check_marc21_coding(input_charset, input_record))
            input_charset = "utf-8";
        cd = yaz_iconv_open(mi->output_charset, input_charset);

        wrbuf_rewind(p->wr_error);
        yaz_marc_xml(mt, mi->output_format_mode);

        yaz_marc_iconv(mt, cd);

        yaz_opac_decode_wrbuf(mt, input_record, res);
        if (ret != -1)
        {
            ret = yaz_record_conv_record_rule(p,
                                              r->next,
                                              wrbuf_buf(res), wrbuf_len(res),
                                              output_record);
        }
        yaz_marc_destroy(mt);
        if (cd)
            yaz_iconv_close(cd);
        wrbuf_destroy(res);
    }
    return ret;
}

int yaz_record_conv_record(yaz_record_conv_t p,
                           const char *input_record_buf,
                           size_t input_record_len,
                           WRBUF output_record)
{
    return yaz_record_conv_record_rule(p, p->rules,
                                       input_record_buf,
                                       input_record_len, output_record);
}

const char *yaz_record_conv_get_error(yaz_record_conv_t p)
{
    return wrbuf_cstr(p->wr_error);
}

void yaz_record_conv_set_path(yaz_record_conv_t p, const char *path)
{
    xfree(p->path);
    p->path = 0;
    if (path)
        p->path = xstrdup(path);
}

yaz_record_conv_t yaz_record_conv_create()
{
    yaz_record_conv_t p = (yaz_record_conv_t) xmalloc(sizeof(*p));
    p->nmem = nmem_create();
    p->wr_error = wrbuf_alloc();
    p->rules = 0;
    p->path = 0;
    return p;
}

/* YAZ_HAVE_XML2 */
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

