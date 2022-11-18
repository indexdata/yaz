/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file marc_read_sax.c
 * \brief Implements reading of MARCXML using SAX parsing.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <yaz/log.h>
#include <yaz/marc_sax.h>
#include <yaz/wrbuf.h>
#include <yaz/nmem_xml.h>

#define MAX_IND 9

#if YAZ_HAVE_XML2

struct yaz_marc_sax_t_
{
        xmlSAXHandler saxHandler;
        WRBUF cdata;
        WRBUF tag;
        void (*cb_func)(yaz_marc_t, void *);
        void *cb_data;
        yaz_marc_t mt;
        WRBUF indicators;
        int indicator_length;
};

static const char *get_attribute(const char *name, int nb_attributes, const xmlChar **attributes,
        size_t *len)
{
        for (int i = 0; i < nb_attributes; i++)
        {        
                if (strcmp(name, (const char *)attributes[5 * i]) == 0)
                {
                        const char *start = (const char *) attributes[5 * i + 3];
                        const char *end = (const char *) attributes[5 * i + 4];
                        *len = end - start;
                        return start;
                }
        }
        return 0;
}

static void get_indicators(yaz_marc_sax_t ctx, int nb_attributes, const xmlChar **attributes) {
        char ind_cstr[5];
        strcpy(ind_cstr, "indX");
        wrbuf_rewind(ctx->indicators);
        for (int i = 0; i < ctx->indicator_length; i++)
        {
                size_t ind_len;
                const char *ind_value;
                ind_cstr[3] = '1' + i;
                ind_value = get_attribute(ind_cstr, nb_attributes, attributes, &ind_len);
                if (ind_len == 0) {
                        wrbuf_putc(ctx->indicators, ' ');
                } else {
                        wrbuf_write(ctx->indicators, ind_value, ind_len);
                }
        }
}

static void startElementNs(void *vp,
                           const xmlChar *localname, const xmlChar *prefix,
                           const xmlChar *URI, int nb_namespaces, const xmlChar **namespaces,
                           int nb_attributes, int nb_defaulted,
                           const xmlChar **attributes)
{
        yaz_marc_sax_t ctx = vp;

        wrbuf_rewind(ctx->cdata);
        if (strcmp((const char *)localname, "controlfield") == 0)
        {
                size_t tag_len;
                const char *tag_buf = get_attribute("tag", nb_attributes, attributes, &tag_len);
                wrbuf_rewind(ctx->tag);
                if (tag_buf) 
                        wrbuf_write(ctx->tag, tag_buf, tag_len);
        }
        else if (strcmp((const char *)localname, "datafield") == 0)
        {
                size_t tag_len;
                const char *tag_buf = get_attribute("tag", nb_attributes, attributes, &tag_len);
                wrbuf_rewind(ctx->tag);
                if (tag_buf) 
                        wrbuf_write(ctx->tag, tag_buf, tag_len);
                get_indicators(ctx, nb_attributes, attributes);
                yaz_marc_add_datafield(ctx->mt, wrbuf_cstr(ctx->tag),
                        wrbuf_buf(ctx->indicators), wrbuf_len(ctx->indicators));
        }
        else if (strcmp((const char *)localname, "subfield") == 0)
        {
                size_t code_len;
                const char *code_buf = get_attribute("code", nb_attributes, attributes, &code_len);
                yaz_log(YLOG_LOG, "code=%.*s", (int) code_len, code_buf);
                if (code_buf) 
                        wrbuf_write(ctx->cdata, code_buf, code_len);
        }
        else if (strcmp((const char *)localname, "record") == 0)
        {
                yaz_marc_reset(ctx->mt);
        }
}

static void endElementNs(void *vp,
                         const xmlChar *localname, const xmlChar *prefix, const xmlChar *URI)
{
        yaz_marc_sax_t ctx = vp;
        if (strcmp((const char *)localname, "leader") == 0)
        {
                int identifier_length;
                int base_address;
                int length_data_entry;
                int length_starting;
                int length_implementation;
                yaz_marc_set_leader(ctx->mt, wrbuf_cstr(ctx->cdata),
                                    &ctx->indicator_length, &identifier_length, &base_address,
                                    &length_data_entry, &length_starting, &length_implementation);
        }
        else if (strcmp((const char *)localname, "controlfield") == 0)
        {
                yaz_marc_add_controlfield(ctx->mt, wrbuf_cstr(ctx->tag),
                                          wrbuf_buf(ctx->cdata), wrbuf_len(ctx->cdata));
        }
        else if (strcmp((const char *)localname, "subfield") == 0)
        {
                yaz_marc_add_subfield(ctx->mt, wrbuf_buf(ctx->cdata), wrbuf_len(ctx->cdata));
        }
        else if (strcmp((const char *)localname, "record") == 0)
        {
                ctx->cb_func(ctx->mt, ctx->cb_data);
        }
        wrbuf_rewind(ctx->cdata);
}

static void characters(void *vp, const xmlChar *text, int len)
{
        yaz_marc_sax_t ctx = vp;
        wrbuf_write(ctx->cdata, (const char *)text, len);
}

yaz_marc_sax_t yaz_marc_sax_new(yaz_marc_t mt, void (*cb)(yaz_marc_t, void *), void *cb_data)
{
        yaz_marc_sax_t ctx = xmalloc(sizeof(*ctx));

        ctx->mt = mt;
        ctx->cb_data = cb_data;
        ctx->cb_func = cb;
        ctx->cdata = wrbuf_alloc();
        ctx->tag = wrbuf_alloc();
        ctx->indicators = wrbuf_alloc();
        memset(&ctx->saxHandler, 0, sizeof(ctx->saxHandler));
        ctx->saxHandler.initialized = XML_SAX2_MAGIC;
        ctx->saxHandler.startElementNs = startElementNs;
        ctx->saxHandler.endElementNs = endElementNs;
        ctx->saxHandler.characters = characters;
        return ctx;
}

xmlSAXHandler *yaz_marc_sax_get(yaz_marc_sax_t ctx)
{
        return &ctx->saxHandler;
}

void yaz_marc_sax_destroy(yaz_marc_sax_t ctx)
{
        wrbuf_destroy(ctx->indicators);
        wrbuf_destroy(ctx->cdata);
        wrbuf_destroy(ctx->tag);
        xfree(ctx);
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
