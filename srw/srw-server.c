
#include <yaz/srw-util.h>
#include <yaz/xmalloc.h>

struct srw_info {
    int (*sr_h)(void *userinfo,
                struct soap * soap,
                xsd__string  *query,
                struct xcql__operandType *xQuery,	
                xsd__string *sortKeys,
                struct xsort__xSortKeysType *xSortKeys,
                xsd__integer *startRecord,
                xsd__integer *maximumRecords,
                xsd__string *recordSchema,
                xsd__string *recordPacking,
                struct zs__searchRetrieveResponse *res);
    int (*e_h)(void *userinfo,
              struct soap *soap,
              struct zs__explainResponse *explainResponse);
    void *userinfo;
};

int zs__explainRequest (struct soap *soap,
			struct zs__explainResponse *explainResponse)
{
    struct srw_info *info = (struct srw_info *) soap->user;
    return (*info->e_h)(info->userinfo, soap, explainResponse);
}

int zs__searchRetrieveRequest(struct soap * soap,
                              xsd__string  *query,
			      struct xcql__operandType *xQuery,	
                              xsd__string *sortKeys,
			      struct xsort__xSortKeysType *xSortKeys,
                              xsd__integer *startRecord,
                              xsd__integer *maximumRecords,
                              xsd__string *recordSchema,
                              xsd__string *recordPacking,
                              struct zs__searchRetrieveResponse *res)
{
    struct srw_info *info = (struct srw_info *) soap->user;
    return (*info->sr_h)(info->userinfo, soap,
                         query, xQuery, sortKeys, xSortKeys,
                         startRecord, maximumRecords,
                         recordSchema, recordPacking,
                         res);
}

void yaz_srw_serve (struct soap *soap,
                    void *userinfo,
                    int (*sr_h)(void *userinfo,
                                struct soap * soap,
                                xsd__string  *query,
                                struct xcql__operandType *xQuery,	
                                xsd__string *sortKeys,
                                struct xsort__xSortKeysType *xSortKeys,
                                xsd__integer *startRecord,
                                xsd__integer *maximumRecords,
                                xsd__string *recordSchema,
                                xsd__string *recordPacking,
                                struct zs__searchRetrieveResponse *res),
                    int (*e_h)(void *userinfo,
                                struct soap *soap,
                               struct zs__explainResponse *explainResponse))
{
    struct srw_info info;

    info.sr_h = sr_h;
    info.e_h = e_h;
    info.userinfo = userinfo;
    soap->user = &info;
    soap->namespaces = srw_namespaces;
    soap_serve(soap);
}
