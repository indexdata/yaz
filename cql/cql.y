/* $Id: cql.y,v 1.5 2003-06-04 09:44:05 adam Exp $
   Copyright (C) 2002-2003
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
%token TERM AND OR NOT PROX EXACT ALL ANY GE LE NE SCR
%expect 8

%%

top: { 
    $$.rel = cql_node_mk_sc("srw.serverChoice", "scr", 0);
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
  cqlQuery boolean { 
      $$.rel = $0.rel; 
  } searchClause {
      struct cql_node *cn = cql_node_mk_boolean($2.buf);
      
      cn->u.boolean.modifiers = $2.rel;
      cn->u.boolean.left = $1.cql;
      cn->u.boolean.right = $4.cql;

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
  index relation {
      $$.rel = $2.rel;
      $$.rel->u.st.index = strdup($1.buf);
  } searchClause {
      $$.cql = $4.cql;
      cql_node_destroy($2.rel);
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

boolean: 
  AND | OR | NOT | PROX proxqualifiers {
      $$ = $1;
      $$.rel = $2.rel;
  }
  ;

proxqualifiers: 
  Prelation { 
      $$.rel = cql_node_mk_proxargs ($1.buf, 0, 0, 0);
  }
|
  PrelationO Pdistance {
      $$.rel = cql_node_mk_proxargs ($1.buf, $2.buf, 0, 0);
  }
|
  PrelationO PdistanceO Punit {
      $$.rel = cql_node_mk_proxargs ($1.buf, $2.buf, $3.buf, 0);
  }
|
  PrelationO PdistanceO PunitO Pordering {
      $$.rel = cql_node_mk_proxargs ($1.buf, $2.buf, $3.buf, $4.buf);
  }
|
{ $$.rel = 0; }
;

Punit: '/' searchTerm { 
      $$ = $2;
   }
;

PunitO: '/' searchTerm {
      $$ = $2;
   } 
| 
'/' { $$.buf[0] = 0; }
;
Prelation: '/' baseRelation {
    $$ = $2;
}
;
PrelationO: '/' baseRelation {
    $$ = $2;
}
| '/' { $$.buf[0] = 0; }
;
Pdistance: '/' searchTerm { 
    $$ = $2;
}
;

PdistanceO: '/' searchTerm {
    $$ = $2;
}
| '/' { $$.buf[0] = 0; }
;
Pordering: '/' searchTerm { 
    $$ = $2;
}
;

relation: baseRelation modifiers {
    struct cql_node *st = cql_node_mk_sc(/* index */ 0, 
                                         /* relation */ $1.buf, 
                                         /* term */ 0);

    st->u.st.modifiers = $2.cql;
    $$.rel = st;
}
;

modifiers: '/' searchTerm modifiers
{ 
    struct cql_node *mod = cql_node_mk_mod(0, $2.buf);

    mod->u.mod.next = $3.cql;
    $$.cql = mod;
}
|  
{ 
    $$.cql = 0;
}
;

baseRelation: 
  '=' 
| '>' 
| '<'
| GE
| LE
| NE
| EXACT 
| ALL
| ANY
| SCR
;

index: 
  searchTerm;

searchTerm:
  TERM
| AND
| OR
| NOT
| EXACT
| ALL
| ANY
| PROX
;

%%

int yyerror(char *s)
{
    return 0;
}

#include "lexer.c"


int cql_parser_stream(CQL_parser cp,
                      int (*getbyte)(void *client_data),
                      void (*ungetbyte)(int b, void *client_data),
                      void *client_data)
{
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
