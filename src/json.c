/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file json.c
 * \brief JSON encoding/decoding
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/json.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <yaz/xmalloc.h>

struct json_subst_info {
    int idx;
    struct json_subst_info *next;
    struct json_node *node;
};

struct json_parser_s {
    const char *buf;
    const char *cp;
    const char *err_msg;
    int parse_level;
    int max_level;
    struct json_subst_info *subst;
};

json_parser_t json_parser_create(void)
{
    json_parser_t p = (json_parser_t) xmalloc(sizeof(*p));

    p->buf = 0;
    p->cp = 0;
    p->subst = 0;
    return p;
}

void json_parser_subst(json_parser_t p, int idx, struct json_node *n)
{
    struct json_subst_info **sb = &p->subst;
    for (; *sb; sb = &(*sb)->next)
        if ((*sb)->idx == idx)
        {
            (*sb)->node = n;
            return;
        }
    *sb = xmalloc(sizeof(**sb));
    (*sb)->next = 0;
    (*sb)->node = n;
    (*sb)->idx = idx;
}

void json_parser_destroy(json_parser_t p)
{
    struct json_subst_info *sb = p->subst;
    while (sb)
    {
        struct json_subst_info *sb_next = sb->next;
        xfree(sb);
        sb = sb_next;
    }
    xfree(p);
}

static int look_ch(json_parser_t p)
{
    while (*p->cp && strchr(" \t\r\n", *p->cp))
        (p->cp)++;
    return *p->cp;
}

static void move_ch(json_parser_t p)
{
    if (*p->cp)
        (p->cp)++;
}

static struct json_node *json_new_node(json_parser_t p, enum json_node_type type)
{
    struct json_node *n = (struct json_node *) xmalloc(sizeof(*n));
    n->type = type;
    n->u.link[0] = n->u.link[1] = 0;
    return n;
}

void json_remove_node(struct json_node *n)
{
    if (!n)
        return;
    switch (n->type)
    {
    case json_node_object:
    case json_node_array:
    case json_node_list:
    case json_node_pair:
        json_remove_node(n->u.link[0]);
        json_remove_node(n->u.link[1]);
        break;
    case json_node_string:
        xfree(n->u.string);
        break;
    case json_node_number:
    case json_node_true:
    case json_node_false:
    case json_node_null:
        break;
    }
    xfree(n);
}

static struct json_node *json_parse_object(json_parser_t p);
static struct json_node *json_parse_array(json_parser_t p);

static int json_one_char(const char **p, char *out)
{
    if (**p == '\\' && p[0][1])
    {
        (*p)++;
        switch(**p)
        {
        case '"':
            *out = '"'; break;
        case '\\':
            *out = '\\'; break;
        case '/':
            *out = '/'; break;
        case 'b':
            *out = '\b'; break;
        case 'f':
            *out = '\f'; break;
        case 'n':
            *out = '\n'; break;
        case 'r':
            *out = '\r'; break;
        case 't':
            *out = '\t'; break;
        case 'u':
            if (p[0][1] > 0 && p[0][2] > 0 && p[0][3] > 0 && p[0][4] > 0)
            {
                unsigned code;
                char *outp = out;
                int error;
                size_t outbytesleft = 6;
                int no_read = 0;
                sscanf(*p + 1, "%4x%n", &code, &no_read);
                if (no_read != 4)
                    return 0;
                if (!yaz_write_UTF8_char(code, &outp, &outbytesleft, &error))
                {
                    *p += 5;
                    return outp - out;
                }
            }
        default:
            return 0;
        }
        (*p)++;
        return 1;
    }
    else if (**p > 0 && **p <= 31)
    {
        return 0;
    }
    else
    {
        *out = **p;
        (*p)++;
        return 1;
    }
}

static struct json_node *json_parse_string(json_parser_t p)
{
    struct json_node *n;
    const char *cp;
    char *dst;
    int l = 0;
    if (look_ch(p) != '\"')
    {
        p->err_msg = "string expected";
        return 0;
    }
    move_ch(p);

    cp = p->cp;
    while (*cp && *cp != '"')
    {
        char out[6];
        int r = json_one_char(&cp, out);
        if (r == 0)
        {
            p->err_msg = "invalid character";
            return 0;
        }
        l += r;
    }
    if (!*cp)
    {
        p->err_msg = "missing \"";
        return 0;
    }
    n = json_new_node(p, json_node_string);
    dst = n->u.string = (char *) xmalloc(l + 1);

    cp = p->cp;
    while (*cp && *cp != '"')
    {
        char out[6];

        l = json_one_char(&cp, out);
        memcpy(dst, out, l);
        dst += l;
    }
    *dst = '\0';
    p->cp = cp+1;
    return n;
}

static struct json_node *json_parse_number(json_parser_t p)
{
    struct json_node *n;
    char *endptr;
    const char *cp;
    double v;

    cp = p->cp;
    if (*cp == '-')
        cp++;
    if (*cp == '0')
        cp++;
    else if (*cp >= '1' && *cp <= '9')
    {
        cp++;
        while (*cp >= '0' && *cp <= '9')
            cp++;
    }
    else
    {
        p->err_msg = "bad number";
        return 0;
    }
    if (*cp == '.')
    {
        cp++;
        if (*cp >= '0' && *cp <= '9')
        {
            while (*cp >= '0' && *cp <= '9')
                cp++;
        }
        else
        {
            p->err_msg = "bad number";
            return 0;
        }
    }
    if (*cp == 'e' || *cp == 'E')
    {
        cp++;
        if (*cp == '+' || *cp == '-')
            cp++;
        if (*cp >= '0' && *cp <= '9')
        {
            while (*cp >= '0' && *cp <= '9')
                cp++;
        }
        else
        {
            p->err_msg = "bad number";
            return 0;
        }
    }
    v = strtod(p->cp, &endptr);

    if (endptr == p->cp)
    {
        p->err_msg = "bad number";
        return 0;
    }
    if (endptr != cp)
    {
        p->err_msg = "bad number";
        return 0;
    }
    p->cp = endptr;
    n = json_new_node(p, json_node_number);
    n->u.number = v;
    return n;
}

static struct json_node *json_parse_value(json_parser_t p)
{
    int c = look_ch(p);
    if (c == '\"')
        return json_parse_string(p);
    else if (strchr("0123456789-", c))
        return json_parse_number(p);
    else if (c == '{')
        return json_parse_object(p);
    else if (c == '[')
        return json_parse_array(p);
    else if (c == '%')
    {
        struct json_subst_info *sb;
        int idx = 0;
        p->cp++;
        c = *p->cp;
        while (c >= '0' && c <= '9')
        {
            idx = idx*10 + (c - '0');
            p->cp++;
            c = *p->cp;
        }
        for (sb = p->subst; sb; sb = sb->next)
            if (sb->idx == idx)
                return sb->node;
    }
    else if (c == 0)
    {
        return 0;
    }
    else
    {
        char tok[8];
        int i = 0;
        while (c >= 'a' && c <= 'z' && i < 7)
        {
            tok[i++] = c;
            p->cp++;
            c = *p->cp;
        }
        tok[i] = 0;
        if (!strcmp(tok, "true"))
            return json_new_node(p, json_node_true);
        else if (!strcmp(tok, "false"))
            return json_new_node(p, json_node_false);
        else if (!strcmp(tok, "null"))
            return json_new_node(p, json_node_null);
    }
    p->err_msg = "bad token";
    return 0;
}

static struct json_node *json_parse_elements(json_parser_t p)
{
    struct json_node *n1 = json_parse_value(p);
    struct json_node *m0, *m1;
    if (!n1)
        return 0;
    m0 = m1 = json_new_node(p, json_node_list);
    m1->u.link[0] = n1;
    while (look_ch(p) == ',')
    {
        struct json_node *n2, *m2;
        move_ch(p);
        n2 = json_parse_value(p);
        if (!n2)
        {
            json_remove_node(m0);
            return 0;
        }
        m2 = json_new_node(p, json_node_list);
        m2->u.link[0] = n2;

        m1->u.link[1] = m2;
        m1 = m2;
    }
    return m0;
}

static struct json_node *json_parse_array(json_parser_t p)
{
    struct json_node *n;
    if (look_ch(p) != '[')
    {
        p->err_msg = "expecting [";
        return 0;
    }
    move_ch(p);
    if (p->parse_level >= p->max_level)
    {
        p->err_msg = "Too much nesting";
        return 0;
    }
    p->parse_level++;
    n = json_new_node(p, json_node_array);
    if (look_ch(p) != ']')
        n->u.link[0] = json_parse_elements(p);

    p->parse_level--;
    if (look_ch(p) != ']')
    {
        if (!p->err_msg)
            p->err_msg = "expecting ]";
        json_remove_node(n);
        return 0;
    }
    move_ch(p);
    return n;
}

static struct json_node *json_parse_pair(json_parser_t p)
{
    struct json_node *s = json_parse_string(p);
    struct json_node *v, *n;
    if (!s)
        return 0;
    if (look_ch(p) != ':')
    {
        p->err_msg = "missing :";
        json_remove_node(s);
        return 0;
    }
    move_ch(p);
    v = json_parse_value(p);
    if (!v)
    {
        json_remove_node(s);
        return 0;
    }
    n = json_new_node(p, json_node_pair);
    n->u.link[0] = s;
    n->u.link[1] = v;
    return n;
}

static struct json_node *json_parse_members(json_parser_t p)
{
    struct json_node *n1 = json_parse_pair(p);
    struct json_node *m0, *m1;
    if (!n1)
        return 0;
    m0 = m1 = json_new_node(p, json_node_list);
    m1->u.link[0] = n1;
    while (look_ch(p) == ',')
    {
        struct json_node *n2, *m2;
        move_ch(p);
        n2 = json_parse_pair(p);
        if (!n2)
        {
            json_remove_node(m0);
            return 0;
        }
        m2 = json_new_node(p, json_node_list);
        m2->u.link[0] = n2;

        m1->u.link[1] = m2;
        m1 = m2;
    }
    return m0;
}

static struct json_node *json_parse_object(json_parser_t p)
{
    struct json_node *n;
    if (look_ch(p) != '{')
    {
        p->err_msg = "{ expected";
        return 0;
    }
    move_ch(p);

    n = json_new_node(p, json_node_object);
    if (look_ch(p) != '}')
    {
        struct json_node *m;
        if (p->parse_level >= p->max_level)
        {
            p->err_msg = "Too much nesting";
            json_remove_node(n);
            return 0;
        }
        p->parse_level++;
        m = json_parse_members(p);
        p->parse_level--;
        if (!m)
        {
            json_remove_node(n);
            return 0;
        }
        n->u.link[0] = m;
    }
    if (look_ch(p) != '}')
    {
        p->err_msg = "Missing }";
        json_remove_node(n);
        return 0;
    }
    move_ch(p);
    return n;
}

struct json_node *json_parser_parse(json_parser_t p, const char *json_str)
{
    int c;
    struct json_node *n;
    p->buf = json_str;
    p->cp = p->buf;
    p->err_msg = 0;
    p->parse_level = 0;
    p->max_level = 1000;

    n = json_parse_value(p);
    if (!n)
        return 0;
    if (p->err_msg)
    {
        json_remove_node(n);
        return 0;
    }
    c = look_ch(p);
    if (c != 0)
    {
        p->err_msg = "extra characters";
        json_remove_node(n);
        return 0;
    }
    return n;
}

struct json_node *json_parse2(const char *json_str, const char **errmsg,
                              size_t *pos)
{
    json_parser_t p = json_parser_create();
    struct json_node *n = 0;
    if (!p)
    {
        if (errmsg)
            *errmsg = "could not create parser";
    }
    else
    {
        n = json_parser_parse(p, json_str);
        if (!n && errmsg)
            *errmsg = json_parser_get_errmsg(p);
        if (pos)
            *pos = json_parser_get_position(p);
        json_parser_destroy(p);
    }
    return n;
}

struct json_node *json_parse(const char *json_str, const char **errmsg)
{
    return json_parse2(json_str, errmsg, 0);
}

static void json_indent(WRBUF result, int indent)
{
    size_t l = wrbuf_len(result);
    if (l == 0 || wrbuf_buf(result)[l-1] == '\n')
    {
        int i;
        for (i = 0; i < indent; i++)
            wrbuf_putc(result, ' ');
    }
}

static void json_write_wrbuf_r(struct json_node *node, WRBUF result, int indent)
{
    int sub_indent = -1;
    if (indent >= 0)
        sub_indent = indent + 2;
    switch (node->type)
    {
    case json_node_object:
        json_indent(result, indent);
        wrbuf_puts(result, "{");
        if (node->u.link[0])
        {
            if (indent >= 0)
            {
                wrbuf_puts(result, "\n");
                json_indent(result, sub_indent);
            }
            json_write_wrbuf_r(node->u.link[0], result, sub_indent);
            if (indent >= 0)
            {
                wrbuf_puts(result, "\n");
                json_indent(result, indent);
            }
        }
        wrbuf_puts(result, "}");
        break;
    case json_node_array:
        json_indent(result, indent);
        wrbuf_puts(result, "[");
        if (node->u.link[0])
        {
            if (indent >= 0)
                wrbuf_puts(result, "\n");
            json_write_wrbuf_r(node->u.link[0], result, sub_indent);
            if (indent >= 0)
            {
                wrbuf_puts(result, "\n");
                json_indent(result, indent);
            }
        }
        wrbuf_puts(result, "]");
        break;
    case json_node_list:
        json_write_wrbuf_r(node->u.link[0], result, indent);
        if (node->u.link[1])
        {
            wrbuf_puts(result, ",");
            if (indent >= 0) {
                wrbuf_puts(result, "\n");
            }
            json_write_wrbuf_r(node->u.link[1], result, indent);
        }
        break;
    case json_node_pair:
        if (indent >= 0)
            json_indent(result, indent);
        json_write_wrbuf_r(node->u.link[0], result, indent);
        wrbuf_puts(result, ":");
        if (indent >= 0)
            wrbuf_puts(result, " ");
        json_write_wrbuf_r(node->u.link[1], result, indent);
        break;
    case json_node_string:
        wrbuf_puts(result, "\"");
        wrbuf_json_puts(result, node->u.string);
        wrbuf_puts(result, "\"");
        break;
    case json_node_number:
        wrbuf_printf(result, "%lg", node->u.number);
        break;
    case json_node_true:
        wrbuf_puts(result, "true");
        break;
    case json_node_false:
        wrbuf_puts(result, "false");
        break;
    case json_node_null:
        wrbuf_puts(result, "null");
        break;
    }
}

void json_write_wrbuf_pretty(struct json_node *node, WRBUF result)
{
    json_write_wrbuf_r(node, result, 0);
}

void json_write_wrbuf(struct json_node *node, WRBUF result)
{
    json_write_wrbuf_r(node, result, -1);
}

static struct json_node **json_get_objectp(struct json_node *n,
                                           const char *name)
{
    if (n && n->type == json_node_object)
    {
        for (n = n->u.link[0]; n; n = n->u.link[1])
        {
            struct json_node *c = n->u.link[0];
            if (c && c->type == json_node_pair &&
                c->u.link[0] && c->u.link[0]->type == json_node_string)
                if (!strcmp(name, c->u.link[0]->u.string))
                    return &c->u.link[1];
        }
    }
    return 0;
}

struct json_node *json_get_object(struct json_node *n, const char *name)
{
    struct json_node **np = json_get_objectp(n, name);

    if (np)
        return *np;
    return 0;
}

struct json_node *json_detach_object(struct json_node *n, const char *name)
{
    struct json_node **np = json_get_objectp(n, name);

    if (np)
    {
        struct json_node *n = *np;
        *np = 0;
        return n;
    }
    return 0;
}

struct json_node *json_get_elem(struct json_node *n, int idx)
{
    if (n && n->type == json_node_array)
    {
        for (n = n->u.link[0]; n; n = n->u.link[1])
        {
            if (--idx < 0)
                return n->u.link[0];
        }
    }
    return 0;
}

int json_count_children(struct json_node *n)
{
    int i = 0;

    if (n && (n->type == json_node_array || n->type == json_node_object))
    {
        for (n = n->u.link[0]; n; n = n->u.link[1])
            i++;
    }
    return i;
}

int json_append_array(struct json_node *dst, struct json_node *src)
{
    if (dst && src &&
        dst->type == json_node_array && src->type == json_node_array)
    {
        struct json_node **np = &dst->u.link[0];
        while (*np)
            np = &(*np)->u.link[1];
        *np = src->u.link[0];
        src->u.link[0] = 0;
        json_remove_node(src);
        return 0;
    }
    return -1;
}

const char *json_parser_get_errmsg(json_parser_t p)
{
    return p->err_msg;
}

size_t json_parser_get_position(json_parser_t p)
{
    return p->cp - p->buf;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
