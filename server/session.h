/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: session.h,v $
 * Revision 1.6  1995-05-15 11:56:41  quinn
 * Asynchronous facilities. Restructuring of seshigh code.
 *
 * Revision 1.5  1995/04/20  15:13:01  quinn
 * Cosmetic
 *
 * Revision 1.4  1995/04/10  10:23:39  quinn
 * Some work to add scan and other things.
 *
 * Revision 1.3  1995/03/30  09:09:27  quinn
 * Added state-handle and some support for asynchronous activities.
 *
 * Revision 1.2  1995/03/27  08:34:29  quinn
 * Added dynamic server functionality.
 * Released bindings to session.c (is now redundant)
 *
 * Revision 1.1  1995/03/14  10:28:02  quinn
 * More work on demo server.
 *
 *
 */

#ifndef SESSION_H
#define SESSION_H

#include <comstack.h>
#include <odr.h>
#include <oid.h>
#include <proto.h>
#include <eventl.h>

typedef struct request
{
    int len_refid;          /* length of referenceid */
    char *refid;            /* referenceid */
    enum {
    	REQUEST_IDLE,    /* the request is just sitting in the queue */
	REQUEST_PENDING  /* operation pending (b'end processing or network I/O*/
	/* this list will have more elements when acc/res control is added */
    } state;

    Z_APDU *request;        /* Current request */
    ODR_MEM request_mem;    /* ODR memory handle for request */

    int size_response;     /* size of buffer */
    int len_response;      /* length of encoded data */
    char *response;        /* encoded data waiting for transmission */

    struct request *next;
} request;

typedef struct request_q
{
    request *head;
    request *tail;
    int num;
} request_q;

/*
 * association state.
 */
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

    /* session parameters */
    int preferredMessageSize;
    int maximumRecordSize;
} association;

association *create_association(IOCHAN channel, COMSTACK link);
void destroy_association(association *h);
void ir_session(IOCHAN h, int event);

void request_enq(request_q *q, request *r);
request *request_head(request_q *q);
request *request_deq(request_q *q);
void request_initq(request_q *q);
request *request_get(void);
void request_release(request *r);

#endif
