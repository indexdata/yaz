/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
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

static yaz_iconv_t iconv_create_charset(const char *record_charset,
                                        yaz_iconv_t *cd2)
{
    char charset_buf[40];
    yaz_iconv_t cd = 0;
    char *from_set1 = 0;
    char *from_set2 = 0;
    char *to_set = 0;
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
        cd = yaz_iconv_open(to_set ? to_set : "UTF-8", from_set1);
    if (cd2)
    {
        if (from_set2)
            *cd2 = yaz_iconv_open(to_set ? to_set : "UTF-8", from_set2);
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
    yaz_iconv_t cd = iconv_create_charset(record_charset, 0);
    yaz_marc_t mt = yaz_marc_create();
    const char *ret_string = 0;

    if (cd)
        yaz_marc_iconv(mt, cd);
    yaz_marc_xml(mt, marc_type);
    if (yaz_marc_decode_wrbuf(mt, buf, sz, wrbuf) > 0)
    {
        if (len)
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
    yaz_iconv_t cd2;
    yaz_iconv_t cd = iconv_create_charset(record_charset, &cd2);
    yaz_marc_t mt = yaz_marc_create();

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
    if (len)
        *len = wrbuf_len(wrbuf);
    return wrbuf_cstr(wrbuf);
}

static const char *return_string_record(WRBUF wrbuf,
                                        int *len,
                                        const char *buf, int sz,
                                        const char *record_charset)
{
    yaz_iconv_t cd = iconv_create_charset(record_charset, 0);

    if (cd)
    {
        wrbuf_iconv_write(wrbuf, cd, buf, sz);
        wrbuf_iconv_reset(wrbuf, cd);

        buf = wrbuf_cstr(wrbuf);
        sz = wrbuf_len(wrbuf);
        yaz_iconv_close(cd);
    }
    if (len)
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
        if (yaz_oid_is_iso2709(oid))
        {
            const char *ret_buf = return_marc_record(
                wrbuf, marctype, len,
                (const char *) r->u.octet_aligned->buf,
                r->u.octet_aligned->len,
                charset);
            if (ret_buf)
                return ret_buf;
            /* bad ISO2709. Return fail unless raw (ISO2709) is wanted */
            if (marctype != YAZ_MARC_ISO2709)
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
    if (*format == '1' && len)
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

const char *yaz_record_render(Z_NamePlusRecord *npr, const char *schema,
                              WRBUF wrbuf,
                              const char *type_spec, int *len)
{
    size_t i;
    char type[40];
    char charset[40];
    char format[3];
    const char *cp = type_spec;

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
            for (j = 0; cp[i] && cp[i] != ';' && cp[i] != ' '; i++)
            {
                if (j < sizeof(format)-1)
                    format[j++] = cp[i];
            }
            format[j] = '\0';
        } 
    }
    if (!strcmp(type, "database"))
    {
        if (len)
            *len = (npr->databaseName ? strlen(npr->databaseName) : 0);
        return npr->databaseName;
    }
    else if (!strcmp(type, "schema"))
    {
        if (len)
            *len = schema ? strlen(schema) : 0;
        return schema;
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
        if (len)
            *len = strlen(desc);
        return desc;
    }
    if (npr->which != Z_NamePlusRecord_databaseRecord)
        return 0;

    /* from now on - we have a database record .. */
    if (!strcmp(type, "render"))
    {
        return get_record_format(wrbuf, len, npr, YAZ_MARC_LINE, charset, format);
    }
    else if (!strcmp(type, "xml"))
    {
        return get_record_format(wrbuf, len, npr, YAZ_MARC_MARCXML, charset,
                                 format);
    }
    else if (!strcmp(type, "txml"))
    {
        return get_record_format(wrbuf, len, npr, YAZ_MARC_TURBOMARC, charset,
                                 format);
    }
    else if (!strcmp(type, "raw"))
    {
        return get_record_format(wrbuf, len, npr, YAZ_MARC_ISO2709, charset,
            format);
    }
    else if (!strcmp(type, "ext"))
    {
        if (len) *len = -1;
        return (const char *) npr->u.databaseRecord;
    }
    else if (!strcmp(type, "opac"))
    {
        if (npr->u.databaseRecord->which == Z_External_OPAC)
            return get_record_format(wrbuf, len, npr, YAZ_MARC_MARCXML, charset,
                                     format);
    }
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

