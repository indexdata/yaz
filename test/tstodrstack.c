#include <stdlib.h>
#include <yaz/pquery.h>
#include <yaz/proto.h>

/** \brief build a 100 level query */
void test1()
{
    ODR odr = odr_createmem(ODR_ENCODE);
    YAZ_PQF_Parser parser = yaz_pqf_create();
    Z_RPNQuery *rpn_query;
    char qstr[10000];
    int i;

    *qstr = '\0';
    for (i = 0; i<100; i++)
        strcat(qstr, "@and 1 ");
    strcat(qstr, "1");

    rpn_query = yaz_pqf_parse (parser, odr, qstr);

    if (!rpn_query)
        exit(1);

    if (!z_RPNQuery(odr, &rpn_query, 0, 0))
    {
        odr_perror(odr, "Encoding query");
        exit(1);
    }

    yaz_pqf_destroy(parser);
    odr_destroy(odr);
}

/** \brief build a circular referenced query */
void test2()
{
    ODR odr = odr_createmem(ODR_ENCODE);
    YAZ_PQF_Parser parser = yaz_pqf_create();
    Z_RPNQuery *rpn_query;

    rpn_query = yaz_pqf_parse (parser, odr, "@and @and a b c");

    if (!rpn_query)
        exit(1);

    /* make a bad recursive refernce */
    rpn_query->RPNStructure->u.complex->s1 = rpn_query->RPNStructure;

    if (!z_RPNQuery(odr, &rpn_query, 0, 0))
        odr_perror(odr, "Encoding query");

    yaz_pqf_destroy(parser);
    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    test1();
    test2();
    exit(0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
