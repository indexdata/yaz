/* $Id: cql.h,v 1.6 2004-03-10 16:34:29 adam Exp $
   Copyright (C) 2002-2004
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#ifndef CQL_H_INCLUDED
#define CQL_H_INCLUDED
#include <stdio.h>
#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

typedef struct cql_parser *CQL_parser;

/**
 * cql_parser_create:
 *
 * creates a CQL parser.
 *
 * Returns CQL parser or NULL if parser could not be created.
 */
YAZ_EXPORT 
CQL_parser cql_parser_create(void);

/**
 * cql_parser_destroy:
 * @cp:  A CQL parser
 *
 * Destroy CQL parser. This function does nothing if
 * NULL pointer is received.
 */
YAZ_EXPORT 
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
YAZ_EXPORT 
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
YAZ_EXPORT 
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
YAZ_EXPORT
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
    } u;
};

struct cql_properties;

struct cql_buf_write_info {
    int max;
    int off;
    char *buf;
};

YAZ_EXPORT
void cql_buf_write_handler (const char *b, void *client_data);

YAZ_EXPORT
void cql_node_print(struct cql_node *cn);
YAZ_EXPORT
struct cql_node *cql_node_mk_sc(const char *index,
                                const char *relation,
                                const char *term);
YAZ_EXPORT
struct cql_node *cql_node_mk_boolean(const char *op);
YAZ_EXPORT
void cql_node_destroy(struct cql_node *cn);
YAZ_EXPORT
struct cql_node *cql_node_prefix(struct cql_node *n, 
                                 const char *prefix,
                                 const char *uri);
YAZ_EXPORT
struct cql_node *cql_node_dup (struct cql_node *cp);
YAZ_EXPORT
struct cql_node *cql_parser_result(CQL_parser cp);

YAZ_EXPORT
void cql_to_xml(struct cql_node *cn, 
                void (*pr)(const char *buf, void *client_data),
                void *client_data);
YAZ_EXPORT
void cql_to_xml_stdio(struct cql_node *cn, FILE *f);
YAZ_EXPORT
int cql_to_xml_buf(struct cql_node *cn, char *out, int max);

YAZ_EXPORT
struct cql_node *cql_node_mk_proxargs(const char *relation,
                                      const char *distance,
                                      const char *unit,
                                      const char *ordering);


YAZ_EXPORT
void cql_fputs(const char *buf, void *client_data);

typedef struct cql_transform_t_ *cql_transform_t;

YAZ_EXPORT
cql_transform_t cql_transform_open_FILE (FILE *f);
YAZ_EXPORT
cql_transform_t cql_transform_open_fname(const char *fname);
YAZ_EXPORT
void cql_transform_close(cql_transform_t ct);

YAZ_EXPORT
void cql_transform_pr(cql_transform_t ct,
                      struct cql_node *cn,
                      void (*pr)(const char *buf, void *client_data),
                      void *client_data);

YAZ_EXPORT
int cql_transform_FILE(cql_transform_t ct,
                       struct cql_node *cn, FILE *f);

YAZ_EXPORT
int cql_transform_buf(cql_transform_t ct,
                      struct cql_node *cn, char *out, int max);
YAZ_EXPORT
int cql_transform_error(cql_transform_t ct, const char **addinfo);

YAZ_EXPORT
const char *cql_strerror(int code);

YAZ_END_CDECL

#endif
/* CQL_H_INCLUDED */
