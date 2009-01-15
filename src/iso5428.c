/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief ISO-5428:1984 encoding and decoding
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "iconv-p.h"

static unsigned long read_iso_5428_1984(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                                        unsigned char *inp,
                                        size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    int tonos = 0;
    int dialitika = 0;

    *no_read = 0;
    while (inbytesleft > 0)
    {
        if (*inp == 0xa2)
        {
            tonos = 1;
        }
        else if (*inp == 0xa3)
        {
            dialitika = 1;
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
    case 0xe1: /*  alpha small */
            if (tonos) 
                x = 0x03ac;
            else 
                x = 0x03b1;
            break;
    case 0xc1: /*  alpha capital */
            if (tonos) 
                x = 0x0386;
            else 
                x = 0x0391;
            break;

    case 0xe2: /*  Beta small */
            x = 0x03b2;
            break;
    case 0xc2: /*  Beta capital */
            x = 0x0392;
            break;

    case 0xe4: /*  Gamma small */
            x = 0x03b3;
            break;
    case 0xc4: /*  Gamma capital */
            x = 0x0393;
            break;

    case 0xe5: /*  Delta small */
            x = 0x03b4;
            break;
    case 0xc5: /*  Delta capital */
            x = 0x0394;
            break;
    case 0xe6: /*  epsilon small */
            if (tonos) 
                x = 0x03ad;
            else 
                x = 0x03b5;
            break;
    case 0xc6: /*  epsilon capital */
            if (tonos) 
                x = 0x0388;
            else 
                x = 0x0395;
            break;
    case 0xe9: /*  Zeta small */
            x = 0x03b6;
            break;
    case 0xc9: /*  Zeta capital */
            x = 0x0396;
            break;
    case 0xea: /*  Eta small */
            if (tonos) 
                x = 0x03ae;
            else 
                x = 0x03b7;
            break;
    case 0xca: /*  Eta capital */
            if (tonos) 
                x = 0x0389;
            else 
                x = 0x0397;
            break;
    case 0xeb: /*  Theta small */
            x = 0x03b8;
            break;
    case 0xcb: /*  Theta capital */
            x = 0x0398;
            break;
    case 0xec: /*  Iota small */
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
    case 0xcc: /*  Iota capital */
            if (tonos) 
                x = 0x038a;
            else 
                if (dialitika) 
                    x = 0x03aa;
                else 
                    x = 0x0399;
            break;
    case 0xed: /*  Kappa small */
            x = 0x03ba;
            break;
    case 0xcd: /*  Kappa capital */
            x = 0x039a;
            break;
    case 0xee: /*  Lambda small */
            x = 0x03bb;
            break;
    case 0xce: /*  Lambda capital */
            x = 0x039b;
            break;
    case 0xef: /*  Mu small */
            x = 0x03bc;
            break;
    case 0xcf: /*  Mu capital */
            x = 0x039c;
            break;
    case 0xf0: /*  Nu small */
            x = 0x03bd;
            break;
    case 0xd0: /*  Nu capital */
            x = 0x039d;
            break;
    case 0xf1: /*  Xi small */
            x = 0x03be;
            break;
    case 0xd1: /*  Xi capital */
            x = 0x039e;
            break;
    case 0xf2: /*  Omicron small */
            if (tonos) 
                x = 0x03cc;
            else 
                x = 0x03bf;
            break;
    case 0xd2: /*  Omicron capital */
            if (tonos) 
                x = 0x038c;
            else 
                x = 0x039f;
            break;
    case 0xf3: /*  Pi small */
            x = 0x03c0;
            break;
    case 0xd3: /*  Pi capital */
            x = 0x03a0;
            break;
    case 0xf5: /*  Rho small */
            x = 0x03c1;
            break;
    case 0xd5: /*  Rho capital */
            x = 0x03a1;
            break;
    case 0xf7: /*  Sigma small (end of words) */
            x = 0x03c2;
            break;
    case 0xf6: /*  Sigma small */
            x = 0x03c3;
            break;
    case 0xd6: /*  Sigma capital */
            x = 0x03a3;
            break;
    case 0xf8: /*  Tau small */
            x = 0x03c4;
            break;
    case 0xd8: /*  Tau capital */
            x = 0x03a4;
            break;
    case 0xf9: /*  Upsilon small */
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
    case 0xd9: /*  Upsilon capital */
            if (tonos) 
                x = 0x038e;
            else 
                if (dialitika) 
                    x = 0x03ab;
                else 
                    x = 0x03a5;
            break;
    case 0xfa: /*  Phi small */
            x = 0x03c6;
            break;
    case 0xda: /*  Phi capital */
            x = 0x03a6;
            break;
    case 0xfb: /*  Chi small */
            x = 0x03c7;
            break;
    case 0xdb: /*  Chi capital */
            x = 0x03a7;
            break;
    case 0xfc: /*  Psi small */
            x = 0x03c8;
            break;
    case 0xdc: /*  Psi capital */
            x = 0x03a8;
            break;
    case 0xfd: /*  Omega small */
            if (tonos) 
                x = 0x03ce;
            else 
                x = 0x03c9;
            break;
    case 0xdd: /*  Omega capital */
            if (tonos) 
                x = 0x038f;
            else 
                x = 0x03a9;
            break;
    default:
        x = *inp;
        break;
    }
    (*no_read)++;
    
    return x;
}

static size_t write_iso_5428_1984(yaz_iconv_t cd, yaz_iconv_encoder_t en,
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
    case 0x03ac : out[k++]=0xa2; out[k++]=0xe1; break;
    case 0x03b1 : out[k++]=0xe1; break;
    case 0x0386 : out[k++]=0xa2; out[k++]=0xc1; break;
    case 0x0391 : out[k++]=0xc1; break;
    case 0x03b2 : out[k++]=0xe2; break;
    case 0x0392 : out[k++]=0xc2; break;
    case 0x03b3 : out[k++]=0xe4; break;
    case 0x0393 : out[k++]=0xc4; break;
    case 0x03b4 : out[k++]=0xe5; break;
    case 0x0394 : out[k++]=0xc5; break;
    case 0x03ad : out[k++]=0xa2; out[k++]=0xe6; break;
    case 0x03b5 : out[k++]=0xe6; break;
    case 0x0388 : out[k++]=0xa2; out[k++]=0xc6; break;
    case 0x0395 : out[k++]=0xc6; break;
    case 0x03b6 : out[k++]=0xe9; break;
    case 0x0396 : out[k++]=0xc9; break;
    case 0x03ae : out[k++]=0xa2; out[k++]=0xea; break;
    case 0x03b7 : out[k++]=0xea; break;
    case 0x0389 : out[k++]=0xa2; out[k++]=0xca; break;
    case 0x0397 : out[k++]=0xca; break;
    case 0x03b8 : out[k++]=0xeb; break;
    case 0x0398 : out[k++]=0xcb; break;
    case 0x0390 : out[k++]=0xa2; out[k++]=0xa3; out[k++]=0xec; break;
    case 0x03af : out[k++]=0xa2; out[k++]=0xec; break;
    case 0x03ca : out[k++]=0xa3; out[k++]=0xec; break;
    case 0x03b9 : out[k++]=0xec; break;
    case 0x038a : out[k++]=0xa2; out[k++]=0xcc; break;
    case 0x03aa : out[k++]=0xa3; out[k++]=0xcc; break;
    case 0x0399 : out[k++]=0xcc; break;
    case 0x03ba : out[k++]=0xed; break;
    case 0x039a : out[k++]=0xcd; break;
    case 0x03bb : out[k++]=0xee; break;
    case 0x039b : out[k++]=0xce; break;
    case 0x03bc : out[k++]=0xef; break;
    case 0x039c : out[k++]=0xcf; break;
    case 0x03bd : out[k++]=0xf0; break;
    case 0x039d : out[k++]=0xd0; break;
    case 0x03be : out[k++]=0xf1; break;
    case 0x039e : out[k++]=0xd1; break;
    case 0x03cc : out[k++]=0xa2; out[k++]=0xf2; break;
    case 0x03bf : out[k++]=0xf2; break;
    case 0x038c : out[k++]=0xa2; out[k++]=0xd2; break;
    case 0x039f : out[k++]=0xd2; break;
    case 0x03c0 : out[k++]=0xf3; break;
    case 0x03a0 : out[k++]=0xd3; break;
    case 0x03c1 : out[k++]=0xf5; break;
    case 0x03a1 : out[k++]=0xd5; break;
    case 0x03c2 : out[k++]=0xf7; break;
    case 0x03c3 : out[k++]=0xf6; break;
    case 0x03a3 : out[k++]=0xd6; break;
    case 0x03c4 : out[k++]=0xf8; break;
    case 0x03a4 : out[k++]=0xd8; break;
    case 0x03b0 : out[k++]=0xa2; out[k++]=0xa3; out[k++]=0xf9; break;
    case 0x03cd : out[k++]=0xa2; out[k++]=0xf9; break;
    case 0x03cb : out[k++]=0xa3; out[k++]=0xf9; break;
    case 0x03c5 : out[k++]=0xf9; break;
    case 0x038e : out[k++]=0xa2; out[k++]=0xd9; break;
    case 0x03ab : out[k++]=0xa3; out[k++]=0xd9; break;
    case 0x03a5 : out[k++]=0xd9; break;
    case 0x03c6 : out[k++]=0xfa; break;
    case 0x03a6 : out[k++]=0xda; break;
    case 0x03c7 : out[k++]=0xfb; break;
    case 0x03a7 : out[k++]=0xdb; break;
    case 0x03c8 : out[k++]=0xfc; break;
    case 0x03a8 : out[k++]=0xdc; break;
    case 0x03ce : out[k++]=0xa2; out[k++]=0xfd; break;
    case 0x03c9 : out[k++]=0xfd; break;
    case 0x038f : out[k++]=0xa2; out[k++]=0xdd; break;
    case 0x03a9 : out[k++]=0xdd; break;
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

yaz_iconv_encoder_t yaz_iso_5428_encoder(const char *name,
                                         yaz_iconv_encoder_t e)
{
    if (!yaz_matchstr(name, "iso54281984")
        || !yaz_matchstr(name, "iso5428:1984"))
    {
        e->write_handle = write_iso_5428_1984;
        return e;
    }
    return 0;
}

yaz_iconv_decoder_t yaz_iso_5428_decoder(const char *name,
                                         yaz_iconv_decoder_t d)
{
    if (!yaz_matchstr(name, "iso54281984")
        || !yaz_matchstr(name, "iso5428:1984"))
    {
        d->read_handle = read_iso_5428_1984;
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

