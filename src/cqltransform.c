/* $Id: cqltransform.c,v 1.1 2003-10-27 12:21:30 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include <stdlib.h>
#include <string.h>
#include <yaz/cql.h>

struct cql_prop_entry {
    char *pattern;
    char *value;
    struct cql_prop_entry *next;
};

struct cql_transform_t_ {
    struct cql_prop_entry *entry;
    int error;
    char *addinfo;
};

cql_transform_t cql_transform_open_FILE(FILE *f)
{
    char line[1024];
    cql_transform_t ct = (cql_transform_t) malloc (sizeof(*ct));
    struct cql_prop_entry **pp = &ct->entry;

    ct->error = 0;
    ct->addinfo = 0;
    while (fgets(line, sizeof(line)-1, f))
    {
        const char *cp_value_start;
        const char *cp_value_end;
        const char *cp_pattern_end;
        const char *cp = line;
        while (*cp && !strchr(" \t=\r\n#", *cp))
            cp++;
        cp_pattern_end = cp;
        if (cp == line)
            continue;
        while (*cp && strchr(" \t\r\n", *cp))
            cp++;
        if (*cp != '=')
            continue;
        cp++;
        while (*cp && strchr(" \t\r\n", *cp))
            cp++;
        cp_value_start = cp;
        if (!(cp_value_end = strchr(cp, '#')))
            cp_value_end = strlen(line) + line;

        if (cp_value_end != cp_value_start &&
            strchr(" \t\r\n", cp_value_end[-1]))
            cp_value_end--;
        *pp = (struct cql_prop_entry *) malloc (sizeof(**pp));
        (*pp)->pattern = (char *) malloc (cp_pattern_end - line + 1);
        memcpy ((*pp)->pattern, line, cp_pattern_end - line);
        (*pp)->pattern[cp_pattern_end-line] = 0;

        (*pp)->value = (char *) malloc (cp_value_end - cp_value_start + 1);
        if (cp_value_start != cp_value_end)
            memcpy ((*pp)->value, cp_value_start, cp_value_end-cp_value_start);
        (*pp)->value[cp_value_end - cp_value_start] = 0;
        pp = &(*pp)->next;
    }
    *pp = 0;
    return ct;
}

void cql_transform_close(cql_transform_t ct)
{
    struct cql_prop_entry *pe;
    if (!ct)
        return;
    pe = ct->entry;
    while (pe)
    {
        struct cql_prop_entry *pe_next = pe->next;
        free (pe->pattern);
        free (pe->value);
        free (pe);
        pe = pe_next;
    }
    if (ct->addinfo)
        free (ct->addinfo);
    free (ct);
}

cql_transform_t cql_transform_open_fname(const char *fname)
{
    cql_transform_t ct;
    FILE *f = fopen(fname, "r");
    if (!f)
        return 0;
    ct = cql_transform_open_FILE(f);
    fclose(f);
    return ct;
}

static const char *cql_lookup_property(cql_transform_t ct,
                                       const char *pat1, const char *pat2)
{
    char pattern[80];
    struct cql_prop_entry *e;

    if (pat2)
        sprintf (pattern, "%.39s%.39s", pat1, pat2);
    else
        sprintf (pattern, "%.39s", pat1);
    for (e = ct->entry; e; e = e->next)
    {
        if (!strcmp(e->pattern, pattern))
            return e->value;
    }
    return 0;
}

static const char *cql_lookup_value(cql_transform_t ct,
                                    const char *prefix,
                                    const char *value)
{
    struct cql_prop_entry *e;
    int len = strlen(prefix);

    for (e = ct->entry; e; e = e->next)
    {
        if (!memcmp(e->pattern, prefix, len) && !strcmp(e->value, value))
            return e->pattern + len;
    }
    return 0;
}


int cql_pr_attr(cql_transform_t ct, const char *category,
                const char *val,
                const char *default_val,
                void (*pr)(const char *buf, void *client_data),
                void *client_data,
                int errcode)
{
    const char *res;
    res = cql_lookup_property(ct, category, val ? val : default_val);
    if (!res)
        res = cql_lookup_property(ct, category, "*");
    if (res)
    {
        char buf[64];

        const char *cp0 = res, *cp1;
        while ((cp1 = strchr(cp0, '=')))
        {
            while (*cp1 && *cp1 != ' ')
                cp1++;
            if (cp1 - cp0 >= sizeof(buf))
                break;
            memcpy (buf, cp0, cp1 - cp0);
            buf[cp1-cp0] = 0;
            (*pr)("@attr ", client_data);
            (*pr)(buf, client_data);
            (*pr)(" ", client_data);
            cp0 = cp1;
            while (*cp0 == ' ')
                cp0++;
        }
        return 1;
    }
    /* error ... */
    if (errcode && !ct->error)
    {
        ct->error = errcode;
        ct->addinfo = strdup(val);
    }
    return 0;
}

void emit_term(cql_transform_t ct,
               const char *term, int length,
               void (*pr)(const char *buf, void *client_data),
               void *client_data)
{
    int i;
    if (length > 0)
    {
        if (length > 1 && term[0] == '^' && term[length-1] == '^')
        {
            cql_pr_attr(ct, "position.", "firstAndLast", 0,
                        pr, client_data, 32);
            term++;
            length -= 2;
        }
        else if (term[0] == '^')
        {
            cql_pr_attr(ct, "position.", "first", 0,
                        pr, client_data, 32);
            term++;
        }
        else if (term[length-1] == '^')
        {
            cql_pr_attr(ct, "position.", "last", 0,
                        pr, client_data, 32);
            length--;
        }
        else
        {
            cql_pr_attr(ct, "position.", "any", 0,
                        pr, client_data, 32);
        }
    }
    (*pr)("\"", client_data);
    for (i = 0; i<length; i++)
    {
        char buf[2];
        buf[0] = term[i];
        buf[1] = 0;
        (*pr)(buf, client_data);
    }
    (*pr)("\" ", client_data);
}

void emit_wordlist(cql_transform_t ct,
                   struct cql_node *cn,
                   void (*pr)(const char *buf, void *client_data),
                   void *client_data,
                   const char *op)
{
    const char *cp0 = cn->u.st.term;
    const char *cp1;
    const char *last_term = 0;
    int last_length = 0;
    while(cp0)
    {
        while (*cp0 == ' ')
            cp0++;
        cp1 = strchr(cp0, ' ');
        if (last_term)
        {
            (*pr)("@", client_data);
            (*pr)(op, client_data);
            (*pr)(" ", client_data);
            emit_term(ct, last_term, last_length, pr, client_data);
        }
        last_term = cp0;
        if (cp1)
            last_length = cp1 - cp0;
        else
            last_length = strlen(cp0);
        cp0 = cp1;
    }
    if (last_term)
        emit_term(ct, last_term, last_length, pr, client_data);
}


static const char *cql_get_ns(cql_transform_t ct,
                              struct cql_node *cn,
                              struct cql_node **prefix_ar, int prefix_level,
                              const char **n_prefix,
                              const char **n_suffix)
{
    int i;
    const char *ns = 0;
    char prefix[32];
    const char *cp = cn->u.st.index;
    const char *cp_dot = strchr(cp, '.');

    /* strz current prefix (empty if not given) */
    if (cp_dot && cp_dot-cp < sizeof(prefix))
    {
        memcpy (prefix, cp, cp_dot - cp);
        prefix[cp_dot - cp] = 0;
    }
    else
        *prefix = 0;

    /* 2. lookup in prefix_ar. and return NS */
    for (i = prefix_level; !ns && --i >= 0; )
    {
        struct cql_node *cn_prefix = prefix_ar[i];
        for (; cn_prefix; cn_prefix = cn_prefix->u.mod.next)
        {
            if (*prefix && cn_prefix->u.mod.name &&
                !strcmp(prefix, cn_prefix->u.mod.name))
            {
                ns = cn_prefix->u.mod.value;
                break;
            }
            else if (!*prefix && !cn_prefix->u.mod.name)
            {
                ns = cn_prefix->u.mod.value;
                break;
            }
        }
    }
    if (!ns)
    {
        if (!ct->error)
        {
            ct->error = 15;
            ct->addinfo = strdup(prefix);
        }
        return 0;
    }
    /* 3. lookup in set.NS for new prefix */
    *n_prefix = cql_lookup_value(ct, "set.", ns);
    if (!*n_prefix)
    {
        if (!ct->error)
        {
            ct->error = 15;
            ct->addinfo = strdup(ns);
        }
        return 0;
    }
    /* 4. lookup qualifier.prefix. */
    
    cp = cn->u.st.index;
    cp_dot = strchr(cp, '.');
    
    *n_suffix = cp_dot ? cp_dot+1 : cp;
    return ns;
}

void cql_transform_r(cql_transform_t ct,
                     struct cql_node *cn,
                     void (*pr)(const char *buf, void *client_data),
                     void *client_data,
                     struct cql_node **prefix_ar, int prefix_level)
{
    const char *ns, *n_prefix, *n_suffix;

    if (!cn)
        return;
    switch (cn->which)
    {
    case CQL_NODE_ST:
        if (cn->u.st.prefixes && prefix_level < 20)
            prefix_ar[prefix_level++] = cn->u.st.prefixes;
        ns = cql_get_ns(ct, cn, prefix_ar, prefix_level, &n_prefix, &n_suffix);
        if (ns)
        {
            char n_full[64];
            sprintf (n_full, "%.20s.%.40s", n_prefix, n_suffix);
        
            if (!strcmp(ns, "http://www.loc.gov/zing/cql/srw-indexes/v1.0/")
                && !strcmp(n_suffix, "resultSet"))
            {
                (*pr)("@set \"", client_data);
                (*pr)(cn->u.st.term, client_data);
                (*pr)("\" ", client_data);
                return ;
            }
            cql_pr_attr(ct, "qualifier.", n_full, "srw.serverChoice",
                        pr, client_data, 16);
        }

        if (cn->u.st.relation && !strcmp(cn->u.st.relation, "="))
            cql_pr_attr(ct, "relation.", "eq", "scr",
                        pr, client_data, 19);
        else if (cn->u.st.relation && !strcmp(cn->u.st.relation, "<="))
            cql_pr_attr(ct, "relation.", "le", "scr",
                        pr, client_data, 19);
        else if (cn->u.st.relation && !strcmp(cn->u.st.relation, ">="))
            cql_pr_attr(ct, "relation.", "ge", "scr",
                        pr, client_data, 19);
        else
            cql_pr_attr(ct, "relation.", cn->u.st.relation, "eq",
                        pr, client_data, 19);
        if (cn->u.st.modifiers)
        {
            struct cql_node *mod = cn->u.st.modifiers;
            for (; mod; mod = mod->u.mod.next)
            {
                cql_pr_attr(ct, "relationModifier.", mod->u.mod.value, 0,
                            pr, client_data, 20);
            }
        }
        cql_pr_attr(ct, "structure.", cn->u.st.relation, 0,
                    pr, client_data, 24);
        if (cn->u.st.relation && !strcmp(cn->u.st.relation, "all"))
        {
            emit_wordlist(ct, cn, pr, client_data, "and");
        }
        else if (cn->u.st.relation && !strcmp(cn->u.st.relation, "any"))
        {
            emit_wordlist(ct, cn, pr, client_data, "or");
        }
        else
        {
            emit_term(ct, cn->u.st.term, strlen(cn->u.st.term),
                      pr, client_data);
        }
        break;
    case CQL_NODE_BOOL:
        if (cn->u.boolean.prefixes && prefix_level < 20)
            prefix_ar[prefix_level++] = cn->u.boolean.prefixes;
        (*pr)("@", client_data);
        (*pr)(cn->u.boolean.value, client_data);
        (*pr)(" ", client_data);

        cql_transform_r(ct, cn->u.boolean.left, pr, client_data,
                        prefix_ar, prefix_level);
        cql_transform_r(ct, cn->u.boolean.right, pr, client_data,
                        prefix_ar, prefix_level);
    }
}

int cql_transform(cql_transform_t ct,
                  struct cql_node *cn,
                  void (*pr)(const char *buf, void *client_data),
                  void *client_data)
{
    struct cql_node *prefix_ar[20], **pp;
    struct cql_prop_entry *e;

    ct->error = 0;
    if (ct->addinfo)
        free (ct->addinfo);
    ct->addinfo = 0;

    prefix_ar[0] = 0;
    pp = &prefix_ar[0];
    for (e = ct->entry; e ; e = e->next)
    {
        if (!memcmp(e->pattern, "set.", 4))
        {
            *pp = cql_node_mk_mod(e->pattern+4, e->value);
            pp = &(*pp)->u.mod.next;
        }
        else if (!strcmp(e->pattern, "set"))
        {
            *pp = cql_node_mk_mod(0, e->value);
            pp = &(*pp)->u.mod.next;
        }
    }
    cql_transform_r (ct, cn, pr, client_data, prefix_ar, 1);
    cql_node_destroy(prefix_ar[0]);
    return ct->error;
}


int cql_transform_FILE(cql_transform_t ct, struct cql_node *cn, FILE *f)
{
    return cql_transform(ct, cn, cql_fputs, f);
}

int cql_transform_buf(cql_transform_t ct, struct cql_node *cn,
                      char *out, int max)
{
    struct cql_buf_write_info info;
    int r;

    info.off = 0;
    info.max = max;
    info.buf = out;
    r = cql_transform(ct, cn, cql_buf_write_handler, &info);
    if (info.off >= 0)
        info.buf[info.off] = '\0';
    return r;
}

int cql_transform_error(cql_transform_t ct, const char **addinfo)
{
    *addinfo = ct->addinfo;
    return ct->error;
}
