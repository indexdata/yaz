/* $Id: cqlstd.y,v 1.1 2005-11-15 12:23:32 adam Exp $ 
  YACC CQL grammar taken verbatim from the official spec. We don't
  use that in YAZ but I don't know of a better place to put it.
 */
%term GE LE NE AND OR NOT PROX CHARSTRING1 CHARSTRING2

%%
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

term: identifier | AND | OR | NOT | PROX ;

identifier: CHARSTRING1 | CHARSTRING2;

%%
