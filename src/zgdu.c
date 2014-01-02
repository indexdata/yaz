/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zgdu.c
 * \brief Implements HTTP and Z39.50 encoding and decoding.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include "odr-priv.h"
#include <yaz/zgdu.h>

int z_GDU(ODR o, Z_GDU **p, int opt, const char *name)
{
    const char *buf = o->op->buf;
    if (o->direction == ODR_DECODE) {
        *p = (Z_GDU *) odr_malloc(o, sizeof(**p));
        if (o->op->size > 10 && !memcmp(buf, "HTTP/", 5))
        {
            (*p)->which = Z_GDU_HTTP_Response;
            return yaz_decode_http_response(o, &(*p)->u.HTTP_Response);

        }
        else if (o->op->size > 5 &&
            buf[0] >= 0x20 && buf[0] < 0x7f
            && buf[1] >= 0x20 && buf[1] < 0x7f
            && buf[2] >= 0x20 && buf[2] < 0x7f
            && buf[3] >= 0x20 && buf[3] < 0x7f)
        {
            (*p)->which = Z_GDU_HTTP_Request;
            return yaz_decode_http_request(o, &(*p)->u.HTTP_Request);
        }
        else
        {
            (*p)->which = Z_GDU_Z3950;
            return z_APDU(o, &(*p)->u.z3950, opt, 0);
        }
    }
    else /* ENCODE or PRINT */
    {
        switch((*p)->which)
        {
        case Z_GDU_HTTP_Response:
            return yaz_encode_http_response(o, (*p)->u.HTTP_Response);
        case Z_GDU_HTTP_Request:
            return yaz_encode_http_request(o, (*p)->u.HTTP_Request);
        case Z_GDU_Z3950:
            return z_APDU(o, &(*p)->u.z3950, opt, 0);
        }
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

