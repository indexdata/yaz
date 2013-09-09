/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2013 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file sru_facet.c
 * \brief Implements SRU 2.0 facets
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <yaz/srw.h>
#include <yaz/wrbuf.h>
#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <assert.h>
#endif

#include "sru-p.h"
#include <yaz/pquery.h>
#include <yaz/facet.h>

void yaz_sru_facet_request(ODR o, Z_FacetList **facetList, const char **limit)
{
    if (o->direction == ODR_ENCODE)
    {
        Z_FacetList *fl = *facetList;
        if (fl)
        {
            int i;
            WRBUF w = wrbuf_alloc();
            for (i = 0; i < fl->num; i++)
            {
                struct yaz_facet_attr av;
                yaz_facet_attr_init(&av);
                yaz_facet_attr_get_z_attributes(fl->elements[i]->attributes,
                                                &av);
                if (av.errcode == 0)
                {
                    wrbuf_printf(w, "%d", av.limit ? av.limit : -1);
                    if (av.useattr)
                        wrbuf_printf(w, ":%s,", av.useattr);
                    /* av.relation not considered yet */
                }
            }
            if (wrbuf_len(w) > 0)
            {
                wrbuf_cut_right(w, 1); /* remove , */
                *limit = odr_strdup(o, wrbuf_cstr(w));
            }
            wrbuf_destroy(w);
        }
    }
    else if (o->direction == ODR_DECODE)
    {
        const char *cp = *limit;
        *facetList = 0;
        if (cp)
        {
            int nor = 0;
            int limit_val = 0;
            WRBUF w = wrbuf_alloc();
            while (sscanf(cp, "%d%n", &limit_val, &nor) >= 1 && nor > 0)
            {
                cp += nor;
                if (wrbuf_len(w))
                    wrbuf_puts(w, ",");
                if (*cp == ':') /* field name follows */
                {
                    wrbuf_puts(w, "@attr 1=");
                    while (*++cp && *cp != ',')
                        wrbuf_putc(w, *cp);
                    wrbuf_puts(w, " ");
                }
                if (limit_val != -1)
                    wrbuf_printf(w, "@attr 3=%d", limit_val);
                if (*cp != ',')
                    break;
                cp++;
            }
            if (wrbuf_len(w))
                *facetList = yaz_pqf_parse_facet_list(o, wrbuf_cstr(w));
            wrbuf_destroy(w);
        }
    }
}

#if YAZ_HAVE_XML2
void yaz_sru_facet_response(ODR o, Z_FacetList **facetList, xmlNodePtr n)
{
    if (o->direction == ODR_ENCODE)
    {
        Z_FacetList *fl = *facetList;
        if (fl)
        {
            int i;
            const char *ns =
                "http://docs.oasis-open.org/ns/search-ws/facetedResults";
            xmlNode *p1 = xmlNewChild(n, 0, BAD_CAST "facetedResults", 0);
            xmlNsPtr ns_fr = xmlNewNs(p1, BAD_CAST ns, BAD_CAST "fr");
            xmlSetNs(p1, ns_fr);
            for (i = 0; i < fl->num; i++)
            {
                Z_FacetField *ff = fl->elements[i];
                xmlNode *p2 = xmlNewChild(p1, 0, BAD_CAST "facet", 0);
                int j;
                xmlNode *p3;
                struct yaz_facet_attr av;
                yaz_facet_attr_init(&av);
                yaz_facet_attr_get_z_attributes(ff->attributes, &av);
                add_xsd_string(p2, "index", av.useattr);
                p3 = xmlNewChild(p2, 0, BAD_CAST "terms", 0);
                for (j = 0; j < ff->num_terms; j++)
                {
                    Z_FacetTerm *ft = ff->terms[j];
                    Z_Term *zt = ft->term;
                    xmlNode *p4 = xmlNewChild(p3, 0, BAD_CAST "term", 0);
                    if (zt->which == Z_Term_general)
                        add_xsd_string_n(p4, "actualTerm",
                                         (char *) zt->u.general->buf,
                                         zt->u.general->len);
                    if (ft->count)
                        add_xsd_integer(p4, "count", ft->count);
                }
            }
        }
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

