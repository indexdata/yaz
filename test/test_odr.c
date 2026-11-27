/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <yaz/oid_util.h>
#include <yaz/log.h>
#include <yaz/charneg.h>
#include <yaz/proto.h>
#include "test_odrcodec.h"

#include <yaz/test.h>

#define MYOID  "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19"

void tst_MySequence1(ODR encode, ODR decode)
{
    int ber_len;
    Yc_MySequence *s = (Yc_MySequence *) odr_malloc(encode, sizeof(*s));
    Yc_MySequence *t;

    YAZ_CHECK(s);
    s->first = odr_intdup(encode, 12345);
    s->second = (Odr_oct *) odr_malloc(encode, sizeof(*s->second));
    s->second->buf = (char *) "hello";
    s->second->len = 5;
    s->third = odr_booldup(encode, 1);
    s->fourth = odr_nullval();
    s->fifth = odr_intdup(encode, YC_MySequence_enum1);

    s->myoid = odr_getoidbystr(decode, MYOID);

    int ret = yc_MySequence(encode, &s, 0, 0);
    YAZ_CHECK(ret);
    if (!ret)
        return;

    char *ber_buf = odr_getbuf(encode, &ber_len, 0);

    odr_setbuf(decode, ber_buf, ber_len, 0);

    ret = yc_MySequence(decode, &t, 0, 0);
    YAZ_CHECK(ret);
    if (!ret)
        return;

    YAZ_CHECK(t);

    YAZ_CHECK(t->first && *t->first == 12345);

    YAZ_CHECK(t->second && t->second->buf && t->second->len == 5);

    YAZ_CHECK(t->second && t->second->buf && t->second->len == 5 &&
              memcmp(t->second->buf, "hello", t->second->len) == 0);

    YAZ_CHECK(t->third && *t->third == 1);

    YAZ_CHECK(t->fourth);

    YAZ_CHECK(t->fifth && *t->fifth == YC_MySequence_enum1);

    YAZ_CHECK(t->myoid);
    if (t->myoid)
    {
        Odr_oid *myoid = odr_getoidbystr(decode, MYOID);

        YAZ_CHECK(oid_oidcmp(myoid, t->myoid) == 0);
    }
}

void tst_MySequence2(ODR encode, ODR decode)
{
    Yc_MySequence *s = (Yc_MySequence *) odr_malloc(encode, sizeof(*s));

    YAZ_CHECK(s);
    s->first = 0;  /* deliberately miss this .. */
    s->second = (Odr_oct *) odr_malloc(encode, sizeof(*s->second));
    s->second->buf = (char *) "hello";
    s->second->len = 5;
    s->third = odr_booldup(encode, 1);
    s->fourth = odr_nullval();
    s->fifth = odr_intdup(encode, YC_MySequence_enum1);
    s->myoid = odr_getoidbystr(encode, MYOID);

    int ret = yc_MySequence(encode, &s, 0, 0); /* should fail */
    YAZ_CHECK(!ret);

    YAZ_CHECK(odr_geterror(encode) == OREQUIRED);

    YAZ_CHECK(strcmp(odr_getelement(encode), "first") == 0);
    odr_reset(encode);

    YAZ_CHECK(odr_geterror(encode) == ONONE);

    YAZ_CHECK(strcmp(odr_getelement(encode), "") == 0);
}

void tst_MySequence3(ODR encode, ODR decode)
{
    char buf[40];
    Yc_MySequence *t;

    srand(123);
    for (int i = 0; i < 1000; i++)
    {
        int j;
        for (j = 0; j < (int) sizeof(buf); j++)
            buf[j] = rand();

        for (j = 1; j < (int) sizeof(buf); j++)
        {
            odr_setbuf(decode, buf, j, 0);
            yc_MySequence(decode, &t, 0, 0);
            odr_reset(decode);
        }
    }
}

static void tst_berint32(ODR encode, ODR decode)
{
    char *buf = 0;
    int len = 0;
    Odr_int ret_val;

    Odr_int val = 0;
    odr_reset(encode);
    int r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 2);
    YAZ_CHECK_EQ(buf[0], 1);
    YAZ_CHECK_EQ(buf[1], 0);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, 0);

    val = 1;
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 2);
    YAZ_CHECK_EQ(buf[0], 1);
    YAZ_CHECK_EQ(buf[1], 1);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, 1);

    val = -1;
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 2);
    YAZ_CHECK_EQ(buf[0], 1);
    YAZ_CHECK_EQ((unsigned char) buf[1], 255);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, -1);

    val = 127;
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 2);
    YAZ_CHECK_EQ(buf[0], 1);
    YAZ_CHECK_EQ(buf[1], 127);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, 127);


    val = 128;
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 3);
    YAZ_CHECK_EQ(buf[0], 2);
    YAZ_CHECK_EQ(buf[1], 0);
    YAZ_CHECK_EQ(((unsigned char *) buf)[2], 128);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, 128);

    val = 2147483647; /* 2^31-1 */
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 5);
    YAZ_CHECK_EQ(buf[0], 4);
    YAZ_CHECK_EQ(buf[1], 127);
    YAZ_CHECK_EQ((unsigned char) buf[2], 255);
    YAZ_CHECK_EQ((unsigned char) buf[3], 255);
    YAZ_CHECK_EQ((unsigned char) buf[4], 255);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, 2147483647);

    val = -2147483647L -1; /* -2^31 */
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 5);
    YAZ_CHECK_EQ(buf[0], 4);
    YAZ_CHECK_EQ((unsigned char) buf[1], 128);
    YAZ_CHECK_EQ(buf[2], 0);
    YAZ_CHECK_EQ(buf[3], 0);
    YAZ_CHECK_EQ(buf[4], 0);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, (Odr_int) -2147483647L -1);
}

static void tst_berint64(ODR encode, ODR decode)
{
    char *buf = 0;
    int len = 0;
    Odr_int ret_val;

    Odr_int val = (Odr_int) 2 * 2147483648UL; /* 2^32 */
    odr_reset(encode);
    int r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 6);
    YAZ_CHECK_EQ(buf[0], 5);
    YAZ_CHECK_EQ(buf[1], 1);
    YAZ_CHECK_EQ(buf[2], 0);
    YAZ_CHECK_EQ(buf[3], 0);
    YAZ_CHECK_EQ(buf[4], 0);
    YAZ_CHECK_EQ(buf[5], 0);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, val);

    val = (Odr_int) -2 * 2147483648UL; /* -2^32 */
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 6);
    YAZ_CHECK_EQ(buf[0], 5);
    YAZ_CHECK_EQ((unsigned char) buf[1], 255);
    YAZ_CHECK_EQ(buf[2], 0);
    YAZ_CHECK_EQ(buf[3], 0);
    YAZ_CHECK_EQ(buf[4], 0);
    YAZ_CHECK_EQ(buf[5], 0);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, val);

    val = (Odr_int) 1000 * 1000000000L; /* 10^12 */
    odr_reset(encode);
    r = ber_integer(encode, &val);
    YAZ_CHECK_EQ(r, 1);
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK(buf);
    YAZ_CHECK_EQ(len, 7);
    YAZ_CHECK_EQ(buf[0], 6);
    YAZ_CHECK_EQ(buf[1], 0);
    YAZ_CHECK_EQ((unsigned char) buf[2], 232);
    YAZ_CHECK_EQ((unsigned char) buf[3], 212);
    YAZ_CHECK_EQ((unsigned char) buf[4], 165);
    YAZ_CHECK_EQ(buf[5], 16);
    YAZ_CHECK_EQ(buf[6], 0);

    odr_reset(decode);
    odr_setbuf(decode, buf, len, 0);
    ber_integer(decode, &ret_val);
    YAZ_CHECK_EQ(ret_val, val);
}

/* a decoder that fails without setting an ODR error */
static int tst_choice_fail(ODR o, char **p, int opt, const char *name)
{
    return 0;
}

/* An arm whose tag is there is present: if it does not decode, that is an
   error. Only a CHOICE with none of its tags is absent (0 and no error). */
static void tst_choice(ODR decode)
{
    static Odr_arm arm[] = {
        {ODR_EXPLICIT, ODR_CONTEXT, 1, 1, (Odr_fun) odr_integer, "first"},
        {ODR_IMPLICIT, ODR_CONTEXT, 2, 2, (Odr_fun) odr_integer, "second"},
        {ODR_IMPLICIT, ODR_CONTEXT, 3, 3, (Odr_fun) tst_choice_fail, "third"},
        {-1, -1, -1, -1, (Odr_fun) 0, 0}
    };
    void *u;
    int which;

    odr_reset(decode);
    odr_setbuf(decode, (char *) "\x82\x01\x05", 3, 0);
    YAZ_CHECK(odr_choice(decode, arm, &u, &which, 0));
    YAZ_CHECK_EQ(which, 2);

    /* no arm has tag [4] */
    odr_reset(decode);
    odr_setbuf(decode, (char *) "\x84\x01\x05", 3, 0);
    YAZ_CHECK(!odr_choice(decode, arm, &u, &which, 0));
    YAZ_CHECK_EQ(odr_geterror(decode), ONONE);

    /* [1] is EXPLICIT, but primitive here */
    odr_reset(decode);
    odr_setbuf(decode, (char *) "\x81\x01\x05", 3, 0);
    YAZ_CHECK(!odr_choice(decode, arm, &u, &which, 0));
    YAZ_CHECK_EQ(odr_geterror(decode), OPROTO);

    /* [3] is there, but its decoder fails and sets no error */
    odr_reset(decode);
    odr_setbuf(decode, (char *) "\x83\x01\x05", 3, 0);
    YAZ_CHECK(!odr_choice(decode, arm, &u, &which, 0));
    YAZ_CHECK_EQ(odr_geterror(decode), OPROTO);
}

static void tst_berint_empty(ODR decode)
{
    /* length 0: no content octets. Second byte is not part of the buffer */
    char buf[] = { 0, (char) 0xff };
    Odr_int ret_val = 0;

    odr_reset(decode);
    odr_setbuf(decode, buf, 1, 0);
    YAZ_CHECK_EQ(ber_integer(decode, &ret_val), 0);
    YAZ_CHECK_EQ(odr_geterror(decode), OPROTO);
    YAZ_CHECK_EQ(ret_val, 0);
    odr_reset(decode);
}

static void tst(void)
{
    ODR odr_encode = odr_createmem(ODR_ENCODE);
    ODR odr_decode = odr_createmem(ODR_DECODE);

    YAZ_CHECK(odr_encode);
    YAZ_CHECK(odr_decode);

    tst_MySequence1(odr_encode, odr_decode);
    tst_MySequence2(odr_encode, odr_decode);
    tst_MySequence3(odr_encode, odr_decode);

    tst_berint32(odr_encode, odr_decode);
    tst_berint64(odr_encode, odr_decode);
    tst_berint_empty(odr_decode);

    tst_choice(odr_decode);

    odr_destroy(odr_encode);
    odr_destroy(odr_decode);
}

/* example from documentation.. 'Using Odr' */
void do_nothing_useful(Odr_int value)
{
    ODR encode, decode;
    Odr_int *valp, *resvalp;
    int len;

    /* allocate streams */
    if (!(encode = odr_createmem(ODR_ENCODE)))
        return;
    if (!(decode = odr_createmem(ODR_DECODE)))
        return;

    valp = &value;
    if (odr_integer(encode, &valp, 0, 0) == 0)
    {
        printf("encoding went bad\n");
        return;
    }
    char *bufferp = odr_getbuf(encode, &len, 0);
    printf("length of encoded data is %d\n", len);

    /* now let's decode the thing again */
    odr_setbuf(decode, bufferp, len, 0);
    if (odr_integer(decode, &resvalp, 0, 0) == 0)
    {
        printf("decoding went bad\n");
        return;
    }
    /* ODR_INT_PRINTF format for printf (such as %d) */
    printf("the value is " ODR_INT_PRINTF "\n", *resvalp);

    /* clean up */
    odr_destroy(encode);
    odr_destroy(decode);
}

static void tst_ber_length(void)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    const int values[] = { 0, 127, 128, 65535, INT_MAX };
    size_t i;
    int exact, len;
    unsigned char overflow[sizeof(int) + 1];
    unsigned int value = (unsigned int) INT_MAX + 1U;

    for (i = 0; i < sizeof(values) / sizeof(*values); i++)
        for (exact = 0; exact <= 1; exact++)
        {
            int size;
            odr_reset(encode);
            YAZ_CHECK(ber_enclen(encode, values[i], sizeof(int) + 2,
                                exact) > 0);
            char *buf = odr_getbuf(encode, &size, 0);
            YAZ_CHECK_EQ(ber_declen(buf, &len, size), size);
            YAZ_CHECK_EQ(len, values[i]);
            YAZ_CHECK_EQ(ber_declen(buf, &len, size - 1), -1);
        }

    overflow[0] = 0x80 | sizeof(int);
    for (i = sizeof(int); i > 0; i--)
    {
        overflow[i] = value & 0xff;
        value >>= 8;
    }
    YAZ_CHECK_EQ(ber_declen((const char *) overflow, &len,
                           sizeof(overflow)), -2);
    memset(overflow + 1, 0xff, sizeof(int));
    YAZ_CHECK_EQ(ber_declen((const char *) overflow, &len,
                           sizeof(overflow)), -2);
    YAZ_CHECK_EQ(ber_declen("\x80", &len, 1), 1);
    YAZ_CHECK_EQ(len, -1);
    YAZ_CHECK_EQ(ber_declen("\xff", &len, 1), -2);
    odr_destroy(encode);
}

static void tst_odr_create_Odr_oct2(void)
{
    ODR o = odr_createmem(ODR_ENCODE);
    const char data[] = { 'a', '\0', 'b' };
    Odr_oct *oct = odr_create_Odr_oct2(o, data, sizeof(data));

    YAZ_CHECK(oct);
    YAZ_CHECK_EQ(oct->len, sizeof(data));
    YAZ_CHECK(memcmp(oct->buf, data, sizeof(data)) == 0);
    YAZ_CHECK_EQ(oct->buf[sizeof(data)], '\0');
    oct = odr_create_Odr_oct2(o, "", 0);
    YAZ_CHECK(oct);
    YAZ_CHECK_EQ(oct->len, 0);
    YAZ_CHECK_EQ(oct->buf[0], '\0');

    size_t total = odr_total(o);
    YAZ_CHECK(!odr_create_Odr_oct2(o, "", (size_t) INT_MAX + 1));
    YAZ_CHECK_EQ(odr_geterror(o), OLENOV);
    YAZ_CHECK_EQ(odr_total(o), total);
    odr_reset(o);
    YAZ_CHECK(!odr_create_Odr_oct2(o, "", (size_t) -1));
    YAZ_CHECK_EQ(odr_geterror(o), OLENOV);
    odr_reset(o);
    YAZ_CHECK(!odr_create_Odr_oct(o, "", -1));
    YAZ_CHECK_EQ(odr_geterror(o), OLENOV);
    odr_destroy(o);
}

static int tst_decode_string(const char *ber, size_t len, const char *expect)
{
    ODR decode = odr_createmem(ODR_DECODE);
    char *buf = (char *) malloc(len); /* exact size, no NUL */
    size_t expect_len = expect ? strlen(expect) : 0;
    Odr_oct *oct = 0;
    char *str = 0;
    int i, ok = 1;

    memcpy(buf, ber, len);
    for (i = 0; i < 3; i++)
    {
        int r;

        odr_setbuf(decode, buf, (int) len, 0);
        if (i == 0)
        {
            r = odr_octetstring(decode, &oct, 0, 0);
            if (r && (!oct || !oct->buf || oct->len != (int) expect_len))
                ok = 0;
            else if (r)
                str = oct->buf;
        }
        else if (i == 1)
            r = odr_cstring(decode, &str, 0, 0);
        else
            r = odr_iconv_string(decode, &str, 0, 0);
        if (expect)
        {
            if (!r || !str || memcmp(str, expect, expect_len + 1))
                ok = 0;
        }
        else if (r || !odr_geterror(decode))
            ok = 0;
        odr_reset(decode);
    }
    odr_destroy(decode);
    free(buf);
    return ok;
}

#define TST_STRING(ber, expect) \
    YAZ_CHECK(tst_decode_string(ber, sizeof(ber) - 1, expect))

static void tst_constructed_string(void)
{
    char nested[4 * 3000]; /* more levels than the decoder accepts */
    size_t i;

    TST_STRING("\x04\x02" "AB", "AB");
    TST_STRING("\x04\x00", "");
    /* one segment, two segments, no segments */
    TST_STRING("\x24\x04\x04\x02" "AB", "AB");
    TST_STRING("\x24\x07\x04\x02" "AB" "\x04\x01" "C", "ABC");
    TST_STRING("\x24\x00", "");
    /* indefinite length */
    TST_STRING("\x24\x80\x04\x02" "AB" "\x04\x01" "C" "\x00\x00", "ABC");
    TST_STRING("\x24\x80\x00\x00", "");
    /* segments that are constructed themselves */
    TST_STRING("\x24\x0a\x04\x01" "A" "\x24\x80\x04\x01" "B" "\x00\x00",
               "AB");
    TST_STRING("\x24\x0b\x24\x06\x24\x04\x04\x02" "AB" "\x04\x01" "C",
               "ABC");

    /* segment that is not an OCTET STRING */
    TST_STRING("\x24\x04\x0c\x02" "AB", 0);
    TST_STRING("\x24\x04\x84\x02" "AB", 0);
    /* segment beyond the end of the string or of the buffer */
    TST_STRING("\x24\x03\x04\x02" "AB", 0);
    TST_STRING("\x24\x04\x04\x03" "AB", 0);
    TST_STRING("\x24\x05\x04\x02" "AB", 0);
    TST_STRING("\x24\x06\x24\x02\x04\x02" "AB", 0);
    TST_STRING("\x24\x03\x04", 0);
    TST_STRING("\x24\x04\x04\x80" "AB", 0);
    /* missing end of indefinite length content */
    TST_STRING("\x24\x80", 0);
    TST_STRING("\x24\x80\x04\x02" "AB", 0);
    TST_STRING("\x24\x80\x04\x02" "AB" "\x00", 0);
    TST_STRING("\x24\x80\x24\x80\x04\x02" "AB" "\x00\x00", 0);

    /* nesting is limited */
    memset(nested, 0, sizeof(nested));
    for (i = 0; i < sizeof(nested) / 2; i += 2)
    {
        nested[i] = 0x24;
        nested[i + 1] = (char) 0x80;
    }
    YAZ_CHECK(tst_decode_string(nested, sizeof(nested), 0));
    memset(nested + 20, 0, sizeof(nested) - 20);
    YAZ_CHECK(tst_decode_string(nested, 40, ""));
}

static void tst_odr_write2(void)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    int len;

    YAZ_CHECK_EQ(odr_write2(encode, "abc", 3), 0);
    char *buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK_EQ(len, 3);
    YAZ_CHECK(memcmp(buf, "abc", 3) == 0);
    YAZ_CHECK_EQ(odr_write2(encode, "", 0), 0);

    /* Reject before reading the source or allocating a huge buffer. */
    YAZ_CHECK_EQ(odr_write2(encode, "", (size_t) INT_MAX + 1), -1);
    YAZ_CHECK_EQ(odr_geterror(encode), OSPACE);
    odr_getbuf(encode, &len, 0);
    YAZ_CHECK_EQ(len, 3);

    odr_reset(encode);
    YAZ_CHECK_EQ(odr_write2(encode, "x", 1), 0);
    /* An int-sized write can still overflow the combined position. */
    YAZ_CHECK_EQ(odr_write2(encode, "", INT_MAX), -1);
    YAZ_CHECK_EQ(odr_geterror(encode), OSPACE);
    odr_getbuf(encode, &len, 0);
    YAZ_CHECK_EQ(len, 1);
    odr_destroy(encode);
}

static void tst_external_no_encoding(void)
{
    /* EXTERNAL with direct-reference SUTRS (a known type) and no encoding */
    char buf[] = { 0x28, 0x09, 0x06, 0x07,
                   0x2a, (char) 0x86, 0x48, (char) 0xce, 0x13, 0x05, 0x65 };
    ODR decode = odr_createmem(ODR_DECODE);
    Z_External *ext = 0;

    odr_setbuf(decode, buf, sizeof(buf), 0);
    /* present, but incomplete: must fail even if optional */
    YAZ_CHECK(!z_External(decode, &ext, 1, 0));
    YAZ_CHECK_EQ(odr_geterror(decode), OREQUIRED);
    odr_destroy(decode);
}

/* end-of-contents octets beyond the end of the buffer must not be used */
static void tst_indefinite_length(void)
{
    /* SEQUENCE { OCTET STRING (constructed) }, both of indefinite length */
    static char buf[] = { 0x30, (char) 0x80, 0x24, (char) 0x80, 0, 0, 0, 0 };
    int len;

    for (len = 2; len <= (int) sizeof(buf); len++)
    {
        ODR decode = odr_createmem(ODR_DECODE);
        char *p = 0;
        Odr_oct *oct = 0;
        int ret;

        odr_setbuf(decode, buf, len, 0);
        ret = odr_constructed_begin(decode, &p, ODR_UNIVERSAL,
                                    ODR_SEQUENCE, 0)
            && odr_octetstring(decode, &oct, 0, 0)
            && odr_constructed_end(decode);
        YAZ_CHECK_EQ(ret, len == (int) sizeof(buf));
        odr_destroy(decode);
    }
}

/* A tag that ber_tag looked at, but did not take, must not be matched
   later at another position in the buffer (or in another buffer). */
static void tst_ber_tag_position(void)
{
    /* SEQUENCE { a [1] INTEGER OPTIONAL, b ANY, c [2] INTEGER OPTIONAL } */
    static char seq[] = "\x30\x06" "\x82\x01\x05" "\x02\x01\x07";
    /* SearchRequest. additionalSearchInfo holds an OPAC record as
       octet-aligned EXTERNAL, which ends at an unknown tag [201].
       otherInfo [201] is absent. The zero bytes after the 71 byte PDU
       would end the contents for a decoder that has moved beyond it */
    static char pdu[80] =
        "\xb6\x45\x8d\x01\x00\x8e\x01\x01\x8f\x01\x00\x90\x01\xff"
        "\x91\x07" "default" "\xb2\x0a\x9f\x69\x07" "Default"
        "\xb5\x05\xa2\x03\x04\x01x"
        "\xbf\x81\x4b\x19\x30\x80\xa4\x80"
        "\x06\x07\x2a\x86\x48\xce\x13\x05\x66"
        "\x81\x08\x30\x80\xa1\x80\x9f\x80\x81\x49"
        "\xff\xff";
    ODR decode = odr_createmem(ODR_DECODE);
    char *p = 0;
    Odr_int *a = 0, *c = 0;
    Odr_any *b = 0;
    Z_APDU *apdu = 0;

    odr_setbuf(decode, seq, sizeof(seq) - 1, 0);
    YAZ_CHECK(odr_sequence_begin(decode, &p, 1, 0));
    YAZ_CHECK(odr_implicit_tag(decode, odr_integer, &a, ODR_CONTEXT, 1, 1, 0));
    YAZ_CHECK(!a);
    YAZ_CHECK(odr_any(decode, &b, 0, 0));
    YAZ_CHECK(odr_implicit_tag(decode, odr_integer, &c, ODR_CONTEXT, 2, 1, 0));
    YAZ_CHECK(!c);

    odr_reset(decode);
    odr_setbuf(decode, pdu, 71, 0);
    YAZ_CHECK(!z_APDU(decode, &apdu, 0, 0));
    odr_destroy(decode);
}


static void tst_charneg_response(void)
{
    /* response [2]: selectedCharSets [1] holding a primitive private [3],
       and selectedCharSets omitted */
    static const char *bufs[] = { "\xa2\x03\xa1\x01\x83", "\xa2\x00" };
    static const char *charsets[] = { "ISO-8859-1", "UTF-8" };
    size_t i;

    for (i = 0; i < sizeof(bufs) / sizeof(*bufs); i++)
    {
        ODR decode = odr_createmem(ODR_DECODE);
        Z_CharSetandLanguageNegotiation *p = 0;

        odr_setbuf(decode, (char *) bufs[i], (int) strlen(bufs[i]), 0);
        if (z_CharSetandLanguageNegotiation(decode, &p, 0, 0))
        {
            char *charset = 0, *lang = 0;
            int selected = -1;

            YAZ_CHECK_EQ(p->which, Z_CharSetandLanguageNegotiation_response);
            YAZ_CHECK(!p->u.response->u.zprivate);
            yaz_get_response_charneg(odr_getmem(decode), p, &charset, &lang,
                                     &selected);
            YAZ_CHECK(!charset);
            YAZ_CHECK(!lang);
            YAZ_CHECK_EQ(selected, -1);
        }
        odr_destroy(decode);
    }
    for (i = 0; i < sizeof(charsets) / sizeof(*charsets); i++)
    {
        ODR encode = odr_createmem(ODR_ENCODE);
        ODR decode = odr_createmem(ODR_DECODE);
        Z_External *ext = yaz_set_response_charneg(encode, charsets[i], 0, 1);
        Z_CharSetandLanguageNegotiation *p = 0;
        char *buf, *charset = 0;
        int len;

        YAZ_CHECK(z_CharSetandLanguageNegotiation(encode, &ext->u.charNeg3,
                                                  0, 0));
        buf = odr_getbuf(encode, &len, 0);
        odr_setbuf(decode, buf, len, 0);
        YAZ_CHECK(z_CharSetandLanguageNegotiation(decode, &p, 0, 0));
        if (p)
            yaz_get_response_charneg(odr_getmem(decode), p, &charset, 0, 0);
        YAZ_CHECK(charset && !strcmp(charset, charsets[i]));
        odr_destroy(decode);
        odr_destroy(encode);
    }
}

static void tst_oid_arc_range(void)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    ODR decode = odr_createmem(ODR_DECODE);
    /* 1.2.840.32767 and 1.2.840.32768 */
    char fits[] = { 6, 6, 0x2a, '\x86', 0x48, '\x81', '\xff', 0x7f };
    char too_big[] = { 6, 6, 0x2a, '\x86', 0x48, '\x82', '\x80', 0 };
    Odr_oid negative[] = { 1, 2, 840, -32768, -1 };
    Odr_oid tmp[OID_SIZE];
    Odr_oid *oid = 0;
    int len;
    char *buf;

    odr_setbuf(decode, fits, sizeof(fits), 0);
    YAZ_CHECK(odr_oid(decode, &oid, 0, 0));
    YAZ_CHECK(oid && oid_oidlen(oid) == 4 && oid[3] == 32767);
    YAZ_CHECK(odr_oid(encode, &oid, 0, 0));
    buf = odr_getbuf(encode, &len, 0);
    YAZ_CHECK_EQ(len, sizeof(fits));
    YAZ_CHECK(memcmp(buf, fits, sizeof(fits)) == 0);

    /* An arc that does not fit in Odr_oid must not be stored truncated. */
    odr_reset(decode);
    odr_setbuf(decode, too_big, sizeof(too_big), 0);
    YAZ_CHECK(!odr_oid(decode, &oid, 0, 0));
    YAZ_CHECK_EQ(odr_geterror(decode), ODATA);

    odr_reset(encode);
    oid = negative;
    YAZ_CHECK(!odr_oid(encode, &oid, 0, 0));
    YAZ_CHECK_EQ(odr_geterror(encode), ODATA);

    YAZ_CHECK_EQ(oid_dotstring_to_oid("1.2.840.32767", tmp), 0);
    YAZ_CHECK_EQ(tmp[3], 32767);
    YAZ_CHECK_EQ(oid_dotstring_to_oid("1.2.840.32768", tmp), -1);
    YAZ_CHECK_EQ(oid_dotstring_to_oid("1.2.840.4294967297", tmp), -1);

    odr_destroy(encode);
    odr_destroy(decode);
}

static int file_has(const char *fname, const char *str)
{
    char buf[1024];
    size_t sz = 0;
    FILE *f = fopen(fname, "r");

    if (f)
    {
        sz = fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
    }
    buf[sz] = '\0';
    return strstr(buf, str) ? 1 : 0;
}

static void tst_odr_setprint_log(void)
{
    const char *fname = "test_odr_print.log";
    const char *fname0 = "test_odr_print.log.0";
    ODR print = odr_createmem(ODR_PRINT);

    remove(fname);
    remove(fname0);
    yaz_log_init_file(fname);
    yaz_log_init_max_size(1);
    odr_setprint_log(print);
    odr_printf(print, "before rotate\n");
    /* Log is beyond max size, so this rotates it and closes the old FILE. */
    yaz_log(YLOG_LOG, "rotate");
    odr_printf(print, "after rotate\n");
    yaz_log_init_max_size(0);
    yaz_log_init_file("");
    odr_destroy(print);

    YAZ_CHECK(file_has(fname0, "before rotate\n"));
    YAZ_CHECK(file_has(fname, "after rotate\n"));
    remove(fname);
    remove(fname0);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst();
    tst_ber_length();
    tst_indefinite_length();
    tst_odr_write2();
    tst_odr_create_Odr_oct2();
    tst_external_no_encoding();
    tst_ber_tag_position();
    tst_charneg_response();
    tst_oid_arc_range();
    tst_odr_setprint_log();
    tst_constructed_string();
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

