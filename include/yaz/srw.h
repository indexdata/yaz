/*
 * Copyright (c) 2002-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: srw.h,v 1.3 2003-02-17 14:35:42 adam Exp $
 */

#ifndef YAZ_SRW_H
#define YAZ_SRW_H

#include <yaz/soap.h>

typedef struct {
    char *recordSchema;
    char *recordData_buf;
    int recordData_len;
    int *recordPosition;
} Z_SRW_record;

typedef struct {
    int  *code;
    char *details;
} Z_SRW_diagnostic;
    
typedef struct {
    char *query;
    char *pQuery;
    void *xQuery;
    char *sortKeys;
    void *xSortKeys;
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
} Z_SRW_searchRetrieve;

YAZ_EXPORT int yaz_srw_codec(ODR o, void * pptr,
                             Z_SRW_searchRetrieve **handler_data,
                             void *client_data, const char *ns);
YAZ_EXPORT Z_SRW_searchRetrieve *yaz_srw_get(ODR o, int which);

YAZ_EXPORT const char *yaz_srw_error_str (int code);


#endif
