/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file cqltransform.c
 * \brief Implements CQL transform (CQL to RPN conversion).
 *
 * Evaluation order of rules:
 *
 * always
 * relation
 * structure
 * position
 * truncation
 * index
 * relationModifier
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <yaz/rpn2cql.h>
#include <yaz/xmalloc.h>
#include <yaz/diagsrw.h>
#include <yaz/tokenizer.h>
#include <yaz/wrbuf.h>
#include <yaz/z-core.h>
#include <yaz/matchstr.h>
#include <yaz/oid_db.h>
#include <yaz/log.h>

struct cql_prop_entry {
    char *pattern;
    char *value;
    Z_AttributeList attr_list;
    struct cql_prop_entry *next;
};

struct cql_transform_t_ {
    struct cql_prop_entry *entry;
    yaz_tok_cfg_t tok_cfg;
    int error;
    char *addinfo;
    WRBUF w;
    NMEM nmem;
};


cql_transform_t cql_transform_create(void)
{
    cql_transform_t ct = (cql_transform_t) xmalloc(sizeof(*ct));
    ct->tok_cfg = yaz_tok_cfg_create();
    ct->w = wrbuf_alloc();
    ct->error = 0;
    ct->addinfo = 0;
    ct->entry = 0;
    ct->nmem = nmem_create();
    return ct;
}

static int cql_transform_parse_tok_line(cql_transform_t ct,
                                        const char *pattern,
                                        yaz_tok_parse_t tp)
{
    int ae_num = 0;
    Z_AttributeElement *ae[20];
    int ret = 0; /* 0=OK, != 0 FAIL */
    int t;
    t = yaz_tok_move(tp);
    
    while (t == YAZ_TOK_STRING && ae_num < 20)
    {
        WRBUF type_str = wrbuf_alloc();
        WRBUF set_str = 0;
        Z_AttributeElement *elem = 0;
        const char *value_str = 0;
        /* attset type=value  OR  type=value */
        
        elem = (Z_AttributeElement *) nmem_malloc(ct->nmem, sizeof(*elem));
        elem->attributeSet = 0;
        ae[ae_num] = elem;
        wrbuf_puts(ct->w, yaz_tok_parse_string(tp));
        wrbuf_puts(type_str, yaz_tok_parse_string(tp));
        t = yaz_tok_move(tp);
        if (t == YAZ_TOK_EOF)
        {
            wrbuf_destroy(type_str);
            if (set_str)
                wrbuf_destroy(set_str);                
            break;
        }
        if (t == YAZ_TOK_STRING)  
        {  
            wrbuf_puts(ct->w, " ");
            wrbuf_puts(ct->w, yaz_tok_parse_string(tp));
            set_str = type_str;
            
            elem->attributeSet =
                yaz_string_to_oid_nmem(yaz_oid_std(), CLASS_ATTSET,
                                       wrbuf_cstr(set_str), ct->nmem);
            
            type_str = wrbuf_alloc();
            wrbuf_puts(type_str, yaz_tok_parse_string(tp));
            t = yaz_tok_move(tp);
        }
        elem->attributeType = nmem_intdup(ct->nmem, 0);
        if (sscanf(wrbuf_cstr(type_str), ODR_INT_PRINTF, elem->attributeType)
            != 1)
        {
            wrbuf_destroy(type_str);
            if (set_str)
                wrbuf_destroy(set_str);                
            yaz_log(YLOG_WARN, "Expected numeric attribute type");
            ret = -1;
            break;
        }

        wrbuf_destroy(type_str);
        if (set_str)
            wrbuf_destroy(set_str);                
        
        if (t != '=')
        {
            yaz_log(YLOG_WARN, "Expected = after after attribute type");
            ret = -1;
            break;
        }
        t = yaz_tok_move(tp);
        if (t != YAZ_TOK_STRING) /* value */
        {
            yaz_log(YLOG_WARN, "Missing attribute value");
            ret = -1;
            break;
        }
        value_str = yaz_tok_parse_string(tp);
        if (isdigit(*value_str))
        {
            elem->which = Z_AttributeValue_numeric;
            elem->value.numeric =
                nmem_intdup(ct->nmem, atoi(value_str));
        }
        else
        {
            Z_ComplexAttribute *ca = (Z_ComplexAttribute *)
                nmem_malloc(ct->nmem, sizeof(*ca));
            elem->which = Z_AttributeValue_complex;
            elem->value.complex = ca;
            ca->num_list = 1;
            ca->list = (Z_StringOrNumeric **)
                nmem_malloc(ct->nmem, sizeof(Z_StringOrNumeric *));
            ca->list[0] = (Z_StringOrNumeric *)
                nmem_malloc(ct->nmem, sizeof(Z_StringOrNumeric));
            ca->list[0]->which = Z_StringOrNumeric_string;
            ca->list[0]->u.string = nmem_strdup(ct->nmem, value_str);
            ca->num_semanticAction = 0;
            ca->semanticAction = 0;
        }
        wrbuf_puts(ct->w, "=");
        wrbuf_puts(ct->w, yaz_tok_parse_string(tp));
        t = yaz_tok_move(tp);
        wrbuf_puts(ct->w, " ");
        ae_num++;
    }
    if (ret == 0) /* OK? */
    {
        struct cql_prop_entry **pp = &ct->entry;
        while (*pp)
            pp = &(*pp)->next;
        *pp = (struct cql_prop_entry *) xmalloc(sizeof(**pp));
        (*pp)->pattern = xstrdup(pattern);
        (*pp)->value = xstrdup(wrbuf_cstr(ct->w));

        (*pp)->attr_list.num_attributes = ae_num;
        if (ae_num == 0)
            (*pp)->attr_list.attributes = 0;
        else
        {
            (*pp)->attr_list.attributes = (Z_AttributeElement **)
                nmem_malloc(ct->nmem,
                            ae_num * sizeof(Z_AttributeElement *));
            memcpy((*pp)->attr_list.attributes, ae, 
                   ae_num * sizeof(Z_AttributeElement *));
        }
        (*pp)->next = 0;

        if (0)
        {
            ODR pr = odr_createmem(ODR_PRINT);
            Z_AttributeList *alp = &(*pp)->attr_list;
            odr_setprint(pr, yaz_log_file());
            z_AttributeList(pr, &alp, 0, 0);
            odr_setprint(pr, 0);
            odr_destroy(pr);
        }
    }
    return ret;
}

int cql_transform_define_pattern(cql_transform_t ct, const char *pattern,
                                 const char *value)
{
    int r;
    yaz_tok_parse_t tp = yaz_tok_parse_buf(ct->tok_cfg, value);
    yaz_tok_cfg_single_tokens(ct->tok_cfg, "=");
    r = cql_transform_parse_tok_line(ct, pattern, tp);
    yaz_tok_parse_destroy(tp);
    return r;
}
    
cql_transform_t cql_transform_open_FILE(FILE *f)
{
    cql_transform_t ct = cql_transform_create();
    char line[1024];

    yaz_tok_cfg_single_tokens(ct->tok_cfg, "=");

    while (fgets(line, sizeof(line)-1, f))
    {
        yaz_tok_parse_t tp = yaz_tok_parse_buf(ct->tok_cfg, line);
        int t;
        wrbuf_rewind(ct->w);
        t = yaz_tok_move(tp);
        if (t == YAZ_TOK_STRING)
        {
            char * pattern = xstrdup(yaz_tok_parse_string(tp));
            t = yaz_tok_move(tp);
            if (t != '=')
            {
                yaz_tok_parse_destroy(tp);
                cql_transform_close(ct);
                return 0;
            }
            if (cql_transform_parse_tok_line(ct, pattern, tp))
            {
                yaz_tok_parse_destroy(tp);
                cql_transform_close(ct);
                return 0;
            }
            xfree(pattern);
        }
        else if (t != YAZ_TOK_EOF)
        {
            yaz_tok_parse_destroy(tp);
            cql_transform_close(ct);
            return 0;
        }
        yaz_tok_parse_destroy(tp);
    }
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
        xfree(pe->pattern);
        xfree(pe->value);
        xfree(pe);
        pe = pe_next;
    }
    xfree(ct->addinfo);
    yaz_tok_cfg_destroy(ct->tok_cfg);
    wrbuf_destroy(ct->w);
    nmem_destroy(ct->nmem);
    xfree(ct);
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

#if 0
struct Z_AttributeElement {
	Z_AttributeSetId *attributeSet; /* OPT */
	int *attributeType;
	int which;
	union {
		int *numeric;
		Z_ComplexAttribute *complex;
#define Z_AttributeValue_numeric 1
#define Z_AttributeValue_complex 2
	} value;
};
#endif

static int compare_attr(Z_AttributeElement *a, Z_AttributeElement *b)
{
    ODR odr_a = odr_createmem(ODR_ENCODE);
    ODR odr_b = odr_createmem(ODR_ENCODE);
    int len_a, len_b;
    char *buf_a, *buf_b;
    int ret;

    z_AttributeElement(odr_a, &a, 0, 0);
    z_AttributeElement(odr_b, &b, 0, 0);
    
    buf_a = odr_getbuf(odr_a, &len_a, 0);
    buf_b = odr_getbuf(odr_b, &len_b, 0);

    ret = yaz_memcmp(buf_a, buf_b, len_a, len_b);

    odr_destroy(odr_a);
    odr_destroy(odr_b);
    return ret;
}

const char *cql_lookup_reverse(cql_transform_t ct, 
                               const char *category,
                               Z_AttributeList *attributes)
{
    struct cql_prop_entry *e;
    size_t clen = strlen(category);
    for (e = ct->entry; e; e = e->next)
    {
        if (!strncmp(e->pattern, category, clen))
        {
            /* category matches.. See if attributes in pattern value
               are all listed in actual attributes */
            int i;
            for (i = 0; i < e->attr_list.num_attributes; i++)
            {
                /* entry attribute */
                Z_AttributeElement *e_ae = e->attr_list.attributes[i];
                int j;
                for (j = 0; j < attributes->num_attributes; j++)
                {
                    /* actual attribute */
                    Z_AttributeElement *a_ae = attributes->attributes[j];
                    int r = compare_attr(e_ae, a_ae);
                    if (r == 0)
                        break;
                }
                if (j == attributes->num_attributes)
                    break; /* i was not found at all.. try next pattern */
                    
            }
            if (i == e->attr_list.num_attributes)
                return e->pattern + clen;
        }
    }
    return 0;
}
                                      
static const char *cql_lookup_property(cql_transform_t ct,
                                       const char *pat1, const char *pat2,
                                       const char *pat3)
{
    char pattern[120];
    struct cql_prop_entry *e;

    if (pat1 && pat2 && pat3)
        sprintf(pattern, "%.39s.%.39s.%.39s", pat1, pat2, pat3);
    else if (pat1 && pat2)
        sprintf(pattern, "%.39s.%.39s", pat1, pat2);
    else if (pat1 && pat3)
        sprintf(pattern, "%.39s.%.39s", pat1, pat3);
    else if (pat1)
        sprintf(pattern, "%.39s", pat1);
    else
        return 0;
    
    for (e = ct->entry; e; e = e->next)
    {
        if (!cql_strcmp(e->pattern, pattern))
            return e->value;
    }
    return 0;
}

int cql_pr_attr_uri(cql_transform_t ct, const char *category,
                   const char *uri, const char *val, const char *default_val,
                   void (*pr)(const char *buf, void *client_data),
                   void *client_data,
                   int errcode)
{
    const char *res = 0;
    const char *eval = val ? val : default_val;
    const char *prefix = 0;
    
    if (uri)
    {
        struct cql_prop_entry *e;
        
        for (e = ct->entry; e; e = e->next)
            if (!memcmp(e->pattern, "set.", 4) && e->value &&
                !strcmp(e->value, uri))
            {
                prefix = e->pattern+4;
                break;
            }
        /* must have a prefix now - if not it's an error */
    }

    if (!uri || prefix)
    {
        if (!res)
            res = cql_lookup_property(ct, category, prefix, eval);
        /* we have some aliases for some relations unfortunately.. */
        if (!res && !prefix && !strcmp(category, "relation"))
        {
            if (!strcmp(val, "=="))
                res = cql_lookup_property(ct, category, prefix, "exact");
            if (!strcmp(val, "="))
                res = cql_lookup_property(ct, category, prefix, "eq");
            if (!strcmp(val, "<="))
                res = cql_lookup_property(ct, category, prefix, "le");
            if (!strcmp(val, ">="))
                res = cql_lookup_property(ct, category, prefix, "ge");
        }
        if (!res)
            res = cql_lookup_property(ct, category, prefix, "*");
    }
    if (res)
    {
        char buf[64];

        const char *cp0 = res, *cp1;
        while ((cp1 = strchr(cp0, '=')))
        {
            int i;
            while (*cp1 && *cp1 != ' ')
                cp1++;
            if (cp1 - cp0 >= sizeof(buf))
                break;
            memcpy(buf, cp0, cp1 - cp0);
            buf[cp1-cp0] = 0;
            (*pr)("@attr ", client_data);

            for (i = 0; buf[i]; i++)
            {
                if (buf[i] == '*')
                    (*pr)(eval, client_data);
                else
                {
                    char tmp[2];
                    tmp[0] = buf[i];
                    tmp[1] = '\0';
                    (*pr)(tmp, client_data);
                }
            }
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
        if (val)
            ct->addinfo = xstrdup(val);
        else
            ct->addinfo = 0;
    }
    return 0;
}

int cql_pr_attr(cql_transform_t ct, const char *category,
                const char *val, const char *default_val,
                void (*pr)(const char *buf, void *client_data),
                void *client_data,
                int errcode)
{
    return cql_pr_attr_uri(ct, category, 0 /* uri */,
                           val, default_val, pr, client_data, errcode);
}


static void cql_pr_int(int val,
                       void (*pr)(const char *buf, void *client_data),
                       void *client_data)
{
    char buf[21];              /* enough characters to 2^64 */
    sprintf(buf, "%d", val);
    (*pr)(buf, client_data);
    (*pr)(" ", client_data);
}


static int cql_pr_prox(cql_transform_t ct, struct cql_node *mods,
                       void (*pr)(const char *buf, void *client_data),
                       void *client_data)
{
    int exclusion = 0;
    int distance;               /* to be filled in later depending on unit */
    int distance_defined = 0;
    int ordered = 0;
    int proxrel = 2;            /* less than or equal */
    int unit = 2;               /* word */

    while (mods)
    {
        const char *name = mods->u.st.index;
        const char *term = mods->u.st.term;
        const char *relation = mods->u.st.relation;

        if (!strcmp(name, "distance")) {
            distance = strtol(term, (char**) 0, 0);
            distance_defined = 1;
            if (!strcmp(relation, "="))
                proxrel = 3;
            else if (!strcmp(relation, ">"))
                proxrel = 5;
            else if (!strcmp(relation, "<"))
                proxrel = 1;
            else if (!strcmp(relation, ">=")) 
                proxrel = 4;
            else if (!strcmp(relation, "<="))
                proxrel = 2;
            else if (!strcmp(relation, "<>"))
                proxrel = 6;
            else 
            {
                ct->error = YAZ_SRW_UNSUPP_PROX_RELATION;
                ct->addinfo = xstrdup(relation);
                return 0;
            }
        } 
        else if (!strcmp(name, "ordered"))
            ordered = 1;
        else if (!strcmp(name, "unordered"))
            ordered = 0;
        else if (!strcmp(name, "unit"))
        {
            if (!strcmp(term, "word"))
                unit = 2;
            else if (!strcmp(term, "sentence"))
                unit = 3;
            else if (!strcmp(term, "paragraph"))
                unit = 4;
            else if (!strcmp(term, "element"))
                unit = 8;
            else 
            {
                ct->error = YAZ_SRW_UNSUPP_PROX_UNIT;
                ct->addinfo = xstrdup(term);
                return 0;
            }
        } 
        else 
        {
            ct->error = YAZ_SRW_UNSUPP_BOOLEAN_MODIFIER;
            ct->addinfo = xstrdup(name);
            return 0;
        }
        mods = mods->u.st.modifiers;
    }

    if (!distance_defined)
        distance = (unit == 2) ? 1 : 0;

    cql_pr_int(exclusion, pr, client_data);
    cql_pr_int(distance, pr, client_data);
    cql_pr_int(ordered, pr, client_data);
    cql_pr_int(proxrel, pr, client_data);
    (*pr)("k ", client_data);
    cql_pr_int(unit, pr, client_data);

    return 1;
}

/* Returns location of first wildcard character in the `length'
 * characters starting at `term', or a null pointer of there are
 * none -- like memchr().
 */
static const char *wcchar(int start, const char *term, int length)
{
    while (length > 0)
    {
        if (start || term[-1] != '\\')
            if (strchr("*?", *term))
                return term;
        term++;
        length--;
        start = 0;
    }
    return 0;
}


/* ### checks for CQL relation-name rather than Type-1 attribute */
static int has_modifier(struct cql_node *cn, const char *name) {
    struct cql_node *mod;
    for (mod = cn->u.st.modifiers; mod != 0; mod = mod->u.st.modifiers) {
        if (!strcmp(mod->u.st.index, name))
            return 1;
    }

    return 0;
}


void emit_term(cql_transform_t ct,
               struct cql_node *cn,
               const char *term, int length,
               void (*pr)(const char *buf, void *client_data),
               void *client_data)
{
    int i;
    const char *ns = cn->u.st.index_uri;
    int process_term = !has_modifier(cn, "regexp");
    char *z3958_mem = 0;

    assert(cn->which == CQL_NODE_ST);

    if (process_term && length > 0)
    {
        if (length > 1 && term[0] == '^' && term[length-1] == '^')
        {
            cql_pr_attr(ct, "position", "firstAndLast", 0,
                        pr, client_data, YAZ_SRW_ANCHORING_CHAR_IN_UNSUPP_POSITION);
            term++;
            length -= 2;
        }
        else if (term[0] == '^')
        {
            cql_pr_attr(ct, "position", "first", 0,
                        pr, client_data, YAZ_SRW_ANCHORING_CHAR_IN_UNSUPP_POSITION);
            term++;
            length--;
        }
        else if (term[length-1] == '^')
        {
            cql_pr_attr(ct, "position", "last", 0,
                        pr, client_data, YAZ_SRW_ANCHORING_CHAR_IN_UNSUPP_POSITION);
            length--;
        }
        else
        {
            cql_pr_attr(ct, "position", "any", 0,
                        pr, client_data, YAZ_SRW_ANCHORING_CHAR_IN_UNSUPP_POSITION);
        }
    }

    if (process_term && length > 0)
    {
        const char *first_wc = wcchar(1, term, length);
        const char *second_wc = first_wc ?
            wcchar(0, first_wc+1, length-(first_wc-term)-1) : 0;

        /* Check for well-known globbing patterns that represent
         * simple truncation attributes as expected by, for example,
         * Bath-compliant server.  If we find such a pattern but
         * there's no mapping for it, that's fine: we just use a
         * general pattern-matching attribute.
         */
        if (first_wc == term && second_wc == term + length-1 
            && *first_wc == '*' && *second_wc == '*' 
            && cql_pr_attr(ct, "truncation", "both", 0, pr, client_data, 0)) 
        {
            term++;
            length -= 2;
        }
        else if (first_wc == term && second_wc == 0 && *first_wc == '*'
                 && cql_pr_attr(ct, "truncation", "left", 0,
                                pr, client_data, 0))
        {
            term++;
            length--;
        }
        else if (first_wc == term + length-1 && second_wc == 0
                 && *first_wc == '*'
                 && cql_pr_attr(ct, "truncation", "right", 0, 
                                pr, client_data, 0))
        {
            length--;
        }
        else if (first_wc)
        {
            /* We have one or more wildcard characters, but not in a
             * way that can be dealt with using only the standard
             * left-, right- and both-truncation attributes.  We need
             * to translate the pattern into a Z39.58-type pattern,
             * which has been supported in BIB-1 since 1996.  If
             * there's no configuration element for "truncation.z3958"
             * we indicate this as error 28 "Masking character not
             * supported".
             */
            int i;
            cql_pr_attr(ct, "truncation", "z3958", 0,
                        pr, client_data, YAZ_SRW_MASKING_CHAR_UNSUPP);
            z3958_mem = (char *) xmalloc(length+1);
            for (i = 0; i < length; i++)
            {
                if (i > 0 && term[i-1] == '\\')
                    z3958_mem[i] = term[i];
                else if (term[i] == '*')
                    z3958_mem[i] = '?';
                else if (term[i] == '?')
                    z3958_mem[i] = '#';
                else
                    z3958_mem[i] = term[i];
            }
            z3958_mem[length] = '\0';
            term = z3958_mem;
        }
        else {
            /* No masking characters.  Use "truncation.none" if given. */
            cql_pr_attr(ct, "truncation", "none", 0,
                        pr, client_data, 0);
        }
    }
    if (ns) {
        cql_pr_attr_uri(ct, "index", ns,
                        cn->u.st.index, "serverChoice",
                        pr, client_data, YAZ_SRW_UNSUPP_INDEX);
    }
    if (cn->u.st.modifiers)
    {
        struct cql_node *mod = cn->u.st.modifiers;
        for (; mod; mod = mod->u.st.modifiers)
        {
            cql_pr_attr(ct, "relationModifier", mod->u.st.index, 0,
                        pr, client_data, YAZ_SRW_UNSUPP_RELATION_MODIFIER);
        }
    }

    (*pr)("\"", client_data);
    for (i = 0; i<length; i++)
    {
        /* pr(int) each character */
        /* we do not need to deal with \-sequences because the
           CQL and PQF terms have same \-format, bug #1988 */
        char buf[2];

        buf[0] = term[i];
        buf[1] = '\0';
        (*pr)(buf, client_data);
    }
    (*pr)("\" ", client_data);
    xfree(z3958_mem);
}

void emit_terms(cql_transform_t ct,
                struct cql_node *cn,
                void (*pr)(const char *buf, void *client_data),
                void *client_data,
                const char *op)
{
    struct cql_node *ne = cn->u.st.extra_terms;
    if (ne)
    {
        (*pr)("@", client_data);
        (*pr)(op, client_data);
        (*pr)(" ", client_data);
    }
    emit_term(ct, cn, cn->u.st.term, strlen(cn->u.st.term),
              pr, client_data);
    for (; ne; ne = ne->u.st.extra_terms)
    {
        if (ne->u.st.extra_terms)
        {
            (*pr)("@", client_data);
            (*pr)(op, client_data);
            (*pr)(" ", client_data);
        }            
        emit_term(ct, cn, ne->u.st.term, strlen(ne->u.st.term),
                  pr, client_data);
    }
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
            emit_term(ct, cn, last_term, last_length, pr, client_data);
        }
        last_term = cp0;
        if (cp1)
            last_length = cp1 - cp0;
        else
            last_length = strlen(cp0);
        cp0 = cp1;
    }
    if (last_term)
        emit_term(ct, cn, last_term, last_length, pr, client_data);
}

void cql_transform_r(cql_transform_t ct,
                     struct cql_node *cn,
                     void (*pr)(const char *buf, void *client_data),
                     void *client_data)
{
    const char *ns;
    struct cql_node *mods;

    if (!cn)
        return;
    switch (cn->which)
    {
    case CQL_NODE_ST:
        ns = cn->u.st.index_uri;
        if (ns)
        {
            if (!strcmp(ns, cql_uri())
                && cn->u.st.index && !cql_strcmp(cn->u.st.index, "resultSet"))
            {
                (*pr)("@set \"", client_data);
                (*pr)(cn->u.st.term, client_data);
                (*pr)("\" ", client_data);
                return ;
            }
        }
        else
        {
            if (!ct->error)
            {
                ct->error = YAZ_SRW_UNSUPP_CONTEXT_SET;
                ct->addinfo = 0;
            }
        }
        cql_pr_attr(ct, "always", 0, 0, pr, client_data, 0);
        cql_pr_attr(ct, "relation", cn->u.st.relation, 0, pr, client_data,
                    YAZ_SRW_UNSUPP_RELATION);
        cql_pr_attr(ct, "structure", cn->u.st.relation, 0,
                    pr, client_data, YAZ_SRW_UNSUPP_COMBI_OF_RELATION_AND_TERM);
        if (cn->u.st.relation && !cql_strcmp(cn->u.st.relation, "all"))
            emit_wordlist(ct, cn, pr, client_data, "and");
        else if (cn->u.st.relation && !cql_strcmp(cn->u.st.relation, "any"))
            emit_wordlist(ct, cn, pr, client_data, "or");
        else
            emit_terms(ct, cn, pr, client_data, "and");
        break;
    case CQL_NODE_BOOL:
        (*pr)("@", client_data);
        (*pr)(cn->u.boolean.value, client_data);
        (*pr)(" ", client_data);
        mods = cn->u.boolean.modifiers;
        if (!strcmp(cn->u.boolean.value, "prox")) 
        {
            if (!cql_pr_prox(ct, mods, pr, client_data))
                return;
        } 
        else if (mods)
        {
            /* Boolean modifiers other than on proximity not supported */
            ct->error = YAZ_SRW_UNSUPP_BOOLEAN_MODIFIER;
            ct->addinfo = xstrdup(mods->u.st.index);
            return;
        }

        cql_transform_r(ct, cn->u.boolean.left, pr, client_data);
        cql_transform_r(ct, cn->u.boolean.right, pr, client_data);
        break;

    default:
        fprintf(stderr, "Fatal: impossible CQL node-type %d\n", cn->which);
        abort();
    }
}

int cql_transform(cql_transform_t ct, struct cql_node *cn,
                  void (*pr)(const char *buf, void *client_data),
                  void *client_data)
{
    struct cql_prop_entry *e;
    NMEM nmem = nmem_create();

    ct->error = 0;
    xfree(ct->addinfo);
    ct->addinfo = 0;

    for (e = ct->entry; e ; e = e->next)
    {
        if (!cql_strncmp(e->pattern, "set.", 4))
            cql_apply_prefix(nmem, cn, e->pattern+4, e->value);
        else if (!cql_strcmp(e->pattern, "set"))
            cql_apply_prefix(nmem, cn, 0, e->value);
    }
    cql_transform_r(ct, cn, pr, client_data);
    nmem_destroy(nmem);
    return ct->error;
}


int cql_transform_FILE(cql_transform_t ct, struct cql_node *cn, FILE *f)
{
    return cql_transform(ct, cn, cql_fputs, f);
}

int cql_transform_buf(cql_transform_t ct, struct cql_node *cn, char *out, int max)
{
    struct cql_buf_write_info info;
    int r;

    info.off = 0;
    info.max = max;
    info.buf = out;
    r = cql_transform(ct, cn, cql_buf_write_handler, &info);
    if (info.off < 0) {
        /* Attempt to write past end of buffer.  For some reason, this
           SRW diagnostic is deprecated, but it's so perfect for our
           purposes that it would be stupid not to use it. */
        char numbuf[30];
        ct->error = YAZ_SRW_TOO_MANY_CHARS_IN_QUERY;
        sprintf(numbuf, "%ld", (long) info.max);
        ct->addinfo = xstrdup(numbuf);
        return -1;
    }
    if (info.off >= 0)
        info.buf[info.off] = '\0';
    return r;
}

int cql_transform_error(cql_transform_t ct, const char **addinfo)
{
    *addinfo = ct->addinfo;
    return ct->error;
}

void cql_transform_set_error(cql_transform_t ct, int error, const char *addinfo)
{
    xfree(ct->addinfo);
    ct->addinfo = addinfo ? xstrdup(addinfo) : 0;
    ct->error = error;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

