/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
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

static void insert_field(WRBUF w, const char *field, size_t length,
                         const char *attr)
{
    const char *cp0 = wrbuf_cstr(w);
    const char *cp = cp0;

    while (1)
    {
        const char *cp2 = strstr(cp, "@attr 1=");
        if (!cp2)
            break;
        cp = cp2 + 8;
        if (!strncmp(cp, field, length) &&
            (cp[length] == ' ' || cp[length] == ',' || cp[length] == '\0'))
        {
            /* found the field */

            cp += length;
            wrbuf_insert(w, cp - cp0, attr, strlen(attr));
            wrbuf_insert(w, cp - cp0, " ", 1);
            return;
        }
        while (*cp && *cp != ',')
            cp++;
    }
    if (wrbuf_len(w))
        wrbuf_puts(w, ",");
    wrbuf_puts(w, "@attr 1=");
    wrbuf_write(w, field, length);
    wrbuf_puts(w, " ");
    wrbuf_puts(w, attr);
}

void yaz_sru_facet_request(ODR o, Z_FacetList **facetList, const char **limit,
                           const char **start, const char **sort)
{
    if (o->direction == ODR_ENCODE)
    {
        Z_FacetList *fl = *facetList;
        if (fl)
        {
            WRBUF w_limit = wrbuf_alloc();
            int general_start = -1;
            int general_sortorder = -1;
            int general_limit = -1;
            int i;
            for (i = 0; i < fl->num; i++)
            {
                struct yaz_facet_attr av;
                yaz_facet_attr_init(&av);
                av.start = -1;
                av.sortorder = -1;
                av.limit = -1;
                yaz_facet_attr_get_z_attributes(fl->elements[i]->attributes,
                                                &av);
                if (av.errcode == 0)
                {
                    if (av.limit != -1)
                    {
                        if (av.useattr)
                        {
                            wrbuf_printf(w_limit, "%d:%s", av.limit,
                                         av.useattr);
                            wrbuf_puts(w_limit, ",");
                        }
                        else
                            general_limit = av.limit;
                    }
                    if (av.start != -1)
                        general_start = av.start;
                    if (av.sortorder != -1)
                        general_sortorder = av.sortorder;
                }
            }
            if (general_limit != -1)
            {
                char tmp[32];
                sprintf(tmp, "%d,", general_limit);
                wrbuf_insert(w_limit, 0, tmp, strlen(tmp));
            }
            if (wrbuf_len(w_limit) > 1)
            {
                wrbuf_cut_right(w_limit, 1);
                *limit = odr_strdup(o, wrbuf_cstr(w_limit));
            }
            if (general_start != -1)
            {
                char tmp[32];
                sprintf(tmp, "%d", general_start);
                *start = odr_strdup(o, tmp);
            }
            if (general_sortorder == 1)
            {
                *sort = odr_strdup(o, "alphanumeric");
            }
            wrbuf_destroy(w_limit);
        }
    }
    else if (o->direction == ODR_DECODE)
    {
        WRBUF w = wrbuf_alloc();
        int general_limit = -1;

        if (*limit)
        {
            const char *cp = *limit;
            int nor = 0;
            int val = 0;
            while (sscanf(cp, "%d%n", &val, &nor) >= 1 && nor > 0)
            {
                cp += nor;
                if (*cp == ':') /* field name follows */
                {
                    char tmp[40];
                    const char *cp0 = ++cp;
                    while (*cp && *cp != ',')
                        cp++;
                    sprintf(tmp, "@attr 3=%d", val);
                    insert_field(w, cp0, cp - cp0, tmp);

                    if (*start && strlen(*start) < 20)
                    {
                        sprintf(tmp, "@attr 4=%s", *start);
                        insert_field(w, cp0, cp - cp0, tmp);
                    }
                    if (*sort && !strcmp(*sort, "alphanumeric"))
                        insert_field(w, cp0, cp - cp0, "@attr 2=1");
                    else
                        insert_field(w, cp0, cp - cp0, "@attr 2=0");
                }
                else
                    general_limit = val;

                if (*cp != ',')
                    break;
                cp++;
            }
        }
        if (*sort || *start || general_limit != -1)
        {
            if (wrbuf_len(w))
                wrbuf_puts(w, ",");
            if (*sort && !strcmp(*sort, "alphanumeric"))
                wrbuf_printf(w, " @attr 2=1");
            else
                wrbuf_printf(w, " @attr 2=0");
            if (general_limit != -1)
                wrbuf_printf(w, " @attr 3=%d", general_limit);
            if (*start)
            {
                wrbuf_printf(w, " @attr 4=%s", *start);
            }
        }
        if (wrbuf_len(w))
            *facetList = yaz_pqf_parse_facet_list(o, wrbuf_cstr(w));
        else
            *facetList = 0;
        wrbuf_destroy(w);
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
            for (i = 0; i < fl->num; i++)
            {
                Z_FacetField *ff = fl->elements[i];
                xmlNode *p2 = xmlNewChild(p1, ns_fr, BAD_CAST "facet", 0);
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
    else if (o->direction == ODR_DECODE)
    {
        Z_FacetList *fl = (Z_FacetList *) odr_malloc(o, sizeof(*fl));
        xmlNode *p1;

        fl->num = 0;
        for (p1 = n->children; p1; p1 = p1->next)
            if (yaz_match_xsd_element(p1, "facet"))
                fl->num++;
        if (fl->num > 0)
        {
            int i = 0;
            *facetList = fl;
            fl->elements = (Z_FacetField **)
                odr_malloc(o, sizeof(*fl->elements) * fl->num);
            for (p1 = n->children; p1; p1 = p1->next)
                if (yaz_match_xsd_element(p1, "facet"))
                {
                    char *index_name = 0;
                    xmlNode *p_terms = 0;
                    xmlNode *p2 = p1->children;
                    Z_FacetField *ff = (Z_FacetField *)
                        odr_malloc(o, sizeof(*ff));
                    fl->elements[i++] = ff;
                    ff->attributes = 0;
                    ff->num_terms = 0;
                    ff->terms = 0;
                    for (; p2; p2 = p2->next)
                    {
                        if (yaz_match_xsd_string(p2, "index", o, &index_name))
                            ;
                        else if (yaz_match_xsd_element(p2, "terms"))
                            p_terms = p2;
                    }
                    if (index_name)
                        ff->attributes =
                            zget_AttributeList_use_string(o, index_name);
                    if (p_terms)
                    {
                        xmlNode *p;
                        int i = 0;
                        for (p = p_terms->children; p; p = p->next)
                        {
                            if (yaz_match_xsd_element(p, "term"))
                                ff->num_terms++;
                        }
                        if (ff->num_terms)
                            ff->terms = (Z_FacetTerm **)
                                odr_malloc(o,
                                           sizeof(*ff->terms) * ff->num_terms);
                        for (p = p_terms->children; p; p = p->next)
                        {
                            if (yaz_match_xsd_element(p, "term"))
                            {
                                char *cstr = 0;
                                Odr_int *count = 0;
                                xmlNode *p2 = p->children;
                                for (; p2; p2 = p2->next)
                                {
                                    if (yaz_match_xsd_string(p2, "actualTerm", o,
                                                         &cstr))
                                        ;
                                    else if (yaz_match_xsd_integer(p2, "count", o,
                                                                   &count))
                                        ;
                                    else
                                        ;
                                }
                                if (cstr && count)
                                {
                                    ff->terms[i++] =
                                        facet_term_create_cstr(o, cstr, *count);
                                }
                            }
                        }
                        ff->num_terms = i;
                        if (ff->num_terms == 0)
                            ff->terms = 0;
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

