/*
 * Copyright (c) 1995-2000, Index Data.
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
 * Revision 1.10  2000-10-02 11:07:44  adam
 * Added peer_name member for bend_init handler. Changed the YAZ
 * client so that tcp: can be avoided in target spec.
 *
 * Revision 1.9  2000/08/31 10:20:12  adam
 * Added member request_format and output_format for backend fetch method.
 *
 * Revision 1.8  2000/08/31 09:51:25  adam
 * Added record_syntax member for fetch method (raw OID).
 *
 * Revision 1.7  2000/04/05 07:39:55  adam
 * Added shared library support (libtool).
 *
 * Revision 1.6  2000/03/20 19:06:25  adam
 * Added Segment request for fronend server. Work on admin for client.
 *
 * Revision 1.5  2000/03/15 12:59:49  adam
 * Added handle member to statserv_control.
 *
 * Revision 1.4  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.3  2000/01/12 14:36:07  adam
 * Added printing stream (ODR) for backend functions.
 *
 * Revision 1.2  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.28  1999/11/04 14:58:44  adam
 * Added status elements for backend delete result set handler.
 * Updated delete result result set command for client.
 *
 * Revision 1.27  1999/10/11 10:01:24  adam
 * Implemented bend_sort_rr handler for frontend server.
 *
 * Revision 1.26  1999/06/17 10:54:44  adam
 * Added facility to specify implementation version - and name
 * for server.
 *
 * Revision 1.25  1999/06/01 14:29:12  adam
 * Work on Extended Services.
 *
 * Revision 1.24  1999/03/31 11:18:24  adam
 * Implemented odr_strdup. Added Reference ID to backend server API.
 *
 * Revision 1.23  1998/10/13 16:12:23  adam
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

#include <yaz/yconfig.h>
#include <yaz/proto.h>

YAZ_BEGIN_CDECL
    
typedef struct request *bend_request;
typedef struct association *bend_association;

/* extended search handler (rr = request response) */
typedef struct {
    char *setname;             /* name to give to this set */
    int replace_set;           /* replace set, if it already exists */
    int num_bases;             /* number of databases in list */
    char **basenames;          /* databases to search */
    Z_ReferenceId *referenceId;/* reference ID */
    Z_Query *query;            /* query structure */
    ODR stream;                /* encode stream */
    ODR decode;                /* decode stream */
    ODR print;                 /* print stream */

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
    Z_ReferenceId *referenceId;/* reference ID */
    Z_RecordComposition *comp; /* Formatting instructions */
    ODR stream;                /* encoding stream - memory source if required */
    ODR print;                 /* printing stream */
    bend_request request;
    bend_association association;

    int hits;                  /* number of hits */
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
} bend_present_rr;

typedef struct bend_fetch_rr {
    char *setname;             /* set name */
    int number;                /* record number */
    Z_ReferenceId *referenceId;/* reference ID */
    oid_value request_format;  /* One of the CLASS_RECSYN members */
    int *request_format_raw;   /* same as above (raw OID) */
    Z_RecordComposition *comp; /* Formatting instructions */
    ODR stream;                /* encoding stream - memory source if req */
    ODR print;                 /* printing stream */

    char *basename;            /* name of database that provided record */
    int len;                   /* length of record or -1 if structured */
    char *record;              /* record */
    int last_in_set;           /* is it?  */
    oid_value output_format;   /* format */
    int *output_format_raw;    /* used instead of above if not-null */
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
    int surrogate_flag;        /* surrogate diagnostic */
} bend_fetch_rr;

struct scan_entry {
    char *term;         /* the returned scan term */
    int occurrences;    /* no of occurrences or -1 if error (see below) */
    int errcode;        /* Bib-1 diagnostic code; only used when occur.= -1 */
    char *errstring;    /* Additional string */
};

typedef enum {
    BEND_SCAN_SUCCESS,  /* ok */
    BEND_SCAN_PARTIAL   /* not all entries could be found */
} bend_scan_status;

typedef struct bend_scan_rr {
    int num_bases;      /* number of elements in databaselist */
    char **basenames;   /* databases to search */
    oid_value attributeset;
    Z_ReferenceId *referenceId; /* reference ID */
    Z_AttributesPlusTerm *term;
    ODR stream;         /* encoding stream - memory source if required */
    ODR print;          /* printing stream */

    int term_position;  /* desired index of term in result list/returned */
    int num_entries;    /* number of entries requested/returned */

    struct scan_entry *entries;
    bend_scan_status status;
    int errcode;
    char *errstring;
} bend_scan_rr;

/* delete handler */
typedef struct bend_delete_rr {
    int function;
    int num_setnames;
    char **setnames;
    Z_ReferenceId *referenceId;
    int delete_status;      /* status for the whole operation */
    int *statuses;          /* status each set - indexed as setnames */
    ODR stream;
    ODR print; 
} bend_delete_rr;

/* sort handler */
typedef struct bend_sort_rr
{
    int num_input_setnames;
    char **input_setnames;
    char *output_setname;
    Z_SortKeySpecList *sort_sequence;
    ODR stream;
    ODR print;
    Z_ReferenceId *referenceId;/* reference ID */

    int sort_status;
    int errcode;
    char *errstring;
} bend_sort_rr;

typedef struct bend_esrequest_rr
{
    int ItemNo;
    Z_ExtendedServicesRequest *esr;
    
    ODR stream;                /* encoding stream */
    ODR decode;                /* decoding stream */
    ODR print;                 /* printing stream */
    Z_ReferenceId *referenceId;/* reference ID */
    bend_request request;
    bend_association association;
    int errcode;               /* 0==success, -1==accepted, >0 = failure */
    char *errstring;           /* system error string or NULL */
} bend_esrequest_rr;

typedef struct bend_segment_rr {
    Z_Segment *segment;
    ODR stream;
    ODR decode;
    ODR print;
    bend_request request;
    bend_association association;
} bend_segment_rr;

typedef struct bend_initrequest
{
    Z_IdAuthentication *auth;
    ODR stream;                /* encoding stream */
    ODR print;                 /* printing stream */
    Z_ReferenceId *referenceId;/* reference ID */
    char *peer_name;           /* dns host of peer (client) */
    
    char *implementation_name;
    char *implementation_version;
    int (*bend_sort) (void *handle, bend_sort_rr *rr);
    int (*bend_search) (void *handle, bend_search_rr *rr);
    int (*bend_fetch) (void *handle, bend_fetch_rr *rr);
    int (*bend_present) (void *handle, bend_present_rr *rr);
    int (*bend_esrequest) (void *handle, bend_esrequest_rr *rr);
    int (*bend_delete)(void *handle, bend_delete_rr *rr);
    int (*bend_scan)(void *handle, bend_scan_rr *rr);
    int (*bend_segment)(void *handle, bend_segment_rr *rr);
} bend_initrequest;

typedef struct bend_initresult
{
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
    void *handle;              /* private handle to the backend module */
} bend_initresult;

YAZ_EXPORT void bend_request_send (bend_association a, bend_request req,
				   Z_APDU *res);

YAZ_EXPORT bend_request bend_request_mk (bend_association a);

YAZ_EXPORT void bend_request_destroy (bend_request *req);

YAZ_EXPORT Z_ReferenceId *bend_request_getid (ODR odr, bend_request req);
YAZ_EXPORT int bend_backend_respond (bend_association a, bend_request req);
YAZ_EXPORT void bend_request_setdata(bend_request r, void *p);
YAZ_EXPORT void *bend_request_getdata(bend_request r);

typedef struct statserv_options_block
{
    int dynamic;                  /* fork on incoming requests */
    int threads;                  /* use threads */
    int one_shot;                 /* one session then exit(1) */
    int loglevel;                 /* desired logging-level */
    char apdufile[ODR_MAXNAME+1]; /* file for pretty-printed PDUs */
    char logfile[ODR_MAXNAME+1];  /* file for diagnostic output */
    char default_listen[1024];    /* 0 == no default listen */
    enum oid_proto default_proto; /* PROTO_SR or PROTO_Z3950 */
    int idle_timeout;             /* how many minutes to wait before closing */
    int maxrecordsize;            /* maximum value for negotiation */
    char configname[ODR_MAXNAME+1];  /* given to the backend in bend_init */
    char setuid[ODR_MAXNAME+1];     /* setuid to this user after binding */
    void (*bend_start)(struct statserv_options_block *p);
    void (*bend_stop)(struct statserv_options_block *p);
    int (*options_func)(int argc, char **argv);
    int (*check_ip)(void *cd, const char *addr, int len, int type);
    char daemon_name[128];
    int inetd;                    /* Do we use the inet deamon or not */

    void *handle;                 /* Handle */
    bend_initresult *(*bend_init)(bend_initrequest *r);
    void (*bend_close)(void *handle);
#ifdef WIN32
    /* We only have these members for the windows version */
    /* They seemed a bit large to have them there in general */
    char service_name[128];         /* NT Service Name */
    char app_name[128];             /* Application Name */
    char service_dependencies[128]; /* The services we are dependent on */
    char service_display_name[128]; /* The service display name */
#endif /* WIN32 */
} statserv_options_block;

YAZ_EXPORT int statserv_main(
    int argc, char **argv,
    bend_initresult *(*bend_init)(bend_initrequest *r),
    void (*bend_close)(void *handle));
YAZ_EXPORT int statserv_start(int argc, char **argv);
YAZ_EXPORT void statserv_closedown(void);
YAZ_EXPORT statserv_options_block *statserv_getcontrol(void);
YAZ_EXPORT void statserv_setcontrol(statserv_options_block *block);
YAZ_EXPORT int check_ip_tcpd(void *cd, const char *addr, int len, int type);

YAZ_END_CDECL

#endif
