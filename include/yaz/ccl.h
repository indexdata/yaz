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

/*
 * CCL - header file
 *
 * $Id: ccl.h,v 1.17 2004-09-22 12:17:24 adam Exp $
 *
 * Old Europagate Log:
 *
 * Revision 1.10  1996/01/08  08:41:22  adam
 * Minor changes.
 *
 * Revision 1.9  1995/07/20  08:15:16  adam
 * Bug fix: Token value for comma and OR were the same!
 *
 * Revision 1.8  1995/07/11  12:28:34  adam
 * New function: ccl_token_simple (split into simple tokens) and
 *  ccl_token_del (delete tokens).
 *
 * Revision 1.7  1995/05/16  09:39:38  adam
 * LICENSE.
 *
 * Revision 1.6  1995/05/11  14:04:03  adam
 * Changes in the reading of qualifier(s). New function: ccl_qual_fitem.
 * New variable ccl_case_sensitive, which controls whether reserved
 * words and field names are case sensitive or not.
 *
 * Revision 1.5  1995/02/23  08:32:11  adam
 * Changed header.
 *
 * Revision 1.3  1995/02/16  13:20:10  adam
 * Spell fix.
 *
 * Revision 1.2  1995/02/15  17:43:08  adam
 * Minor changes to the ccl interface. Bug fix in iso2709 module.
 *
 * Revision 1.1  1995/02/14  19:55:21  adam
 * Header files ccl.h/cclp.h are gone! They have been merged an
 * moved to ../include/ccl.h.
 *
 */

#ifndef CCL_H
#define CCL_H

#include <yaz/yconfig.h>
#include <stdio.h>
#include <yaz/xmalloc.h>
#include <yaz/wrbuf.h>

YAZ_BEGIN_CDECL
    
/* CCL error numbers */
#define CCL_ERR_OK                0
#define CCL_ERR_TERM_EXPECTED     1
#define CCL_ERR_RP_EXPECTED       2
#define CCL_ERR_SETNAME_EXPECTED  3
#define CCL_ERR_OP_EXPECTED       4
#define CCL_ERR_BAD_RP            5
#define CCL_ERR_UNKNOWN_QUAL      6
#define CCL_ERR_DOUBLE_QUAL       7
#define CCL_ERR_EQ_EXPECTED       8
#define CCL_ERR_BAD_RELATION      9
#define CCL_ERR_TRUNC_NOT_LEFT   10
#define CCL_ERR_TRUNC_NOT_BOTH   11
#define CCL_ERR_TRUNC_NOT_RIGHT  12
    
/* attribute pair (type, value) */
struct ccl_rpn_attr {
    struct ccl_rpn_attr *next;
    char *set;
    int type;
    int kind;
#define CCL_RPN_ATTR_NUMERIC 1
#define CCL_RPN_ATTR_STRING 2
    union {
	int numeric;
	char *str;
    } value;
};

#define CCL_RPN_AND 1
#define CCL_RPN_OR 2
#define CCL_RPN_NOT 3
#define CCL_RPN_TERM 4
#define CCL_RPN_SET 5
#define CCL_RPN_PROX 6

/* RPN tree structure */
struct ccl_rpn_node {
    int kind;
    union {
	struct ccl_rpn_node *p[3];
	struct {
	    char *term;
	    struct ccl_rpn_attr *attr_list;
	} t;
	char *setname;
    } u;
};

typedef struct ccl_qualifiers *CCL_bibset;

/* use (1)

   relation (2)
                            -1  none
                             0  ordered
                           1-6  relation (<, <=, =, >=, >, <>)

   position (3)
                            -1  none
                             1  first in field
                             2  first in sub field
                             3  any position in field
   structure (4)
                            -1  none
                             0  word/phrase auto select
                             1  phrase
                             2  word
                             3  key
                             4  year
                             5  date (normalized)
                             6  word list 
                           100  date (un-normalized)
                           101  name (normalized)
                           102  name (un-normalized)
   truncation (5)                            
   completeness (6)
*/

#define CCL_BIB1_USE 1
#define CCL_BIB1_REL 2
#define CCL_BIB1_POS 3
#define CCL_BIB1_STR 4
#define CCL_BIB1_TRU 5
#define CCL_BIB1_COM 6

#define CCL_BIB1_STR_WP (-1)
#define CCL_BIB1_STR_AND_LIST (-2)
#define CCL_BIB1_STR_OR_LIST (-3)
#define CCL_BIB1_REL_ORDER (-1)

#define CCL_BIB1_TRU_CAN_LEFT (-1)
#define CCL_BIB1_TRU_CAN_RIGHT (-2)
#define CCL_BIB1_TRU_CAN_BOTH  (-3)
#define CCL_BIB1_TRU_CAN_NONE  (-4)

#define CCL_TOK_EOL   0
#define CCL_TOK_TERM  1
#define CCL_TOK_REL   2
#define CCL_TOK_EQ    3
#define CCL_TOK_PROX  4
#define CCL_TOK_LP    5
#define CCL_TOK_RP    6
#define CCL_TOK_COMMA 7
#define CCL_TOK_AND   8
#define CCL_TOK_OR    9
#define CCL_TOK_NOT   10
#define CCL_TOK_SET   11

/* CCL token */
struct ccl_token {
    char kind;
    size_t len;
    const char *name;
    struct ccl_token *next;
    struct ccl_token *prev;
};

/* CCL Qualifier */
struct ccl_qualifier {
    char *name;
    int no_sub;
    struct ccl_qualifier **sub;
    struct ccl_rpn_attr *attr_list;
    struct ccl_qualifier *next;
};

struct ccl_parser {
/* current lookahead token */
    struct ccl_token *look_token;
    
/* holds error code if error occur (and approx position of error) */
    int error_code;
    const char *error_pos;
    
/* current bibset */
    CCL_bibset bibset;

    char *ccl_token_and;
    char *ccl_token_or;
    char *ccl_token_not;
    char *ccl_token_set;
    int ccl_case_sensitive;
};
    
typedef struct ccl_parser *CCL_parser;
    
/* Generate tokens from command string - obeys all CCL opererators */
YAZ_EXPORT struct ccl_token *ccl_parser_tokenize (CCL_parser cclp,
				       const char *command);
YAZ_EXPORT struct ccl_token *ccl_tokenize (const char *command);
    
/* Generate tokens from command string - oebeys only simple tokens and 
   quoted strings */
YAZ_EXPORT struct ccl_token *ccl_token_simple (const char *command);

/* Delete token list */
YAZ_EXPORT void ccl_token_del (struct ccl_token *list);

/* Parse CCL Find command - NULL-terminated string */
YAZ_EXPORT struct ccl_rpn_node *ccl_find_str (CCL_bibset bibset,
                                   const char *str, int *error, int *pos);

/* Parse CCL Find command - Tokens read by ccl_tokenize */
YAZ_EXPORT struct ccl_rpn_node *ccl_find (CCL_bibset abibset, struct ccl_token *list,
                               int *error, const char **pos);

/* Parse CCL Find command */
YAZ_EXPORT struct ccl_rpn_node *ccl_parser_find (CCL_parser cclp, struct ccl_token *list);

/* Set various OPs */
YAZ_EXPORT void ccl_parser_set_op_and (CCL_parser p, const char *op);
YAZ_EXPORT void ccl_parser_set_op_or (CCL_parser p, const char *op);
YAZ_EXPORT void ccl_parser_set_op_not (CCL_parser p, const char *op);
YAZ_EXPORT void ccl_parser_set_op_set (CCL_parser p, const char *op);

YAZ_EXPORT void ccl_parser_set_case (CCL_parser p, int case_sensitivity_flag);

/* Return english-readable error message */
YAZ_EXPORT const char *ccl_err_msg (int ccl_errno);

/* Delete RPN tree returned by ccl_find */
YAZ_EXPORT void ccl_rpn_delete (struct ccl_rpn_node *rpn);

/* Dump RPN tree in readable format to fd_out */
YAZ_EXPORT void ccl_pr_tree (struct ccl_rpn_node *rpn, FILE *fd_out);

/* Add CCL qualifier */
YAZ_EXPORT void ccl_qual_add (CCL_bibset b, const char *name, int no,
			      int *attr);

YAZ_EXPORT void ccl_qual_add_set (CCL_bibset b, const char *name, int no,
				  int *type, int *value, char **svalue,
				  char **attsets);

YAZ_EXPORT void ccl_qual_add_special (CCL_bibset bibset,
                                      const char *n, const char *v);

YAZ_EXPORT void ccl_qual_add_combi (CCL_bibset b, const char *n,
                                    const char *names);

/* Read CCL qualifier list spec from file inf */
YAZ_EXPORT void ccl_qual_file (CCL_bibset bibset, FILE *inf);

/* Read CCL qualifier list spec from file inf */
YAZ_EXPORT int ccl_qual_fname (CCL_bibset bibset, const char *fname);

/* Add CCL qualifier as buf spec (multiple lines). */
YAZ_EXPORT void ccl_qual_buf(CCL_bibset bibset, const char *buf);

/* Add CCL qualifier as line spec. Note: line is _modified_ */
YAZ_EXPORT void ccl_qual_line(CCL_bibset bibset, char *line);

/* Add CCL qualifier by using qual_name + value pair */
YAZ_EXPORT void ccl_qual_fitem (CCL_bibset bibset, const char *value,
                                const char *qual_name);

/* Make CCL qualifier set */
YAZ_EXPORT CCL_bibset ccl_qual_mk (void);

/* Delete CCL qualifier set */
YAZ_EXPORT void ccl_qual_rm (CCL_bibset *b);

/* Char-to-upper function */
extern int (*ccl_toupper)(int c);

/* String utilities */
YAZ_EXPORT int ccl_stricmp (const char *s1, const char *s2);
YAZ_EXPORT int ccl_memicmp (const char *s1, const char *s2, size_t n);

/* Search for qualifier 'name' in set 'b'. */
YAZ_EXPORT struct ccl_rpn_attr *ccl_qual_search (CCL_parser cclp,
                                                 const char *name,
                                                 size_t len,
                                                 int seq);

/* Create CCL parser */
YAZ_EXPORT CCL_parser ccl_parser_create (void);

/* Destroy CCL parser */
YAZ_EXPORT void ccl_parser_destroy (CCL_parser p);

YAZ_EXPORT char *ccl_strdup (const char *str);

YAZ_EXPORT const char *ccl_qual_search_special (CCL_bibset b,
						const char *name);

YAZ_EXPORT void ccl_pquery (WRBUF w, struct ccl_rpn_node *p);

#ifndef ccl_assert
#define ccl_assert(x) ;
#endif

YAZ_END_CDECL

#endif

