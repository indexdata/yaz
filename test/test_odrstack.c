/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <yaz/pquery.h>
#include <yaz/proto.h>
#include <yaz/test.h>

/** \brief build a 100 level query */
void test1(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    YAZ_PQF_Parser parser = yaz_pqf_create();
    char qstr[10000];

    YAZ_CHECK(odr);
    YAZ_CHECK(parser);

    *qstr = '\0';
    for (int i = 0; i<100; i++)
        strcat(qstr, "@and 1 ");
    strcat(qstr, "1");

    Z_RPNQuery *rpn_query = yaz_pqf_parse (parser, odr, qstr);
    YAZ_CHECK(rpn_query);

    int ret = z_RPNQuery(odr, &rpn_query, 0, 0);
    YAZ_CHECK(ret);

    yaz_pqf_destroy(parser);
    odr_destroy(odr);
}

/** \brief build a circular referenced query */
void test2(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    YAZ_PQF_Parser parser = yaz_pqf_create();

    YAZ_CHECK(odr);

    Z_RPNQuery *rpn_query = yaz_pqf_parse (parser, odr, "@and @and a b c");
    YAZ_CHECK(rpn_query);

    /* make the circular reference */
    rpn_query->RPNStructure->u.complex->s1 = rpn_query->RPNStructure;

    int ret = z_RPNQuery(odr, &rpn_query, 0, 0);  /* should fail */
    YAZ_CHECK(!ret);

    yaz_pqf_destroy(parser);
    odr_destroy(odr);
}

/** \brief decode a string of nested constructed segments in a SEQUENCE */
static int decode_nested_string(int tag, int depth)
{
    ODR odr = odr_createmem(ODR_DECODE);
    int len = 4 * depth + 4;
    char *buf = (char *) odr_malloc(odr, len);
    void *seq = 0;
    Odr_oct *oct = 0;
    Odr_bitmask *bits = 0;
    int i;
    int ret;

    memset(buf, 0, len);                  /* end-of-contents for all */
    buf[0] = 0x20 | ODR_SEQUENCE;
    buf[1] = (char) 0x80;
    for (i = 1; i <= depth; i++)
    {
        buf[2 * i] = 0x20 | tag;          /* constructed */
        buf[2 * i + 1] = (char) 0x80;     /* indefinite length */
    }
    odr_setbuf(odr, buf, len, 0);
    ret = odr_sequence_begin(odr, &seq, 0, 0) &&
        (tag == ODR_OCTETSTRING ?
         odr_octetstring(odr, &oct, 0, 0) :
         odr_bitstring(odr, &bits, 0, 0)) &&
        odr_sequence_end(odr);
    odr_destroy(odr);
    return ret;
}

/** \brief nested constructed strings must not exhaust the C stack */
void test3(void)
{
    YAZ_CHECK(decode_nested_string(ODR_OCTETSTRING, 10));
    YAZ_CHECK(decode_nested_string(ODR_BITSTRING, 10));
    YAZ_CHECK(!decode_nested_string(ODR_OCTETSTRING, 100000));
    YAZ_CHECK(!decode_nested_string(ODR_BITSTRING, 100000));
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    test1();
    test2();
    test3();
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

