/*
 * Copyright (C) 2005-2008, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: zoom-p.h,v 1.25 2007-09-11 08:40:28 adam Exp $
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

size_t yaz_write_iso5428_1984(yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft);

size_t yaz_init_UTF8(yaz_iconv_t cd, unsigned char *inp,
                     size_t inbytesleft, size_t *no_read);
unsigned long yaz_read_UTF8(yaz_iconv_t cd, unsigned char *inp,
                            size_t inbytesleft, size_t *no_read);


size_t yaz_write_UTF8(yaz_iconv_t cd, unsigned long x,
                      char **outbuf, size_t *outbytesleft);

unsigned long yaz_read_UCS4(yaz_iconv_t cd, unsigned char *inp,
                            size_t inbytesleft, size_t *no_read);
unsigned long yaz_read_UCS4LE(yaz_iconv_t cd, unsigned char *inp,
                              size_t inbytesleft, size_t *no_read);
size_t yaz_write_UCS4(yaz_iconv_t cd, unsigned long x,
                      char **outbuf, size_t *outbytesleft);
size_t yaz_write_UCS4LE(yaz_iconv_t cd, unsigned long x,
                        char **outbuf, size_t *outbytesleft);
unsigned long yaz_read_advancegreek(yaz_iconv_t cd, unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read);
size_t yaz_write_advancegreek(yaz_iconv_t cd, unsigned long x,
                              char **outbuf, size_t *outbytesleft);

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

