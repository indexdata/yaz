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
#if YAZ_HAVE_XML2
    char *marcxml = "<collection xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                    "<record>\n"
                    "  <leader>00062cgm a2200037Ia 4500</leader>\n"
                    "  <datafield tag=\"02&amp;\" ind1=\" \" ind2=\"á\">\n"
                    "    <subfield code=\"&#197;\">DVD CHI 791.43</subfield>\n"
                    "    <subfield code=\"&amp;\">Q&amp;A</subfield>\n"
                    "  </datafield>\n"
                    "</record>\n"
                    "</collection>\n";

    struct user_data user_data;
    user_data.wrbuf = wrbuf_alloc();
    yaz_marc_t mt = yaz_marc_create();
    yaz_marc_sax_t yt = yaz_marc_sax_new(mt, handler, &user_data);
    xmlSAXHandlerPtr sax_ptr = yaz_marc_sax_get_handler(yt);
    xmlParserCtxtPtr parser_ctxt =
        xmlCreatePushParserCtxt(sax_ptr, yt, marcxml, (int) strlen(marcxml), 0);

    YAZ_CHECK(parser_ctxt);
    if (parser_ctxt)
    {
        YAZ_CHECK_EQ(xmlParseChunk(parser_ctxt, 0, 0, 1), 0);
        xmlFreeParserCtxt(parser_ctxt);
    }

    const char *expect = "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                     "  <leader>00062cgm a2200037Ia 4500</leader>\n"
                     "  <datafield tag=\"02&amp;\" ind1=\" \" ind2=\"á\">\n"
                     "    <subfield code=\"Å\">DVD CHI 791.43</subfield>\n"
                     "    <subfield code=\"&amp;\">Q&amp;A</subfield>\n"
                     "  </datafield>\n"
                     "</record>\n";

    YAZ_CHECK(strcmp(wrbuf_cstr(user_data.wrbuf), expect) == 0);
    yaz_marc_sax_destroy(yt);
    yaz_marc_destroy(mt);
    wrbuf_destroy(user_data.wrbuf);
#endif
}


#if YAZ_HAVE_XML2
struct leader_test_data
{
    int records;
    WRBUF output;
};

static void leader_handler(yaz_marc_t mt, void *vp)
{
    struct leader_test_data *data = vp;
    data->records++;
    YAZ_CHECK_EQ(yaz_marc_write_marcxml(mt, data->output), 0);
}

static void check_leaders(const char *xml, int records, const char *expected)
{
    struct leader_test_data data;
    yaz_marc_t mt = yaz_marc_create();
    yaz_marc_sax_t sax;
    xmlParserCtxtPtr parser;
    size_t i;
    data.records = 0;
    data.output = wrbuf_alloc();
    sax = yaz_marc_sax_new(mt, leader_handler, &data);
    parser = xmlCreatePushParserCtxt(yaz_marc_sax_get_handler(sax), sax,
                                     0, 0, 0);
    YAZ_CHECK(parser != 0);
    if (parser)
    {
        /* Character callbacks may split the leader at any byte boundary. */
        for (i = 0; xml[i]; i++)
            YAZ_CHECK_EQ(xmlParseChunk(parser, xml + i, 1, 0), 0);
        YAZ_CHECK_EQ(xmlParseChunk(parser, 0, 0, 1), 0);
        YAZ_CHECK_EQ(data.records, records);
        if (!records)
            YAZ_CHECK_EQ(wrbuf_len(data.output), 0);
        if (expected)
            YAZ_CHECK(strstr(wrbuf_cstr(data.output), expected) != 0);
        xmlFreeParserCtxt(parser);
    }
    yaz_marc_sax_destroy(sax);
    yaz_marc_destroy(mt);
    wrbuf_destroy(data.output);
}

static void tst_leader_lengths(void)
{
    const char *leader = "00062cgm a2200037Ia 4500X";
    const char *fields = "<datafield tag=\"245\" ind1=\"1\" ind2=\"2\">"
        "<subfield code=\"a\">title</subfield></datafield>";
    WRBUF xml = wrbuf_alloc();
    int len;
    for (len = 0; len <= 25; len++)
    {
        wrbuf_rewind(xml);
        wrbuf_puts(xml, "<record><leader>");
        wrbuf_write(xml, leader, len);
        wrbuf_printf(xml, "</leader>%s</record>", fields);
        check_leaders(wrbuf_cstr(xml), len == 24 ? 1 : 0,
                      len == 24 ? "ind1=\"1\" ind2=\"2\"" : 0);
    }
    check_leaders("<record><leader/></record>", 0, 0);
    check_leaders("<record/>", 0, 0);
    wrbuf_rewind(xml);
    wrbuf_printf(xml, "<record>%s<leader>%.24s</leader></record>", fields, leader);
    check_leaders(wrbuf_cstr(xml), 0, 0);
    wrbuf_rewind(xml);
    wrbuf_printf(xml, "<record><leader>x</leader><leader>%.24s</leader>"
                 "%s</record>", leader, fields);
    check_leaders(wrbuf_cstr(xml), 0, 0);

    /* Reusing the character buffer must not leak a previous record's
     * content or indicator count into a short/missing leader. */
    wrbuf_rewind(xml);
    wrbuf_printf(xml, "<collection>"
                 "<record><leader>00062cgm a9200037Ia 4500</leader>%s</record>"
                 "<record><leader>x</leader>%s</record>"
                 "<record>%s</record>"
                 "<record><leader>%.24s</leader>%s</record>"
                 "</collection>", fields, fields, fields, leader, fields);
    check_leaders(wrbuf_cstr(xml), 2,
                  "<datafield tag=\"245\" ind1=\"1\" ind2=\"2\">");
    wrbuf_destroy(xml);
}
#endif


int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst1();
#if YAZ_HAVE_XML2
    tst_leader_lengths();
#endif
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
