/* $Id: zing.h,v 1.1 2003-01-06 08:20:28 adam Exp $
   Copyright (C) 2002
   Index Data Aps

This file is part of the YAZ toolkit.
 
See file LICENSE for details.
*/
//gsoap zs service name: SRW
//gsoap zs service encoding: literal
//gsoap zs service namespace: http://www.loc.gov/zing/srw/v1.0/
//gsoap zs schema namespace: http://www.loc.gov/zing/srw/v1.0/
//gsoap xcql schema namespace: http://www.loc.gov/zing/cql/v1.0/xcql/
//gsoap xsort schema namespace: http://www.loc.gov/zing/srw/v1.0/xsortkeys/
//gsoap diag schema namespace: http://www.loc.gov/zing/srw/v1.0/diagnostic/

typedef char *xsd__string;
typedef int xsd__integer;
typedef int xsd__boolean;

typedef xsd__string zs__idType;
typedef char *XML;

struct zs__recordType {
    xsd__string recordSchema;  
    xsd__string recordData;
    xsd__integer recordPosition 0; 
};

struct zs__records {
    int __sizeRecords;
    struct zs__recordType **record;
};

struct diag__diagnosticType {
    xsd__integer code;
    xsd__string details 0;
};

struct zs__diagnostics {
    int __sizeDiagnostics;
    struct diag__diagnosticType **diagnostic;
};

struct zs__searchRetrieveResponse {
    xsd__integer numberOfRecords;
    xsd__string resultSetId 1; 
    xsd__integer resultSetIdleTime 0;
    
    struct zs__records records 0;
    struct zs__diagnostics diagnostics 0;
    xsd__integer *nextRecordPosition 0;
    xsd__string debugInfo;
};

struct xcql__prefixType {
    xsd__string name;
    xsd__string identifier;
};

struct xcql__prefixesType {
    int __sizePrefix;
    struct xcql__prefixType **prefix;
};

struct xcql__relationType {
    xsd__string value;
    struct xcql__modifiersType *modifiers 0;
};

struct xcql__searchClauseType {
    struct xcql__prefixesType *prefixes 0;
    xsd__string index 0;
    struct xcql__relationType *relation 0;
    xsd__string term;
}; 

struct xcql__modifierType {
    xsd__string type 0;
    xsd__string value;
};

struct xcql__modifiersType {
    int __sizeModifier;
    struct xcql__modifierType **modifier; 
};

struct xcql__booleanType {
    xsd__string value;
    struct xcql__modifiersType *modifiers 0;
};

struct xcql__operandType {
    struct xcql__searchClauseType *searchClause 0;
    struct xcql__tripleType *triple 0;
};

struct xcql__tripleType {
    struct xcql__prefixesType *prefixes 0;
    struct xcql__booleanType *boolean;
    struct xcql__operandType *leftOperand;
    struct xcql__operandType *rightOperand;
};

struct xsort__sortKeyType {
    xsd__string path;
    xsd__string schema 0;
    xsd__boolean ascending 0;
    xsd__boolean caseSensitive 0;
    xsd__string missingValue 0;
};

struct xsort__xSortKeysType {
    int __sizeSortKey;
    struct xsort__sortKeyType **sortKey; 
};

int zs__searchRetrieveRequest (
    xsd__string *query,
    struct xcql__operandType *xQuery,
    xsd__string *sortKeys,
    struct xsort__xSortKeysType *xSortKeys,
    xsd__integer *startRecord,
    xsd__integer *maximumRecords,
    xsd__string *recordSchema,
    xsd__string *recordPacking,
    struct zs__searchRetrieveResponse *searchRetrieveResponse
);

struct zs__explainResponse {
    xsd__string Explain;
};

int zs__explainRequest (
    struct zs__explainResponse *explainResponse
);
