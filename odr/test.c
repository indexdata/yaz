#include <stdio.h>

#include <odr.h>

int odr_dummy(ODR o, int **p, int opt)
{
    return odr_implicit(o, odr_integer, p, ODR_PRIVATE, 10, opt);
}

struct dummy
{
    int *alfa;
    int *beta;
};

int odr_dummy2(ODR o, struct dummy **p, int opt)
{
    struct dummy *pp;

    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt;
    pp = *p;
    return
    	odr_implicit(o, odr_integer, &pp->alfa, ODR_CONTEXT, 1, 1) &&
    	odr_implicit(o, odr_integer, &pp->beta, ODR_CONTEXT, 2, 1) &&
    	odr_sequence_end(o);
}

int main()
{
    int i;
    unsigned char buf[1024];
    struct odr o;
    int test=-99999;
    int *tp = &test, *tp2;
    ODR_OCT bbb, *bbb1, *bbb2;
    ODR_OCT ccc, *ccc1;
    char *str1 = "FOO", *str2 = "BAR";

    o.buf = buf;
    o.bp=o.buf;
    o.left = o.buflen = 1024;
    o.direction = ODR_ENCODE;
    o.t_class = -1;

    bbb.buf = (unsigned char *) str1;
    bbb.len = bbb.size = strlen(str1);
    bbb1 = &bbb;

    ccc.buf = (unsigned char*) str2;
    ccc.len = ccc.size = strlen(str2);
    ccc1 = &ccc;

    odr_constructed_begin(&o, &bbb1, ODR_UNIVERSAL, ODR_OCTETSTRING, 0);
    odr_octetstring(&o, &bbb1, 0);
    odr_octetstring(&o, &ccc1, 0);
    odr_constructed_end(&o);

    o.direction = ODR_DECODE;
    o.bp = o.buf;

    odr_octetstring(&o, &bbb2, 0);
}    
