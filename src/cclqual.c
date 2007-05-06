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
/** 
 * \file cclqual.c
 * \brief Implements CCL qualifier utilities
 */
/* CCL qualifiers
 * Europagate, 1995
 *
 * $Id: cclqual.c,v 1.12 2007-05-06 20:12:20 adam Exp $
 *
 * Old Europagate Log:
 *
 * Revision 1.9  1995/05/16  09:39:27  adam
 * LICENSE.
 *
 * Revision 1.8  1995/05/11  14:03:57  adam
 * Changes in the reading of qualifier(s). New function: ccl_qual_fitem.
 * New variable ccl_case_sensitive, which controls whether reserved
 * words and field names are case sensitive or not.
 *
 * Revision 1.7  1995/04/17  09:31:46  adam
 * Improved handling of qualifiers. Aliases or reserved words.
 *
 * Revision 1.6  1995/02/23  08:32:00  adam
 * Changed header.
 *
 * Revision 1.4  1995/02/14  19:55:12  adam
 * Header files ccl.h/cclp.h are gone! They have been merged an
 * moved to ../include/ccl.h.
 * Node kind(s) in ccl_rpn_node have changed names.
 *
 * Revision 1.3  1995/02/14  16:20:56  adam
 * Qualifiers are read from a file now.
 *
 * Revision 1.2  1995/02/14  10:25:56  adam
 * The constructions 'qualifier rel term ...' implemented.
 *
 * Revision 1.1  1995/02/13  15:15:07  adam
 * Added handling of qualifiers. Not finished yet.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaz/snprintf.h>
#include <yaz/tokenizer.h>
#include "cclp.h"

/** CCL Qualifier */
struct ccl_qualifier {
    char *name;
    int no_sub;
    struct ccl_qualifier **sub;
    struct ccl_rpn_attr *attr_list;
    struct ccl_qualifier *next;
};


/** Definition of CCL_bibset pointer */
struct ccl_qualifiers {
    struct ccl_qualifier *list;
    struct ccl_qualifier_special *special;
};


/** CCL Qualifier special */
struct ccl_qualifier_special {
    char *name;
    const char **values;
    struct ccl_qualifier_special *next;
};


static struct ccl_qualifier *ccl_qual_lookup(CCL_bibset b,
                                             const char *n, size_t len)
{
    struct ccl_qualifier *q;
    for (q = b->list; q; q = q->next)
        if (len == strlen(q->name) && !memcmp(q->name, n, len))
            break;
    return q;
}

void ccl_qual_add_special_ar(CCL_bibset bibset, const char *n,
                             const char **values)
{
    struct ccl_qualifier_special *p;
    for (p = bibset->special; p && strcmp(p->name, n); p = p->next)
        ;
    if (p)
    {
        if (p->values)
        {
            int i;
            for (i = 0; p->values[i]; i++)
                xfree((char *) p->values[i]);
            xfree((char **)p->values);
        }
    }
    else
    {
        p = (struct ccl_qualifier_special *) xmalloc(sizeof(*p));
        p->name = xstrdup(n);
        p->next = bibset->special;
        bibset->special = p;
    }
    p->values = values;
}

void ccl_qual_add_special(CCL_bibset bibset, const char *n, const char *cp)
{
    size_t no = 2;
    char **vlist = (char **) xmalloc(no * sizeof(*vlist));
    yaz_tok_cfg_t yt = yaz_tok_cfg_create();
    int t;
    size_t i = 0;
    
    yaz_tok_parse_t tp = yaz_tok_parse_buf(yt, cp);
    
    yaz_tok_cfg_destroy(yt);
    
    t = yaz_tok_move(tp);
    while (t == YAZ_TOK_STRING)
    {
        if (i >= no-1)
            vlist = (char **) xrealloc(vlist, (no = no * 2) * sizeof(*vlist));
        vlist[i++] = xstrdup(yaz_tok_parse_string(tp));
        t = yaz_tok_move(tp); 
    }
    vlist[i] = 0;
    ccl_qual_add_special_ar(bibset, n, (const char **) vlist);
    
    yaz_tok_parse_destroy(tp);
}


/** \brief adds specifies qualifier aliases
    
    \param b bibset
    \param n qualifier name
    \param names list of qualifier aliases
*/
void ccl_qual_add_combi(CCL_bibset b, const char *n, const char **names)
{
    int i;
    struct ccl_qualifier *q;
    for (q = b->list; q && strcmp(q->name, n); q = q->next)
        ;
    if (q)
        return ;
    q = (struct ccl_qualifier *) xmalloc(sizeof(*q));
    q->name = xstrdup(n);
    q->attr_list = 0;
    q->next = b->list;
    b->list = q;
    
    for (i = 0; names[i]; i++)
        ;
    q->no_sub = i;
    q->sub = (struct ccl_qualifier **)
        xmalloc(sizeof(*q->sub) * (1+q->no_sub));
    for (i = 0; names[i]; i++)
        q->sub[i] = ccl_qual_lookup(b, names[i], strlen(names[i]));
}

/** \brief adds specifies attributes for qualifier
    
    \param b bibset
    \param name qualifier name
    \param no number of attribute type+value pairs
    \param type_ar attributes type of size no
    \param value_ar attribute value of size no
    \param svalue_ar attribute string values ([i] only used  if != NULL)
    \param attsets attribute sets of size no
*/

void ccl_qual_add_set(CCL_bibset b, const char *name, int no,
                       int *type_ar, int *value_ar, char **svalue_ar,
                       char **attsets)
{
    struct ccl_qualifier *q;
    struct ccl_rpn_attr **attrp;

    ccl_assert(b);
    for (q = b->list; q; q = q->next)
        if (!strcmp(name, q->name))
            break;
    if (!q)
    {
        q = (struct ccl_qualifier *)xmalloc(sizeof(*q));
        ccl_assert(q);
        
        q->next = b->list;
        b->list = q;
        
        q->name = xstrdup(name);
        q->attr_list = 0;

        q->no_sub = 0;
        q->sub = 0;
    }
    attrp = &q->attr_list;
    while (*attrp)
        attrp = &(*attrp)->next;
    while (--no >= 0)
    {
        struct ccl_rpn_attr *attr;

        attr = (struct ccl_rpn_attr *)xmalloc(sizeof(*attr));
        ccl_assert(attr);
        attr->set = *attsets++;
        attr->type = *type_ar++;
        if (*svalue_ar)
        {
            attr->kind = CCL_RPN_ATTR_STRING;
            attr->value.str = *svalue_ar;
        }
        else
        {
            attr->kind = CCL_RPN_ATTR_NUMERIC;
            attr->value.numeric = *value_ar;
        }
        svalue_ar++;
        value_ar++;
        *attrp = attr;
        attrp = &attr->next;
    }
    *attrp = NULL;
}

/** \brief creates Bibset
    \returns bibset
 */
CCL_bibset ccl_qual_mk(void)
{
    CCL_bibset b = (CCL_bibset)xmalloc(sizeof(*b));
    ccl_assert(b);
    b->list = NULL;     
    b->special = NULL;
    return b;
}

/** \brief destroys Bibset
    \param b pointer to Bibset
    
    *b will be set to NULL.
 */
void ccl_qual_rm(CCL_bibset *b)
{
    struct ccl_qualifier *q, *q1;
    struct ccl_qualifier_special *sp, *sp1;

    if (!*b)
        return;
    for (q = (*b)->list; q; q = q1)
    {
        struct ccl_rpn_attr *attr, *attr1;

        for (attr = q->attr_list; attr; attr = attr1)
        {
            attr1 = attr->next;
            if (attr->set)
                xfree(attr->set);
            if (attr->kind == CCL_RPN_ATTR_STRING)
                xfree(attr->value.str);
            xfree(attr);
        }
        q1 = q->next;
        xfree(q->name);
        if (q->sub)
            xfree(q->sub);
        xfree(q);
    }
    for (sp = (*b)->special; sp; sp = sp1)
    {
        sp1 = sp->next;
        xfree(sp->name);
        if (sp->values)
        {
            int i;
            for (i = 0; sp->values[i]; i++)
                xfree((char*) sp->values[i]);
            xfree((char **)sp->values);
        }
        xfree(sp);
    }
    xfree(*b);
    *b = NULL;
}

ccl_qualifier_t ccl_qual_search(CCL_parser cclp, const char *name, 
                                size_t name_len, int seq)
{
    struct ccl_qualifier *q = 0;
    const char **aliases;
    int case_sensitive = cclp->ccl_case_sensitive;

    ccl_assert(cclp);
    if (!cclp->bibset)
        return 0;

    aliases = ccl_qual_search_special(cclp->bibset, "case");
    if (aliases)
        case_sensitive = atoi(aliases[0]);

    for (q = cclp->bibset->list; q; q = q->next)
        if (strlen(q->name) == name_len)
        {
            if (case_sensitive)
            {
                if (!memcmp(name, q->name, name_len))
                    break;
            }
            else
            {
                if (!ccl_memicmp(name, q->name, name_len))
                    break;
            }
        }
    if (q)
    {
        if (q->no_sub)
        {
            if (seq < q->no_sub)
                q = q->sub[seq];
            else
                q = 0;
        }
        else if (seq)
            q = 0;
    }
    return q;
}

struct ccl_rpn_attr *ccl_qual_get_attr(ccl_qualifier_t q)
{
    return q->attr_list;
}

const char *ccl_qual_get_name(ccl_qualifier_t q)
{
    return q->name;
}

const char **ccl_qual_search_special(CCL_bibset b, const char *name)
{
    struct ccl_qualifier_special *q;
    if (!b)
        return 0;
    for (q = b->special; q && strcmp(q->name, name); q = q->next)
        ;
    if (q)
        return q->values;
    return 0;
}

int ccl_search_stop(CCL_bibset bibset, const char *qname,
                    const char *src_str, size_t src_len)
{
    const char **slist = 0;
    if (qname)
    {
        char qname_buf[80];
        yaz_snprintf(qname_buf, sizeof(qname_buf)-1, "stop.%s",
                     qname);
        slist = ccl_qual_search_special(bibset, qname_buf);
    }
    if (!slist)
        slist = ccl_qual_search_special(bibset, "stop.*");
    if (slist)
    {
        int i;
        for (i = 0; slist[i]; i++)
            if (src_len == strlen(slist[i]) 
                && ccl_memicmp(slist[i], src_str, src_len) == 0)
                return 1;
    }
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

