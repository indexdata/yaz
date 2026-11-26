/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data.
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
 * \file comstack-p.h
 * \brief Private Header for COMSTACK
 */

#ifndef COMSTACK_P_H
#define COMSTACK_P_H

#include <yaz/comstack.h>

/* Host/port parsing buffer size, including the terminating NUL. */
#define YAZ_TCPIP_HOST_PORT_BUFSIZE 512

YAZ_BEGIN_CDECL

const char *yaz_tcpip_get_error_details(COMSTACK cs);
int yaz_tcpip_set_head_only(COMSTACK cs, int head_only);
#ifndef _WIN32
int yaz_unix_set_head_only(COMSTACK cs, int head_only);
#endif

/* How far yaz_cs_complete got with a message that is being received. */
struct yaz_cs_complete_state {
    int off;   /* BER: end of the last complete element
                  HTTP: start, within a chunked body, of the first
                  incomplete chunk */
    int level; /* BER: number of open indefinite-length elements */
};

/* Prepares st for a new message. */
void yaz_cs_complete_reset(struct yaz_cs_complete_state *st);

/*
 * As cs_complete_auto, or cs_complete_auto_head if head_only is non-zero,
 * but resumes where the previous call stopped rather than examine all of
 * buf again.  buf must start with the bytes that were given in that call.
 * Resets st when the result is non-zero.
 */
int yaz_cs_complete(const char *buf, int len, int head_only,
                    struct yaz_cs_complete_state *st);

/*
 * Parses an HTTP chunked body.
 *
 * Returns the number of bytes consumed, 0 if the body is incomplete, or -1
 * if it is malformed.  If content_buf is non-NULL, it must have room for len
 * bytes and the chunk payload is copied to it.  content_len, when non-NULL,
 * receives the payload length on success.  resume, when non-NULL, holds the
 * offset of the chunk to start with and receives the offset of the first
 * chunk that is incomplete; it can not be combined with content_buf.
 */
int yaz_http_parse_chunks(const char *buf, int len, char *content_buf,
                          int *content_len, int *resume);

/* Return the body offset, 0 for incomplete headers, or -1 for malformed
 * headers. content_len retains its caller-supplied default if absent. */
int yaz_http_parse_headers(const char *buf, int len, int off,
                           int *content_len, int *chunked);

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
