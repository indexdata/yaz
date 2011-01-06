/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */
/*
  YACC CQL grammar taken verbatim from the official 1.2 spec
 */
%term GE LE NE AND OR NOT PROX CHARSTRING1 CHARSTRING2 SORTBY

%%
sortedQuery : prefixAssignment sortedQuery 
            | scopedClause
            | scopedClause SORTBY sortSpec;

sortSpec : sortSpec singleSpec | singleSpec;
singleSpec : index modifierList | index ;

cqlQuery : prefixAssignment cqlQuery | scopedClause;

prefixAssignment : '>' prefix '=' uri | '>' uri;

scopedClause : scopedClause booleanGroup searchClause | searchClause ;

booleanGroup: boolean | boolean modifierList;

boolean : AND | OR | NOT | PROX ;

searchClause : '(' cqlQuery ')'
             | index relation searchTerm
	     | searchTerm
	     ;

relation : comparitor | comparitor modifierList;

comparitor : comparitorSymbol | namedComparitor ;

comparitorSymbol : '=' | '>' | '<' | GE | LE | NE;

namedComparitor : identifier;

modifierList : modifierList modifier  | modifier;

modifier : '/' modifierName 
         | '/' modifierName comparitorSymbol modifierValue
	 ;
     

prefix : term;
uri : term;
modifierName: term;
modifierValue: term;
searchTerm: term;
index: term;

term: identifier | AND | OR | NOT | PROX | SORTBY ;

identifier: CHARSTRING1 | CHARSTRING2;

%%
