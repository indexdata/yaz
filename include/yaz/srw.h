/*
 * Copyright (c) 2002-2005, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: srw.h,v 1.19 2005-01-09 21:52:48 adam Exp $
 */
/**
 * \file srw.h
 * \brief Header for SRW/SRU
 */

#ifndef YAZ_SRW_H
#define YAZ_SRW_H

#include <yaz/soap.h>
#include <yaz/zgdu.h>

YAZ_BEGIN_CDECL

typedef struct {
    char *recordSchema;
    int recordPacking;
#define Z_SRW_recordPacking_string 0
#define Z_SRW_recordPacking_XML 1
    char *recordData_buf;
    int recordData_len;
    int *recordPosition;
} Z_SRW_record;

typedef struct {
    char *uri;
    char *details;
    char *message;
} Z_SRW_diagnostic;
    
typedef struct {

#define Z_SRW_query_type_cql  1
#define Z_SRW_query_type_xcql 2
#define Z_SRW_query_type_pqf  3
    int query_type;
    union {
        char *cql;
        char *xcql;
        char *pqf;
    } query;

#define Z_SRW_sort_type_none 1
#define Z_SRW_sort_type_sort 2
#define Z_SRW_sort_type_xSort 3
    int sort_type;
    union {
        char *none;
        char *sortKeys;
        char *xSortKeys;
    } sort;
    int *startRecord;
    int *maximumRecords;
    char *recordSchema;
    char *recordPacking;
    char *recordXPath;
    char *database;
    char *stylesheet;
    int *resultSetTTL;
} Z_SRW_searchRetrieveRequest;

typedef struct {
    int * numberOfRecords;
    char * resultSetId;
    int * resultSetIdleTime;
    
    Z_SRW_record *records;
    int num_records;

    Z_SRW_diagnostic *diagnostics;
    int num_diagnostics;
    int *nextRecordPosition;
} Z_SRW_searchRetrieveResponse;

typedef struct {
    char *recordPacking;
    char *database;
    char *stylesheet;
} Z_SRW_explainRequest;

typedef struct {
    Z_SRW_record record;
    Z_SRW_diagnostic *diagnostics;
    int num_diagnostics;
} Z_SRW_explainResponse;
    
typedef struct {
    int query_type;
    union {
        char *cql;
        char *xcql;
        char *pqf;
    } scanClause;
    int *responsePosition;
    int *maximumTerms;
    char *stylesheet;
    char *database;
} Z_SRW_scanRequest;

typedef struct {
    char *value;
    int *numberOfRecords;
    char *displayTerm;
    char *whereInList;
} Z_SRW_scanTerm;

typedef struct {
    Z_SRW_scanTerm *terms;
    int num_terms;
    Z_SRW_diagnostic *diagnostics;
    int num_diagnostics;
} Z_SRW_scanResponse;

#define Z_SRW_searchRetrieve_request  1
#define Z_SRW_searchRetrieve_response 2
#define Z_SRW_explain_request 3
#define Z_SRW_explain_response 4
#define Z_SRW_scan_request 5
#define Z_SRW_scan_response 6

typedef struct {
    int which;
    union {
        Z_SRW_searchRetrieveRequest *request;
        Z_SRW_searchRetrieveResponse *response;
        Z_SRW_explainRequest *explain_request;
        Z_SRW_explainResponse *explain_response;
        Z_SRW_scanRequest *scan_request;
        Z_SRW_scanResponse *scan_response;
    } u;
    char *srw_version;
} Z_SRW_PDU;

YAZ_EXPORT int yaz_srw_codec(ODR o, void * pptr,
                             Z_SRW_PDU **handler_data,
                             void *client_data, const char *ns);
YAZ_EXPORT Z_SRW_PDU *yaz_srw_get(ODR o, int which);

YAZ_EXPORT const char *yaz_diag_srw_str (int code);

YAZ_EXPORT int yaz_diag_bib1_to_srw (int bib1_code);

YAZ_EXPORT int yaz_diag_srw_to_bib1(int srw_code);

YAZ_EXPORT char *yaz_uri_val(const char *path, const char *name, ODR o);
YAZ_EXPORT void yaz_uri_val_int(const char *path, const char *name,
				ODR o, int **intp);
YAZ_EXPORT int yaz_srw_decode(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
			      Z_SOAP **soap_package, ODR decode, char **charset);
YAZ_EXPORT int yaz_sru_decode(Z_HTTP_Request *hreq, Z_SRW_PDU **srw_pdu,
			      Z_SOAP **soap_package, ODR decode, 
			      char **charset,
			      Z_SRW_diagnostic **, int *num_diagnostic);

YAZ_EXPORT void yaz_add_srw_diagnostic(ODR o, Z_SRW_diagnostic **d,
				       int *num, int code,
				       const char *addinfo);
    
YAZ_EXPORT void yaz_mk_std_diagnostic(ODR o, Z_SRW_diagnostic *d, 
				      int code, const char *details);
YAZ_END_CDECL

#endif
