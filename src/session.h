/*
 * Copyright (C) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: session.h,v 1.1 2003-10-27 12:21:35 adam Exp $
 */

#ifndef SESSION_H
#define SESSION_H

#include <sys/types.h>
#include <yaz/comstack.h>
#include <yaz/odr.h>
#include <yaz/oid.h>
#include <yaz/proto.h>
#include "eventl.h"

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
    ASSOC_NEW,                /* not initialized yet */
    ASSOC_UP,                 /* normal operation */
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
    int preferredMessageSize;
    int maximumRecordSize;
    int version;                  /* highest version-bit set (2 or 3) */

    unsigned cs_get_mask;
    unsigned cs_put_mask;
    unsigned cs_accept_mask;

    struct bend_initrequest *init;
} association;

association *create_association(IOCHAN channel, COMSTACK link);
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

#endif
