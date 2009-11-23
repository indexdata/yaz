/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file srwutil.c
 * \brief Implements SRW/SRU utilities.
 */

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
    }
    *name = (char **) odr_malloc(o, no * sizeof(char*));
    *val = (char **) odr_malloc(o, no * sizeof(char*));

    for (no = 0; *path; no++)
    {
        const char *p1 = strchr(path, '=');
        size_t i = 0;
        char *ret;
        if (!p1)
            break;

        (*name)[no] = (char *) odr_malloc(o, (p1-path)+1);
        memcpy((*name)[no], path, p1-path);
        (*name)[no][p1-path] = '\0';

        path = p1 + 1;
        p1 = strchr(path, '&');
        if (!p1)
            p1 = strlen(path) + path;
        (*val)[no] = ret = (char *) odr_malloc(o, p1 - path + 1);
        while (*path && *path != '&')
        {
            size_t l = 3;
            ret[i++] = decode_uri_char(path, &l);
            path += l;
        }
        ret[i] = '\0';

        if (*path)
            path++;
    }
    (*name)[no] = 0;
    (*val)[no] = 0;
    return no;
}

char *yaz_uri_val(const char *path, const char *name, ODR o)
{
    size_t nlen = strlen(name);
    if (*path != '?')
        return 0;
    path++;
    while (path && *path)
    {
        const char *p1 = strchr(path, '=');
        if (!p1)
            break;
        if ((size_t)(p1 - path) == nlen && !memcmp(path, name, nlen))
        {
            size_t i = 0;
            char *ret;
            
            path = p1 + 1;
            p1 = strchr(path, '&');
            if (!p1)
                p1 = strlen(path) + path;
            ret = (char *) odr_malloc(o, p1 - path + 1);
            while (*path && *path != '&')
            {
                size_t l = 3;
                ret[i++] = decode_uri_char(path, &l);
                path += l;
            }
            ret[i] = '\0';
            return ret;
        }
        path = strchr(p1, '&');
        if (path)
            path++;
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

