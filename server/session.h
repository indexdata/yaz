/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: session.h,v $
 * Revision 1.1  1995-03-14 10:28:02  quinn
 * More work on demo server.
 *
 *
 */

#ifndef SESSION_H
#define SESSION_H

#include <comstack.h>
#include <odr.h>

typedef struct association
{
    /* comms-related handles */
    IOCHAN client_chan;
    COMSTACK client_link;
    ODR decode;
    ODR encode;
    char *encode_buffer;
    int encoded_len;
    char *input_buffer;
    int input_buffer_len;
    int input_apdu_len;
    int state;
#define ASSOC_UNINIT       0
#define ASSOC_IDLE         1

    /* session parameters */
    int preferredMessageSize;
    int maximumRecordSize;
} association;

association *create_association(IOCHAN channel, COMSTACK link);
void ir_session(IOCHAN h, int event);

#endif
