/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/* bison parser for CQL grammar. */
%{
/**
 * \file cql.c
 * \brief Implements CQL parser.
 *
 * This is a YACC parser, but since it must be reentrant, Bison is required.
 * The original source file is cql.y.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
/* avoid that bison stuff defines malloc/free - already in stdlib.h */
#ifdef _MSC_VER
#define _STDLIB_H 1
#endif
#include <string.h>
#include <yaz/yaz-iconv.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/cql.h>

    /** Node in the LALR parse tree. */
    typedef struct {
        /** Inhereted attribute: relation */
        struct cql_node *rel;
        /** Synthesized attribute: CQL node */
        struct cql_node *cql;
        /** string buffer with token */
        char *buf;
        /** length of token */
        size_t len;
        /** size of buffer (len <= size) */
        size_t size;
    } token;

    struct cql_parser {
        int (*getbyte)(void *client_data);
        void (*ungetbyte)(int b, void *client_data);
        void *client_data;
        int last_error;
        int last_pos;
        struct cql_node *top;
        NMEM nmem;
        int strict;
    };

#define YYSTYPE token

int yylex(YYSTYPE *lval, void *vp);
int yyerror(void *lval, char *msg);

%}


%lex-param {void *parm}
%parse-param {void *parm}
%define api.pure
%token PREFIX_NAME SIMPLE_STRING AND OR NOT PROX GE LE NE EXACT SORTBY

%%

top: {
    $$.rel = cql_node_mk_sc(((CQL_parser) parm)->nmem,
                            "cql.serverChoice", "=", 0);
    ((CQL_parser) parm)->top = 0;
} cqlQuery1 sortby {
    cql_node_destroy($$.rel);
    if ($3.cql)
    {
        $3.cql->u.sort.search = $2.cql;
        ((CQL_parser) parm)->top = $3.cql;
    } else {
        ((CQL_parser) parm)->top = $2.cql;
    }
}
;

sortby: /* empty */
  { $$.cql = 0; }
| SORTBY sortSpec {
    $$.cql = $2.cql;
 };

sortSpec: sortSpec singleSpec {
    $$.cql = $1.cql;
    $$.cql->u.sort.next = $2.cql;
 }
| singleSpec
{
    $$.cql = $1.cql;
};

singleSpec: index modifiers {
    $$.cql = cql_node_mk_sort(((CQL_parser) parm)->nmem, $1.buf, $2.cql);
 }
;

cqlQuery1: cqlQuery
| cqlQuery error {
    cql_node_destroy($1.cql);
    $$.cql = 0;
}
;

cqlQuery:
  scopedClause
 |
  '>' searchTerm '=' searchTerm {
    $$.rel = $0.rel;
  } cqlQuery {
    $$.cql = cql_apply_prefix(((CQL_parser) parm)->nmem,
                              $6.cql, $2.buf, $4.buf);
  }
| '>' searchTerm {
      $$.rel = $0.rel;
  } cqlQuery {
    $$.cql = cql_apply_prefix(((CQL_parser) parm)->nmem,
                              $4.cql, 0, $2.buf);
   }
;

scopedClause:
  searchClause
|
  scopedClause boolean modifiers {
      $$.rel = $0.rel;
  } searchClause {
      struct cql_node *cn = cql_node_mk_boolean(((CQL_parser) parm)->nmem,
                                                $2.buf);

      cn->u.boolean.modifiers = $3.cql;
      cn->u.boolean.left = $1.cql;
      cn->u.boolean.right = $5.cql;

      $$.cql = cn;
  }
;

searchClause:
  '(' {
      $$.rel = $0.rel;

  } cqlQuery ')' {
      $$.cql = $3.cql;
  }
|
searchTerm extraTerms {
      struct cql_node *st = cql_node_dup(((CQL_parser) parm)->nmem, $0.rel);
      st->u.st.extra_terms = $2.cql;
      st->u.st.term = nmem_strdup(((CQL_parser)parm)->nmem, $1.buf);
      $$.cql = st;
  }
|
  index relation modifiers {
      $$.rel = cql_node_mk_sc(((CQL_parser) parm)->nmem, $1.buf, $2.buf, 0);
      $$.rel->u.st.modifiers = $3.cql;
  } searchClause {
      $$.cql = $5.cql;
      cql_node_destroy($4.rel);
  }
;

extraTerms:
SIMPLE_STRING extraTerms {
    struct cql_node *st = cql_node_mk_sc(((CQL_parser) parm)->nmem,
                                         /* index */ 0, /* rel */ 0, $1.buf);
    st->u.st.extra_terms = $2.cql;
    $$.cql = st;
}
|
{ $$.cql = 0; }
;


/* unary NOT search SIMPLE_STRING here .. */

boolean:
  AND | OR | NOT | PROX ;

modifiers: modifiers '/' searchTerm
{
    struct cql_node *mod = cql_node_mk_sc(((CQL_parser)parm)->nmem,
                                          $3.buf, 0, 0);

    mod->u.st.modifiers = $1.cql;
    $$.cql = mod;
}
|
modifiers '/' searchTerm relation_symbol searchTerm
{
    struct cql_node *mod = cql_node_mk_sc(((CQL_parser)parm)->nmem,
                                          $3.buf, $4.buf, $5.buf);

    mod->u.st.modifiers = $1.cql;
    $$.cql = mod;
}
|
{
    $$.cql = 0;
}
;

relation: PREFIX_NAME | relation_symbol;

relation_symbol:
  '='
| '>'
| '<'
| GE
| LE
| NE
| EXACT
;

index:
  searchTerm;

searchTerm:
  SIMPLE_STRING
| PREFIX_NAME
| AND
| OR
| NOT
| PROX
| SORTBY
;

%%

int yyerror(void *locp, char *s)
{
    return 0;
}

/**
 * putb is a utility that puts one character to the string
 * in current lexical token. This routine deallocates as
 * necessary using NMEM.
 */

static void putb(YYSTYPE *lval, CQL_parser cp, int c)
{
    if (lval->len+1 >= lval->size)
    {
        char *nb = (char *)
            nmem_malloc(cp->nmem, (lval->size = lval->len * 2 + 20));
        memcpy(nb, lval->buf, lval->len);
        lval->buf = nb;
    }
    if (c)
        lval->buf[lval->len++] = c;
    lval->buf[lval->len] = '\0';
}


/**
 * yylex returns next token for Bison to be read. In this
 * case one of the CQL terminals are returned.
 */
int yylex(YYSTYPE *lval, void *vp)
{
    CQL_parser cp = (CQL_parser) vp;
    int c;
    lval->cql = 0;
    lval->rel = 0;
    lval->len = 0;
    lval->size = 10;
    lval->buf = (char *) nmem_malloc(cp->nmem, lval->size);
    lval->buf[0] = '\0';
    do
    {
        c = cp->getbyte(cp->client_data);
        if (c == 0)
            return 0;
        if (c == '\n')
            return 0;
    } while (yaz_isspace(c));
    if (strchr("()=></", c))
    {
        int c1;
        putb(lval, cp, c);
        if (c == '=')
        {
            c1 = cp->getbyte(cp->client_data);
            if (c1 == '=')
            {
                putb(lval, cp, c1);
                return EXACT;
            }
            else
                cp->ungetbyte(c1, cp->client_data);
        }
        else if (c == '>')
        {
            c1 = cp->getbyte(cp->client_data);
            if (c1 == '=')
            {
                putb(lval, cp, c1);
                return GE;
            }
            else
                cp->ungetbyte(c1, cp->client_data);
        }
        else if (c == '<')
        {
            c1 = cp->getbyte(cp->client_data);
            if (c1 == '=')
            {
                putb(lval, cp, c1);
                return LE;
            }
            else if (c1 == '>')
            {
                putb(lval, cp, c1);
                return NE;
            }
            else
                cp->ungetbyte(c1, cp->client_data);
        }
        return c;
    }
    if (c == '"')
    {
        while ((c = cp->getbyte(cp->client_data)) != 0 && c != '"')
        {
            if (c == '\\')
            {
                putb(lval, cp, c);
                c = cp->getbyte(cp->client_data);
                if (!c)
                    break;
            }
            putb(lval, cp, c);
        }
        putb(lval, cp, 0);
        return SIMPLE_STRING;
    }
    else
    {
        int relation_like = 0;
        while (c != 0 && !strchr(" \n()=<>/", c))
        {
            if (c == '.')
                relation_like = 1;
            if (c == '\\')
            {
                putb(lval, cp, c);
                c = cp->getbyte(cp->client_data);
                if (!c)
                    break;
            }
            putb(lval, cp, c);
            c = cp->getbyte(cp->client_data);
        }
        putb(lval, cp, 0);
#if YYDEBUG
        printf ("got %s\n", lval->buf);
#endif
        if (c != 0)
            cp->ungetbyte(c, cp->client_data);
        if (!cql_strcmp(lval->buf, "and"))
        {
            lval->buf = "and";
            return AND;
        }
        if (!cql_strcmp(lval->buf, "or"))
        {
            lval->buf = "or";
            return OR;
        }
        if (!cql_strcmp(lval->buf, "not"))
        {
            lval->buf = "not";
            return NOT;
        }
        if (!cql_strcmp(lval->buf, "prox"))
        {
            lval->buf = "prox";
            return PROX;
        }
        if (!cql_strcmp(lval->buf, "sortby"))
        {
            lval->buf = "sortby";
            return SORTBY;
        }
        if (cp->strict)
            return PREFIX_NAME;
        if (!cql_strcmp(lval->buf, "all"))
            relation_like = 1;
        if (!cql_strcmp(lval->buf, "any"))
            relation_like = 1;
        if (!cql_strcmp(lval->buf, "adj"))
            relation_like = 1;
        if (relation_like)
            return PREFIX_NAME;
    }
    return SIMPLE_STRING;
}


int cql_parser_stream(CQL_parser cp,
                      int (*getbyte)(void *client_data),
                      void (*ungetbyte)(int b, void *client_data),
                      void *client_data)
{
    nmem_reset(cp->nmem);
    cp->getbyte = getbyte;
    cp->ungetbyte = ungetbyte;
    cp->client_data = client_data;
    cql_node_destroy(cp->top);
    cql_parse(cp);
    if (cp->top)
        return 0;
    return -1;
}

CQL_parser cql_parser_create(void)
{
    CQL_parser cp = (CQL_parser) xmalloc(sizeof(*cp));

    cp->top = 0;
    cp->getbyte = 0;
    cp->ungetbyte = 0;
    cp->client_data = 0;
    cp->last_error = 0;
    cp->last_pos = 0;
    cp->nmem = nmem_create();
    cp->strict = 0;
    return cp;
}

void cql_parser_destroy(CQL_parser cp)
{
    cql_node_destroy(cp->top);
    nmem_destroy(cp->nmem);
    xfree (cp);
}

struct cql_node *cql_parser_result(CQL_parser cp)
{
    return cp->top;
}

void cql_parser_strict(CQL_parser cp, int mode)
{
    cp->strict = mode;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
