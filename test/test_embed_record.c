/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2012 Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/record_conv.h>
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include <string.h>
#include <yaz/log.h>
#include <yaz/record_render.h>

#if YAZ_HAVE_XML2

#include <yaz/base64.h>
#include <yaz/marcdisp.h>
#include <yaz/proto.h>
#include <yaz/prt-ext.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

void test1(void)
{
  char base_enc[] = 
    "MDA3NjZuYW0gIDIyMDAyNjU4YSA0NTAwMDAxMDAxMjAwMDAwMDAzMDAwNjAwMDEyMDA1MDAx"
    "NzAwMDE4MDA4MDA0MTAwMDM1MDEwMDAxNzAwMDc2MDIwMDAxNTAwMDkzMDM1MDAxODAwMTA4"
    "MDQwMDAxODAwMTI2MDQ5MDAwOTAwMTQ0MDUwMDAyNjAwMTUzMDgyMDAxNzAwMTc5MTAwMDAx"
    "ODAwMTk2MjQ1MDA2NzAwMjE0MjYwMDA1MjAwMjgxMjYzMDAwOTAwMzMzMzAwMDAyNzAwMzQy"
    "NTAwMDAyNzAwMzY5NTA0MDA1MTAwMzk2NjUwMDA0NDAwNDQ3OTk5MDAwOTAwNDkxHm9jbTI4"
    "MzM5ODYzHk9Db0xDHjE5OTQwMTA1MDc0NTIyLjAeOTMwNjA5czE5OTQgICAgY291ICAgICAg"
    "YiAgICAwMDEgMCBlbmcgIB4gIB9hICAgOTMwMDkwNDcgHiAgH2EwMTMwMzA1NTI5HiAgH2Eo"
    "b2NtKTI4MzM5ODYzHiAgH2FETEMfY0RMQx9kS0tVHiAgH2FLS1VKHjAwH2FRQTc2LjczLkMy"
    "OB9iRzczIDE5OTQeMDAfYTAwNS4xMy8zHzIyMB4xIB9hR3JhaGFtLCBQYXVsLh4xMB9hT24g"
    "TGlzcCA6H2JhZHZhbmNlZCB0ZWNobmlxdWVzIGZvciBjb21tb24gTGlzcCAvH2NQYXVsIEdy"
    "YWhhbS4eICAfYUVuZ2xld29vZCBDbGlmZnMsIE4uSi4gOh9iUHJlbnRpY2UgSGFsbCwfYzE5"
    "OTQuHiAgH2E5NDEwHiAgH2F4aWlpLCA0MTMgcC4gOx9jMjMgY20uHiAgH2EiQW4gQWxhbiBS"
    "LiBBcHQgYm9vay4iHiAgH2FJbmNsdWRlcyBiaWJsaW9ncmFwaGljYWwgcmVmZXJlbmNlcyBh"
    "bmQgaW5kZXguHiAwH2FDT01NT04gTElTUCAoQ29tcHV0ZXIgcHJvZ3JhbSBsYW5ndWFnZSke"
    "H2xVQUhJTEweHQ==";

    char bin_marc[] = 
      "00766nam  22002658a 4500001001200000003000600012005001700018008004100035010001700076020001500093035001800108040001800126049000900144050002600153082001700179100001800196245006700214260005200281263000900333300002700342500002700369504005100396650004400447999000900491\036ocm28339863\036OCoLC\03619940105074522.0\036930609s1994    cou      b    001 0 eng  \036  \037a   93009047 \036  \037a0130305529\036  \037a(ocm)28339863\036  \037aDLC\037cDLC\037dKKU\036  \037aKKUJ\03600\037aQA76.73.C28\037bG73 1994\03600\037a005.13/3\037220\0361 \037aGraham, Paul.\03610\037aOn Lisp :\037badvanced techniques for common Lisp /\037cPaul Graham.\036  \037aEnglewood Cliffs, N.J. :\037bPrentice Hall,\037c1994.\036  \037a9410\036  \037axiii, 413 p. ;\037c23 cm.\036  \037a\"An Alan R. Apt book.\"\036  \037aIncludes bibliographical references and index.\036 0\037aCOMMON LISP (Computer program language)\036\037lUAHILL\036\035";

    int marc_size = strlen(bin_marc);
    char out_rec[1000];
    yaz_marc_t marc = yaz_marc_create();
    WRBUF buf = wrbuf_alloc();

    yaz_base64decode(base_enc, out_rec);
    YAZ_CHECK(strcmp(out_rec, bin_marc) == 0);

    yaz_marc_read_iso2709(marc, out_rec, marc_size);

    yaz_marc_write_marcxml(marc, buf);

    yaz_marc_destroy(marc);
    wrbuf_destroy(buf);

}
#endif

static int test_render(const char *type_spec, int is_marc, const char *input,
                    const char *expected_output)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    const char *actual_output;
    int actual_len;
    int res = 0;
    WRBUF wrbuf = wrbuf_alloc();

    Z_NamePlusRecord *npr = odr_malloc(odr, sizeof(*npr));
    npr->which = Z_NamePlusRecord_databaseRecord;
    if (is_marc)
        npr->u.databaseRecord = z_ext_record_usmarc(odr, input, strlen(input));
    else
        npr->u.databaseRecord = z_ext_record_xml(odr, input, strlen(input));

    actual_output = yaz_record_render(npr, 0, wrbuf, type_spec, &actual_len);

    if (actual_output && expected_output)
    {
        if (strlen(expected_output) == actual_len &&
            !memcmp(expected_output, actual_output, actual_len))
            res = 1;
        else
        {
            yaz_log(YLOG_LOG, "Got result");
            yaz_log(YLOG_LOG, "%.*s", actual_len, actual_output);
            yaz_log(YLOG_LOG, "Expected result");
            yaz_log(YLOG_LOG, "%s", expected_output);
        }
    }
    else if (!actual_output && !expected_output)
        res = 1;
    else if (!actual_output && expected_output)
    {
        yaz_log(YLOG_LOG, "Got null result, but expected");
        yaz_log(YLOG_LOG, "%s", expected_output);
    }
    else
    {
        yaz_log(YLOG_LOG, "Got result, but expected no result");
        yaz_log(YLOG_LOG, "%.*s", actual_len, actual_output);
    }
    wrbuf_destroy(wrbuf);
    odr_destroy(odr);
    return res;
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
#if YAZ_HAVE_XML2
    test1();
    YAZ_CHECK(test_render("xml", 0, "<my/>", "<my/>"));

    YAZ_CHECK(test_render(
                  "xml", 1, 
                  "\x30\x30\x31\x33\x38\x6E\x61\x6D\x20\x20\x32\x32\x30\x30\x30\x37"
                  "\x33\x38\x61\x20\x34\x35\x30\x30\x30\x30\x31\x30\x30\x31\x33\x30"
                  "\x30\x30\x30\x30\x30\x30\x33\x30\x30\x30\x34\x30\x30\x30\x31\x33"
                  "\x31\x30\x30\x30\x30\x31\x37\x30\x30\x30\x31\x37\x32\x34\x35\x30"
                  "\x30\x33\x30\x30\x30\x30\x33\x34\x1E\x20\x20\x20\x31\x31\x32\x32"
                  "\x34\x34\x36\x36\x20\x1E\x44\x4C\x43\x1E\x31\x30\x1F\x61\x4A\x61"
                  "\x63\x6B\x20\x43\x6F\x6C\x6C\x69\x6E\x73\x1E\x31\x30\x1F\x61\x48"
                  "\x6F\x77\x20\x74\x6F\x20\x70\x72\x6F\x67\x72\x61\x6D\x20\x61\x20"
                  "\x63\x6F\x6D\x70\x75\x74\x65\x72\x1E\x1D",
                  "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                  "  <leader>00138nam a22000738a 4500</leader>\n"
                  "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
                  "  <controlfield tag=\"003\">DLC</controlfield>\n"
                  "  <datafield tag=\"100\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">Jack Collins</subfield>\n"
                  "  </datafield>\n"
                  "  <datafield tag=\"245\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">How to program a computer</subfield>\n"
                  "  </datafield>\n"
                  "</record>\n"));

    YAZ_CHECK(test_render("xml", 0, "<my/>", "<my/>"));

    YAZ_CHECK(test_render(
                  "xml; base64(/my/text(),xml)", 0,
                  "<my>"
                  "MDAxMzhuYW0gIDIyMDAwNzM4YSA0NTAwMDAxMDAxMzAwMDAwMDAzMDAwNDAwMDEzMTAwMDAxNzAw"
                  "MDE3MjQ1MDAzMDAwMDM0HiAgIDExMjI0NDY2IB5ETEMeMTAfYUphY2sgQ29sbGlucx4xMB9hSG93"
                  "IHRvIHByb2dyYW0gYSBjb21wdXRlch4d"
                  "</my>",
                  "<?xml version=\"1.0\"?>\n"
                  "<my><record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                  "  <leader>00138nam a22000738a 4500</leader>\n"
                  "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
                  "  <controlfield tag=\"003\">DLC</controlfield>\n"
                  "  <datafield tag=\"100\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">Jack Collins</subfield>\n"
                  "  </datafield>\n"
                  "  <datafield tag=\"245\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">How to program a computer</subfield>\n"
                  "  </datafield>\n"
                  "</record></my>\n"));

    YAZ_CHECK(test_render(
                  "xml; charset=utf-8; base64(/my/text(),xml)", 0,
                  "<my>"
                  "MDAxMzhuYW0gIDIyMDAwNzM4YSA0NTAwMDAxMDAxMzAwMDAwMDAzMDAwNDAwMDEzMTAwMDAxNzAw"
                  "MDE3MjQ1MDAzMDAwMDM0HiAgIDExMjI0NDY2IB5ETEMeMTAfYUphY2sgQ29sbGlucx4xMB9hSG93"
                  "IHRvIHByb2dyYW0gYSBjb21wdXRlch4d"
                  "</my>",
                  "<?xml version=\"1.0\"?>\n"
                  "<my><record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                  "  <leader>00138nam a22000738a 4500</leader>\n"
                  "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
                  "  <controlfield tag=\"003\">DLC</controlfield>\n"
                  "  <datafield tag=\"100\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">Jack Collins</subfield>\n"
                  "  </datafield>\n"
                  "  <datafield tag=\"245\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">How to program a computer</subfield>\n"
                  "  </datafield>\n"
                  "</record></my>\n"));

    YAZ_CHECK(test_render(
                  "xml; base64(/my/text(),xml);charset=utf-8", 0,
                  "<my>"
                  "MDAxMzhuYW0gIDIyMDAwNzM4YSA0NTAwMDAxMDAxMzAwMDAwMDAzMDAwNDAwMDEzMTAwMDAxNzAw"
                  "MDE3MjQ1MDAzMDAwMDM0HiAgIDExMjI0NDY2IB5ETEMeMTAfYUphY2sgQ29sbGlucx4xMB9hSG93"
                  "IHRvIHByb2dyYW0gYSBjb21wdXRlch4d"
                  "</my>",
                  "<?xml version=\"1.0\"?>\n"
                  "<my><record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                  "  <leader>00138nam a22000738a 4500</leader>\n"
                  "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
                  "  <controlfield tag=\"003\">DLC</controlfield>\n"
                  "  <datafield tag=\"100\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">Jack Collins</subfield>\n"
                  "  </datafield>\n"
                  "  <datafield tag=\"245\" ind1=\"1\" ind2=\"0\">\n"
                  "    <subfield code=\"a\">How to program a computer</subfield>\n"
                  "  </datafield>\n"
                  "</record></my>\n"));

    YAZ_CHECK(test_render(
                  "xml; base64(/my/text(),txml;charset=utf-8)", 0,
                  "<my>"
                  "MDAxMzhuYW0gIDIyMDAwNzM4YSA0NTAwMDAxMDAxMzAwMDAwMDAzMDAwNDAwMDEzMTAwMDAxNzAw"
                  "MDE3MjQ1MDAzMDAwMDM0HiAgIDExMjI0NDY2IB5ETEMeMTAfYUphY2sgQ29sbGlucx4xMB9hSG93"
                  "IHRvIHByb2dyYW0gYSBjb21wdXRlch4d"
                  "</my>",
                  "<?xml version=\"1.0\"?>\n"
                  "<my><r xmlns=\"http://www.indexdata.com/turbomarc\">\n"
                  "  <l>00138nam a22000738a 4500</l>\n"
                  "  <c001>   11224466 </c001>\n"
                  "  <c003>DLC</c003>\n"
                  "  <d100 i1=\"1\" i2=\"0\">\n"
                  "    <sa>Jack Collins</sa>\n"
                  "  </d100>\n"
                  "  <d245 i1=\"1\" i2=\"0\">\n"
                  "    <sa>How to program a computer</sa>\n"
                  "  </d245>\n"
                  "</r></my>\n"));
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

