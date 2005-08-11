#include <stdlib.h>
#include <yaz/pquery.h>
#include <yaz/proto.h>

void test1()
{
    ODR odr = odr_createmem(ODR_ENCODE);
    YAZ_PQF_Parser parser = yaz_pqf_create();
    Z_RPNQuery *rpn_query;

    rpn_query = yaz_pqf_parse (parser, odr, "@and @and a b c");

    if (!rpn_query)
        exit(1);
    rpn_query->RPNStructure->u.complex->s1 = rpn_query->RPNStructure;

    z_RPNQuery(odr, &rpn_query, 0, 0);

    yaz_pqf_destroy(parser);
    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    test1();
    exit(0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
