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
/* CCL qualifiers
 * Europagate, 1995
 *
 * $Log: cclqual.c,v $
 * Revision 1.8  1997-09-29 08:56:38  adam
 * Changed CCL parser to be thread safe. New type, CCL_parser, declared
 * and a create/destructers ccl_parser_create/ccl_parser/destory has
 * been added.
 *
 * Revision 1.7  1997/09/01 08:48:12  adam
 * New windows NT/95 port using MSV5.0. Only a few changes made
 * to avoid warnings.
 *
 * Revision 1.6  1997/04/30 08:52:07  quinn
 * Null
 *
 * Revision 1.5  1996/10/11  15:00:25  adam
 * CCL parser from Europagate Email gateway 1.0.
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
#include <assert.h>
#include <string.h>

#include <ccl.h>

/* Definition of CCL_bibset pointer */
struct ccl_qualifiers {
    struct ccl_qualifier *list;
};

/*
 * ccl_qual_add: Add qualifier to Bibset. If qualifier already
 *               exists, then attributes are appendend to old
 *               definition.
 * name:    name of qualifier
 * no:      No of attribute type/value pairs.
 * pairs:   Attributes. pairs[0] first type, pair[1] first value,
 *          ... pair[2*no-2] last type, pair[2*no-1] last value.
 */
void ccl_qual_add (CCL_bibset b, const char *name, int no, int *pairs)
{
    struct ccl_qualifier *q;
    struct ccl_rpn_attr **attrp;

    assert (b);
    for (q = b->list; q; q = q->next)
        if (!strcmp (name, q->name))
            break;
    if (!q)
    {
        struct ccl_qualifier *new_qual = malloc (sizeof(*new_qual));
        assert (new_qual);
        
        new_qual->next = b->list;
        b->list = new_qual;
        
        new_qual->name = malloc (strlen(name)+1);
        assert (new_qual->name);
        strcpy (new_qual->name, name);
        attrp = &new_qual->attr_list;
    }
    else
    {
        attrp = &q->attr_list;
        while (*attrp)
            attrp = &(*attrp)->next;
    }
    while (--no >= 0)
    {
        struct ccl_rpn_attr *attr;

        attr = malloc (sizeof(*attr));
        assert (attr);
        attr->type = *pairs++;
        attr->value = *pairs++;
        *attrp = attr;
        attrp = &attr->next;
    }
    *attrp = NULL;
}

/*
 * ccl_qual_mk: Make new (empty) bibset.
 * return:   empty bibset.
 */
CCL_bibset ccl_qual_mk (void)
{
    CCL_bibset b = malloc (sizeof(*b));
    assert (b);
    b->list = NULL;     
    return b;
}

/*
 * ccl_qual_rm: Delete bibset.
 * b:        pointer to bibset
 */
void ccl_qual_rm (CCL_bibset *b)
{
    struct ccl_qualifier *q, *q1;

    if (!*b)
        return;
    for (q = (*b)->list; q; q = q1)
    {
        struct ccl_rpn_attr *attr, *attr1;

        for (attr = q->attr_list; attr; attr = attr1)
	{
	    attr1 = attr->next;
	    free (attr);
	}
        q1 = q->next;
	free (q);
    }
    free (*b);
    *b = NULL;
}

/*
 * ccl_qual_search: Search for qualifier in bibset.
 * b:      Bibset
 * name:   Name of qualifier to search for (need no null-termination)
 * len:    Length of name.
 * return: Attribute info. NULL if not found.
 */
struct ccl_rpn_attr *ccl_qual_search (CCL_parser cclp,
				      const char *name, size_t len)
{
    struct ccl_qualifier *q;

    assert (cclp);
    if (!cclp->bibset)
	return NULL;
    for (q = cclp->bibset->list; q; q = q->next)
        if (strlen(q->name) == len)
            if (cclp->ccl_case_sensitive)
            {
                if (!memcmp (name, q->name, len))
                    return q->attr_list;
            }
            else
            {
                if (!ccl_memicmp (name, q->name, len))
                    return q->attr_list;
            }
    return NULL;
}

