/*
 * Copyright (c) 2002-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: srw.h,v 1.6 2003-03-20 21:15:00 adam Exp $
 */

#ifndef YAZ_SRW_H
#define YAZ_SRW_H

#include <yaz/soap.h>

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
    int  *code;
    char *details;
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
    int  *maximumRecords;
    char *recordSchema;
    char *recordPacking;
    char *database;
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

#define Z_SRW_searchRetrieve_request  1
#define Z_SRW_searchRetrieve_response 2

typedef struct {
    int which;
    union {
        Z_SRW_searchRetrieveRequest *request;
        Z_SRW_searchRetrieveResponse *response;
    } u;
} Z_SRW_PDU;

YAZ_EXPORT int yaz_srw_codec(ODR o, void * pptr,
                             Z_SRW_PDU **handler_data,
                             void *client_data, const char *ns);
YAZ_EXPORT Z_SRW_PDU *yaz_srw_get(ODR o, int which);

YAZ_EXPORT const char *yaz_diag_srw_str (int code);

YAZ_EXPORT int yaz_diag_bib1_to_srw (int bib1_code);

YAZ_EXPORT int yaz_diag_srw_to_bib1(int srw_code);


#endif
