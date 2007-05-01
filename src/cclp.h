/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: cclp.h,v 1.5 2007-05-01 12:22:11 adam Exp $
 */

/** 
 * \file cclp.h 
 * \brief CCL header with private definitions 
 */
#include <yaz/ccl.h>

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

typedef struct ccl_qualifier *ccl_qualifier_t;

/** CCL token */
struct ccl_token {
    char kind;
    size_t len;                 /* length of name below */
    const char *name;           /* string / name of token */
    struct ccl_token *next;
    struct ccl_token *prev;
    const char *ws_prefix_buf;  /* leading white space buf */
    size_t ws_prefix_len;       /* leading white space len */
};

/** CCL parser structure */
struct ccl_parser {
    /** current lookahead token */
    struct ccl_token *look_token;
    
    /** holds error code if error occur */
    int error_code;
    /** start of CCL string buffer */
    const char *start_pos;
    /** if error occurs, this holds position (starting from 0). */
    const char *error_pos;
    
    /** current bibset */
    CCL_bibset bibset;

    /** names of and operator */
    const char **ccl_token_and;
    /** names of or operator */
    const char **ccl_token_or;
    /** names of not operator */
    const char **ccl_token_not;
    /** names of set operator */
    const char **ccl_token_set;
    /** 1=CCL parser is case sensitive, 0=case insensitive */
    int ccl_case_sensitive;
};

/**
 * Splits CCL command string into individual tokens using
 * a CCL parser.
 */
YAZ_EXPORT
struct ccl_token *ccl_parser_tokenize (CCL_parser cclp, const char *command);

/** 
 * Deletes token list
 */
YAZ_EXPORT
void ccl_token_del (struct ccl_token *list);

/**
 * Add single token after a given onde.
 */
YAZ_EXPORT
struct ccl_token *ccl_token_add (struct ccl_token *at);


YAZ_EXPORT
struct ccl_rpn_node *ccl_parser_find_token(CCL_parser cclp,
                                           struct ccl_token *list);


YAZ_EXPORT
ccl_qualifier_t ccl_qual_search(CCL_parser cclp, const char *name, 
                                size_t name_len, int seq);

YAZ_EXPORT
struct ccl_rpn_attr *ccl_qual_get_attr(ccl_qualifier_t q);

YAZ_EXPORT
const char *ccl_qual_get_name(ccl_qualifier_t q);

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

