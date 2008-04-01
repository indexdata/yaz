/*
 * Copyright (C) 2005-2008, Index Data ApS
 * See the file LICENSE for details.
 *
 */
/**
 * \file
 * \brief Internal header for conv
 */

#ifndef ICONV_P_H
#define ICONV_P_H

#include <yaz/yconfig.h>

#include <yaz/yaz-iconv.h>

void yaz_iconv_set_errno(yaz_iconv_t cd, int no);

unsigned long yaz_read_iso5428_1984(yaz_iconv_t cd, unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read);

size_t yaz_init_UTF8(yaz_iconv_t cd, unsigned char *inp,
                     size_t inbytesleft, size_t *no_read);
unsigned long yaz_read_UTF8(yaz_iconv_t cd, unsigned char *inp,
                            size_t inbytesleft, size_t *no_read);


unsigned long yaz_read_UCS4(yaz_iconv_t cd, unsigned char *inp,
                            size_t inbytesleft, size_t *no_read);
unsigned long yaz_read_UCS4LE(yaz_iconv_t cd, unsigned char *inp,
                              size_t inbytesleft, size_t *no_read);
unsigned long yaz_read_advancegreek(yaz_iconv_t cd, unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read);

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

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

