/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: session.h,v $
 * Revision 1.4  1995-04-10 10:23:39  quinn
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

typedef struct association
{
    /* comms-related handles */
    IOCHAN client_chan;
    COMSTACK client_link;
    ODR decode;
    ODR encode;
    ODR print;
    char *encode_buffer;
    int encoded_len;
    char *input_buffer;
    int input_buffer_len;
    int input_apdu_len;
    int state;
    oid_proto proto;
    void *backend;
#define ASSOC_UNINIT       0
#define ASSOC_IDLE         1

    /* session parameters */
    int preferredMessageSize;
    int maximumRecordSize;
} association;

association *create_association(IOCHAN channel, COMSTACK link);
void destroy_association(association *h);
void ir_session(IOCHAN h, int event);

#endif
