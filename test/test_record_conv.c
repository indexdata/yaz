/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
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
#include <yaz/proto.h>
#include <yaz/prt-ext.h>
#include <yaz/oid_db.h>
#if YAZ_HAVE_XML2

#include <libxml/parser.h>
#include <libxml/tree.h>

#if YAZ_HAVE_XSLT
#include <libxslt/xslt.h>
#endif

yaz_record_conv_t conv_configure(const char *xmlstring, WRBUF w)
{
    xmlDocPtr doc = xmlParseMemory(xmlstring, strlen(xmlstring));
    if (!doc)
    {
        wrbuf_printf(w, "xmlParseMemory");
        return 0;
    }
    else
    {
        xmlNodePtr ptr = xmlDocGetRootElement(doc);
        yaz_record_conv_t p = yaz_record_conv_create();

        if (p)
        {
            const char *srcdir = getenv("srcdir");
            if (srcdir)
                yaz_record_conv_set_path(p, srcdir);
        }
        if (!ptr)
        {
            wrbuf_printf(w, "xmlDocGetRootElement");
            yaz_record_conv_destroy(p);
            p = 0;
        }
        else if (!p)
        {
            wrbuf_printf(w, "yaz_record_conv_create");
        }
        else
        {


            int r = yaz_record_conv_configure(p, ptr);

            if (r)
            {
                wrbuf_puts(w, yaz_record_conv_get_error(p));
                yaz_record_conv_destroy(p);
                p = 0;
            }
        }
        xmlFreeDoc(doc);
        return p;
    }
}

int conv_configure_test(const char *xmlstring, const char *expect_error,
                        yaz_record_conv_t *pt)
{
    WRBUF w = wrbuf_alloc();
    int ret;

    yaz_record_conv_t p = conv_configure(xmlstring, w);

    if (!p)
    {
        if (expect_error && !strcmp(wrbuf_cstr(w), expect_error))
            ret = 1;
        else
        {
            ret = 0;
            printf("%s\n", wrbuf_cstr(w));
        }
    }
    else
    {
        if (expect_error)
            ret = 0;
        else
            ret = 1;
    }

    if (pt)
        *pt = p;
    else
        if (p)
            yaz_record_conv_destroy(p);

    wrbuf_destroy(w);
    return ret;
}

static void tst_configure(void)
{



    YAZ_CHECK(conv_configure_test("<bad", "xmlParseMemory", 0));


    YAZ_CHECK(conv_configure_test("<backend syntax='usmarc' name='F'>"
                                  "<bad/></backend>",
                                  "Element <backend>: expected <marc> or "
                                  "<xslt> element, got <bad>", 0));

#if YAZ_HAVE_XSLT
    YAZ_CHECK(conv_configure_test("<backend syntax='usmarc' name='F'>"
                                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"marc-8\""
                                  " outputcharset=\"marc-8\""
                                  "/>"
                                  "</backend>",
                                  "Element <marc>: attribute 'inputformat' "
                                  "required", 0));
    YAZ_CHECK(conv_configure_test("<backend syntax='usmarc' name='F'>"
                                  "<xslt/>"
                                  "</backend>",
                                  "Element <xslt>: attribute 'stylesheet' "
                                  "expected", 0));
    YAZ_CHECK(conv_configure_test("<backend syntax='usmarc' name='F'>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                                  "</backend>",
                                  0, 0));
#else
    YAZ_CHECK(conv_configure_test("<backend syntax='usmarc' name='F'>"
                                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                                  "</backend>",
                                  "xslt unsupported."
                                  " YAZ compiled without XSLT support", 0));
#endif
}

static int conv_convert_test(yaz_record_conv_t p,
                             const char *input_record,
                             const char *output_expect_record)
{
    int ret = 0;
    if (!p)
    {
        YAZ_CHECK(ret);
    }
    else
    {
        WRBUF output_record = wrbuf_alloc();
        int r = yaz_record_conv_record(p, input_record, strlen(input_record),
                                       output_record);
        if (r)
        {
            if (output_expect_record)
            {
                printf("yaz_record_conv error=%s\n",
                       yaz_record_conv_get_error(p));
                ret = 0;
            }
            else
                ret = 1;
        }
        else
        {
            if (!output_expect_record)
            {
                ret = 0;
            }
            else if (strcmp(output_expect_record, wrbuf_cstr(output_record)))
            {
                ret = 0;
                printf("got-output_record len=%ld: %s\n",
                       (long) wrbuf_len(output_record),
                       wrbuf_cstr(output_record));
                printf("output_expect_record len=%ld %s\n",
                       (long) strlen(output_expect_record),
                       output_expect_record);
            }
            else
            {
                ret = 1;
            }
        }
        wrbuf_destroy(output_record);
    }
    return ret;
}

static int conv_convert_test_iter(yaz_record_conv_t p,
                                  const char *input_record,
                                  const char *output_expect_record,
                                  int num_iter)
{
    int i;
    int ret;
    for (i = 0; i < num_iter; i++)
    {
        ret = conv_convert_test(p, input_record, output_expect_record);
        if (!ret)
            break;
    }
    return ret;
}

static void tst_convert1(void)
{
    yaz_record_conv_t p = 0;
    const char *marcxml_rec =
        "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
        "  <leader>00080nam a22000498a 4500</leader>\n"
        "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
        "  <datafield tag=\"010\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"a\">   11224466 </subfield>\n"
        "  </datafield>\n"
        "</record>\n";
    const char *tmarcxml_rec =
        "<r xmlns=\"http://www.indexdata.com/MARC21/turboxml\">\n"
        "  <l>00080nam a22000498a 4500</l>\n"
        "  <c001>   11224466 </c001>\n"
        "  <d010 i1=\" \" i2=\" \">\n"
        "    <sa>   11224466 </sa>\n"
        "  </d010>\n"
        "</r>\n";
    const char *iso2709_rec =
        "\x30\x30\x30\x38\x30\x6E\x61\x6D\x20\x61\x32\x32\x30\x30\x30\x34"
        "\x39\x38\x61\x20\x34\x35\x30\x30\x30\x30\x31\x30\x30\x31\x33\x30"
        "\x30\x30\x30\x30\x30\x31\x30\x30\x30\x31\x37\x30\x30\x30\x31\x33"
        "\x1E\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20\x1E\x20\x20"
        "\x1F\x61\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20\x1E\x1D";

    const char *solrmarc_rec =
        "\x30\x30\x30\x38\x30\x6E\x61\x6D\x20\x61\x32\x32\x30\x30\x30\x34"
        "\x39\x38\x61\x20\x34\x35\x30\x30\x30\x30\x31\x30\x30\x31\x33\x30"
        "\x30\x30\x30\x30\x30\x31\x30\x30\x30\x31\x37\x30\x30\x30\x31\x33"
        "#30;\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20#30;\x20\x20"
        "#31;\x61\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20#30;#29;";
    const char *raw_rec = /* raw is xml-string of marcxml_rec */
        "<raw>&lt;record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
        "  &lt;leader>00080nam a22000498a 4500&lt;/leader>\n"
        "  &lt;controlfield tag=\"001\">   11224466 &lt;/controlfield>\n"
        "  &lt;datafield tag=\"010\" ind1=\" \" ind2=\" \">\n"
        "    &lt;subfield code=\"a\">   11224466 &lt;/subfield>\n"
        "  &lt;/datafield>\n"
        "&lt;/record>\n</raw>\n";

    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "</backend>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, marcxml_rec, iso2709_rec));
    YAZ_CHECK(conv_convert_test(p, tmarcxml_rec, iso2709_rec));
    yaz_record_conv_destroy(p);

    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<marc"
                                  " outputcharset=\"utf-8\""
                                  " inputcharset=\"marc-8\""
                                  " outputformat=\"marcxml\""
                                  " inputformat=\"marc\""
                                  "/>"
                                  "</backend>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, iso2709_rec, marcxml_rec));
    yaz_record_conv_destroy(p);

    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<solrmarc/>"
                                  "<marc"
                                  " outputcharset=\"utf-8\""
                                  " inputcharset=\"marc-8\""
                                  " outputformat=\"marcxml\""
                                  " inputformat=\"marc\""
                                  "/>"
                                  "</backend>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, solrmarc_rec, marcxml_rec));
    yaz_record_conv_destroy(p);

    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "<marc"
                                  " outputcharset=\"utf-8\""
                                  " inputcharset=\"marc-8\""
                                  " outputformat=\"marcxml\""
                                  " inputformat=\"marc\""
                                  "/>"
                                  "</backend>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, marcxml_rec, marcxml_rec));
    yaz_record_conv_destroy(p);


    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                                  "<xslt stylesheet=\"test_record_conv.xsl\"/>"
                                  "<marc"
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "<marc"
                                  " inputcharset=\"marc-8\""
                                  " outputformat=\"marcxml\""
                                  " inputformat=\"marc\""
                                  "/>"
                                  "</backend>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, marcxml_rec, marcxml_rec));
    yaz_record_conv_destroy(p);

    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<select path=\"/raw\"/>"
                                  "</backend>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test(p, raw_rec, marcxml_rec));
    yaz_record_conv_destroy(p);
}

static void tst_convert2(void)
{
    yaz_record_conv_t p = 0;
    const char *marcxml_rec =
        "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
        "  <leader>00080nam a22000498a 4500</leader>\n"
        "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
        "  <datafield tag=\"010\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"a\">k&#xf8;benhavn</subfield>\n"
        "  </datafield>\n"
        "</record>\n";
    const char *iso2709_rec =
        "\x30\x30\x30\x37\x37\x6E\x61\x6D\x20\x61\x32\x32\x30\x30\x30\x34"
        "\x39\x38\x61\x20\x34\x35\x30\x30\x30\x30\x31\x30\x30\x31\x33\x30"
        "\x30\x30\x30\x30\x30\x31\x30\x30\x30\x31\x34\x30\x30\x30\x31\x33"
        "\x1E\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20\x1E\x20\x20"
        "\x1F\x61\x6b\xb2\x62\x65\x6e\x68\x61\x76\x6e\x1E\x1D";

    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<marc"
                                  " inputcharset=\"utf-8\""
                                  " outputcharset=\"marc-8\""
                                  " inputformat=\"xml\""
                                  " outputformat=\"marc\""
                                  "/>"
                                  "</backend>",
                                  0, &p));
    YAZ_CHECK(conv_convert_test_iter(p, marcxml_rec, iso2709_rec, 100));
    yaz_record_conv_destroy(p);
}

static void tst_convert3(void)
{
    NMEM nmem = nmem_create();
    int ret;
    yaz_record_conv_t p = 0;

    const char *iso2709_rec =
        "\x30\x30\x30\x37\x37\x6E\x61\x6D\x20\x20\x32\x32\x30\x30\x30\x34"
        "\x39\x38\x61\x20\x34\x35\x30\x30\x30\x30\x31\x30\x30\x31\x33\x30"
        "\x30\x30\x30\x30\x30\x31\x30\x30\x30\x31\x34\x30\x30\x30\x31\x33"
        "\x1E\x20\x20\x20\x31\x31\x32\x32\x34\x34\x36\x36\x20\x1E\x20\x20"
        "\x1F\x61\x6b\xb2\x62\x65\x6e\x68\x61\x76\x6e\x1E\x1D";

    const char *opacxml_rec =
        "<opacRecord>\n"
        "  <bibliographicRecord>\n"
        "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
        "  <leader>00077nam a22000498a 4500</leader>\n"
        "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
        "  <datafield tag=\"010\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"a\">k" "\xc3" "\xb8" /* oslash in UTF_8 */
        "benhavn</subfield>\n"
        "  </datafield>\n"
        "</record>\n"
        "  </bibliographicRecord>\n"
        "<holdings>\n"
        " <holding>\n"
        "  <typeOfRecord>u</typeOfRecord>\n"
        "  <encodingLevel>U</encodingLevel>\n"
        "  <receiptAcqStatus>0</receiptAcqStatus>\n"
        "  <dateOfReport>000000</dateOfReport>\n"
        "  <nucCode>s-FM/GC</nucCode>\n"
        "  <localLocation>Main or Science/Business Reading Rms - STORED OFFSITE</localLocation>\n"
        "  <callNumber>MLCM 89/00602 (N)</callNumber>\n"
        "  <shelvingData>FT MEADE</shelvingData>\n"
        "  <copyNumber>Copy 1</copyNumber>\n"
        "  <volumes>\n"
        "   <volume>\n"
        "    <enumeration>1</enumeration>\n"
        "    <chronology>2</chronology>\n"
        "    <enumAndChron>3</enumAndChron>\n"
        "   </volume>\n"
        "   <volume>\n"
        "    <enumeration>1</enumeration>\n"
        "    <chronology>2</chronology>\n"
        "    <enumAndChron>3</enumAndChron>\n"
        "   </volume>\n"
        "  </volumes>\n"
        "  <circulations>\n"
        "   <circulation>\n"
        "    <availableNow value=\"1\"/>\n"
        "    <availabilityDate>20130129</availabilityDate>\n"
        "    <itemId>1226176</itemId>\n"
        "    <renewable value=\"0\"/>\n"
        "    <onHold value=\"0\"/>\n"
        "   </circulation>\n"
        "  </circulations>\n"
        " </holding>\n"
        "</holdings>\n"
        "</opacRecord>\n";

    Z_OPACRecord *z_opac = nmem_malloc(nmem, sizeof(*z_opac));
    Z_HoldingsAndCircData *h;
    Z_CircRecord *circ;

    z_opac->bibliographicRecord =
        z_ext_record_oid_nmem(nmem, yaz_oid_recsyn_usmarc,
                              iso2709_rec, strlen(iso2709_rec));
    z_opac->num_holdingsData = 1;
    z_opac->holdingsData = (Z_HoldingsRecord **)
        nmem_malloc(nmem, sizeof(Z_HoldingsRecord *) * 1);
    z_opac->holdingsData[0] = (Z_HoldingsRecord *)
        nmem_malloc(nmem, sizeof(Z_HoldingsRecord));
    z_opac->holdingsData[0]->which = Z_HoldingsRecord_holdingsAndCirc;
    h = z_opac->holdingsData[0]->u.holdingsAndCirc = (Z_HoldingsAndCircData *)
         nmem_malloc(nmem, sizeof(*h));
    h->typeOfRecord = nmem_strdup(nmem, "u");
    h->encodingLevel = nmem_strdup(nmem, "U");
    h->format = 0;
    h->receiptAcqStatus = nmem_strdup(nmem, "0");
    h->generalRetention = 0;
    h->completeness = 0;
    h->dateOfReport = nmem_strdup(nmem, "000000");
    h->nucCode = nmem_strdup(nmem, "s-FM/GC");
    h->localLocation = nmem_strdup(nmem,
                                   "Main or Science/Business Reading "
                                   "Rms - STORED OFFSITE");
    h->shelvingLocation = 0;
    h->callNumber = nmem_strdup(nmem, "MLCM 89/00602 (N)");
    h->shelvingData = nmem_strdup(nmem, "FT MEADE");
    h->copyNumber = nmem_strdup(nmem, "Copy 1");
    h->publicNote = 0;
    h->reproductionNote = 0;
    h->termsUseRepro = 0;
    h->enumAndChron = 0;
    h->num_volumes = 2;
    h->volumes = 0;

    h->volumes = (Z_Volume **)
        nmem_malloc(nmem, 2 * sizeof(Z_Volume *));

    h->volumes[0] = (Z_Volume *)
        nmem_malloc(nmem, sizeof(Z_Volume));
    h->volumes[1] = h->volumes[0];

    h->volumes[0]->enumeration = nmem_strdup(nmem, "1");
    h->volumes[0]->chronology = nmem_strdup(nmem, "2");
    h->volumes[0]->enumAndChron = nmem_strdup(nmem, "3");

    h->num_circulationData = 1;
    h->circulationData = (Z_CircRecord **)
        nmem_malloc(nmem, 1 * sizeof(Z_CircRecord *));
    circ = h->circulationData[0] = (Z_CircRecord *)
        nmem_malloc(nmem, sizeof(Z_CircRecord));
    circ->availableNow = nmem_booldup(nmem, 1);
    circ->availablityDate = nmem_strdup(nmem, "20130129");
    circ->availableThru = 0;
    circ->restrictions = 0;
    circ->itemId = nmem_strdup(nmem, "1226176");
    circ->renewable = nmem_booldup(nmem, 0);
    circ->onHold = nmem_booldup(nmem, 0);
    circ->enumAndChron = 0;
    circ->midspine = 0;
    circ->temporaryLocation = 0;

    YAZ_CHECK(conv_configure_test("<backend>"
                                  "<marc"
                                  " inputcharset=\"marc-8\""
                                  " outputcharset=\"utf-8\""
                                  " inputformat=\"marc\""
                                  " outputformat=\"marcxml\""
                                  "/>"
                                  "</backend>",
                                  0, &p));

    if (p)
    {
        WRBUF output_record = wrbuf_alloc();
        ret = yaz_record_conv_opac_record(p, z_opac, output_record);
        YAZ_CHECK(ret == 0);
        if (ret == 0)
        {
            ret = strcmp(wrbuf_cstr(output_record), opacxml_rec);
            YAZ_CHECK(ret == 0);
            if (ret)
            {
                printf("got-output_record len=%ld: %s\n",
                       (long) wrbuf_len(output_record),
                       wrbuf_cstr(output_record));
                printf("output_expect_record len=%ld %s\n",
                       (long) strlen(opacxml_rec),
                       opacxml_rec);
            }
        }
        yaz_record_conv_destroy(p);
        wrbuf_destroy(output_record);
    }
    {
        Z_OPACRecord *opac = 0;
        yaz_marc_t mt =  yaz_marc_create();
        ret = yaz_xml_to_opac(mt, opacxml_rec, strlen(opacxml_rec),
                              &opac, 0 /* iconv */, nmem, 0);
        YAZ_CHECK(ret);
        YAZ_CHECK(opac);

        if (opac)
        {
            WRBUF output_record = wrbuf_alloc();
            char *p;

            yaz_marc_xml(mt, YAZ_MARC_MARCXML);
            yaz_opac_decode_wrbuf(mt, opac, output_record);

            /* change MARC size to 00077 from 00078, due to
               encoding of the aring (two bytes in UTF-8) */
            p = strstr(wrbuf_buf(output_record), "00078");
            YAZ_CHECK(p);
            if (p)
                p[4] = '7';

            ret = strcmp(wrbuf_cstr(output_record), opacxml_rec);
            YAZ_CHECK(ret == 0);
            if (ret)
            {
                printf("got-output_record len=%ld: %s\n",
                       (long) wrbuf_len(output_record),
                       wrbuf_cstr(output_record));
                printf("output_expect_record len=%ld %s\n",
                       (long) strlen(opacxml_rec),
                       opacxml_rec);
            }
            wrbuf_destroy(output_record);
        }
        yaz_marc_destroy(mt);
    }
    nmem_destroy(nmem);
}

static void tst_convert4(void)
{
    NMEM nmem = nmem_create();
    int ret;

    const char *opacxml_rec =
        "<opacRecord>\n"
        "  <bibliographicRecord>\n"
        "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
        "  <leader>00077nam a22000498a 4500</leader>\n"
        "  <controlfield tag=\"001\">   11224466 </controlfield>\n"
        "  <datafield tag=\"010\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"a\">k" "\xc3" "\xb8" /* oslash in UTF_8 */
        "benhavn</subfield>\n"
        "  </datafield>\n"
        "</record>\n"
        "  </bibliographicRecord>\n"
        "  <holdings>\n"
        "   <holding>\n"
        "    <shelvingLocation>Sprague Library hidden basement</shelvingLocation>\n"
        "    <callNumber>E98.L7L44 1976 </callNumber>\n"
        "    <volumes/>\n"
        "   </holding>\n"
        "  </holdings>\n"
        " </opacRecord>\n"
        ;

    Z_OPACRecord *opac = 0;
    yaz_marc_t mt =  yaz_marc_create();
    ret = yaz_xml_to_opac(mt, opacxml_rec, strlen(opacxml_rec),
                          &opac, 0 /* iconv */, nmem, 0);
    YAZ_CHECK(ret);
    YAZ_CHECK(opac);
    yaz_marc_destroy(mt);
    nmem_destroy(nmem);
}

#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    yaz_log_xml_errors(0, 0 /* disable log */);
#if YAZ_HAVE_XML2
    tst_configure();
#endif
#if YAZ_HAVE_XSLT
    tst_convert1();
    tst_convert2();
    tst_convert3();
    tst_convert4();
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

