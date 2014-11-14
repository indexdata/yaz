/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file record_render.c
 * \brief Render Z39.50 records (NamePlusRecord)
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>
#include <errno.h>

#include <yaz/marcdisp.h>
#include <yaz/record_render.h>
#include <yaz/yaz-iconv.h>
#include <yaz/proto.h>
#include <yaz/oid_db.h>
#include <yaz/nmem_xml.h>
#include <yaz/base64.h>

#if YAZ_HAVE_XML2
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#endif

static yaz_iconv_t iconv_create_charset(const char *record_charset,
                                        yaz_iconv_t *cd2,
                                        const char *marc_buf,
                                        int sz)
{
    char charset_buf[40];
    yaz_iconv_t cd = 0;
    char *from_set1 = 0;
    char *from_set2 = 0;
    char *to_set = "utf-8";
    if (record_charset && *record_charset)
    {
        char *cp = charset_buf;

        strncpy(charset_buf, record_charset, sizeof(charset_buf)-1);
        charset_buf[sizeof(charset_buf)-1] = '\0';

        from_set1 = cp;
        while (*cp && *cp != ',' && *cp != '/')
            cp++;
        if (*cp == '/')
        {
            *cp++ = '\0'; /* terminate from_set1 */
            from_set2 = cp;
            while (*cp && *cp != ',')
                cp++;
        }
        if (*cp == ',')
        {
            *cp++ = '\0';  /* terminate from_set1 or from_set2 */
            to_set = cp;
            while (*cp)
                cp++;
        }
    }

    if (from_set1)
    {
        if (yaz_marc_check_marc21_coding(from_set1, marc_buf, sz))
            from_set1 = "utf-8";
        cd = yaz_iconv_open(to_set, from_set1);
    }
    if (cd2)
    {
        if (from_set2)
            *cd2 = yaz_iconv_open(to_set, from_set2);
        else
            *cd2 = 0;
    }
    return cd;
}

static const char *return_marc_record(WRBUF wrbuf,
                                      int marc_type,
                                      int *len,
                                      const char *buf, int sz,
                                      const char *record_charset)
{
    yaz_iconv_t cd = iconv_create_charset(record_charset, 0, buf, sz);
    yaz_marc_t mt = yaz_marc_create();
    const char *ret_string = 0;

    if (cd)
        yaz_marc_iconv(mt, cd);
    yaz_marc_xml(mt, marc_type);
    if (yaz_marc_decode_wrbuf(mt, buf, sz, wrbuf) > 0)
    {
        *len = wrbuf_len(wrbuf);
        ret_string = wrbuf_cstr(wrbuf);
    }
    yaz_marc_destroy(mt);
    if (cd)
        yaz_iconv_close(cd);
    return ret_string;
}

static const char *return_opac_record(WRBUF wrbuf,
                                      int marc_type,
                                      int *len,
                                      Z_OPACRecord *opac_rec,
                                      const char *record_charset)
{
    yaz_iconv_t cd, cd2;
    const char *marc_buf = 0;
    int marc_sz = 0;
    yaz_marc_t mt = yaz_marc_create();

    if (opac_rec->bibliographicRecord)
    {
        Z_External *ext = opac_rec->bibliographicRecord;
        if (ext->which == Z_External_octet)
        {
            marc_buf = (const char *) ext->u.octet_aligned->buf;
            marc_sz = ext->u.octet_aligned->len;
        }
    }
    cd = iconv_create_charset(record_charset, &cd2, marc_buf, marc_sz);

    if (cd)
        yaz_marc_iconv(mt, cd);
    yaz_marc_xml(mt, marc_type);

    if (cd2)
        yaz_opac_decode_wrbuf2(mt, opac_rec, wrbuf, cd2);
    else
        yaz_opac_decode_wrbuf(mt, opac_rec, wrbuf);

    yaz_marc_destroy(mt);

    if (cd)
        yaz_iconv_close(cd);
    if (cd2)
        yaz_iconv_close(cd2);
    *len = wrbuf_len(wrbuf);
    return wrbuf_cstr(wrbuf);
}

static const char *return_string_record(WRBUF wrbuf,
                                        int *len,
                                        const char *buf, int sz,
                                        const char *record_charset)
{
    yaz_iconv_t cd = iconv_create_charset(record_charset, 0, 0, 0);

    if (cd)
    {
        wrbuf_iconv_write(wrbuf, cd, buf, sz);
        wrbuf_iconv_reset(wrbuf, cd);

        buf = wrbuf_cstr(wrbuf);
        sz = wrbuf_len(wrbuf);
        yaz_iconv_close(cd);
    }
    *len = sz;
    return buf;
}

static const char *return_record_wrbuf(WRBUF wrbuf, int *len,
                                       Z_NamePlusRecord *npr,
                                       int marctype, const char *charset)
{
    Z_External *r = (Z_External *) npr->u.databaseRecord;
    const Odr_oid *oid = r->direct_reference;

    wrbuf_rewind(wrbuf);
    /* render bibliographic record .. */
    if (r->which == Z_External_OPAC)
    {
        return return_opac_record(wrbuf, marctype, len,
                                  r->u.opac, charset);
    }
    if (r->which == Z_External_sutrs)
        return return_string_record(wrbuf, len,
                                    (char*) r->u.sutrs->buf,
                                    r->u.sutrs->len,
                                    charset);
    else if (r->which == Z_External_octet)
    {
        if (oid_oidcmp(oid, yaz_oid_recsyn_xml)
            && oid_oidcmp(oid, yaz_oid_recsyn_application_xml)
            && oid_oidcmp(oid, yaz_oid_recsyn_mab)
            && oid_oidcmp(oid, yaz_oid_recsyn_html))
        {
            const char *ret_buf = return_marc_record(
                wrbuf, marctype, len,
                (const char *) r->u.octet_aligned->buf,
                r->u.octet_aligned->len,
                charset);
            if (ret_buf)
                return ret_buf;
            /* not ISO2709. Return fail unless raw (ISO2709) is wanted */
            if (yaz_oid_is_iso2709(oid) && marctype != YAZ_MARC_ISO2709)
                return 0;
        }
        return return_string_record(wrbuf, len,
                                    (const char *) r->u.octet_aligned->buf,
                                    r->u.octet_aligned->len,
                                    charset);
    }
    else if (r->which == Z_External_grs1)
    {
        yaz_display_grs1(wrbuf, r->u.grs1, 0);
        return return_string_record(wrbuf, len,
                                    wrbuf_buf(wrbuf),
                                    wrbuf_len(wrbuf),
                                    charset);
    }
    return 0;
}

static const char *get_record_format(WRBUF wrbuf, int *len,
                                     Z_NamePlusRecord *npr,
                                     int marctype, const char *charset,
                                     const char *format)
{
    const char *res = return_record_wrbuf(wrbuf, len, npr, marctype, charset);
#if YAZ_HAVE_XML2
    if (*format == '1')
    {
        /* try to XML format res */
        xmlDocPtr doc;
        xmlKeepBlanksDefault(0); /* get get xmlDocFormatMemory to work! */
        doc = xmlParseMemory(res, *len);
        if (doc)
        {
            xmlChar *xml_mem;
            int xml_size;
            xmlDocDumpFormatMemory(doc, &xml_mem, &xml_size, 1);
            wrbuf_rewind(wrbuf);
            wrbuf_write(wrbuf, (const char *) xml_mem, xml_size);
            xmlFree(xml_mem);
            xmlFreeDoc(doc);
            res = wrbuf_cstr(wrbuf);
            *len = wrbuf_len(wrbuf);
        }
    }
#endif
    return res;
}

#if YAZ_HAVE_XML2
static int replace_node(NMEM nmem, xmlNode *ptr,
                        const char *type_spec, char *record_buf)
{
    int ret = -1;
    const char *res;
    int len;
    int m_len;
    WRBUF wrbuf = wrbuf_alloc();
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_NamePlusRecord *npr = odr_malloc(odr, sizeof(*npr));
    npr->which = Z_NamePlusRecord_databaseRecord;

    if (atoi_n_check(record_buf, 5, &m_len))
        npr->u.databaseRecord =
            z_ext_record_usmarc(odr, record_buf, strlen(record_buf));
    else
        npr->u.databaseRecord =
            z_ext_record_xml(odr, record_buf, strlen(record_buf));
    res = yaz_record_render(npr, 0, wrbuf, type_spec, &len);
    if (res)
    {
        xmlDoc *doc = xmlParseMemory(res, strlen(res));
        if (doc)
        {
            xmlNode *nptr = xmlCopyNode(xmlDocGetRootElement(doc), 1);
            xmlReplaceNode(ptr, nptr);
            xmlFreeDoc(doc);
        }
        else
        {
            xmlNode *nptr = xmlNewText(BAD_CAST res);
            xmlReplaceNode(ptr, nptr);
        }
        ret = 0;
    }
    wrbuf_destroy(wrbuf);
    odr_destroy(odr);
    return ret;
}
#endif

static const char *base64_render(NMEM nmem, WRBUF wrbuf,
                                 const char *buf, int *len,
                                 const char *expr, const char *type_spec)
{
#if YAZ_HAVE_XML2
    xmlDocPtr doc = xmlParseMemory(buf, *len);
    if (doc)
    {
        xmlChar *buf_out;
        int len_out;
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
        if (xpathCtx)
        {
            xmlXPathObjectPtr xpathObj =
                xmlXPathEvalExpression((const xmlChar *) expr, xpathCtx);
            if (xpathObj)
            {
                xmlNodeSetPtr nodes = xpathObj->nodesetval;
                if (nodes)
                {
                    int i;
                    for (i = 0; i < nodes->nodeNr; i++)
                    {
                        xmlNode *ptr = nodes->nodeTab[i];
                        if (ptr->type == XML_TEXT_NODE)
                        {
                            const char *input =
                                nmem_text_node_cdata(ptr, nmem);
                            char *output = nmem_malloc(
                                nmem, strlen(input) + 1);
                            if (yaz_base64decode(input, output) == 0)
                            {
                                if (!replace_node(nmem, ptr, type_spec, output))
                                {
                                    /* replacement OK */
                                    xmlFreeNode(ptr);
                                    /* unset below to avoid a bad reference in
                                       xmlXPathFreeObject below */
                                    nodes->nodeTab[i] = 0;
                                }
                            }
                        }
                    }
                }
                xmlXPathFreeObject(xpathObj);
            }
            xmlXPathFreeContext(xpathCtx);
        }
        xmlDocDumpMemory(doc, &buf_out, &len_out);
        if (buf_out)
        {
            wrbuf_rewind(wrbuf);
            wrbuf_write(wrbuf, (const char *) buf_out, len_out);
            buf = wrbuf_cstr(wrbuf);
            *len = len_out;
        }
        xmlFreeDoc(doc);
        xmlFree(buf_out);
    }
#endif
    return buf;
}

const char *yaz_record_render(Z_NamePlusRecord *npr, const char *schema,
                              WRBUF wrbuf,
                              const char *type_spec, int *len)
{
    const char *ret = 0;
    NMEM nmem = 0;
    char *base64_xpath = 0;
    size_t i;
    char type[40];
    char charset[40];
    char format[3];
    const char *cp = type_spec;
    int len0;

    if (!len)
        len = &len0;

    for (i = 0; cp[i] && cp[i] != ';' && cp[i] != ' ' && i < sizeof(type)-1;
         i++)
        type[i] = cp[i];
    type[i] = '\0';
    charset[0] = '\0';
    format[0] = '\0';
    while (1)
    {
        while (cp[i] == ' ')
            i++;
        if (cp[i] != ';')
            break;
        i++;
        while (cp[i] == ' ')
            i++;
        if (!strncmp(cp + i, "charset=", 8))
        {
            size_t j = 0;
            i = i + 8; /* skip charset= */
            while (cp[i] == ' ')
                i++;
            for (j = 0; cp[i] && cp[i] != ';' && cp[i] != ' '; i++)
            {
                if (j < sizeof(charset)-1)
                    charset[j++] = cp[i];
            }
            charset[j] = '\0';
        }
        else if (!strncmp(cp + i, "format=", 7))
        {
            size_t j = 0;
            i = i + 7;
            while (cp[i] == ' ')
                i++;
            for (j = 0; cp[i] && cp[i] != ';' && cp[i] != ' '; i++)
            {
                if (j < sizeof(format)-1)
                    format[j++] = cp[i];
            }
            format[j] = '\0';
        }
        else if (!strncmp(cp + i, "base64=", 7))
        {
            size_t i0;
            i = i + 7;
            while (cp[i] == ' ')
                i++;
            i0 = i;
            while (cp[i] && cp[i] != ';')
                i++;

            nmem = nmem_create();
            base64_xpath = nmem_strdupn(nmem, cp + i0, i - i0);
        }
    }
    if (!strcmp(type, "database"))
    {
        *len = (npr->databaseName ? strlen(npr->databaseName) : 0);
        ret = npr->databaseName;
    }
    else if (!strcmp(type, "schema"))
    {
        *len = schema ? strlen(schema) : 0;
        ret = schema;
    }
    else if (!strcmp(type, "syntax"))
    {
        const char *desc = 0;
        if (npr->which == Z_NamePlusRecord_databaseRecord)
        {
            Z_External *r = (Z_External *) npr->u.databaseRecord;
            desc = yaz_oid_to_string(yaz_oid_std(), r->direct_reference, 0);
        }
        if (!desc)
            desc = "none";
        *len = strlen(desc);
        ret = desc;
    }
    if (npr->which != Z_NamePlusRecord_databaseRecord)
        ;
    else if (!strcmp(type, "render"))
    {
        ret = get_record_format(wrbuf, len, npr, YAZ_MARC_LINE, charset, format);
    }
    else if (!strcmp(type, "xml"))
    {
        ret = get_record_format(wrbuf, len, npr, YAZ_MARC_MARCXML, charset,
                                format);
    }
    else if (!strcmp(type, "txml"))
    {
        ret = get_record_format(wrbuf, len, npr, YAZ_MARC_TURBOMARC, charset,
                                format);
    }
    else if (!strcmp(type, "json"))
    {
        ret = get_record_format(wrbuf, len, npr, YAZ_MARC_JSON, charset,
                                format);
    }
    else if (!strcmp(type, "raw"))
    {
        ret = get_record_format(wrbuf, len, npr, YAZ_MARC_ISO2709, charset,
                                format);
    }
    else if (!strcmp(type, "ext"))
    {
        *len = -1;
        ret = (const char *) npr->u.databaseRecord;
    }
    else if (!strcmp(type, "opac"))
    {
        if (npr->u.databaseRecord->which == Z_External_OPAC)
            ret = get_record_format(wrbuf, len, npr, YAZ_MARC_MARCXML, charset,
                                    format);
    }

    if (base64_xpath && *len != -1)
    {
        char *type_spec = nmem_malloc(nmem,
                                      strlen(type) + strlen(charset) + 11);
        strcpy(type_spec, type);
        if (*charset)
        {
            strcat(type_spec, "; charset=");
            strcat(type_spec, charset);
        }
        ret = base64_render(nmem, wrbuf, ret, len, base64_xpath, type_spec);
    }
    nmem_destroy(nmem);
    return ret;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

