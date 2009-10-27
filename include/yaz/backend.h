/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** 
 * \file backend.h
 * \brief Header for GFS
 *
 * This header includes all public definitions for the
 * Generic Frontend Server (GFS).
 */

#ifndef BACKEND_H
#define BACKEND_H

#include <yaz/yconfig.h>
#include <yaz/proto.h>
#include <yaz/srw.h>
#include <yaz/oid_db.h>

YAZ_BEGIN_CDECL
    
typedef struct request *bend_request;
typedef struct association *bend_association;

/** \brief Information for Z39.50/SRU search handler */
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
    Odr_int hits;                  /* number of hits */
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
    Z_OtherInformation *search_info; /* additional search info */
    char *srw_sortKeys;        /* holds SRU/SRW sortKeys info */
    char *srw_setname;         /* holds SRU/SRW generated resultsetID */
    int *srw_setnameIdleTime;  /* holds SRU/SRW life-time */
    int estimated_hit_count;   /* if hit count is estimated */
    int partial_resultset;     /* if result set is partial */
    Z_SRW_extra_arg *extra_args; /* extra URL arguments */
    char *extra_response_data;   /* extra XML response. */
} bend_search_rr;

/** \brief Information for present handler. Does not replace bend_fetch. */
typedef struct {
    char *setname;             /* set name */
    int start;
    int number;                /* record number */
    Odr_oid *format;           /* format, transfer syntax (OID) */
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

/** \brief Information for fetch record handler */
typedef struct bend_fetch_rr {
    char *setname;             /* set name */
    int number;                /* record number */
    Z_ReferenceId *referenceId;/* reference ID */
    Odr_oid *request_format;        /* format, transfer syntax (OID) */
    Z_RecordComposition *comp; /* Formatting instructions */
    ODR stream;                /* encoding stream - memory source if req */
    ODR print;                 /* printing stream */

    char *basename;            /* name of database that provided record */
    int len;                   /* length of record or -1 if structured */
    char *record;              /* record */
    int last_in_set;           /* is it?  */
    Odr_oid *output_format;        /* response format/syntax (OID) */
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
    int surrogate_flag;        /* surrogate diagnostic */
    char *schema;              /* string record schema input/output */
} bend_fetch_rr;

/** \brief Information for scan entry */
struct scan_entry {
    char *term;         /* the returned scan term */
    Odr_int occurrences;/* no of occurrences or -1 if error (see below) */
    int errcode;        /* Bib-1 diagnostic code; only used when occur.= -1 */
    char *errstring;    /* Additional string */
    char *display_term;
};

typedef enum {
    BEND_SCAN_SUCCESS,  /* ok */
    BEND_SCAN_PARTIAL   /* not all entries could be found */
} bend_scan_status;

/** \brief Information for SRU / Z39.50 scan handler */
typedef struct bend_scan_rr {
    int num_bases;      /* number of elements in databaselist */
    char **basenames;   /* databases to search */
    Odr_oid *attributeset;
    Z_ReferenceId *referenceId; /* reference ID */
    Z_AttributesPlusTerm *term;
    ODR stream;         /* encoding stream - memory source if required */
    ODR print;          /* printing stream */

    Odr_int *step_size;     /* step size */
    Odr_int term_position;  /* desired index of term in result list/returned */
    int num_entries;    /* number of entries requested/returned */

    /* scan term entries. The called handler does not have
       to allocate this. Size of entries is num_entries (see above) */
    struct scan_entry *entries;
    bend_scan_status status;
    int errcode;
    char *errstring;
    char *scanClause;   /* CQL scan clause */
    char *setname;      /* Scan in result set (NULL if omitted) */
} bend_scan_rr;

/** \brief Information for SRU record update handler */
typedef struct bend_update_rr {
    int num_bases;      /* number of elements in databaselist */
    char **basenames;   /* databases to search */
    Z_ReferenceId *referenceId; /* reference ID */
    ODR stream;         /* encoding stream - memory source if required */
    ODR print;          /* printing stream */
    char *operation;
    char *operation_status;
    char *record_id;
    Z_SRW_recordVersion *record_versions;
    int num_versions;
    char *record_packing;
    char *record_schema;
    char *record_data;
    char *extra_record_data;
    char *extra_request_data;
    char *extra_response_data;
    char *uri;
    char *message;
    char *details;
} bend_update_rr;

/** \brief Information for Z39.50 delete result set handler */
typedef struct bend_delete_rr {
    int function;
    int num_setnames;
    char **setnames;
    Z_ReferenceId *referenceId;
    int delete_status;    /* status for the whole operation */
    Odr_int *statuses;    /* status each set - indexed as setnames */
    ODR stream;
    ODR print; 
} bend_delete_rr;

/** \brief Information for Z39.50 sort handler */
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

/** \brief Information for Z39.50 extended services handler */
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
    Z_TaskPackage *taskPackage;
} bend_esrequest_rr;

/** \brief Information for Z39.50 segment handler */
typedef struct bend_segment_rr {
    Z_Segment *segment;
    ODR stream;
    ODR decode;
    ODR print;
    bend_request request;
    bend_association association;
} bend_segment_rr;

/** \brief Information for SRU Explain handler */
typedef struct {
    ODR stream;
    ODR decode;
    ODR print;
    char *explain_buf;
    char *database;
    char *schema;
    void *server_node_ptr;
} bend_explain_rr;

/** \brief Information for the Init handler

This includes both request
information (to be read) and response information which should be
set by the bend_init handler 
*/
typedef struct bend_initrequest
{
    /** \brief user/name/password to be read */
    Z_IdAuthentication *auth; 
    /** \brief encoding stream (for results) */
    ODR stream;
    /** \brief printing stream */
    ODR print;
    /** \brief decoding stream (use stream for results) */
    ODR decode; 
    /** \brief reference ID */
    Z_ReferenceId *referenceId;
    /** \brief peer address of client */
    char *peer_name;           
    
    /** \brief character set and language negotiation 

    see include/yaz/z-charneg.h 
    */
    Z_CharSetandLanguageNegotiation *charneg_request;

    /** \brief character negotiation response */
    Z_External *charneg_response;

    /** \brief character set (encoding) for query terms 
        
    This is NULL by default. It should be set to the native character
    set that the backend assumes for query terms */
    char *query_charset;      

    /** \brief whehter query_charset also applies to recors 
    
    Is 0 (No) by default. Set to 1 (yes) if records is in the same
    character set as queries. If in doubt, use 0 (No).
    */
    int records_in_same_charset;

    char *implementation_id;
    char *implementation_name;
    char *implementation_version;

    /** \brief Z39.50 sort handler */
    int (*bend_sort)(void *handle, bend_sort_rr *rr);
    /** \brief SRU/Z39.50 search handler */
    int (*bend_search)(void *handle, bend_search_rr *rr);
    /** \brief SRU/Z39.50 fetch handler */
    int (*bend_fetch)(void *handle, bend_fetch_rr *rr);
    /** \brief SRU/Z39.50 present handler */
    int (*bend_present)(void *handle, bend_present_rr *rr);
    /** \brief Z39.50 extended services handler */
    int (*bend_esrequest) (void *handle, bend_esrequest_rr *rr);
    /** \brief Z39.50 delete result set handler */
    int (*bend_delete)(void *handle, bend_delete_rr *rr);
    /** \brief Z39.50 scan handler */
    int (*bend_scan)(void *handle, bend_scan_rr *rr);
    /** \brief Z39.50 segment facility handler */
    int (*bend_segment)(void *handle, bend_segment_rr *rr);
    /** \brief SRU explain handler */
    int (*bend_explain)(void *handle, bend_explain_rr *rr);
    /** \brief SRU scan handler */
    int (*bend_srw_scan)(void *handle, bend_scan_rr *rr);
    /** \brief SRU record update handler */
    int (*bend_srw_update)(void *handle, bend_update_rr *rr);

    /** \brief whether named result sets are supported (0=disable, 1=enable) */
    int named_result_sets;
} bend_initrequest;

/** \brief result for init handler (must be filled by handler) */
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

/** \brief control block for server */
typedef struct statserv_options_block
{
    int dynamic;                  /* fork on incoming requests */
    int threads;                  /* use threads */
    int one_shot;                 /* one session then exit(1) */
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
    struct bend_soap_handler *soap_handlers;
    char pid_fname[128];            /* pid fname */
    int background;                 /* auto daemon */
    char cert_fname[128];           /* SSL certificate fname */
    char xml_config[128];           /* XML config filename */
} statserv_options_block;

YAZ_EXPORT int statserv_main(
    int argc, char **argv,
    bend_initresult *(*bend_init)(bend_initrequest *r),
    void (*bend_close)(void *handle));
YAZ_EXPORT statserv_options_block *statserv_getcontrol(void);
YAZ_EXPORT void statserv_setcontrol(statserv_options_block *block);
YAZ_EXPORT int check_ip_tcpd(void *cd, const char *addr, int len, int type);

YAZ_EXPORT int bend_assoc_is_alive(bend_association assoc);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

