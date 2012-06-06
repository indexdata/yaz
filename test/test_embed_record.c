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

#if YAZ_HAVE_XML2

#include <yaz/base64.h>
#include <yaz/marcdisp.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

void test(void)
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
    yaz_base64decode(base_enc, out_rec);
    YAZ_CHECK(strcmp(out_rec, bin_marc) == 0);

    yaz_marc_t marc = yaz_marc_create();
    yaz_marc_read_iso2709(marc, out_rec, marc_size);

    WRBUF buf = wrbuf_alloc();
    yaz_marc_write_marcxml(marc, buf);

    yaz_marc_destroy(marc);
    wrbuf_destroy(buf);

}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
#if YAZ_HAVE_XML2
    test();
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

