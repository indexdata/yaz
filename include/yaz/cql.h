/* $Id: cql.h,v 1.2 2003-02-14 18:49:23 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#ifndef CQL_H_INCLUDED
#define CQL_H_INCLUDED
#include <stdio.h>

typedef struct cql_parser *CQL_parser;

/**
 * cql_parser_create:
 *
 * creates a CQL parser.
 *
 * Returns CQL parser or NULL if parser could not be created.
 */
CQL_parser cql_parser_create(void);

/**
 * cql_parser_destroy:
 * @cp:  A CQL parser
 *
 * Destroy CQL parser. This function does nothing if
 * NULL pointer is received.
 */
void cql_parser_destroy(CQL_parser cp);

/**
 * cql_parser_string:
 * @cp:  A CQL parser.
 * @str:  A query string to be parsed.
 *
 * Parses a CQL query string.
 *
 * Returns 0 if parsing was succesful; non-zero (error code) if
 * unsuccesful.
 */
int cql_parser_string(CQL_parser cp, const char *str);

/**
 * cql_parser_stream:
 * @cp:  A CQL parser.
 * @getbyte:  Handler to read one character (for parsing).
 * @ungetbyte: Handler to unread one byte (for parsing).
 * @client_data:  User data associated with getbyte/ungetbyte handlers.
 *
 * Parses a CQL query from a user defined stream.
 *
 * Returns 0 if parsing was succesful; non-zero (error code) if
 * unsuccesful.
 */
int cql_parser_stream(CQL_parser cp,
                      int (*getbyte)(void *client_data),
                      void (*ungetbyte)(int b, void *client_data),
                      void *client_data);

/**
 * cql_parser_stdio:
 * @cp:  A CQL parser.
 * @f: FILE handler in read mode.
 *
 * Parses a CQL query from a file.
 *
 * Returns 0 if parsing was succesful; non-zero (error code) if
 * unsuccesful.
 */
int cql_parser_stdio(CQL_parser cp, FILE *f);

#define CQL_NODE_ST 1
#define CQL_NODE_BOOL 2
#define CQL_NODE_MOD 3
struct cql_node {
    int which;
    union {
        struct {
            char *index;
            char *term;
            char *relation;
            struct cql_node *modifiers;
            struct cql_node *prefixes;
        } st;
        struct {
            char *value;
            struct cql_node *left;
            struct cql_node *right;
            struct cql_node *modifiers;
            struct cql_node *prefixes;
        } boolean;
        struct {
            char *name;
            char *value;
            struct cql_node *next;
        } mod;
    } u;
};

struct cql_properties;

struct cql_buf_write_info {
    int max;
    int off;
    char *buf;
};

void cql_buf_write_handler (const char *b, void *client_data);

void cql_node_print(struct cql_node *cn);
struct cql_node *cql_node_mk_sc(const char *index,
                                const char *relation,
                                const char *term);
struct cql_node *cql_node_mk_boolean(const char *op);
void cql_node_destroy(struct cql_node *cn);
struct cql_node *cql_node_prefix(struct cql_node *n, const char *prefix,
                                 const char *uri);
struct cql_node *cql_node_mk_mod(const char *name,
                                 const char *value);

struct cql_node *cql_node_dup (struct cql_node *cp);
struct cql_node *cql_parser_result(CQL_parser cp);

void cql_to_xml(struct cql_node *cn, 
                void (*pr)(const char *buf, void *client_data),
                void *client_data);
void cql_to_xml_stdio(struct cql_node *cn, FILE *f);
int cql_to_xml_buf(struct cql_node *cn, char *out, int max);

struct cql_node *cql_node_mk_proxargs(const char *relation,
                                      const char *distance,
                                      const char *unit,
                                      const char *ordering);


void cql_fputs(const char *buf, void *client_data);

typedef struct cql_transform_t_ *cql_transform_t;

cql_transform_t cql_transform_open_FILE (FILE *f);
cql_transform_t cql_transform_open_fname(const char *fname);
void cql_transform_close(cql_transform_t ct);

void cql_transform_pr(cql_transform_t ct,
                      struct cql_node *cn,
                      void (*pr)(const char *buf, void *client_data),
                      void *client_data);

int cql_transform_FILE(cql_transform_t ct,
                       struct cql_node *cn, FILE *f);

int cql_transform_buf(cql_transform_t ct,
                      struct cql_node *cn, char *out, int max);
int cql_transform_error(cql_transform_t ct, const char **addinfo);

/*
10 Illegal query
11 Unsupported query type (XCQL vs CQL)
12 Too many characters in query
13 Unbalanced or illegal use of parentheses
14 Unbalanced or illegal use of quotes
15 Illegal or unsupported index set
16 Illegal or unsupported index
17 Illegal or unsupported combination of index and index set
18 Illegal or unsupported combination of indexes
19 Illegal or unsupported relation
20 Illegal or unsupported relation modifier
21 Illegal or unsupported combination of relation modifers
22 Illegal or unsupported combination of relation and index
23 Too many characters in term
24 Illegal combination of relation and term
25 Special characters not quoted in term
26 Non special character escaped in term
27 Empty term unsupported
28 Masking character not supported
29 Masked words too short
30 Too many masking characters in term
31 Anchoring character not supported
32 Anchoring character in illegal or unsupported position
33 Combination of proximity/adjacency and masking characters not supported
34 Combination of proximity/adjacency and anchoring characters not supported
35 Terms only exclusion (stop) words
36 Term in invalid format for index or relation
37 Illegal or unsupported boolean operator
38 Too many boolean operators in query
39 Proximity not supported
40 Illegal or unsupported proximity relation
41 Illegal or unsupported proximity distance
42 Illegal or unsupported proximity unit
43 Illegal or unsupported proximity ordering
44 Illegal or unsupported combination of proximity modifiers
45 Index set name (prefix) assigned to multiple identifiers
*/

#endif
/* CQL_H_INCLUDED */
