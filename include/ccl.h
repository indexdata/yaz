/* CCL - header file
 * Europagate, 1995
 *
 * $Log: ccl.h,v $
 * Revision 1.1  1995-04-10 10:28:27  quinn
 * Added copy of CCL.
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

/* RPN tree structure */
struct ccl_rpn_node {
    enum rpn_node_kind {
        CCL_RPN_AND, CCL_RPN_OR, CCL_RPN_NOT,
        CCL_RPN_TERM,
        CCL_RPN_SET,
        CCL_RPN_PROX
    } kind;
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
#define CCL_TOK_OR    7
#define CCL_TOK_NOT   9
#define CCL_TOK_MINUS 10
#define CCL_TOK_SET   11

struct ccl_token {
    char kind;
    char len;
    const char *name;
    struct ccl_token *next;
    struct ccl_token *prev;
};

struct ccl_qualifier {
    char *name;
    struct ccl_rpn_attr *attr_list;
    struct ccl_qualifier *next;
};

struct ccl_token *ccl_tokenize (const char *command);

struct ccl_rpn_node *ccl_find_str (CCL_bibset bibset,
                                   const char *str, int *error, int *pos);

struct ccl_rpn_node *ccl_find (CCL_bibset abibset, struct ccl_token *list,
                               int *error, const char **pos);
char *ccl_err_msg (int ccl_errno);
void ccl_rpn_delete (struct ccl_rpn_node *rpn);
void ccl_pr_tree (struct ccl_rpn_node *rpn, FILE *fd_out);

void ccl_qual_add (CCL_bibset b, const char *name, int no, int *attr);
void ccl_qual_file (CCL_bibset bibset, FILE *inf);
CCL_bibset ccl_qual_mk (void);
void ccl_qual_rm (CCL_bibset *b);

extern const char *ccl_token_and;
extern const char *ccl_token_or;
extern const char *ccl_token_not;
extern const char *ccl_token_set;


struct ccl_rpn_attr *ccl_qual_search (CCL_bibset b, const char *name, int len);
#endif

