/* CCL - lexical analysis
 * Europagate, 1995
 *
 * $Log: ccltoken.c,v $
 * Revision 1.4  1995-11-01 13:54:22  quinn
 * Minor adjustments
 *
 * Revision 1.3  1995/09/29  17:12:00  quinn
 * Smallish
 *
 * Revision 1.2  1995/09/27  15:02:44  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.1  1995/04/10  10:28:22  quinn
 * Added copy of CCL.
 *
 * Revision 1.5  1995/02/23  08:32:00  adam
 * Changed header.
 *
 * Revision 1.3  1995/02/15  17:42:16  adam
 * Minor changes of the api of this module. FILE* argument added
 * to ccl_pr_tree.
 *
 * Revision 1.2  1995/02/14  19:55:13  adam
 * Header files ccl.h/cclp.h are gone! They have been merged an
 * moved to ../include/ccl.h.
 * Node kind(s) in ccl_rpn_node have changed names.
 *
 * Revision 1.1  1995/02/13  12:35:21  adam
 * First version of CCL. Qualifiers aren't handled yet.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <ccl.h>

static int strin (const char *s, const char *cset)
{
    while (*cset)
    {
	if (*cset++ == *s)
	    return 1;
    }
    return 0;
}

const char *ccl_token_and = "and";
const char *ccl_token_or = "or";
const char *ccl_token_not = "not";
const char *ccl_token_set = "set";

struct ccl_token *ccl_tokenize (const char *command)
{
    const char *cp = command;
    struct ccl_token *first = NULL;
    struct ccl_token *last = NULL;

    while (1)
    {
	while (*cp && strin (cp, " \t\r\n"))
	{
	    cp++;
	    continue;
	}
	if (!first)
	{
	    first = last = xmalloc (sizeof (*first));
	    assert (first);
	    last->prev = NULL;
	}
	else
	{
	    last->next = xmalloc (sizeof(*first));
	    assert (last->next);
	    last->next->prev = last;
	    last = last->next;
	}
	last->next = NULL;
	last->name = cp;
	last->len = 1;
	switch (*cp++)
	{
        case '\0':
            last->kind = CCL_TOK_EOL;
            return first;
	case '(':
	    last->kind = CCL_TOK_LP;
	    break;
	case ')':
	    last->kind = CCL_TOK_RP;
	    break;
	case ',':
	    last->kind = CCL_TOK_COMMA;
	    break;
	case '%':
	case '!':
	    last->kind = CCL_TOK_PROX;
	    while (*cp == '%' || *cp == '!')
	    {
		++ last->len;
		cp++;
	    }
	    break;
	case '>':
	case '<':
	case '=':
	    if (*cp == '=' || *cp == '<' || *cp == '>')
	    {
		cp++;
		last->kind = CCL_TOK_REL;
		++ last->len;
	    }
	    else if (cp[-1] == '=')
		last->kind = CCL_TOK_EQ;
	    else
		last->kind = CCL_TOK_REL;
	    break;
	case '-':
	    last->kind = CCL_TOK_MINUS;
	    break;
	case '\"':
	    last->kind = CCL_TOK_TERM;
	    last->name = cp;
	    last->len = 0;
	    while (*cp && *cp != '\"')
	    {
		cp++;
		++ last->len;
	    }
	    if (*cp == '\"')
		cp++;
	    break;
	default:
	    while (*cp && !strin (cp, "(),%!><=- \t\n\r"))
	    {
		cp++;
		++ last->len;
	    }
	    if (strlen (ccl_token_and)==last->len &&
		!memcmp (ccl_token_and, last->name, last->len))
		last->kind = CCL_TOK_AND;
	    else if (strlen (ccl_token_or)==last->len &&
		!memcmp (ccl_token_or, last->name, last->len))
		last->kind = CCL_TOK_OR;
	    else if (strlen (ccl_token_not)==last->len &&
		!memcmp (ccl_token_not, last->name, last->len))
		last->kind = CCL_TOK_NOT;
	    else if (strlen (ccl_token_set)==last->len &&
		!memcmp (ccl_token_set, last->name, last->len))
		last->kind = CCL_TOK_SET;
	    else
		last->kind = CCL_TOK_TERM;
	}
    }
    return first;
}
