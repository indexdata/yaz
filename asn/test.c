#include <stdio.h>
#include <yaz/odr.h>
#include <yaz/proto.h>

int main()
{
    int i;
    unsigned char buf[10000];
    struct odr o;
    Z_APDU apdu, *papdu, *papdu2;
    Z_SearchRequest sreq;
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    bool_t rep;
    static char *names[] = {"Erik", "William", "George", "Bob"};
    Z_Query query;
    Z_RPNQuery rpnquery;
    Odr_oid att[] = { 1, 2, 3, 4, 5, 6, -1};
    Z_RPNStructure rpnstructure;
    Z_Operand operand;
    Z_AttributesPlusTerm apt;
    Odr_oct term;

    papdu = &apdu;
    apdu.which = Z_APDU_searchRequest;
    apdu.u.searchRequest = &sreq;
    a1 = 1000; sreq.smallSetUpperBound = &a1;
    a2 = 2000; sreq.largeSetLowerBound = &a2;
    a3 = 100; sreq.mediumSetPresentNumber = &a3;
    rep = 1; sreq.replaceIndicator = &rep;
    sreq.resultSetName = "FOOBAR";
    sreq.num_databaseNames = 4;
    sreq.databaseNames = names;
    sreq.smallSetElementSetNames = 0;
    sreq.mediumSetElementSetNames = 0;
    sreq.preferredRecordSyntax = 0;
    query.which = Z_Query_type_1;
    query.u.type_1 = &rpnquery;
    sreq.query = &query;
    rpnquery.attributeSetId = att;
    rpnquery.RPNStructure = &rpnstructure;
    rpnstructure.which = Z_RPNStructure_simple;
    rpnstructure.u.simple = &operand;
    operand.which = Z_Operand_APT;
    operand.u.attributesPlusTerm = &apt;
    apt.num_attributes=0;
    apt.attributeList = 0;
    apt.term = &term;
    term.buf = (unsigned char*) "BARFOO";
    term.len = term.size = strlen((char*)term.buf);

    o.buf = buf;
    o.bp=o.buf;
    o.left = o.buflen = 10000;
    o.direction = ODR_PRINT;
    o.print = stdout;
    o.indent = 0;
    o.t_class = -1;

    printf("status=%d\n", z_APDU(&o, &papdu, 0));

    return 0;

    o.direction = ODR_DECODE;
    o.bp = o.buf;

    z_APDU(&o, &papdu2, 0);
}    
