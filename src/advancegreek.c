/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief Advance Greek encoding and decoding
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "iconv-p.h"

static unsigned long read_advancegreek(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                                       unsigned char *inp,
                                       size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    int shift = 0;
    int tonos = 0;
    int dialitika = 0;

    *no_read = 0;
    while (inbytesleft > 0)
    {
        if (*inp == 0x9d)
        {
            tonos = 1;
        }
        else if (*inp == 0x9e)
        {
            dialitika = 1;
        }
        else if (*inp == 0x9f)
        {
            shift = 1;
        }
        else
            break;
        inp++;
        --inbytesleft;
        (*no_read)++;
    }    
    if (inbytesleft == 0)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL); /* incomplete input */
        *no_read = 0;
        return 0;
    }
    switch (*inp) {
    case 0x81:
        if (shift) 
            if (tonos) 
                x = 0x0386;
            else 
                x = 0x0391;
        else 
            if (tonos) 
                x = 0x03ac;
            else 
                x = 0x03b1;
        break;
    case 0x82:
        if (shift) 
            x = 0x0392;
        else 
            x = 0x03b2;
        
        break;
    case 0x83:
        if (shift) 
            x = 0x0393;
        else 
            x = 0x03b3;
        break;
    case 0x84:
        if (shift) 
            x = 0x0394;
        else 
            x = 0x03b4;
        break;
    case 0x85:
        if (shift) 
            if (tonos) 
                x = 0x0388;
            else 
                x = 0x0395;
        else 
            if (tonos) 
                x = 0x03ad;
            else 
                x = 0x03b5;
        break;
    case 0x86:
        if (shift) 
            x = 0x0396;
        else 
            x = 0x03b6;
        break;
    case 0x87:
        if (shift) 
            if (tonos) 
                x = 0x0389;
            else 
                x = 0x0397;
        else 
            if (tonos) 
                x = 0x03ae;
            else 
                x = 0x03b7;
        break;
    case 0x88:
        if (shift) 
            x = 0x0398;
        else 
            x = 0x03b8;
        break;
    case 0x89:
        if (shift) 
            if (tonos) 
                x = 0x038a;
            else 
                if (dialitika) 
                    x = 0x03aa;
                else 
                    x = 0x0399;
        else 
            if (tonos) 
                if (dialitika) 
                    x = 0x0390;
                else 
                    x = 0x03af;
        
            else 
                if (dialitika) 
                    x = 0x03ca;
                else 
                    x = 0x03b9;
        break;
    case 0x8a:
        if (shift) 
            x = 0x039a;
        else 
            x = 0x03ba;
        
        break;
    case 0x8b:
        if (shift) 
            x = 0x039b;
        else 
            x = 0x03bb;
        break;
    case 0x8c:
        if (shift) 
            x = 0x039c;
        else 
            x = 0x03bc;
        
        break;
    case 0x8d:
        if (shift) 
            x = 0x039d;
        else 
            x = 0x03bd;
        break;
    case 0x8e:
        if (shift) 
            x = 0x039e;
        else 
            x = 0x03be;
        break;
    case 0x8f:
        if (shift) 
            if (tonos) 
                x = 0x038c;
            else 
                x = 0x039f;
        else 
            if (tonos) 
                x = 0x03cc;
            else 
                x = 0x03bf;
        break;
    case 0x90:
        if (shift) 
            x = 0x03a0;
        else 
            x = 0x03c0;
        break;
    case 0x91:
        if (shift) 
            x = 0x03a1;
        else 
            x = 0x03c1;
        break;
    case 0x92:
        x = 0x03c2;
        break;
    case 0x93:
        if (shift) 
            x = 0x03a3;
        else 
            x = 0x03c3;
        break;
    case 0x94:
        if (shift) 
            x = 0x03a4;
        else 
            x = 0x03c4;
        break;
    case 0x95:
        if (shift) 
            if (tonos) 
                x = 0x038e;
            else 
                if (dialitika) 
                    x = 0x03ab;
                else 
                    x = 0x03a5;
        else 
            if (tonos) 
                if (dialitika) 
                    x = 0x03b0;
                else 
                    x = 0x03cd;
        
            else 
                if (dialitika) 
                    x = 0x03cb;
                else 
                    x = 0x03c5;
        break;
    case 0x96:
        if (shift) 
            x = 0x03a6;
        else 
            x = 0x03c6;
        break;
    case 0x97:
        if (shift) 
            x = 0x03a7;
        else 
            x = 0x03c7;
        break;
    case 0x98:
        if (shift) 
            x = 0x03a8;
        else 
            x = 0x03c8;
        
        break;
        
    case 0x99:
        if (shift) 
            if (tonos) 
                x = 0x038f;
            else 
                x = 0x03a9;
        else 
            if (tonos) 
                x = 0x03ce;
            else 
                x = 0x03c9;
        break;
    default:
        x = *inp;
        break;
    }
    (*no_read)++;
    
    return x;
}

static size_t write_advancegreek(yaz_iconv_t cd, yaz_iconv_encoder_t w,
                                 unsigned long x,
                                 char **outbuf, size_t *outbytesleft)
{
    size_t k = 0;
    unsigned char *out = (unsigned char*) *outbuf;
    if (*outbytesleft < 3)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);  /* not room for output */
        return (size_t)(-1);
    }
    switch (x)
    {
    case 0x03ac : out[k++]=0x9d; out[k++]=0x81; break;
    case 0x03ad : out[k++]=0x9d; out[k++]=0x85; break;
    case 0x03ae : out[k++]=0x9d; out[k++]=0x87; break;
    case 0x03af : out[k++]=0x9d; out[k++]=0x89; break;
    case 0x03cc : out[k++]=0x9d; out[k++]=0x8f; break;
    case 0x03cd : out[k++]=0x9d; out[k++]=0x95; break;
    case 0x03ce : out[k++]=0x9d; out[k++]=0x99; break;
    case 0x0390 : out[k++]=0x9d; out[k++]=0x9e; out[k++]=0x89; break;
    case 0x03b0 : out[k++]=0x9d; out[k++]=0x9e; out[k++]=0x95; break;
    case 0x0386 : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x81; break;
    case 0x0388 : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x85; break;
    case 0x0389 : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x87; break;
    case 0x038a : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x89; break;
    case 0x038c : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x8f; break;
    case 0x038e : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x95; break;
    case 0x038f : out[k++]=0x9d; out[k++]=0x9f; out[k++]=0x99; break;
    case 0x03ca : out[k++]=0x9e; out[k++]=0x89; break;
    case 0x03cb : out[k++]=0x9e; out[k++]=0x95; break;
    case 0x03aa : out[k++]=0x9e; out[k++]=0x9f; out[k++]=0x89; break;
    case 0x03ab : out[k++]=0x9e; out[k++]=0x9f; out[k++]=0x95; break;
    case 0x0391 : out[k++]=0x9f; out[k++]=0x81; break;
    case 0x0392 : out[k++]=0x9f; out[k++]=0x82; break;
    case 0x0393 : out[k++]=0x9f; out[k++]=0x83; break;
    case 0x0394 : out[k++]=0x9f; out[k++]=0x84; break;
    case 0x0395 : out[k++]=0x9f; out[k++]=0x85; break;
    case 0x0396 : out[k++]=0x9f; out[k++]=0x86; break;
    case 0x0397 : out[k++]=0x9f; out[k++]=0x87; break;
    case 0x0398 : out[k++]=0x9f; out[k++]=0x88; break;
    case 0x0399 : out[k++]=0x9f; out[k++]=0x89; break;
    case 0x039a : out[k++]=0x9f; out[k++]=0x8a; break;
    case 0x039b : out[k++]=0x9f; out[k++]=0x8b; break;
    case 0x039c : out[k++]=0x9f; out[k++]=0x8c; break;
    case 0x039d : out[k++]=0x9f; out[k++]=0x8d; break;
    case 0x039e : out[k++]=0x9f; out[k++]=0x8e; break;
    case 0x039f : out[k++]=0x9f; out[k++]=0x8f; break;
    case 0x03a0 : out[k++]=0x9f; out[k++]=0x90; break;
    case 0x03a1 : out[k++]=0x9f; out[k++]=0x91; break;
    case 0x03a3 : out[k++]=0x9f; out[k++]=0x93; break;
    case 0x03a4 : out[k++]=0x9f; out[k++]=0x94; break;
    case 0x03a5 : out[k++]=0x9f; out[k++]=0x95; break;
    case 0x03a6 : out[k++]=0x9f; out[k++]=0x96; break;
    case 0x03a7 : out[k++]=0x9f; out[k++]=0x97; break;
    case 0x03a8 : out[k++]=0x9f; out[k++]=0x98; break;
    case 0x03a9 : out[k++]=0x9f; out[k++]=0x99; break;
    case 0x03b1 : out[k++]=0x81; break;
    case 0x03b2 : out[k++]=0x82; break;
    case 0x03b3 : out[k++]=0x83; break;
    case 0x03b4 : out[k++]=0x84; break;
    case 0x03b5 : out[k++]=0x85; break;
    case 0x03b6 : out[k++]=0x86; break;
    case 0x03b7 : out[k++]=0x87; break;
    case 0x03b8 : out[k++]=0x88; break;
    case 0x03b9 : out[k++]=0x89; break;
    case 0x03ba : out[k++]=0x8a; break;
    case 0x03bb : out[k++]=0x8b; break;
    case 0x03bc : out[k++]=0x8c; break;
    case 0x03bd : out[k++]=0x8d; break;
    case 0x03be : out[k++]=0x8e; break;
    case 0x03bf : out[k++]=0x8f; break;
    case 0x03c0 : out[k++]=0x90; break;
    case 0x03c1 : out[k++]=0x91; break;
    case 0x03c2 : out[k++]=0x92; break;
    case 0x03c3 : out[k++]=0x93; break;
    case 0x03c4 : out[k++]=0x94; break;
    case 0x03c5 : out[k++]=0x95; break;
    case 0x03c6 : out[k++]=0x96; break;
    case 0x03c7 : out[k++]=0x97; break;
    case 0x03c8 : out[k++]=0x98; break;
    case 0x03c9 : out[k++]=0x99; break;
    default:
        if (x > 255)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_EILSEQ);
            return (size_t) -1;
        }
        out[k++] = x;
        break;
    }
    *outbytesleft -= k;
    (*outbuf) += k;
    return 0;
}

yaz_iconv_encoder_t yaz_advancegreek_encoder(const char *name,
                                             yaz_iconv_encoder_t e)
{
    if (!yaz_matchstr(name, "advancegreek"))
    {
        e->write_handle = write_advancegreek;
        return e;
    }
    return 0;
}

yaz_iconv_decoder_t yaz_advancegreek_decoder(const char *name,
                                             yaz_iconv_decoder_t d)
{
    if (!yaz_matchstr(name, "advancegreek"))
    {
        d->read_handle = read_advancegreek;
        return d;
    }
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

