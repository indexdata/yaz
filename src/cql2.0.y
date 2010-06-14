/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
/*
  YACC CQL grammar taken verbatim from the 2.0 draft (June 2010).
 */
%term PREFIX_NAME QUOTED_URI_STRING QUOTED_STRING AND OR NOT PROX SORTBY SIMPLE_STRING

%%
cql_query : query | query sort_spec;

query : prefix_assignment search_clause_group | search_clause_group ;

search_clause_group: search_clause_group boolean_modified subquery | subquery;

subquery : '(' query ')' | search_clause;

search_clause: index relation_modified search_term | search_term;

search_term : SIMPLE_STRING | QUOTED_STRING;

sort_spec : sort_by index_modified_list;

sort_by: SORTBY;

index_modified_list: index_modified_list index_modified | index_modified;

prefix_assignment: '>' prefix '=' uri | '>' uri;

prefix: SIMPLE_STRING;

uri : QUOTED_URI_STRING;

index_modified: index modifier_list | index;

index : simple_name | PREFIX_NAME;

relation_modified : relation modifier_list | relation;

relation : relation_name | relation_symbol;

relation_name: simple_name | PREFIX_NAME;

relation_symbol : '=' | '>' | '<' ;

boolean_modified : boolean modifier_list | boolean;

boolean : AND | OR | NOT | PROX;

modifier_list : modifier_list modifier | modifier;

modifier : '/' modifier_name modifier_relation | '/' modifier_name;

modifier_name: simple_name;

modifier_relation : relation_symbol modifier_value;

modifier_value : SIMPLE_STRING | QUOTED_STRING;

simple_name: SIMPLE_STRING;

%%
