/*
 * Copyright (c) 1995, Index Data.
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
 * $Log: statserv.h,v $
 * Revision 1.3  1995-05-16 08:50:38  quinn
 * License, documentation, and memory fixes
 *
 *
 */

#ifndef STATSERVER_H
#define STATSERVER_H

#include <oid.h>

typedef struct statserv_options_block
{
    int dynamic;                  /* fork on incoming requests */
    int loglevel;                 /* desired logging-level */
    char apdufile[ODR_MAXNAME+1];      /* file for pretty-printed PDUs */
    char logfile[ODR_MAXNAME+1];       /* file for diagnostic output */
    char default_listen[1024];    /* 0 == no default listen */
    enum oid_proto default_proto; /* PROTO_SR or PROTO_Z3950 */
    int idle_timeout;             /* how many minutes to wait before closing */
    int maxrecordsize;            /* maximum value for negotiation */
    char configname[ODR_MAXNAME+1];    /* given to the backend in bend_init */
} statserv_options_block;

int statserv_main(int argc, char **argv);
statserv_options_block *statserv_getcontrol(void);
void statserv_setcontrol(statserv_options_block *block);

#endif
