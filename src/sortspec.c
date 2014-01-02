/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file sortspec.c
 * \brief Implements SortSpec parsing.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <yaz/matchstr.h>

#include <yaz/z-core.h>
#include <yaz/sortspec.h>
#include <yaz/oid_db.h>
#include <yaz/wrbuf.h>

Z_SortKeySpecList *yaz_sort_spec(ODR out, const char *arg)
{
    char sort_string_buf[64], sort_flags[64];
    Z_SortKeySpecList *sksl = (Z_SortKeySpecList *)
        odr_malloc(out, sizeof(*sksl));
    int off;

    sksl->num_specs = 0;
    sksl->specs = (Z_SortKeySpec **)odr_malloc(out, sizeof(sksl->specs) * 20);

    while ((sscanf(arg, "%63s %63s%n", sort_string_buf,
                   sort_flags, &off)) == 2  && off > 1)
    {
        int i;
        char *sort_string_sep;
        char *sort_string = sort_string_buf;
        Z_SortKeySpec *sks = (Z_SortKeySpec *) odr_malloc(out, sizeof(*sks));
        Z_SortKey *sk = (Z_SortKey *) odr_malloc(out, sizeof(*sk));

        arg += off;
        sksl->specs[sksl->num_specs++] = sks;
        sks->sortElement = (Z_SortElement *)
            odr_malloc(out, sizeof(*sks->sortElement));
        sks->sortElement->which = Z_SortElement_generic;
        sks->sortElement->u.generic = sk;

        if ((sort_string_sep = strchr(sort_string, '=')))
        {
            int i = 0;
            sk->which = Z_SortKey_sortAttributes;
            sk->u.sortAttributes = (Z_SortAttributes *)
                odr_malloc(out, sizeof(*sk->u.sortAttributes));
            sk->u.sortAttributes->id = odr_oiddup(out, yaz_oid_attset_bib_1);
            sk->u.sortAttributes->list = (Z_AttributeList *)
                odr_malloc(out, sizeof(*sk->u.sortAttributes->list));
            sk->u.sortAttributes->list->attributes = (Z_AttributeElement **)
                odr_malloc(out, 10 *
                            sizeof(*sk->u.sortAttributes->list->attributes));
            while (i < 10 && sort_string && sort_string_sep)
            {
                Z_AttributeElement *el = (Z_AttributeElement *)
                    odr_malloc(out, sizeof(*el));
                sk->u.sortAttributes->list->attributes[i] = el;
                el->attributeSet = 0;
                el->attributeType = odr_intdup(out, atoi(sort_string));
                el->which = Z_AttributeValue_numeric;
                el->value.numeric =
                    odr_intdup(out, odr_atoi(sort_string_sep + 1));
                i++;
                sort_string = strchr(sort_string, ',');
                if (sort_string)
                {
                    sort_string++;
                    sort_string_sep = strchr(sort_string, '=');
                }
            }
            sk->u.sortAttributes->list->num_attributes = i;
        }
        else
        {
            sk->which = Z_SortKey_sortField;
            sk->u.sortField = odr_strdup (out, sort_string);
        }
        sks->sortRelation = odr_intdup(out, Z_SortKeySpec_ascending);
        sks->caseSensitivity = odr_intdup(out, Z_SortKeySpec_caseInsensitive);

        sks->which = Z_SortKeySpec_null;
        sks->u.null = odr_nullval ();

        for (i = 0; sort_flags[i]; i++)
        {
            switch (sort_flags[i])
            {
            case 'd':
            case 'D':
            case '>':
                *sks->sortRelation = Z_SortKeySpec_descending;
                break;
            case 'a':
            case 'A':
            case '<':
                *sks->sortRelation = Z_SortKeySpec_ascending;
                break;
            case 'i':
            case 'I':
                *sks->caseSensitivity = Z_SortKeySpec_caseInsensitive;
                break;
            case 'S':
            case 's':
                *sks->caseSensitivity = Z_SortKeySpec_caseSensitive;
                break;
            case '!':
                sks->which = Z_SortKeySpec_abort;
                sks->u.abort = odr_nullval();
                break;
            case '=':
                sks->which = Z_SortKeySpec_missingValueData;
                sks->u.missingValueData = (Odr_oct*)
                    odr_malloc(out, sizeof(Odr_oct));
                i++;
                sks->u.missingValueData->len = strlen(sort_flags+i);
                sks->u.missingValueData->buf = odr_strdup(out, sort_flags+i);
                i += strlen(sort_flags+i) - 1;
                break;
            }
        }
    }
    if (!sksl->num_specs)
        return 0;
    return sksl;
}

int yaz_sort_spec_to_cql(Z_SortKeySpecList *sksl, WRBUF w)
{
    int i;
    for (i = 0; i < sksl->num_specs; i++)
    {
        Z_SortKeySpec *sks = sksl->specs[i];
        Z_SortKey *sk;

        if (sks->sortElement->which != Z_SortElement_generic)
            return -1;

        sk = sks->sortElement->u.generic;
        if (i)
            wrbuf_puts(w, " ");
        else
            wrbuf_puts(w, " SORTBY ");
        if (sk->which == Z_SortKey_sortAttributes)
            return -1;
        else if (sk->which == Z_SortKey_sortField)
            wrbuf_puts(w, sk->u.sortField);
        switch (*sks->sortRelation)
        {
        case Z_SortKeySpec_ascending:
            wrbuf_puts(w, "/ascending");
            break;
        case Z_SortKeySpec_descending:
            wrbuf_puts(w, "/descending");
            break;
        }
        switch (*sks->caseSensitivity)
        {
        case Z_SortKeySpec_caseSensitive:
            wrbuf_puts(w, "/respectCase");
            break;
        case Z_SortKeySpec_caseInsensitive:
            wrbuf_puts(w, "/ignoreCase");
            break;
        }
        switch (sks->which)
        {
        case Z_SortKeySpec_null:
            break;
        case Z_SortKeySpec_abort:
            wrbuf_puts(w, "/missingFail");
            break;
        case Z_SortKeySpec_missingValueData:
            wrbuf_puts(w, "/missingValue=");
            wrbuf_write(w, (const char *) sks->u.missingValueData->buf,
                        sks->u.missingValueData->len);
            break;
        }
    }
    return 0;
}

int yaz_sort_spec_to_type7(Z_SortKeySpecList *sksl, WRBUF pqf)
{
    int i;
    for (i = 0; i < sksl->num_specs; i++)
    {
        Z_SortKeySpec *sks = sksl->specs[i];
        Z_SortKey *sk;

        if (sks->sortElement->which != Z_SortElement_generic)
            return -1;

        sk = sks->sortElement->u.generic;

        wrbuf_insert(pqf, 0, "@or ", 4);

        if (sk->which == Z_SortKey_sortAttributes)
        {
            int j;
            for (j = 0; j < sk->u.sortAttributes->list->num_attributes; j++)
            {
                Z_AttributeElement *el =
                    sk->u.sortAttributes->list->attributes[j];
                if (el->which != Z_AttributeValue_numeric)
                    return -1;
                wrbuf_printf(pqf, " @attr " ODR_INT_PRINTF "=" ODR_INT_PRINTF,
                             *el->attributeType, *el->value.numeric);
            }
        }
        else if (sk->which == Z_SortKey_sortField)
        {
            wrbuf_puts(pqf, " @attr 1=");
            wrbuf_puts(pqf, sk->u.sortField);
        }
        switch (*sks->sortRelation)
        {
        case Z_SortKeySpec_ascending:
            wrbuf_puts(pqf, " @attr 7=1 ");
            break;
        case Z_SortKeySpec_descending:
            wrbuf_puts(pqf, " @attr 7=2 ");
            break;
        }
        wrbuf_printf(pqf, "%d", i);
    }
    return 0;
}

int yaz_sort_spec_to_srw_sortkeys(Z_SortKeySpecList *sksl, WRBUF w)
{
    int i;
    for (i = 0; i < sksl->num_specs; i++)
    {
        Z_SortKeySpec *sks = sksl->specs[i];
        Z_SortKey *sk;

        if (sks->sortElement->which != Z_SortElement_generic)
            return -1;

        sk = sks->sortElement->u.generic;

        if (i)
            wrbuf_puts(w, " ");

        if (sk->which == Z_SortKey_sortAttributes)
            return -1;
        else if (sk->which == Z_SortKey_sortField)
        {
            wrbuf_puts(w, sk->u.sortField);
        }
        wrbuf_puts(w, ",,"); /* path is absent */
        switch (*sks->sortRelation)
        {
        case Z_SortKeySpec_ascending:
            wrbuf_puts(w, "1");
            break;
        case Z_SortKeySpec_descending:
            wrbuf_puts(w, "0");
            break;
        }
        wrbuf_puts(w, ",");
        switch (*sks->caseSensitivity)
        {
        case Z_SortKeySpec_caseSensitive:
            wrbuf_puts(w, "1");
            break;
        case Z_SortKeySpec_caseInsensitive:
            wrbuf_puts(w, "0");
            break;
        }
        wrbuf_puts(w, ",");
        switch (sks->which)
        {
        case Z_SortKeySpec_null:
            wrbuf_puts(w, "highValue");
            break;
        case Z_SortKeySpec_abort:
            wrbuf_puts(w, "abort");
            break;
        case Z_SortKeySpec_missingValueData:
            wrbuf_write(w, (const char *) sks->u.missingValueData->buf,
                        sks->u.missingValueData->len);
            break;
        }
    }
    return 0;
}

int yaz_sort_spec_to_solr_sortkeys(Z_SortKeySpecList *sksl, WRBUF w)
{
    int i;
    for (i = 0; i < sksl->num_specs; i++)
    {
        Z_SortKeySpec *sks = sksl->specs[i];
        Z_SortKey *sk;

        if (sks->sortElement->which != Z_SortElement_generic)
            return -1;

        sk = sks->sortElement->u.generic;

        if (i)
            wrbuf_puts(w, ",");

        if (sk->which == Z_SortKey_sortAttributes)
            return -1;
        else if (sk->which == Z_SortKey_sortField)
        {
            wrbuf_puts(w, sk->u.sortField);
        }
        switch (*sks->sortRelation)
        {
        case Z_SortKeySpec_ascending:
            wrbuf_puts(w, " asc");
            break;
        case Z_SortKeySpec_descending:
            wrbuf_puts(w, " desc");
            break;
        }
    }
    return 0;
}


int yaz_srw_sortkeys_to_sort_spec(const char *srw_sortkeys, WRBUF w)
{
    /* sru sortkey layout: path,schema,ascending,caseSensitive,missingValue */
    /* see cql_sortby_to_sortkeys of YAZ. */
    char **sortspec;
    int num_sortspec = 0;
    int i;
    NMEM nmem = nmem_create();

    if (srw_sortkeys)
        nmem_strsplit_blank(nmem, srw_sortkeys, &sortspec, &num_sortspec);
    for (i = 0; i < num_sortspec; i++)
    {
        char **arg;
        int num_arg;
        int ascending = 1;
        int case_sensitive = 0;
        const char *missing = 0;
        nmem_strsplitx(nmem, ",", sortspec[i], &arg, &num_arg, 0);

        if (num_arg > 2 && arg[2][0])
            ascending = atoi(arg[2]);
        if (num_arg > 3 && arg[3][0])
            case_sensitive = atoi(arg[3]);
        if (num_arg > 4 && arg[4][0])
            missing = arg[4];

        if (i)
            wrbuf_puts(w, " ");

        wrbuf_puts(w, arg[0]); /* field */
        wrbuf_puts(w, " ");

        wrbuf_puts(w, ascending ? "a" : "d");
        wrbuf_puts(w, case_sensitive ? "s" : "i");
        if (missing)
        {
            if (!strcmp(missing, "omit")) {
                ;
            }
            else if (!strcmp(missing, "abort"))
                wrbuf_puts(w, "!");
            else if (!strcmp(missing, "lowValue")) {
                ;
            }
            else if (!strcmp(missing, "highValue")) {
                ;
            }
            else
            {
                wrbuf_puts(w, "=");
                wrbuf_puts(w, missing);
            }
        }
    }
    nmem_destroy(nmem);
    return 0;
}

int yaz_solr_sortkeys_to_sort_spec(const char *solr_sortkeys, WRBUF w)
{
    /* Solr sortkey layout: field order[, field order] */
    /* see cql_sortby_to_sortkeys of YAZ. */
    char **sortspec;
    int num_sortspec = 0;
    int i;
    NMEM nmem = nmem_create();

    if (solr_sortkeys)
        nmem_strsplit(nmem, ",", solr_sortkeys, &sortspec, &num_sortspec);
    for (i = 0; i < num_sortspec; i++)
    {
        char **arg;
        int num_arg;
        char order = 'a';
        int case_sensitive = 0;
        nmem_strsplitx(nmem, " ", sortspec[i], &arg, &num_arg, 0);

        if (num_arg != 2)
            return -1;

        if (!yaz_matchstr(arg[1], "asc"))
                order = 'a';
        else if (!yaz_matchstr(arg[1], "desc"))
            order = 'd';
        else
            return -1;

        if (i)
            wrbuf_puts(w, " ");

        wrbuf_puts(w, arg[0]); /* field */
        wrbuf_puts(w, " ");

        wrbuf_putc(w, order);
        // Always in-sensitive
        wrbuf_puts(w, case_sensitive ? "s" : "i");
    }
    nmem_destroy(nmem);
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

