/* CCL find (to rpn conversion)
 * Europagate, 1995
 *
 * $Log: cclfind.c,v $
 * Revision 1.4  1995-11-01 13:54:20  quinn
 * Minor adjustments
 *
 * Revision 1.3  1995/09/29  17:11:59  quinn
 * Smallish
 *
 * Revision 1.2  1995/09/27  15:02:44  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.1  1995/04/10  10:28:19  quinn
 * Added copy of CCL.
 *
 * Revision 1.12  1995/03/20  15:27:43  adam
 * Minor changes.
 *
 * Revision 1.11  1995/02/23  08:31:59  adam
 * Changed header.
 *
 * Revision 1.9  1995/02/16  13:20:06  adam
 * Spell fix.
 *
 * Revision 1.8  1995/02/14  19:59:42  adam
 * Removed a syntax error.
 *
 * Revision 1.7  1995/02/14  19:55:10  adam
 * Header files ccl.h/cclp.h are gone! They have been merged an
 * moved to ../include/ccl.h.
 * Node kind(s) in ccl_rpn_node have changed names.
 *
 * Revision 1.6  1995/02/14  16:20:55  adam
 * Qualifiers are read from a file now.
 *
 * Revision 1.5  1995/02/14  14:12:41  adam
 * Ranges for ordered qualfiers implemented (e.g. pd=1980-1990).
 *
 * Revision 1.4  1995/02/14  13:16:29  adam
 * Left and/or right truncation implemented.
 *
 * Revision 1.3  1995/02/14  10:25:56  adam
 * The constructions 'qualifier rel term ...' implemented.
 *
 * Revision 1.2  1995/02/13  15:15:07  adam
 * Added handling of qualifiers. Not finished yet.
 *
 * Revision 1.1  1995/02/13  12:35:20  adam
 * First version of CCL. Qualifiers aren't handled yet.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <ccl.h>

static struct ccl_token *look_token;
static int ccl_error;
static CCL_bibset bibset;

#define KIND (look_token->kind)
#define ADVANCE look_token = look_token->next
#define ADVX(x) x=(x)->next

static struct ccl_rpn_attr *qual_val (struct ccl_rpn_attr *list, int type)
{
    while (list)
    {
        if (list->type == type)
            return list;
        list = list->next;
    }
    return NULL;
}

static int qual_val_type (struct ccl_rpn_attr *list, int type, int value)
{
    while (list)
    {
        if (list->type == type && list->value == value)
            return 1;
        list = list->next;
    }
    return 0;
}

static void strxcat (char *n, const char *src, int len)
{
    while (*n)
	n++;
    while (--len >= 0)
	*n++ = *src++;
    *n = '\0';
}

static char *copy_token_name (struct ccl_token *tp)
{
    char *str = xmalloc (tp->len + 1);
    assert (str);
    memcpy (str, tp->name, tp->len);
    str[tp->len] = '\0';
    return str;
}

static struct ccl_rpn_node *mk_node (enum rpn_node_kind kind)
{
    struct ccl_rpn_node *p;
    p = xmalloc (sizeof(*p));
    assert (p);
    p->kind = kind;
    return p;
}

void ccl_rpn_delete (struct ccl_rpn_node *rpn)
{
    struct ccl_rpn_attr *attr, *attr1;
    if (!rpn)
        return;
    switch (rpn->kind)
    {
    case CCL_RPN_AND:
    case CCL_RPN_OR:
    case CCL_RPN_NOT:
        ccl_rpn_delete (rpn->u.p[0]);
        ccl_rpn_delete (rpn->u.p[1]);
        break;
    case CCL_RPN_TERM:
        xfree (rpn->u.t.term);
        for (attr = rpn->u.t.attr_list; attr; attr = attr1)
        {
            attr1 = attr->next;
            xfree (attr);
        }
        break;
    case CCL_RPN_SET:
        xfree (rpn->u.setname);
        break;
    case CCL_RPN_PROX:
        ccl_rpn_delete (rpn->u.p[0]);
        ccl_rpn_delete (rpn->u.p[1]);
        break;
    }
    xfree (rpn);
}

static struct ccl_rpn_node *find_spec (struct ccl_rpn_attr **qa);
static struct ccl_rpn_node *search_terms (struct ccl_rpn_attr **qa);

static void add_attr (struct ccl_rpn_node *p, int type, int value)
{
    struct ccl_rpn_attr *n;

    n = xmalloc (sizeof(*n));
    assert (n);
    n->type = type;
    n->value = value;
    n->next = p->u.t.attr_list;
    p->u.t.attr_list = n;
}

static struct ccl_rpn_node *search_term (struct ccl_rpn_attr **qa)
{
    struct ccl_rpn_node *p;
    struct ccl_rpn_attr *attr;
    struct ccl_token *lookahead = look_token;
    int len = 0;
    int no, i;
    int left_trunc = 0;
    int right_trunc = 0;
    int mid_trunc = 0;

    if (KIND != CCL_TOK_TERM)
    {
        ccl_error = CCL_ERR_TERM_EXPECTED;
        return NULL;
    }
    for (no = 0; lookahead->kind == CCL_TOK_TERM; no++)
    {
        for (i = 0; i<lookahead->len; i++)
            if (lookahead->name[i] == '?')
            {
                if (no == 0 && i == 0 && lookahead->len >= 1)
                    left_trunc = 1;
                else if (lookahead->next->kind != CCL_TOK_TERM &&
                         i == lookahead->len-1 && i >= 1)
                    right_trunc = 1;
                else
                    mid_trunc = 1;
            }
        len += 1+lookahead->len;
	lookahead = lookahead->next;
    }
    p = mk_node (CCL_RPN_TERM);
    p->u.t.term = xmalloc (len);
    assert (p->u.t.term);
    p->u.t.attr_list = NULL;
    p->u.t.term[0] = '\0';
    for (i = 0; i<no; i++)
    {
        const char *src_str = look_token->name;
        int src_len = look_token->len;
        
        if (i == 0 && left_trunc)
        {
            src_len--;
            src_str++;
        }
        else if (i == no-1 && right_trunc)
            src_len--;
        if (i)
            strcat (p->u.t.term, " ");
	strxcat (p->u.t.term, src_str, src_len);
        ADVANCE;
    }
    if (qa)
    {
        int i;
        for (i=0; qa[i]; i++)
        {
            struct ccl_rpn_attr *attr;

            for (attr = qa[i]; attr; attr = attr->next)
                if (attr->value > 0)
                    add_attr (p, attr->type, attr->value);
        }
        attr = qa[0];
    }
    else 
        attr = ccl_qual_search (bibset, "term", 4);
    if (attr && qual_val_type (attr, CCL_BIB1_STR, CCL_BIB1_STR_WP))
    {
        if (no == 1)
            add_attr (p, CCL_BIB1_STR, 2);
        else
            add_attr (p, CCL_BIB1_STR, 1);
    }
    if (left_trunc && right_trunc)
    {
        if (attr && !qual_val_type (attr, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_BOTH))
        {
            ccl_error = CCL_ERR_TRUNC_NOT_BOTH;
            if (qa)
                xfree (qa);
            ccl_rpn_delete (p);
            return NULL;
        }
        add_attr (p, CCL_BIB1_TRU, 3);
    }
    else if (right_trunc)
    {
        if (attr && !qual_val_type (attr, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_RIGHT))
        {
            ccl_error = CCL_ERR_TRUNC_NOT_RIGHT;
            if (qa)
                xfree (qa);
            ccl_rpn_delete (p);
            return NULL;
        }
        add_attr (p, CCL_BIB1_TRU, 1);
    }
    else if (left_trunc)
    {
        if (attr && !qual_val_type (attr, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_LEFT))
        {
            ccl_error = CCL_ERR_TRUNC_NOT_LEFT;
            if (qa)
                xfree (qa);
            ccl_rpn_delete (p);
            return NULL;
        }
        add_attr (p, CCL_BIB1_TRU, 2);
    }
    else
    {
        if (attr && qual_val_type (attr, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_NONE))
            add_attr (p, CCL_BIB1_TRU, 100);
    }
    return p;
}

static struct ccl_rpn_node *qualifiers (struct ccl_token *la,
                                        struct ccl_rpn_attr **qa)
{
    struct ccl_token *lookahead = look_token;
    struct ccl_rpn_attr **ap;
    int no = 1;
    int i, rel;
    struct ccl_rpn_attr *attr;

    if (qa)
    {
        ccl_error = CCL_ERR_DOUBLE_QUAL;
        return NULL;
    }
    for (lookahead = look_token; lookahead != la; lookahead=lookahead->next)
        no++;
    ap = xmalloc (no * sizeof(*ap));
    assert (ap);
    for (i=0; look_token != la; i++)
    {
        ap[i] = ccl_qual_search (bibset, look_token->name, look_token->len);
        if (!ap[i])
        {
            ccl_error = CCL_ERR_UNKNOWN_QUAL;
            xfree (ap);
            return NULL;
        }
        ADVANCE;
        if (KIND == CCL_TOK_COMMA)
            ADVANCE;
    }
    ap[i] = NULL;
    if (! (attr = qual_val (ap[0], CCL_BIB1_REL)) ||
        attr->value != CCL_BIB1_REL_ORDER)
    {                
        /* unordered relation */
        struct ccl_rpn_node *p;
        if (KIND != CCL_TOK_EQ)
        {
            ccl_error = CCL_ERR_EQ_EXPECTED;
            xfree (ap);
            return NULL;
        }
        ADVANCE;
        if (KIND == CCL_TOK_LP)
        {
            ADVANCE;
            if (!(p = find_spec (ap)))
            {
                xfree (ap);
                return NULL;
            }
            if (KIND != CCL_TOK_RP)
            {
                ccl_error = CCL_ERR_RP_EXPECTED;
                ccl_rpn_delete (p);
                xfree (ap);
                return NULL;
            }
            ADVANCE;
        }
        else
            p = search_terms (ap);
        xfree (ap);
        return p;
    }
    rel = 0;
    if (look_token->len == 1)
    {
        if (look_token->name[0] == '<')
            rel = 1;
        else if (look_token->name[0] == '=')
            rel = 3;
        else if (look_token->name[0] == '>')
            rel = 5;
    }
    else if (look_token->len == 2)
    {
        if (!memcmp (look_token->name, "<=", 2))
            rel = 2;
        else if (!memcmp (look_token->name, ">=", 2))
            rel = 4;
        else if (!memcmp (look_token->name, "<>", 2))
            rel = 6;
    }
    if (!rel)
        ccl_error = CCL_ERR_BAD_RELATION;
    else
    {
        struct ccl_rpn_node *p;

        ADVANCE;                      /* skip relation */
        if (KIND == CCL_TOK_TERM)
        {
            struct ccl_rpn_node *p1;
            p1 = search_term (ap);
            if (KIND == CCL_TOK_MINUS)
            {
                ADVANCE;                   /* skip '-' */
                if (KIND == CCL_TOK_TERM)  /* = term - term  ? */
                {
                    struct ccl_rpn_node *p2;
                    
                    p2 = search_term (ap);
                    p = mk_node (CCL_RPN_AND);
                    p->u.p[0] = p1;
                    add_attr (p1, CCL_BIB1_REL, 4);
                    p->u.p[1] = p2;
                    add_attr (p2, CCL_BIB1_REL, 2);
                    xfree (ap);
                    return p;
                }
                else                       /* = term -    */
                {
                    add_attr (p1, CCL_BIB1_REL, 4);
                    xfree (ap);
                    return p1;
                }
            }
            else
            {
                add_attr (p1, CCL_BIB1_REL, rel);
                xfree (ap);
                return p1;
            }
        }
        else if (KIND == CCL_TOK_MINUS)   /* = - term  ? */
        {
            ADVANCE;
            p = search_term (ap);
            add_attr (p, CCL_BIB1_REL, 2);
            xfree (ap);
            return p;
        }
        ccl_error = CCL_ERR_TERM_EXPECTED;
    }
    xfree (ap);
    return NULL;
}

static struct ccl_rpn_node *search_terms (struct ccl_rpn_attr **qa)
{
    struct ccl_rpn_node *p1, *p2, *pn;
    p1 = search_term (qa);
    if (!p1)
        return NULL;
    while (1)
    {
	if (KIND == CCL_TOK_PROX)
	{
	    ADVANCE;
	    p2 = search_term (qa);
            if (!p2)
            {
                ccl_rpn_delete (p1);
                return NULL;
            }
	    pn = mk_node (CCL_RPN_PROX);
	    pn->u.p[0] = p1;
	    pn->u.p[1] = p2;
	    p1 = pn;
	}
	else if (KIND == CCL_TOK_TERM)
	{
	    p2 = search_term (qa);
            if (!p2)
            {
                ccl_rpn_delete (p1);
                return NULL;
            }
	    pn = mk_node (CCL_RPN_PROX);
	    pn->u.p[0] = p1;
	    pn->u.p[1] = p2;
	    p1 = pn;
	}
	else
	    break;
    }
    return p1;
}

static struct ccl_rpn_node *search_elements (struct ccl_rpn_attr **qa)
{
    struct ccl_rpn_node *p1;
    struct ccl_token *lookahead;
    if (KIND == CCL_TOK_LP)
    {
	ADVANCE;
	p1 = find_spec (qa);
        if (!p1)
            return NULL;
        if (KIND != CCL_TOK_RP)
        {
            ccl_error = CCL_ERR_RP_EXPECTED;
            ccl_rpn_delete (p1);
            return NULL;
        }
	ADVANCE;
	return p1;
    }
    else if (KIND == CCL_TOK_SET)
    {
	ADVANCE;
        if (KIND == CCL_TOK_EQ)
            ADVANCE;
        if (KIND != CCL_TOK_TERM)
        {
            ccl_error = CCL_ERR_SETNAME_EXPECTED;
            return NULL;
        }
	p1 = mk_node (CCL_RPN_SET);
	p1->u.setname = copy_token_name (look_token);
	ADVANCE;
	return p1;
    }
    lookahead = look_token;

    while (lookahead->kind==CCL_TOK_TERM || lookahead->kind==CCL_TOK_COMMA)
	lookahead = lookahead->next;
    if (lookahead->kind == CCL_TOK_REL || lookahead->kind == CCL_TOK_EQ)
	return qualifiers (lookahead, qa);
    return search_terms (qa);
}

static struct ccl_rpn_node *find_spec (struct ccl_rpn_attr **qa)
{
    struct ccl_rpn_node *p1, *p2, *pn;
    if (!(p1 = search_elements (qa)))
        return NULL;
    while (1)
    {
	switch (KIND)
	{
	case CCL_TOK_AND:
	    ADVANCE;
	    p2 = search_elements (qa);
            if (!p2)
            {
                ccl_rpn_delete (p1);
                return NULL;
            }
	    pn = mk_node (CCL_RPN_AND);
	    pn->u.p[0] = p1;
	    pn->u.p[1] = p2;
	    p1 = pn;
	    continue;
	case CCL_TOK_OR:
	    ADVANCE;
	    p2 = search_elements (qa);
            if (!p2)
            {
                ccl_rpn_delete (p1);
                return NULL;
            }
	    pn = mk_node (CCL_RPN_OR);
	    pn->u.p[0] = p1;
	    pn->u.p[1] = p2;
	    p1 = pn;
	    continue;
	case CCL_TOK_NOT:
	    ADVANCE;
	    p2 = search_elements (qa);
            if (!p2)
            {
                ccl_rpn_delete (p1);
                return NULL;
            }
	    pn = mk_node (CCL_RPN_NOT);
	    pn->u.p[0] = p1;
	    pn->u.p[1] = p2;
	    p1 = pn;
	    continue;
	}
	break;
    }
    return p1;
}

struct ccl_rpn_node *ccl_find (CCL_bibset abibset, struct ccl_token *list,
                               int *error, const char **pos)
{
    struct ccl_rpn_node *p;

    look_token = list;
    bibset = abibset;
    p = find_spec (NULL);
    if (p && KIND != CCL_TOK_EOL)
    {
        if (KIND == CCL_TOK_RP)
            ccl_error = CCL_ERR_BAD_RP;
        else
            ccl_error = CCL_ERR_OP_EXPECTED;
        ccl_rpn_delete (p);
        p = NULL;
    }
    *pos = look_token->name;
    if (p)
        *error = CCL_ERR_OK;
    else
        *error = ccl_error;
    return p;
}

struct ccl_rpn_node *ccl_find_str (CCL_bibset bibset, const char *str,
                                   int *error, int *pos)
{
    struct ccl_token *list;
    struct ccl_rpn_node *rpn;
    const char *char_pos;

    list = ccl_tokenize (str);
    rpn = ccl_find (bibset, list, error, &char_pos);
    if (*error)
        *pos = char_pos - str;
    return rpn;
}
