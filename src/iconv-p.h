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
 * \file
 * \brief Internal header for iconv
 */

#ifndef ICONV_P_H
#define ICONV_P_H

#include <yaz/yconfig.h>

#include <yaz/matchstr.h>
#include <yaz/yaz-iconv.h>

void yaz_iconv_set_errno(yaz_iconv_t cd, int no);

typedef struct yaz_iconv_encoder_s *yaz_iconv_encoder_t;
struct yaz_iconv_encoder_s {
    void *data;
    size_t (*write_handle)(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                           unsigned long x,
                           char **outbuf, size_t *outbytesleft);
    size_t (*flush_handle)(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                           char **outbuf, size_t *outbytesleft);
    void (*init_handle)(yaz_iconv_encoder_t e);
    void (*destroy_handle)(yaz_iconv_encoder_t e);
};

yaz_iconv_encoder_t yaz_marc8_encoder(const char *name,
                                      yaz_iconv_encoder_t e);
yaz_iconv_encoder_t yaz_utf8_encoder(const char *name,
                                     yaz_iconv_encoder_t e);
yaz_iconv_encoder_t yaz_ucs4_encoder(const char *name,
                                     yaz_iconv_encoder_t e);
yaz_iconv_encoder_t yaz_iso_8859_1_encoder(const char *name,
                                           yaz_iconv_encoder_t e);
yaz_iconv_encoder_t yaz_iso_5428_encoder(const char *name,
                                         yaz_iconv_encoder_t e);
yaz_iconv_encoder_t yaz_advancegreek_encoder(const char *name,
                                             yaz_iconv_encoder_t e);
yaz_iconv_encoder_t yaz_wchar_encoder(const char *name,
                                      yaz_iconv_encoder_t e);
typedef unsigned long yaz_conv_func_t(unsigned char *inp, size_t inbytesleft,
                                      size_t *no_read, int *combining,
                                      unsigned mask, int boffset);

int yaz_iso_8859_1_lookup_y(unsigned long v,
                            unsigned long *x1, unsigned long *x2);

int yaz_iso_8859_1_lookup_x12(unsigned long x1, unsigned long x2,
                              unsigned long *y);

typedef struct yaz_iconv_decoder_s *yaz_iconv_decoder_t;
struct yaz_iconv_decoder_s {
    void *data;
    size_t (*init_handle)(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                          unsigned char *inbuf,
                          size_t inbytesleft, size_t *no_read);
    unsigned long (*read_handle)(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                                 unsigned char *inbuf,
                                 size_t inbytesleft, size_t *no_read);
    void (*destroy_handle)(yaz_iconv_decoder_t d);
};

yaz_iconv_decoder_t yaz_marc8_decoder(const char *fromcode,
                                      yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_iso5426_decoder(const char *fromcode,
                                      yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_utf8_decoder(const char *fromcode,
                                     yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_ucs4_decoder(const char *tocode,
                                     yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_iso_8859_1_decoder(const char *fromcode,
                                           yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_iso_5428_decoder(const char *name,
                                         yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_advancegreek_decoder(const char *name,
                                             yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_wchar_decoder(const char *fromcode,
				      yaz_iconv_decoder_t d);
yaz_iconv_decoder_t yaz_danmarc_decoder(const char *fromcode,
                                        yaz_iconv_decoder_t d);

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

