/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstsoap2.c,v 1.1 2005-05-06 11:11:37 adam Exp $
 */

#include <stdlib.h>
#if HAVE_XML2
#include <libxml/parser.h>

#include <yaz/srw.h>
#include <yaz/soap.h>

static void tst_srw(void)
{
    const char *charset = 0;
    char *content_buf = 0;
    int content_len;
    int ret;
    ODR o = odr_createmem(ODR_ENCODE);
    Z_SOAP_Handler h[2] = {
        {"http://www.loc.gov/zing/srw/", 0, (Z_SOAP_fun) yaz_srw_codec},
        {0, 0, 0}
    };
    Z_SRW_PDU *sr = yaz_srw_get(o, Z_SRW_searchRetrieve_request);
    Z_SOAP *p = odr_malloc(o, sizeof(*p));

#if 0
    sr->u.request->query.cql = "jordbær"; 
#else
    sr->u.request->query.cql = "jordbaer"; 
#endif

    p->which = Z_SOAP_generic;
    p->u.generic = odr_malloc(o, sizeof(*p->u.generic));
    p->u.generic->no = 0;
    p->u.generic->ns = 0;
    p->u.generic->p = sr;
    p->ns = "http://schemas.xmlsoap.org/soap/envelope/";

    ret = z_soap_codec_enc(o, &p, &content_buf, &content_len, h, charset);
    odr_destroy(o);
    if (ret)
    {
	printf("z_soap_codec_enc failed\n");
	exit(1);
    }
}
#endif

int main(int argc, char **argv)
{
#if HAVE_XML2
    LIBXML_TEST_VERSION
    if (argc <= 1)
    {
	tst_srw();
    }
#endif
    return 0;
}
