/*
 * Copyright (c) 1995-1998, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * $Log: backend.h,v $
 * Revision 1.23  1998-10-13 16:12:23  adam
 * Added support for Surrogate Diagnostics for Scan Term entries.
 *
 * Revision 1.22  1998/09/02 12:41:51  adam
 * Added decode stream in bend search structures.
 *
 * Revision 1.21  1998/07/20 12:38:41  adam
 * Implemented delete result set service to server API.
 *
 * Revision 1.20  1998/05/27 16:57:06  adam
 * Support for surrogate diagnostic records added for bend_fetch.
 *
 * Revision 1.19  1998/03/31 11:07:45  adam
 * Furhter work on UNIverse resource report.
 * Added Extended Services handling in frontend server.
 *
 * Revision 1.18  1998/02/10 11:03:56  adam
 * Added support for extended handlers in backend server interface.
 *
 * Revision 1.17  1998/01/29 13:15:35  adam
 * Implemented sort for the backend interface.
 *
 * Revision 1.16  1997/09/17 12:10:31  adam
 * YAZ version 1.4.
 *
 */

#ifndef BACKEND_H
#define BACKEND_H

#include <yconfig.h>
#include <proto.h>
#include <statserv.h>

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct request *bend_request;
typedef struct association *bend_association;

/* old search request input */ 
typedef struct 
{
    char *setname;             /* name to give to this set */
    int replace_set;           /* replace set, if it already exists */
    int num_bases;             /* number of databases in list */
    char **basenames;          /* databases to search */
    Z_Query *query;            /* query structure */
    ODR stream;                /* encoding stream */
    ODR decode;                /* decoding stream */
} bend_searchrequest;

/* old search request output */
typedef struct
{
    int hits;                  /* number of hits */
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
} bend_searchresult;

/* extended search handler (rr = request response) */
typedef struct {
    char *setname;             /* name to give to this set */
    int replace_set;           /* replace set, if it already exists */
    int num_bases;             /* number of databases in list */
    char **basenames;          /* databases to search */
    Z_Query *query;            /* query structure */
    ODR stream;                /* encode stream */
    ODR decode;                /* decode stream */

    bend_request request;
    bend_association association;
    int *fd;
    int hits;                  /* number of hits */
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
} bend_search_rr;

/* extended present handler. Does not replace bend_fetch. */
typedef struct {
    char *setname;             /* set name */
    int start;
    int number;                /* record number */
    oid_value format;          /* One of the CLASS_RECSYN members */
    Z_RecordComposition *comp; /* Formatting instructions */
    ODR stream;                /* encoding stream - memory source if required */
    bend_request request;
    bend_association association;

    int hits;                  /* number of hits */
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
} bend_present_rr;

YAZ_EXPORT bend_searchresult *bend_search(void *handle, bend_searchrequest *r,
                                          int *fd);
YAZ_EXPORT int bend_searchresponse(void *handle, bend_search_rr *bsrr);

typedef struct
{
    char *setname;             /* set name */
    int number;                /* record number */
    oid_value format;          /* One of the CLASS_RECSYN members */
    Z_RecordComposition *comp; /* Formatting instructions */
    ODR stream;                /* encoding stream - memory source if req */
    int surrogate_flag;        /* surrogate diagnostic flag (rw) */
} bend_fetchrequest;

typedef struct
{
    char *basename;            /* name of database that provided record */
    int len;                   /* length of record or -1 if structured */
    char *record;              /* record */
    int last_in_set;           /* is it?  */
    oid_value format;          /* format */
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
} bend_fetchresult;

YAZ_EXPORT bend_fetchresult *bend_fetch(void *handle, bend_fetchrequest *r,
                                        int *fd);
YAZ_EXPORT bend_fetchresult *bend_fetchresponse(void *handle);

typedef struct
{
    int num_bases;      /* number of elements in databaselist */
    char **basenames;   /* databases to search */
    oid_value attributeset;
    Z_AttributesPlusTerm *term;
    int term_position;  /* desired index of term in result list */
    int num_entries;    /* number of entries requested */
    ODR stream;         /* encoding stream - memory source if required */
} bend_scanrequest;

struct scan_entry {
    char *term;         /* the returned scan term */
    int occurrences;    /* no of occurrences or -1 if error (see below) */
    int errcode;        /* Bib-1 diagnostic code; only used when occur.= -1 */
    char *errstring;    /* Additional string */
};

typedef enum {
    BEND_SCAN_SUCCESS,   /* ok */
    BEND_SCAN_PARTIAL   /* not all entries could be found */
} bend_scan_status;

typedef struct bend_scanresult
{
    int num_entries;
    struct scan_entry *entries;
    int term_position;
    bend_scan_status status;
    int errcode;
    char *errstring;
} bend_scanresult;

YAZ_EXPORT bend_scanresult *bend_scan(void *handle, bend_scanrequest *r,
                                      int *fd);
YAZ_EXPORT bend_scanresult *bend_scanresponse(void *handle);

/* delete handler */
typedef struct bend_delete_rr {
    int function;
    int num_setnames;
    char **setnames;
    int delete_status;
    ODR stream;
} bend_delete_rr;

/* close handler */
YAZ_EXPORT void bend_close(void *handle);

/* sort handler */
typedef struct bend_sort_rr
{
    int num_input_setnames;
    char **input_setnames;
    char *output_setname;
    Z_SortKeySpecList *sort_sequence;
    ODR stream;

    int sort_status;
    int errcode;
    char *errstring;
} bend_sort_rr;

/* extended services handler. Added in from DALI */
typedef struct bend_esrequest_rr
{
    int ItemNo;
    Z_ExtendedServicesRequest *esr;
    ODR stream;                /* encoding stream */
    bend_request request;
    bend_association association;
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
} bend_esrequest_rr;

typedef struct bend_initrequest
{
    char *configname;
    Z_IdAuthentication *auth;
    ODR stream;                /* encoding stream */
    
    int (*bend_sort) (void *handle, bend_sort_rr *rr);
    int (*bend_search) (void *handle, bend_search_rr *rr);
    int (*bend_present) (void *handle, bend_present_rr *rr);
    int (*bend_esrequest) (void *handle, bend_esrequest_rr *rr);
    int (*bend_delete)(void *handle, bend_delete_rr *rr);
} bend_initrequest;

typedef struct bend_initresult
{
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
    void *handle;              /* private handle to the backend module */
} bend_initresult;

YAZ_EXPORT bend_initresult *bend_init(bend_initrequest *r);

YAZ_EXPORT void bend_request_send (bend_association a, bend_request req,
				   Z_APDU *res);

YAZ_EXPORT bend_request bend_request_mk (bend_association a);

YAZ_EXPORT void bend_request_destroy (bend_request *req);

YAZ_EXPORT Z_ReferenceId *bend_request_getid (ODR odr, bend_request req);
YAZ_EXPORT int bend_backend_respond (bend_association a, bend_request req);
YAZ_EXPORT void bend_request_setdata(bend_request r, void *p);
YAZ_EXPORT void *bend_request_getdata(bend_request r);

#ifdef __cplusplus
}
#endif

#endif
