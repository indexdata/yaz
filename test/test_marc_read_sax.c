/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#include <yaz/tpath.h>
#include <yaz/test.h>
#include <string.h>
#include <yaz/log.h>
#include <yaz/marc_sax.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

struct ctx {
    WRBUF wrbuf;
};

static void handler(yaz_marc_t mt, void *cb)
{
    struct ctx *ctx = cb;
    yaz_marc_write_marcxml(mt, ctx->wrbuf);
}

static void tst1(void)
{
    char *marcxml = "<collection xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
     "<record>\n"
     "  <leader>00062cgm a2200037Ia 4500</leader>\n"
     "  <datafield tag=\"092\" ind1=\" \" ind2=\"á\">\n"
     "    <subfield code=\"a\">DVD CHI 791.43</subfield>\n"
     "    <subfield code=\"b\">QI</subfield>\n"
     "  </datafield>\n"
     "</record>\n"
     "</collection>\n"
    ;

    struct ctx ctx;
    ctx.wrbuf = wrbuf_alloc();
    yaz_marc_t mt = yaz_marc_create();
    yaz_marc_sax_t yt = yaz_marc_sax_new(mt, handler, &ctx);
    xmlSAXUserParseMemory(yaz_marc_sax_get(yt), yt, marcxml, strlen(marcxml));
    yaz_marc_sax_destroy(yt);
    yaz_marc_destroy(mt);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst1();
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

