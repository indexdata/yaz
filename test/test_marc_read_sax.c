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


static void handler(yaz_marc_t mt, void *cb)
{

}

static void tst1(void)
{
    char *marcxml = "<collection xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
     "<record>\n"
     "  <leader>00062cgm a2200037Ia 4500</leader>\n"
     "  <datafield tag=\"092\" ind1=\" \" ind2=\" \">\n"
     "    <subfield code=\"a\">DVD CHI 791.43</subfield>\n"
     "    <subfield code=\"b\">QI</subfield>\n"
     "  </datafield>\n"
     "</record>\n"
     "</collection>\n"
     ;
     
    yaz_marc_t mt = yaz_marc_create();
    void *cb = mt;
    yaz_marc_sax_t yt = yaz_marc_sax_new(mt, cb, handler);
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

