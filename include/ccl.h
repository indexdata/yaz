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
 * $Log: ccl.h,v $
 * Revision 1.8  1997-09-29 09:01:19  adam
 * Changed CCL parser to be thread safe. New type, CCL-parser, declared
 * and a create/destructor ccl_parser_create/ccl_parser_destroy has been
 * added.
 *
 * Revision 1.7  1997/09/01 08:49:47  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.6  1997/05/14 06:53:37  adam
 * C++ support.
 *
 * Revision 1.5  1997/04/30 08:52:08  quinn
 * Null
 *
 * Revision 1.4  1996/10/11  15:02:26  adam
 * CCL parser from Europagate Email gateway 1.0.
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

#ifdef __cplusplus
extern "C" {
#endif
    
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
    int type;
    int value;
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
	struct ccl_rpn_node *p[2];
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
#define CCL_TOK_MINUS 11
#define CCL_TOK_SET   12

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
struct ccl_token *ccl_parser_tokenize (CCL_parser cclp,
				       const char *command);
struct ccl_token *ccl_tokenize (const char *command);
    
/* Generate tokens from command string - oebeys only simple tokens and 
   quoted strings */
struct ccl_token *ccl_token_simple (const char *command);

/* Delete token list */
void ccl_token_del (struct ccl_token *list);

/* Parse CCL Find command - NULL-terminated string */
struct ccl_rpn_node *ccl_find_str (CCL_bibset bibset,
                                   const char *str, int *error, int *pos);

/* Parse CCL Find command - Tokens read by ccl_tokenize */
struct ccl_rpn_node *ccl_find (CCL_bibset abibset, struct ccl_token *list,
                               int *error, const char **pos);

/* Return english-readable error message */
char *ccl_err_msg (int ccl_errno);

/* Delete RPN tree returned by ccl_find */
void ccl_rpn_delete (struct ccl_rpn_node *rpn);

/* Dump RPN tree in readable format to fd_out */
void ccl_pr_tree (struct ccl_rpn_node *rpn, FILE *fd_out);

/* Add CCL qualifier */
void ccl_qual_add (CCL_bibset b, const char *name, int no, int *attr);

/* Read CCL qualifier list spec from file inf */
void ccl_qual_file (CCL_bibset bibset, FILE *inf);

/* Add CCL qualifier by using single-line spec */
void ccl_qual_fitem (CCL_bibset bibset, const char *cp, const char *qual_name);

/* Make CCL qualifier set */
CCL_bibset ccl_qual_mk (void);

/* Delete CCL qualifier set */
void ccl_qual_rm (CCL_bibset *b);

/* Char-to-upper function */
extern int (*ccl_toupper)(int c);

/* String utilities */
int ccl_stricmp (const char *s1, const char *s2);
int ccl_memicmp (const char *s1, const char *s2, size_t n);

/* Search for qualifier 'name' in set 'b'. */
struct ccl_rpn_attr *ccl_qual_search (CCL_parser cclp, const char *name,
                                      size_t len);

/* Create CCL parser */
CCL_parser ccl_parser_create (void);

/* Destroy CCL parser */
void ccl_parser_destroy (CCL_parser p);

#ifdef __cplusplus
}
#endif

#endif

