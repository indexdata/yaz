/* $Id: cql.y,v 1.2 2004-03-10 16:34:29 adam Exp $
   Copyright (C) 2002-2004
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.

 bison parser for CQL grammar.
*/
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <yaz/nmem.h>
#include <yaz/cql.h>
    
    typedef struct {
        struct cql_node *rel;
        struct cql_node *cql;
        char *buf;
        size_t len;
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
    };

#define YYSTYPE token
    
#define YYPARSE_PARAM parm
#define YYLEX_PARAM parm
    
    int yylex(YYSTYPE *lval, void *vp);
    int yyerror(char *s);
%}

%pure_parser
%token TERM AND OR NOT PROX GE LE NE
%expect 9

%%

top: { 
    $$.rel = cql_node_mk_sc("cql.serverChoice", "scr", 0);
    ((CQL_parser) parm)->top = 0;
} cqlQuery1 {
    cql_node_destroy($$.rel);
    ((CQL_parser) parm)->top = $2.cql; 
}
;

cqlQuery1: cqlQuery
| cqlQuery error {
    cql_node_destroy($1.cql);
    $$.cql = 0;
}
;

cqlQuery: 
  searchClause
|
  cqlQuery boolean modifiers { 
      $$.rel = $0.rel;
  } searchClause {
      struct cql_node *cn = cql_node_mk_boolean($2.buf);
      
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
  searchTerm {
      struct cql_node *st = cql_node_dup ($0.rel);
      st->u.st.term = strdup($1.buf);
      $$.cql = st;
  }
| 
  index relation modifiers {
      $$.rel = cql_node_mk_sc($1.buf, $2.buf, 0);
      $$.rel->u.st.modifiers = $3.cql;
  } searchClause {
      $$.cql = $5.cql;
      cql_node_destroy($4.rel);
  }
| '>' searchTerm '=' searchTerm {
      $$.rel = $0.rel;
  } cqlQuery {
    $$.cql = cql_node_prefix($6.cql, $2.buf, $4.buf);
  }
| '>' searchTerm {
      $$.rel = $0.rel;
  } cqlQuery {
    $$.cql = cql_node_prefix($4.cql, 0, $2.buf);
   }
;

/* unary NOT search TERM here .. */

boolean: 
  AND | OR | NOT | PROX 
  ;

modifiers: modifiers '/' searchTerm
{ 
    struct cql_node *mod = cql_node_mk_sc($3.buf, "=", 0);

    mod->u.st.modifiers = $1.cql;
    $$.cql = mod;
}
|
modifiers '/' searchTerm mrelation searchTerm
{
    struct cql_node *mod = cql_node_mk_sc($3.buf, $4.buf, $5.buf);

    mod->u.st.modifiers = $1.cql;
    $$.cql = mod;
}
|
{ 
    $$.cql = 0;
}
;

mrelation:
  '=' 
| '>' 
| '<'
| GE
| LE
| NE
;

relation: 
  '=' 
| '>' 
| '<'
| GE
| LE
| NE
| TERM
;

index: 
  searchTerm;

searchTerm:
  TERM
| AND
| OR
| NOT
| PROX
;

%%

int yyerror(char *s)
{
    return 0;
}

/*
 * bison lexer for CQL.
 */

static void putb(YYSTYPE *lval, CQL_parser cp, int c)
{
    if (lval->len+1 >= lval->size)
    {
        char *nb = nmem_malloc(cp->nmem, (lval->size = lval->len * 2 + 20));
        memcpy (nb, lval->buf, lval->len);
        lval->buf = nb;
    }
    if (c)
        lval->buf[lval->len++] = c;
    lval->buf[lval->len] = '\0';
}


int yylex(YYSTYPE *lval, void *vp)
{
    CQL_parser cp = (CQL_parser) vp;
    int c;
    lval->cql = 0;
    lval->rel = 0;
    lval->len = 0;
    lval->size = 10;
    lval->buf = nmem_malloc(cp->nmem, lval->size);
    lval->buf[0] = '\0';
    do
    {
        c = cp->getbyte(cp->client_data);
        if (c == 0)
            return 0;
        if (c == '\n')
            return 0;
    } while (isspace(c));
    if (strchr("()=></", c))
    {
        int c1;
        putb(lval, cp, c);
        if (c == '>')
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
        while ((c = cp->getbyte(cp->client_data)) != EOF && c != '"')
        {
            if (c == '\\')
                c = cp->getbyte(cp->client_data);
            putb(lval, cp, c);
        }
        putb(lval, cp, 0);
    }
    else
    {
        putb(lval, cp, c);
        while ((c = cp->getbyte(cp->client_data)) != 0 &&
               !strchr(" \n()=<>/", c))
        {
            if (c == '\\')
                c = cp->getbyte(cp->client_data);
            putb(lval, cp, c);
        }
#if YYDEBUG
        printf ("got %s\n", lval->buf);
#endif
        if (c != 0)
            cp->ungetbyte(c, cp->client_data);
        if (!strcmp(lval->buf, "and"))
            return AND;
        if (!strcmp(lval->buf, "or"))
            return OR;
        if (!strcmp(lval->buf, "not"))
            return NOT;
        if (!strncmp(lval->buf, "prox", 4))
            return PROX;
    }
    return TERM;
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
    if (cp->top)
        cql_node_destroy(cp->top);
    cql_parse(cp);
    if (cp->top)
        return 0;
    return -1;
}

CQL_parser cql_parser_create(void)
{
    CQL_parser cp = (CQL_parser) malloc (sizeof(*cp));

    cp->top = 0;
    cp->getbyte = 0;
    cp->ungetbyte = 0;
    cp->client_data = 0;
    cp->last_error = 0;
    cp->last_pos = 0;
    cp->nmem = nmem_create();
    return cp;
}

void cql_parser_destroy(CQL_parser cp)
{
    cql_node_destroy(cp->top);
    nmem_destroy(cp->nmem);
    free (cp);
}

struct cql_node *cql_parser_result(CQL_parser cp)
{
    return cp->top;
}
