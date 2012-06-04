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

#include <libxml/parser.h>
#include <libxml/tree.h>

void test(void)
{
    char sample_rec[] = "MTIzNAo=";
    char out_rec[10];
    yaz_base64decode(sample_rec, out_rec);
    YAZ_CHECK(strcmp(out_rec, "1234\n") == 0);
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

