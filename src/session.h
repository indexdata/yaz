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
 * \file session.h
 * \brief Internal Header for GFS.
 */
#ifndef SESSION_H
#define SESSION_H

#include <yaz/comstack.h>
#include <yaz/cql.h>
#include <yaz/ccl.h>
#include <yaz/odr.h>
#include <yaz/proto.h>
#include <yaz/backend.h>
#include <yaz/retrieval.h>
#include "eventl.h"

struct gfs_server {
    statserv_options_block cb;
    char *host;
    int listen_ref;
    cql_transform_t cql_transform;
    CCL_bibset ccl_transform;
    void *server_node_ptr;
    char *directory;
    char *docpath;
    char *stylesheet;
    yaz_retrieval_t retrieval;
    struct gfs_server *next;
};

struct gfs_listen {
    char *id;
    char *address;
    struct gfs_listen *next;
};

typedef enum {
    REQUEST_IDLE,    /* the request is just sitting in the queue */
    REQUEST_PENDING  /* operation pending (b'end processing or network I/O*/
    /* this list will have more elements when acc/res control is added */
} request_state;

typedef struct request
{
    int len_refid;          /* length of referenceid */
    char *refid;            /* referenceid */
    request_state state;

    Z_GDU *gdu_request;     /* Current request */
    Z_APDU *apdu_request;   /* Current Z39.50 request */
    NMEM request_mem;    /* memory handle for request */

    int size_response;     /* size of buffer */
    int len_response;      /* length of encoded data */
    char *response;        /* encoded data waiting for transmission */

    void *clientData;
    struct request *next;
    struct request_q *q; 
} request;

typedef struct request_q
{
    request *head;
    request *tail;
    request *list;
    int num;
} request_q;

/*
 * association state.
 */
typedef enum
{
    ASSOC_NEW,                /* not initialized yet or HTTP session */
    ASSOC_UP,                 /* Z39.50 session is UP */
    ASSOC_DEAD                /* dead. Close if input arrives */
} association_state;

typedef struct association
{
    IOCHAN client_chan;           /* event-loop control */
    COMSTACK client_link;         /* communication handle */
    ODR decode;                   /* decoding stream */
    ODR encode;                   /* encoding stream */
    ODR print;                    /* printing stream (for -a) */
    char *encode_buffer;          /* temporary buffer for encoded data */
    int encoded_len;              /* length of encoded data */
    char *input_buffer;           /* input buffer (allocated by comstack) */
    int input_buffer_len;         /* length (size) of buffer */
    int input_apdu_len;           /* length of current incoming APDU */
    oid_proto proto;              /* protocol (PROTO_Z3950/PROTO_SR) */
    void *backend;                /* backend handle */
    request_q incoming;           /* Q of incoming PDUs */
    request_q outgoing;           /* Q of outgoing data buffers (enc. PDUs) */
    association_state state;

    /* session parameters */
    Odr_int preferredMessageSize;
    Odr_int maximumRecordSize;
    int version;                  /* highest version-bit set (2 or 3) */

    unsigned cs_get_mask;
    unsigned cs_put_mask;
    unsigned cs_accept_mask;

    struct bend_initrequest *init;
    statserv_options_block *last_control;

    struct gfs_server *server;
} association;

association *create_association(IOCHAN channel, COMSTACK link,
                                const char *apdufile);
void destroy_association(association *h);
void ir_session(IOCHAN h, int event);

void request_enq(request_q *q, request *r);
request *request_head(request_q *q);
request *request_deq(request_q *q);
request *request_deq_x(request_q *q, request *r);
void request_initq(request_q *q);
void request_delq(request_q *q);
request *request_get(request_q *q);
void request_release(request *r);

int statserv_must_terminate(void);

int control_association(association *assoc, const char *host, int force);

int ir_read(IOCHAN h, int event);

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

