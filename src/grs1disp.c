/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file grs1disp.c
 * \brief Implements display of GRS-1 records
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <yaz/proto.h>
#include <yaz/oid_db.h>

static void display_variant(WRBUF w, Z_Variant *v, int level)
{
    int i;

    for (i = 0; i < v->num_triples; i++)
    {
        printf("%*sclass=" ODR_INT_PRINTF ",type=" ODR_INT_PRINTF,
               level * 4, "", *v->triples[i]->zclass,
               *v->triples[i]->type);
        if (v->triples[i]->which == Z_Triple_internationalString)
            printf(",value=%s\n", v->triples[i]->value.internationalString);
        else
            printf("\n");
    }
}

static void display_grs1(WRBUF w, Z_GenericRecord *r, int level)
{
    int i;

    if (!r)
    {
        return;
    }
    for (i = 0; i < r->num_elements; i++)
    {
        Z_TaggedElement *t;

        wrbuf_printf(w, "%*s", level * 4, "");
        t = r->elements[i];
        wrbuf_printf(w, "(");
        if (t->tagType)
            wrbuf_printf(w, ODR_INT_PRINTF, *t->tagType);
        else
            wrbuf_printf(w, "?,");
        if (t->tagValue->which == Z_StringOrNumeric_numeric)
            wrbuf_printf(w, ODR_INT_PRINTF ") ", *t->tagValue->u.numeric);
        else
            wrbuf_printf(w, "%s) ", t->tagValue->u.string);
        if (t->content->which == Z_ElementData_subtree)
        {
            if (!t->content->u.subtree)
                printf (" (no subtree)\n");
            else
            {
                wrbuf_printf(w, "\n");
                display_grs1(w, t->content->u.subtree, level+1);
            }
        }
        else if (t->content->which == Z_ElementData_string)
        {
            wrbuf_puts(w, t->content->u.string);
            wrbuf_puts(w, "\n");
        }
        else if (t->content->which == Z_ElementData_numeric)
        {
            wrbuf_printf(w, ODR_INT_PRINTF "\n", *t->content->u.numeric);
        }
        else if (t->content->which == Z_ElementData_oid)
        {
            Odr_oid *ip = t->content->u.oid;

            if (ip)
            {
                char oid_name_str[OID_STR_MAX];
                oid_class oclass;
                const char *oid_name 
                    = yaz_oid_to_string_buf(ip, &oclass, oid_name_str);
            
                if (oid_name)
                    wrbuf_printf(w, "OID: %s\n", oid_name);
            }
        }
        else if (t->content->which == Z_ElementData_noDataRequested)
            wrbuf_printf(w, "[No data requested]\n");
        else if (t->content->which == Z_ElementData_elementEmpty)
            wrbuf_printf(w, "[Element empty]\n");
        else if (t->content->which == Z_ElementData_elementNotThere)
            wrbuf_printf(w, "[Element not there]\n");
        else if (t->content->which == Z_ElementData_date)
            wrbuf_printf(w, "Date: %s\n", t->content->u.date);
        else if (t->content->which == Z_ElementData_ext)
        {
            printf ("External\n");
            /* we cannot print externals here. Srry */
        } 
        else
            wrbuf_printf(w, "? type = %d\n",t->content->which);
        if (t->appliedVariant)
            display_variant(w, t->appliedVariant, level+1);
        if (t->metaData && t->metaData->supportedVariants)
        {
            int c;

            wrbuf_printf(w, "%*s---- variant list\n", (level+1)*4, "");
            for (c = 0; c < t->metaData->num_supportedVariants; c++)
            {
                wrbuf_printf(w, "%*svariant #%d\n", (level+1)*4, "", c);
                display_variant(w, t->metaData->supportedVariants[c], level+2);
            }
        }
    }
}

void yaz_display_grs1(WRBUF wrbuf, Z_GenericRecord *r, int flags)
{
    display_grs1 (wrbuf, r, 0);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

