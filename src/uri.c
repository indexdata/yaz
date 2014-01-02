/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file uri.c
 * \brief Implements URI utilities.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <yaz/srw.h>
#include <yaz/matchstr.h>
#include <yaz/yaz-iconv.h>

static int hex_digit (int ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a'+10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A'+10;
    return -1;
}

static void encode_uri_char(char *dst, char ch)
{
    /*  mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")" */
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
        (ch >= '0' && ch <= '9') || strchr("-_.!~*'(|)", ch))
    {
        dst[0] = ch;
        dst[1] = '\0';
    }
    else
    {
        dst[0] = '%';
        sprintf(dst+1, "%02X", (unsigned char ) ch);
    }
}

void yaz_encode_uri_component(char *dst, const char *uri)
{
    for (; *uri; uri++)
    {
        encode_uri_char(dst, *uri);
        dst += strlen(dst);
    }
    *dst = '\0';
}

static unsigned char decode_uri_char(const char *path, size_t *len)
{
    unsigned char ch;
    if (*path == '+')
    {
        ch = ' ';
        *len = 1;
    }
    else if (*path == '%' && *len >= 3)
    {
        int d1 = hex_digit(path[1]);
        int d2 = hex_digit(path[2]);
        if (d1 >= 0 && d2 >= 0)
        {
            ch = d1 * 16 + d2;
            *len = 3;
        }
        else
        {
            ch = *path;
            *len = 1;
        }
    }
    else
    {
        ch = *path;
        *len = 1;
    }
    return ch;
}

void yaz_decode_uri_component(char *dst, const char *uri, size_t len)
{
    while (len)
    {
        size_t sz = len;
        *dst++ = decode_uri_char(uri, &sz);
        uri += sz;
        len = len - sz;
    }
    *dst = '\0';
}

void yaz_array_to_uri(char **path, ODR o, char **name, char **value)
{
    size_t i, szp = 0, sz = 1;
    for(i = 0; name[i]; i++)
        sz += strlen(name[i]) + 3 + strlen(value[i]) * 3;
    *path = (char *) odr_malloc(o, sz);

    for(i = 0; name[i]; i++)
    {
        size_t ilen;
        if (i)
            (*path)[szp++] = '&';
        ilen = strlen(name[i]);
        memcpy(*path+szp, name[i], ilen);
        szp += ilen;
        (*path)[szp++] = '=';

        yaz_encode_uri_component(*path + szp, value[i]);
        szp += strlen(*path + szp);
    }
    (*path)[szp] = '\0';
}

int yaz_uri_to_array(const char *path, ODR o, char ***name, char ***val)
{
    int no = 2;
    const char *cp;
    *name = 0;
    if (*path == '?')
        path++;
    if (!*path)
        return 0;
    cp = path;
    while ((cp = strchr(cp, '&')))
    {
        cp++;
        no++;
        while (*cp && *cp != '=' && *cp != '&')
        {
            /* check that x-form names looks sane */
            if (*cp <= ' ' || *cp >= 127)
                return 0;
            cp++;
        }
    }
    *name = (char **) odr_malloc(o, no * sizeof(char*));
    *val = (char **) odr_malloc(o, no * sizeof(char*));

    for (no = 0; *path; no++)
    {
        while (*path == '&')
            path++;
        if (!*path)
            break;

        for (cp = path; *cp && *cp != '=' && *cp != '&'; cp++)
            ;

        (*name)[no] = odr_strdupn(o, path, cp - path);
        path = cp;
        if (*path == '=')
        {
            size_t i = 0;
            char *ret;
            path++;
            for (cp = path; *cp && *cp != '&'; cp++)
                ;
            (*val)[no] = ret = (char *) odr_malloc(o, cp - path + 1);
            while (*path && *path != '&')
            {
                size_t l = 3;
                ret[i++] = decode_uri_char(path, &l);
                path += l;
            }
            ret[i] = '\0';
        }
        else
            (*val)[no] = odr_strdup(o, "");
    }
    (*name)[no] = 0;
    (*val)[no] = 0;
    return no;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

