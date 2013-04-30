/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2013 Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <yaz/test.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/zgdu.h>

static void tst_http_response(void)
{
    {
        /* response, content  */
        const char *http_buf =
            /*123456789012345678 */
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 2\r\n"
            "\r\n"
            "12";

        int r;
        Z_GDU *zgdu;
        ODR odr = odr_createmem(ODR_DECODE);
        odr_setbuf(odr, (char *) http_buf, strlen(http_buf), 0);
        r = z_GDU(odr, &zgdu, 0, 0);
        YAZ_CHECK(r);
        if (r)
        {
            YAZ_CHECK_EQ(zgdu->which, Z_GDU_HTTP_Response);
        }
        odr_destroy(odr);
    }
}


int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_http_response();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

