/*
 * Copyright (C) 1995, Index Data I/S 
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: session.h,v $
 * Revision 1.12  1997-09-01 08:53:01  adam
 * New windows NT/95 port using MSV5.0. The test server 'ztest' was
 * moved a separate directory. MSV5.0 project server.dsp created.
 * As an option, the server can now operate as an NT service.
 *
 * Revision 1.11  1995/11/08 17:41:40  quinn
 * Smallish.
 *
 * Revision 1.10  1995/08/29  11:18:01  quinn
 * Added code to receive close
 *
 * Revision 1.9  1995/06/16  10:31:38  quinn
 * Added session timeout.
 *
 * Revision 1.8  1995/05/17  08:42:28  quinn
 * Transfer auth info to backend. Allow backend to reject init gracefully.
 *
 * Revision 1.7  1995/05/16  08:51:08  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.6  1995/05/15  11:56:41  quinn
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
#include <sys/types.h>
#include "eventl.h"

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
    NMEM request_mem;    /* memory handle for request */

    int size_response;     /* size of buffer */
    int len_response;      /* length of encoded data */
    char *response;        /* encoded data waiting for transmission */

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
    enum
    {
    	ASSOC_NEW,                /* not initialized yet */
	ASSOC_UP,                 /* normal operation */
	ASSOC_DEAD                /* dead. Close if input arrives */
    } state;

    /* session parameters */
    int preferredMessageSize;
    int maximumRecordSize;
    int version;                  /* highest version-bit set (2 or 3) */
} association;

association *create_association(IOCHAN channel, COMSTACK link);
void destroy_association(association *h);
void ir_session(IOCHAN h, int event);

void request_enq(request_q *q, request *r);
request *request_head(request_q *q);
request *request_deq(request_q *q);
void request_initq(request_q *q);
void request_delq(request_q *q);
request *request_get(request_q *q);
void request_release(request *r);

#endif
