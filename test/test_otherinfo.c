/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/test.h>
#include <yaz/otherinfo.h>
#include <yaz/charneg.h>
#include <yaz/oid_db.h>

static Z_InitRequest *decode_init(ODR odr, const char *buf, int len)
{
    Z_APDU *apdu = 0;
    odr_setbuf(odr, (char *) buf, len, 0);
    YAZ_CHECK(z_APDU(odr, &apdu, 0, 0));
    if (!apdu || apdu->which != Z_APDU_initRequest)
        return 0;
    return apdu->u.initRequest;
}

/* category with categoryValue, but without the optional categoryTypeId */
static void tst_category_without_oid(void)
{
    static const char buf[] =
        "\xb4\x20"
        "\x83\x02\x05\xe0"
        "\x84\x02\x06\xc0"
        "\x85\x03\x10\x00\x00"
        "\x86\x03\x10\x00\x00"
        "\xbf\x81\x49\x0a"
        "\x30\x08"
        "\xa1\x03\x82\x01\x01"
        "\x82\x01\x78";
    ODR odr = odr_createmem(ODR_DECODE);
    Z_InitRequest *req = decode_init(odr, buf, sizeof(buf) - 1);

    YAZ_CHECK(req && req->otherInfo);
    if (req && req->otherInfo)
    {
        const char *v;
        YAZ_CHECK_EQ(req->otherInfo->num_elements, 1);
        YAZ_CHECK(req->otherInfo->list[0]->category);
        YAZ_CHECK(!req->otherInfo->list[0]->category->categoryTypeId);

        YAZ_CHECK(!yaz_oi_get_string_oid(&req->otherInfo,
                                         yaz_oid_userinfo_proxy, 1, 0));
        YAZ_CHECK(!yaz_oi_get_string_oid(&req->otherInfo,
                                         yaz_oid_userinfo_proxy, 1, 1));
        YAZ_CHECK_EQ(req->otherInfo->num_elements, 1);

        /* unit is passed over; a new one is added for the OID */
        yaz_oi_set_string_oid(&req->otherInfo, odr,
                              yaz_oid_userinfo_proxy, 1, "host");
        YAZ_CHECK_EQ(req->otherInfo->num_elements, 2);
        v = yaz_oi_get_string_oid(&req->otherInfo,
                                  yaz_oid_userinfo_proxy, 1, 0);
        YAZ_CHECK(v && !strcmp(v, "host"));
        YAZ_CHECK(!yaz_oi_get_string_oid(&req->otherInfo,
                                         yaz_oid_userinfo_client_ip, 1, 0));
    }
    odr_destroy(odr);
}

/* externallyDefinedInfo without the optional direct-reference */
static void tst_external_without_oid(void)
{
    static const char buf[] =
        "\xb4\x1d"
        "\x83\x02\x05\xe0"
        "\x84\x02\x06\xc0"
        "\x85\x03\x10\x00\x00"
        "\x86\x03\x10\x00\x00"
        "\xbf\x81\x49\x07"
        "\x30\x05"
        "\xa4\x03\x81\x01\x78";
    ODR odr = odr_createmem(ODR_DECODE);
    Z_InitRequest *req = decode_init(odr, buf, sizeof(buf) - 1);

    YAZ_CHECK(req && req->otherInfo);
    if (req && req->otherInfo)
    {
        Z_OtherInformationUnit *unit = req->otherInfo->list[0];
        YAZ_CHECK_EQ(req->otherInfo->num_elements, 1);
        YAZ_CHECK_EQ(unit->which, Z_OtherInfo_externallyDefinedInfo);
        YAZ_CHECK(!unit->information.externallyDefinedInfo->direct_reference);

        YAZ_CHECK(!yaz_get_charneg_record(req->otherInfo));
        YAZ_CHECK_EQ(yaz_del_charneg_record(&req->otherInfo), 0);
        YAZ_CHECK(req->otherInfo && req->otherInfo->num_elements == 1);
    }
    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_category_without_oid();
    tst_external_without_oid();
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
