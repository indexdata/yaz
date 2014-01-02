/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file nmemsdup.c
 * \brief Implements NMEM dup utilities
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/nmem_xml.h>

char *nmem_strdup(NMEM mem, const char *src)
{
    char *dst = (char *) nmem_malloc(mem, strlen(src)+1);
    strcpy(dst, src);
    return dst;
}

char *nmem_strdup_null(NMEM mem, const char *src)
{
    if (!src)
        return 0;
    else
        return nmem_strdup(mem, src);
}

char *nmem_strdupn(NMEM mem, const char *src, size_t n)
{
    char *dst = (char *) nmem_malloc(mem, n+1);
    memcpy(dst, src, n);
    dst[n] = '\0';
    return dst;
}

nmem_int_t *nmem_intdup(NMEM mem, nmem_int_t v)
{
    nmem_int_t *dst = (nmem_int_t*) nmem_malloc(mem, sizeof(*dst));
    *dst = v;
    return dst;
}

nmem_bool_t *nmem_booldup(NMEM mem, nmem_bool_t v)
{
    nmem_bool_t *dst = (nmem_bool_t*) nmem_malloc(mem, sizeof(*dst));
    *dst = v;
    return dst;
}

void nmem_strsplit_blank(NMEM nmem, const char *dstr, char ***darray, int *num)
{
    nmem_strsplit(nmem, " ", dstr, darray, num);
}


void nmem_strsplit(NMEM nmem, const char *delim, const char *dstr,
                   char ***darray, int *num)
{
    nmem_strsplitx(nmem, delim, dstr, darray, num, 1);
}

void nmem_strsplitx(NMEM nmem, const char *delim, const char *dstr,
                    char ***darray, int *num, int collapse)
{
    nmem_strsplit_escape(nmem, delim, dstr, darray, num, collapse, 0);
}

void nmem_strsplit_escape(NMEM nmem, const char *delim, const char *dstr,
                          char ***darray, int *num, int collapse,
                          int escape_char)
{
    nmem_strsplit_escape2(nmem, delim, dstr, darray, num, collapse,
                          escape_char, 1);
}

void nmem_strsplit_escape2(NMEM nmem, const char *delim, const char *dstr,
                           char ***darray, int *num, int collapse,
                           int escape_char, int subst_escape)
{
    *darray = 0;
    /* two passes over the input string.. */
    while (1)
    {
        size_t i = 0;
        const char *cp = dstr;
        while (1)
        {
            const char *cp0;
            if (collapse)
            {
                if (!*cp)
                    break;
                while (*cp && strchr(delim, *cp) && *cp != escape_char)
                    cp++;
                if (!*cp)
                    break;
            }

            cp0 = cp;
            while (*cp && !strchr(delim, *cp))
            {
                if (*cp == escape_char)
                    cp++;
                cp++;
            }
            if (*darray)
            {
                (*darray)[i] = nmem_strdupn(nmem, cp0, cp - cp0);
                if (subst_escape)
                {
                    char *dst = (*darray)[i];
                    const char *src = dst;
                    while (*src != '\0')
                    {
                        if (*src == escape_char && src[1])
                            src++;
                        *dst++ = *src++;
                    }
                    *dst = '\0';
                }
            }
            i++;
            if (!collapse)
            {
                if (!*cp)
                    break;
                cp++;
            }
        }
        *num = i;
        if (!*num)
            break; /* no items, so stop, *darray=0 already */
        else if (*darray)
            break; /* second pass, stop */
        *darray = (char **) nmem_malloc(nmem, *num * sizeof(**darray));
    }
}

#if YAZ_HAVE_XML2
char *nmem_text_node_cdata(const xmlNode *ptr_cdata, NMEM nmem)
{
    char *cdata;
    int len = 0;
    const xmlNode *ptr;

    for (ptr = ptr_cdata; ptr; ptr = ptr->next)
        if (ptr->type == XML_TEXT_NODE)
            len += xmlStrlen(ptr->content);
    cdata = (char *) nmem_malloc(nmem, len+1);
    *cdata = '\0';
    for (ptr = ptr_cdata; ptr; ptr = ptr->next)
        if (ptr->type == XML_TEXT_NODE)
            strcat(cdata, (const char *) ptr->content);
    return cdata;
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

