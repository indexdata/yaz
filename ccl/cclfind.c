/*
 * Copyright (c) 1995, the EUROPAGATE consortium (see below).
 *
 * The EUROPAGATE consortium members are:
 *
 *    University College Dublin
 *    Danmarks Teknologiske Videnscenter
 *    An Chomhairle Leabharlanna
 *    Consejo Superior de Investigaciones Cientificas
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of EUROPAGATE or the project partners may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 3. Users of this software (implementors and gateway operators) agree to
 * inform the EUROPAGATE consortium of their use of the software. This
 * information will be used to evaluate the EUROPAGATE project and the
 * software, and to plan further developments. The consortium may use
 * the information in later publications.
 * 
 * 4. Users of this software agree to make their best efforts, when
 * documenting their use of the software, to acknowledge the EUROPAGATE
 * consortium, and the role played by the software in their work.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL THE EUROPAGATE CONSORTIUM OR ITS MEMBERS BE LIABLE
 * FOR ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF
 * ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND
 * ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* CCL find (to rpn conversion)
 * Europagate, 1995
 *
 * $Log: cclfind.c,v $
 * Revision 1.6  1997-04-30 08:52:06  quinn
 * Null
 *
 * Revision 1.5  1996/10/11  15:00:24  adam
 * CCL parser from Europagate Email gateway 1.0.
 *
 * Revision 1.16  1996/01/08  08:41:13  adam
 * Removed unused function.
 *
 * Revision 1.15  1995/07/20  08:14:34  adam
 * Qualifiers were observed too often. Instead tokens are treated as
 * qualifiers only when separated by comma.
 *
 * Revision 1.14  1995/05/16  09:39:26  adam
 * LICENSE.
 *
 * Revision 1.13  1995/04/17  09:31:42  adam
 * Improved handling of qualifiers. Aliases or reserved words.
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

/* current lookahead token */
static struct ccl_token *look_token;

/* holds error no if error occur */
static int ccl_error;

/* current bibset */
static CCL_bibset bibset;

/* returns type of current lookahead */
#define KIND (look_token->kind)

/* move one token forward */
#define ADVANCE look_token = look_token->next

/* 
 * qual_val_type: test for existance of attribute type/value pair.
 * qa:     Attribute array
 * type:   Type of attribute to search for
 * value:  Value of attribute to seach for
 * return: 1 if found; 0 otherwise.
 */
static int qual_val_type (struct ccl_rpn_attr **qa, int type, int value)
{
    int i;
    struct ccl_rpn_attr *q;

    if (!qa)
        return 0;
    for (i = 0;  (q=qa[i]); i++)
        while (q)
	{
            if (q->type == type && q->value == value)
	        return 1;
	    q = q->next;
	}
    return 0;
}

/*
 * strxcat: concatenate strings.
 * n:      Null-terminated Destination string 
 * src:    Source string to be appended (not null-terminated)
 * len:    Length of source string.
 */
static void strxcat (char *n, const char *src, int len)
{
    while (*n)
	n++;
    while (--len >= 0)
	*n++ = *src++;
    *n = '\0';
}

/*
 * copy_token_name: Return copy of CCL token name
 * tp:      Pointer to token info.
 * return:  malloc(3) allocated copy of token name.
 */
static char *copy_token_name (struct ccl_token *tp)
{
    char *str = malloc (tp->len + 1);
    assert (str);
    memcpy (str, tp->name, tp->len);
    str[tp->len] = '\0';
    return str;
}

/*
 * mk_node: Create RPN node.
 * kind:   Type of node.
 * return: pointer to allocated node.
 */
static struct ccl_rpn_node *mk_node (enum rpn_node_kind kind)
{
    struct ccl_rpn_node *p;
    p = malloc (sizeof(*p));
    assert (p);
    p->kind = kind;
    return p;
}

/*
 * ccl_rpn_delete: Delete RPN tree.
 * rpn:   Pointer to tree.
 */
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
        free (rpn->u.t.term);
        for (attr = rpn->u.t.attr_list; attr; attr = attr1)
        {
            attr1 = attr->next;
            free (attr);
        }
        break;
    case CCL_RPN_SET:
        free (rpn->u.setname);
        break;
    case CCL_RPN_PROX:
        ccl_rpn_delete (rpn->u.p[0]);
        ccl_rpn_delete (rpn->u.p[1]);
        break;
    }
    free (rpn);
}

static struct ccl_rpn_node *find_spec (struct ccl_rpn_attr **qa);
static struct ccl_rpn_node *search_terms (struct ccl_rpn_attr **qa);

/*
 * add_attr: Add attribute (type/value) to RPN term node.
 * p:     RPN node of type term.
 * type:  Type of attribute
 * value: Value of attribute
 */
static void add_attr (struct ccl_rpn_node *p, int type, int value)
{
    struct ccl_rpn_attr *n;

    n = malloc (sizeof(*n));
    assert (n);
    n->type = type;
    n->value = value;
    n->next = p->u.t.attr_list;
    p->u.t.attr_list = n;
}

/*
 * search_term: Parse CCL search term. 
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
static struct ccl_rpn_node *search_term (struct ccl_rpn_attr **qa)
{
    struct ccl_rpn_node *p;
    struct ccl_token *lookahead = look_token;
    int len = 0;
    int no, i;
    int left_trunc = 0;
    int right_trunc = 0;
    int mid_trunc = 0;
    int relation_value = -1;
    int position_value = -1;
    int structure_value = -1;
    int truncation_value = -1;
    int completeness_value = -1;

    if (KIND != CCL_TOK_TERM)
    {
        ccl_error = CCL_ERR_TERM_EXPECTED;
        return NULL;
    }
    /* create the term node, but wait a moment before adding the term */
    p = mk_node (CCL_RPN_TERM);
    p->u.t.attr_list = NULL;
    p->u.t.term = NULL;

    if (!qa)
    {
        /* no qualifier(s) applied. Use 'term' if it is defined */

        qa = malloc (2*sizeof(*qa));
	assert (qa);
	qa[0] = ccl_qual_search (bibset, "term", 4);
	qa[1] = NULL;
    }

    /* go through all attributes and add them to the attribute list */
    for (i=0; qa && qa[i]; i++)
    {
        struct ccl_rpn_attr *attr;

        for (attr = qa[i]; attr; attr = attr->next)
            if (attr->value > 0)
	    {   /* deal only with REAL attributes (positive) */
	        switch (attr->type)
		{
		case CCL_BIB1_REL:
		    if (relation_value != -1)
		        continue;
		    relation_value = attr->value;
		    break;
		case CCL_BIB1_POS:
		    if (position_value != -1)
		        continue;
		    position_value = attr->value;
		    break;
		case CCL_BIB1_STR:
		    if (structure_value != -1)
		        continue;
		    structure_value = attr->value;
		    break;
		case CCL_BIB1_TRU:
		    if (truncation_value != -1)
		        continue;
		    truncation_value = attr->value;
		    break;
		case CCL_BIB1_COM:
		    if (completeness_value != -1)
		        continue;
		    completeness_value = attr->value;
		    break;
		}
                add_attr (p, attr->type, attr->value);
	    }
    }
    /* go through each TERM token. If no truncation attribute is yet
       met, then look for left/right truncation markers (?) and
       set left_trunc/right_trunc/mid_trunc accordingly */
    for (no = 0; lookahead->kind == CCL_TOK_TERM; no++)
    {
        for (i = 0; i<lookahead->len; i++)
            if (truncation_value == -1 && lookahead->name[i] == '?')
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
    /* len now holds the number of characters in the RPN term */
    /* no holds the number of CCL tokens (1 or more) */

    if (structure_value == -1 && 
        qual_val_type (qa, CCL_BIB1_STR, CCL_BIB1_STR_WP))
    {   /* no structure attribute met. Apply either structure attribute 
           WORD or PHRASE depending on number of CCL tokens */
        if (no == 1)
            add_attr (p, CCL_BIB1_STR, 2);
        else
            add_attr (p, CCL_BIB1_STR, 1);
    }

    /* make the RPN token */
    p->u.t.term = malloc (len);
    assert (p->u.t.term);
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
    if (left_trunc && right_trunc)
    {
        if (!qual_val_type (qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_BOTH))
        {
            ccl_error = CCL_ERR_TRUNC_NOT_BOTH;
            free (qa);
            ccl_rpn_delete (p);
            return NULL;
        }
        add_attr (p, CCL_BIB1_TRU, 3);
    }
    else if (right_trunc)
    {
        if (!qual_val_type (qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_RIGHT))
        {
            ccl_error = CCL_ERR_TRUNC_NOT_RIGHT;
            free (qa);
            ccl_rpn_delete (p);
            return NULL;
        }
        add_attr (p, CCL_BIB1_TRU, 1);
    }
    else if (left_trunc)
    {
        if (!qual_val_type (qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_LEFT))
        {
            ccl_error = CCL_ERR_TRUNC_NOT_LEFT;
            free (qa);
            ccl_rpn_delete (p);
            return NULL;
        }
        add_attr (p, CCL_BIB1_TRU, 2);
    }
    else
    {
        if (qual_val_type (qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_NONE))
            add_attr (p, CCL_BIB1_TRU, 100);
    }
    return p;
}

/*
 * qualifiers: Parse CCL qualifiers and search terms. 
 * la:     Token pointer to RELATION token.
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
static struct ccl_rpn_node *qualifiers (struct ccl_token *la,
                                        struct ccl_rpn_attr **qa)
{
    struct ccl_token *lookahead = look_token;
    struct ccl_rpn_attr **ap;
    int no = 0;
    int i, rel;
#if 0
    if (qa)
    {
        ccl_error = CCL_ERR_DOUBLE_QUAL;
        return NULL;
    }
#endif
    for (lookahead = look_token; lookahead != la; lookahead=lookahead->next)
        no++;
    if (qa)
        for (i=0; qa[i]; i++)
	    no++;
    ap = malloc ((no+1) * sizeof(*ap));
    assert (ap);
    for (i = 0; look_token != la; i++)
    {
        ap[i] = ccl_qual_search (bibset, look_token->name, look_token->len);
        if (!ap[i])
        {
            ccl_error = CCL_ERR_UNKNOWN_QUAL;
            free (ap);
            return NULL;
        }
        ADVANCE;
        if (KIND == CCL_TOK_COMMA)
            ADVANCE;
    }
    if (qa)
        while (*qa)
	    ap[i++] = *qa++;
    ap[i] = NULL;
    if (!qual_val_type (ap, CCL_BIB1_REL, CCL_BIB1_REL_ORDER))
    {                
        /* unordered relation */
        struct ccl_rpn_node *p;
        if (KIND != CCL_TOK_EQ)
        {
            ccl_error = CCL_ERR_EQ_EXPECTED;
            free (ap);
            return NULL;
        }
        ADVANCE;
        if (KIND == CCL_TOK_LP)
        {
            ADVANCE;
            if (!(p = find_spec (ap)))
            {
                free (ap);
                return NULL;
            }
            if (KIND != CCL_TOK_RP)
            {
                ccl_error = CCL_ERR_RP_EXPECTED;
                ccl_rpn_delete (p);
                free (ap);
                return NULL;
            }
            ADVANCE;
        }
        else
            p = search_terms (ap);
        free (ap);
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
        if (KIND == CCL_TOK_TERM && look_token->next->kind == CCL_TOK_MINUS)
        {
            struct ccl_rpn_node *p1;
            if (!(p1 = search_term (ap)))
	    {
	        free (ap);
	        return NULL;
	    }
            ADVANCE;                   /* skip '-' */
            if (KIND == CCL_TOK_TERM)  /* = term - term  ? */
            {
                struct ccl_rpn_node *p2;
                
                if (!(p2 = search_term (ap)))
		{
                    ccl_rpn_delete (p1);
		    free (ap);
		    return NULL;
		}
                p = mk_node (CCL_RPN_AND);
                p->u.p[0] = p1;
                add_attr (p1, CCL_BIB1_REL, 4);
                p->u.p[1] = p2;
                add_attr (p2, CCL_BIB1_REL, 2);
                free (ap);
                return p;
            }
            else                       /* = term -    */
            {
                add_attr (p1, CCL_BIB1_REL, 4);
                free (ap);
                return p1;
            }
        }
        else if (KIND == CCL_TOK_MINUS)   /* = - term  ? */
        {
            ADVANCE;
            if (!(p = search_term (ap)))
	    {
	        free (ap);
		return NULL;
	    }
            add_attr (p, CCL_BIB1_REL, 2);
            free (ap);
            return p;
        }
	else if (KIND == CCL_TOK_LP)
	{
            ADVANCE;
            if (!(p = find_spec (ap)))
            {
                free (ap);
                return NULL;
            }
            if (KIND != CCL_TOK_RP)
            {
                ccl_error = CCL_ERR_RP_EXPECTED;
                ccl_rpn_delete (p);
                free (ap);
                return NULL;
            }
            ADVANCE;
	    free (ap);
	    return p;
	}
	else
	{
            if (!(p = search_terms (ap)))
	    {
	        free (ap);
		return NULL;
	    }
            add_attr (p, CCL_BIB1_REL, rel);
	    free (ap);
	    return p;
	}
        ccl_error = CCL_ERR_TERM_EXPECTED;
    }
    free (ap);
    return NULL;
}

/*
 * search_terms: Parse CCL search terms - including proximity.
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
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

/*
 * search_elements: Parse CCL search elements
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
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

    while (lookahead->kind==CCL_TOK_TERM)
    {
        lookahead = lookahead->next;
        if (lookahead->kind == CCL_TOK_REL || lookahead->kind == CCL_TOK_EQ)
	    return qualifiers (lookahead, qa);
        if (lookahead->kind != CCL_TOK_COMMA)
            break;
        lookahead = lookahead->next;
    }
    return search_terms (qa);
}

/*
 * find_spec: Parse CCL find specification
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
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

/*
 * ccl_find: Parse CCL find - token representation
 * abibset: Bibset to be used for the parsing
 * list:    List of tokens
 * error:   Pointer to integer. Holds error no. on completion.
 * pos:     Pointer to char position. Holds approximate error position.
 * return:  RPN tree on successful completion; NULL otherwise.
 */
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

/*
 * ccl_find_str: Parse CCL find - string representation
 * bibset:  Bibset to be used for the parsing
 * str:     String to be parsed
 * error:   Pointer to integer. Holds error no. on completion.
 * pos:     Pointer to char position. Holds approximate error position.
 * return:  RPN tree on successful completion; NULL otherwise.
 */
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
