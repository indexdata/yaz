/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file xml_get.c
 * \brief XML node getter/creation utilities
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <yaz/xmalloc.h>
#include <yaz/xml_get.h>
#if YAZ_HAVE_XML2

const char *yaz_xml_get_prop(const xmlNode *n, const char *fmt, ...)
{
    int no = 0;
    va_list ap;
    const char *cp;
    struct _xmlAttr *attr;

    va_start(ap, fmt);
    for (cp = fmt; *cp; cp++)
        if (*cp == '%')
            no++;
    if (no > 0)
    {
        const char ***ar = xmalloc(sizeof(*ar) * no);
        int i;
        for (i = 0; i < no; i++)
        {
            const char **s = va_arg(ap, const char **);
            ar[i] = s;
        }
        for (attr = n->properties; attr; attr = attr->next)
        {
            const char *cp1 = fmt;
            for (i = 0; *cp1; i++)
            {
                const char *cp2 = cp1;
                size_t l;
                while (*cp2 != '\0' && *cp2 != '%')
                    cp2++;
                if (*cp2 != '\0')
                { /* no % following, break out (bad fmt really) */
                    cp1 = cp2;
                    break;
                }
                l = cp2 - cp1;
                if (l > 0 && strlen((const char *) attr->name) == l &&
                    !memcmp((const char *) attr->name, cp1, l))
                    break;
                cp1 = 1 + cp2;
                if (*cp1)
                    cp1++; /* skip char following % */
            }
            if (!*cp1)
            {
                /* attribute not listed in fmt: return first unknown one */
                xfree(ar);
                return (const char *) attr->name;
            }
            *ar[i] = (const char *) attr->children->content;
        }
        xfree(ar);
    }
    else
    {
        for (attr = n->properties; attr; attr = attr->next)
        {
            if (!strcmp((const char *) attr->name, fmt))
                return (const char *) attr->children->content;
        }
    }
    va_end(ap);
    return 0; /* failure for simple mode; successful for %mode */
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

