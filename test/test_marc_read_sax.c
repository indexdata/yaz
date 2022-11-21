/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/test.h>
#include <string.h>
#include <yaz/log.h>
#include <yaz/marc_sax.h>

struct user_data
{
    WRBUF wrbuf;
};

static void handler(yaz_marc_t mt, void *cb)
{
    struct user_data *ctx = cb;
    yaz_marc_write_marcxml(mt, ctx->wrbuf);
}

static void tst1(void)
{
    char *marcxml = "<collection xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                    "<record>\n"
                    "  <leader>00062cgm a2200037Ia 4500</leader>\n"
                    "  <datafield tag=\"092\" ind1=\" \" ind2=\"á\">\n"
                    "    <subfield code=\"a\">DVD CHI 791.43</subfield>\n"
                    "    <subfield code=\"&amp;\">Q&amp;A</subfield>\n"
                    "  </datafield>\n"
                    "</record>\n"
                    "</collection>\n";

    struct user_data user_data;
    user_data.wrbuf = wrbuf_alloc();
    yaz_marc_t mt = yaz_marc_create();
    yaz_marc_sax_t yt = yaz_marc_sax_new(mt, handler, &user_data);
    xmlSAXHandlerPtr sax_ptr = yaz_marc_sax_get_handler(yt);

    size_t lead = 10;
    xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt(sax_ptr, yt, marcxml, lead, 0);
    ctxt->replaceEntities = 1;
    xmlParseChunk(ctxt, marcxml + lead, strlen(marcxml) - lead, 1);

    YAZ_CHECK(strcmp(wrbuf_cstr(user_data.wrbuf),
                     "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                     "  <leader>00062cgm a2200037Ia 4500</leader>\n"
                     "  <datafield tag=\"092\" ind1=\" \" ind2=\"á\">\n"
                     "    <subfield code=\"a\">DVD CHI 791.43</subfield>\n"
                     "    <subfield code=\"&amp;\">Q&amp;A</subfield>\n"
                     "  </datafield>\n"
                     "</record>\n") == 0);
    xmlFreeParserCtxt(ctxt);
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
