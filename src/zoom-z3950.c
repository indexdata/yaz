/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-z3950.c
 * \brief Implements ZOOM Z39.50 handling
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include "zoom-p.h"

#include <yaz/yaz-util.h>
#include <yaz/xmalloc.h>
#include <yaz/otherinfo.h>
#include <yaz/log.h>
#include <yaz/pquery.h>
#include <yaz/marcdisp.h>
#include <yaz/diagbib1.h>
#include <yaz/charneg.h>
#include <yaz/ill.h>
#include <yaz/srw.h>
#include <yaz/cql.h>
#include <yaz/ccl.h>
#include <yaz/query-charset.h>
#include <yaz/copy_types.h>
#include <yaz/snprintf.h>
#include <yaz/facet.h>

#include <yaz/shptr.h>

/*
 * This wrapper is just for logging failed lookups.  It would be nicer
 * if it could cause failure when a lookup fails, but that's hard.
 */
static Odr_oid *zoom_yaz_str_to_z3950oid(ZOOM_connection c,
                                     oid_class oid_class, const char *str) {
    Odr_oid *res = yaz_string_to_oid_odr(yaz_oid_std(), oid_class, str,
                                     c->odr_out);
    if (res == 0)
        yaz_log(YLOG_WARN, "%p OID lookup (%d, '%s') failed",
                c, (int) oid_class, str);
    return res;
}

static Z_APDU *create_es_package(ZOOM_package p, const Odr_oid *oid)
{
    const char *str;
    Z_APDU *apdu = zget_APDU(p->odr_out, Z_APDU_extendedServicesRequest);
    Z_ExtendedServicesRequest *req = apdu->u.extendedServicesRequest;
    
    str = ZOOM_options_get(p->options, "package-name");
    if (str && *str)
        req->packageName = odr_strdup(p->odr_out, str);
    
    str = ZOOM_options_get(p->options, "user-id");
    if (str)
        req->userId = odr_strdup_null(p->odr_out, str);
    
    req->packageType = odr_oiddup(p->odr_out, oid);

    str = ZOOM_options_get(p->options, "function");
    if (str)
    {
        if (!strcmp (str, "create"))
            *req->function = Z_ExtendedServicesRequest_create;
        if (!strcmp (str, "delete"))
            *req->function = Z_ExtendedServicesRequest_delete;
        if (!strcmp (str, "modify"))
            *req->function = Z_ExtendedServicesRequest_modify;
    }

    str = ZOOM_options_get(p->options, "waitAction");
    if (str)
    {
        if (!strcmp (str, "wait"))
            *req->waitAction = Z_ExtendedServicesRequest_wait;
        if (!strcmp (str, "waitIfPossible"))
            *req->waitAction = Z_ExtendedServicesRequest_waitIfPossible;
        if (!strcmp (str, "dontWait"))
            *req->waitAction = Z_ExtendedServicesRequest_dontWait;
        if (!strcmp (str, "dontReturnPackage"))
            *req->waitAction = Z_ExtendedServicesRequest_dontReturnPackage;
    }
    return apdu;
}

static const char *ill_array_lookup(void *clientData, const char *idx)
{
    ZOOM_package p = (ZOOM_package) clientData;
    return ZOOM_options_get(p->options, idx+4);
}

static Z_External *encode_ill_request(ZOOM_package p)
{
    ODR out = p->odr_out;
    ILL_Request *req;
    Z_External *r = 0;
    struct ill_get_ctl ctl;
        
    ctl.odr = p->odr_out;
    ctl.clientData = p;
    ctl.f = ill_array_lookup;
        
    req = ill_get_ILLRequest(&ctl, "ill", 0);
        
    if (!ill_Request(out, &req, 0, 0))
    {
        int ill_request_size;
        char *ill_request_buf = odr_getbuf(out, &ill_request_size, 0);
        if (ill_request_buf)
            odr_setbuf(out, ill_request_buf, ill_request_size, 1);
        return 0;
    }
    else
    {
        int illRequest_size = 0;
        char *illRequest_buf = odr_getbuf(out, &illRequest_size, 0);
                
        r = (Z_External *) odr_malloc(out, sizeof(*r));
        r->direct_reference = odr_oiddup(out, yaz_oid_general_isoill_1);
        r->indirect_reference = 0;
        r->descriptor = 0;
        r->which = Z_External_single;
                
        r->u.single_ASN1_type =
            odr_create_Odr_oct(out,
                               (unsigned char *)illRequest_buf,
                               illRequest_size);
    }
    return r;
}

static Z_ItemOrder *encode_item_order(ZOOM_package p)
{
    Z_ItemOrder *req = (Z_ItemOrder *) odr_malloc(p->odr_out, sizeof(*req));
    const char *str;
    int len;
    
    req->which = Z_IOItemOrder_esRequest;
    req->u.esRequest = (Z_IORequest *) 
        odr_malloc(p->odr_out,sizeof(Z_IORequest));

    /* to keep part ... */
    req->u.esRequest->toKeep = (Z_IOOriginPartToKeep *)
        odr_malloc(p->odr_out,sizeof(Z_IOOriginPartToKeep));
    req->u.esRequest->toKeep->supplDescription = 0;
    req->u.esRequest->toKeep->contact = (Z_IOContact *)
        odr_malloc(p->odr_out, sizeof(*req->u.esRequest->toKeep->contact));
        
    str = ZOOM_options_get(p->options, "contact-name");
    req->u.esRequest->toKeep->contact->name =
        odr_strdup_null(p->odr_out, str);
        
    str = ZOOM_options_get(p->options, "contact-phone");
    req->u.esRequest->toKeep->contact->phone =
        odr_strdup_null(p->odr_out, str);
        
    str = ZOOM_options_get(p->options, "contact-email");
    req->u.esRequest->toKeep->contact->email =
        odr_strdup_null(p->odr_out, str);
        
    req->u.esRequest->toKeep->addlBilling = 0;
        
    /* not to keep part ... */
    req->u.esRequest->notToKeep = (Z_IOOriginPartNotToKeep *)
        odr_malloc(p->odr_out,sizeof(Z_IOOriginPartNotToKeep));
        
    str = ZOOM_options_get(p->options, "itemorder-setname");
    if (!str)
        str = "default";

    if (!*str) 
        req->u.esRequest->notToKeep->resultSetItem = 0;
    else
    {
        req->u.esRequest->notToKeep->resultSetItem = (Z_IOResultSetItem *)
            odr_malloc(p->odr_out, sizeof(Z_IOResultSetItem));

        req->u.esRequest->notToKeep->resultSetItem->resultSetId =
            odr_strdup(p->odr_out, str);
        req->u.esRequest->notToKeep->resultSetItem->item =
            odr_intdup(p->odr_out, 0);
        
        str = ZOOM_options_get(p->options, "itemorder-item");
        *req->u.esRequest->notToKeep->resultSetItem->item =
            (str ? atoi(str) : 1);
    }

    str = ZOOM_options_getl(p->options, "doc", &len);
    if (str)
    {
        req->u.esRequest->notToKeep->itemRequest =
            z_ext_record_xml(p->odr_out, str, len);
    }
    else
        req->u.esRequest->notToKeep->itemRequest = encode_ill_request(p);
    
    return req;
}

Z_APDU *create_admin_package(ZOOM_package p, int type, 
                             Z_ESAdminOriginPartToKeep **toKeepP,
                             Z_ESAdminOriginPartNotToKeep **notToKeepP)
{
    Z_APDU *apdu = create_es_package(p, yaz_oid_extserv_admin);
    if (apdu)
    {
        Z_ESAdminOriginPartToKeep  *toKeep;
        Z_ESAdminOriginPartNotToKeep  *notToKeep;
        Z_External *r = (Z_External *) odr_malloc(p->odr_out, sizeof(*r));
        const char *first_db = "Default";
        int num_db;
        char **db = ZOOM_connection_get_databases(p->connection,
                                                  p->options, &num_db,
                                                  p->odr_out);
        if (num_db > 0)
            first_db = db[0];
            
        r->direct_reference = odr_oiddup(p->odr_out, yaz_oid_extserv_admin);
        r->descriptor = 0;
        r->indirect_reference = 0;
        r->which = Z_External_ESAdmin;
        
        r->u.adminService = (Z_Admin *)
            odr_malloc(p->odr_out, sizeof(*r->u.adminService));
        r->u.adminService->which = Z_Admin_esRequest;
        r->u.adminService->u.esRequest = (Z_AdminEsRequest *)
            odr_malloc(p->odr_out, sizeof(*r->u.adminService->u.esRequest));
        
        toKeep = r->u.adminService->u.esRequest->toKeep =
            (Z_ESAdminOriginPartToKeep *) 
            odr_malloc(p->odr_out, sizeof(*r->u.adminService->u.esRequest->toKeep));
        toKeep->which = type;
        toKeep->databaseName = odr_strdup(p->odr_out, first_db);
        toKeep->u.create = odr_nullval();
        apdu->u.extendedServicesRequest->taskSpecificParameters = r;
        
        r->u.adminService->u.esRequest->notToKeep = notToKeep =
            (Z_ESAdminOriginPartNotToKeep *)
            odr_malloc(p->odr_out,
                       sizeof(*r->u.adminService->u.esRequest->notToKeep));
        notToKeep->which = Z_ESAdminOriginPartNotToKeep_recordsWillFollow;
        notToKeep->u.recordsWillFollow = odr_nullval();
        if (toKeepP)
            *toKeepP = toKeep;
        if (notToKeepP)
            *notToKeepP = notToKeep;
    }
    return apdu;
}

static Z_APDU *create_xmlupdate_package(ZOOM_package p)
{
    Z_APDU *apdu = create_es_package(p, yaz_oid_extserv_xml_es);
    Z_ExtendedServicesRequest *req = apdu->u.extendedServicesRequest;
    Z_External *ext = (Z_External *) odr_malloc(p->odr_out, sizeof(*ext));
    int len;
    const char *doc = ZOOM_options_getl(p->options, "doc", &len);

    if (!doc)
    {
        doc = "";
        len = 0;
    }

    req->taskSpecificParameters = ext;
    ext->direct_reference = req->packageType;
    ext->descriptor = 0;
    ext->indirect_reference = 0;
    
    ext->which = Z_External_octet;
    ext->u.single_ASN1_type =
        odr_create_Odr_oct(p->odr_out, (const unsigned char *) doc, len);
    return apdu;
}

static Z_APDU *create_update_package(ZOOM_package p)
{
    Z_APDU *apdu = 0;
    const char *first_db = "Default";
    int num_db;
    char **db = ZOOM_connection_get_databases(p->connection, p->options,
                                              &num_db, p->odr_out);
    const char *action = ZOOM_options_get(p->options, "action");
    int recordIdOpaque_len;
    const char *recordIdOpaque = ZOOM_options_getl(p->options, "recordIdOpaque",
        &recordIdOpaque_len);
    const char *recordIdNumber = ZOOM_options_get(p->options, "recordIdNumber");
    int record_len;
    const char *record_buf = ZOOM_options_getl(p->options, "record",
        &record_len);
    int recordOpaque_len;
    const char *recordOpaque_buf = ZOOM_options_getl(p->options, "recordOpaque",
        &recordOpaque_len);
    const char *syntax_str = ZOOM_options_get(p->options, "syntax");
    const char *version = ZOOM_options_get(p->options, "updateVersion");

    const char *correlationInfo_note =
        ZOOM_options_get(p->options, "correlationInfo.note");
    const char *correlationInfo_id =
        ZOOM_options_get(p->options, "correlationInfo.id");
    int action_no = -1;
    Odr_oid *syntax_oid = 0;
    const Odr_oid *package_oid = yaz_oid_extserv_database_update;

    if (!version)
        version = "3";
    if (!syntax_str)
        syntax_str = "xml";
    if (!record_buf && !recordOpaque_buf)
    {
        record_buf = "void";
        record_len = 4;
        syntax_str = "SUTRS";
    }

    if (syntax_str)
    {
        syntax_oid = yaz_string_to_oid_odr(yaz_oid_std(),
                                           CLASS_RECSYN, syntax_str,
                                           p->odr_out);
    }
    if (!syntax_oid)
        return 0;

    if (num_db > 0)
        first_db = db[0];
    
    switch(*version)
    {
    case '1':
        package_oid = yaz_oid_extserv_database_update_first_version;
        /* old update does not support specialUpdate */
        if (!action)
            action = "recordInsert";
        break;
    case '2':
        if (!action)
            action = "specialUpdate";
        package_oid = yaz_oid_extserv_database_update_second_version;
        break;
    case '3':
        if (!action)
            action = "specialUpdate";
        package_oid = yaz_oid_extserv_database_update;
        break;
    default:
        return 0;
    }
    
    if (!strcmp(action, "recordInsert"))
        action_no = Z_IUOriginPartToKeep_recordInsert;
    else if (!strcmp(action, "recordReplace"))
        action_no = Z_IUOriginPartToKeep_recordReplace;
    else if (!strcmp(action, "recordDelete"))
        action_no = Z_IUOriginPartToKeep_recordDelete;
    else if (!strcmp(action, "elementUpdate"))
        action_no = Z_IUOriginPartToKeep_elementUpdate;
    else if (!strcmp(action, "specialUpdate"))
        action_no = Z_IUOriginPartToKeep_specialUpdate;
    else
        return 0;

    apdu = create_es_package(p, package_oid);
    if (apdu)
    {
        Z_IUOriginPartToKeep *toKeep;
        Z_IUSuppliedRecords *notToKeep;
        Z_External *r = (Z_External *)
            odr_malloc(p->odr_out, sizeof(*r));
        const char *elementSetName =
            ZOOM_options_get(p->options, "elementSetName");
        
        apdu->u.extendedServicesRequest->taskSpecificParameters = r;
        
        r->direct_reference = odr_oiddup(p->odr_out, package_oid);
        r->descriptor = 0;
        r->which = Z_External_update;
        r->indirect_reference = 0;
        r->u.update = (Z_IUUpdate *)
            odr_malloc(p->odr_out, sizeof(*r->u.update));
        
        r->u.update->which = Z_IUUpdate_esRequest;
        r->u.update->u.esRequest = (Z_IUUpdateEsRequest *)
            odr_malloc(p->odr_out, sizeof(*r->u.update->u.esRequest));
        toKeep = r->u.update->u.esRequest->toKeep = 
            (Z_IUOriginPartToKeep *)
            odr_malloc(p->odr_out, sizeof(*toKeep));
        
        toKeep->databaseName = odr_strdup(p->odr_out, first_db);
        toKeep->schema = 0;
        
        toKeep->elementSetName = odr_strdup_null(p->odr_out, elementSetName);
            
        toKeep->actionQualifier = 0;
        toKeep->action = odr_intdup(p->odr_out, action_no);
        
        notToKeep = r->u.update->u.esRequest->notToKeep = 
            (Z_IUSuppliedRecords *)
            odr_malloc(p->odr_out, sizeof(*notToKeep));
        notToKeep->num = 1;
        notToKeep->elements = (Z_IUSuppliedRecords_elem **)
            odr_malloc(p->odr_out, sizeof(*notToKeep->elements));
        notToKeep->elements[0] = (Z_IUSuppliedRecords_elem *)
            odr_malloc(p->odr_out, sizeof(**notToKeep->elements));
        notToKeep->elements[0]->which = Z_IUSuppliedRecords_elem_opaque;
        if (recordIdOpaque)
        {
            notToKeep->elements[0]->u.opaque = 
                odr_create_Odr_oct(p->odr_out,
                                   (const unsigned char *) recordIdOpaque,
                                   recordIdOpaque_len);
        }
        else if (recordIdNumber)
        {
            notToKeep->elements[0]->which = Z_IUSuppliedRecords_elem_number;
            
            notToKeep->elements[0]->u.number =
                odr_intdup(p->odr_out, atoi(recordIdNumber));
        }
        else
            notToKeep->elements[0]->u.opaque = 0;
        notToKeep->elements[0]->supplementalId = 0;
        if (correlationInfo_note || correlationInfo_id)
        {
            Z_IUCorrelationInfo *ci;
            ci = notToKeep->elements[0]->correlationInfo =
                (Z_IUCorrelationInfo *) odr_malloc(p->odr_out, sizeof(*ci));
            ci->note = odr_strdup_null(p->odr_out, correlationInfo_note);
            ci->id = correlationInfo_id ?
                odr_intdup(p->odr_out, atoi(correlationInfo_id)) : 0;
        }
        else
            notToKeep->elements[0]->correlationInfo = 0;
        if (recordOpaque_buf)
        {
            notToKeep->elements[0]->record =
                z_ext_record_oid_any(p->odr_out, syntax_oid,
                                 recordOpaque_buf, recordOpaque_len);
        }
        else
        {
            notToKeep->elements[0]->record =
                z_ext_record_oid(p->odr_out, syntax_oid,
                                 record_buf, record_len);
        }
    }
    if (0 && apdu)
    {
        ODR print = odr_createmem(ODR_PRINT);

        z_APDU(print, &apdu, 0, 0);
        odr_destroy(print);
    }
    return apdu;
}


static void otherInfo_attach(ZOOM_connection c, Z_APDU *a, ODR out)
{
    int i;
    for (i = 0; i<200; i++)
    {
        size_t len;
        Odr_oid *oid;
        Z_OtherInformation **oi;
        char buf[80];
        const char *val;
        const char *cp;

        sprintf(buf, "otherInfo%d", i);
        val = ZOOM_options_get(c->options, buf);
        if (!val)
            break;
        cp = strchr(val, ':');
        if (!cp)
            continue;
        len = cp - val;
        if (len >= sizeof(buf))
            len = sizeof(buf)-1;
        memcpy(buf, val, len);
        buf[len] = '\0';
        
        oid = yaz_string_to_oid_odr(yaz_oid_std(), CLASS_USERINFO,
                                    buf, out);
        if (!oid)
            continue;
        
        yaz_oi_APDU(a, &oi);
        yaz_oi_set_string_oid(oi, out, oid, 1, cp+1);
    }
}



static int encode_APDU(ZOOM_connection c, Z_APDU *a, ODR out)
{
    assert(a);
    if (c->cookie_out)
    {
        Z_OtherInformation **oi;
        yaz_oi_APDU(a, &oi);
        yaz_oi_set_string_oid(oi, out, yaz_oid_userinfo_cookie, 
                              1, c->cookie_out);
    }
    if (c->client_IP)
    {
        Z_OtherInformation **oi;
        yaz_oi_APDU(a, &oi);
        yaz_oi_set_string_oid(oi, out, yaz_oid_userinfo_client_ip, 
                              1, c->client_IP);
    }
    otherInfo_attach(c, a, out);
    if (!z_APDU(out, &a, 0, 0))
    {
        FILE *outf = fopen("/tmp/apdu.txt", "a");
        if (a && outf)
        {
            ODR odr_pr = odr_createmem(ODR_PRINT);
            fprintf(outf, "a=%p\n", a);
            odr_setprint(odr_pr, outf);
            z_APDU(odr_pr, &a, 0, 0);
            odr_destroy(odr_pr);
        }
        yaz_log(c->log_api, "%p encoding_APDU: encoding failed", c);
        ZOOM_set_error(c, ZOOM_ERROR_ENCODE, 0);
        odr_reset(out);
        return -1;
    }
    if (c->odr_print)
        z_APDU(c->odr_print, &a, 0, 0);
    yaz_log(c->log_details, "%p encoding_APDU encoding OK", c);
    return 0;
}

static zoom_ret send_APDU(ZOOM_connection c, Z_APDU *a)
{
    ZOOM_Event event;
    assert(a);
    if (encode_APDU(c, a, c->odr_out))
        return zoom_complete;
    yaz_log(c->log_details, "%p send APDU type=%d", c, a->which);
    c->buf_out = odr_getbuf(c->odr_out, &c->len_out, 0);
    event = ZOOM_Event_create(ZOOM_EVENT_SEND_APDU);
    ZOOM_connection_put_event(c, event);
    odr_reset(c->odr_out);
    return ZOOM_send_buf(c);
}

zoom_ret ZOOM_connection_Z3950_send_init(ZOOM_connection c)
{
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_initRequest);
    Z_InitRequest *ireq = apdu->u.initRequest;
    Z_IdAuthentication *auth = (Z_IdAuthentication *)
        odr_malloc(c->odr_out, sizeof(*auth));

    ODR_MASK_SET(ireq->options, Z_Options_search);
    ODR_MASK_SET(ireq->options, Z_Options_present);
    ODR_MASK_SET(ireq->options, Z_Options_scan);
    ODR_MASK_SET(ireq->options, Z_Options_sort);
    ODR_MASK_SET(ireq->options, Z_Options_extendedServices);
    ODR_MASK_SET(ireq->options, Z_Options_namedResultSets);
    
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_2);
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_3);
    
    ireq->implementationId =
        odr_prepend(c->odr_out,
                    ZOOM_options_get(c->options, "implementationId"),
                    ireq->implementationId);
    
    ireq->implementationName = 
        odr_prepend(c->odr_out,
                    ZOOM_options_get(c->options, "implementationName"),
                    odr_prepend(c->odr_out, "ZOOM-C",
                                ireq->implementationName));
    
    ireq->implementationVersion = 
        odr_prepend(c->odr_out,
                    ZOOM_options_get(c->options, "implementationVersion"),
                                ireq->implementationVersion);
    
    *ireq->maximumRecordSize = c->maximum_record_size;
    *ireq->preferredMessageSize = c->preferred_message_size;
    
    if (c->group || c->password)
    {
        Z_IdPass *pass = (Z_IdPass *) odr_malloc(c->odr_out, sizeof(*pass));
        pass->groupId = odr_strdup_null(c->odr_out, c->group);
        pass->userId = odr_strdup_null(c->odr_out, c->user);
        pass->password = odr_strdup_null(c->odr_out, c->password);
        auth->which = Z_IdAuthentication_idPass;
        auth->u.idPass = pass;
        ireq->idAuthentication = auth;
    }
    else if (c->user)
    {
        auth->which = Z_IdAuthentication_open;
        auth->u.open = odr_strdup(c->odr_out, c->user);
        ireq->idAuthentication = auth;
    }
    if (c->proxy)
    {
        yaz_oi_set_string_oid(&ireq->otherInfo, c->odr_out,
                              yaz_oid_userinfo_proxy, 1, c->host_port);
    }
    if (c->charset || c->lang)
    {
        Z_OtherInformation **oi;
        Z_OtherInformationUnit *oi_unit;
        
        yaz_oi_APDU(apdu, &oi);
        
        if ((oi_unit = yaz_oi_update(oi, c->odr_out, NULL, 0, 0)))
        {
            ODR_MASK_SET(ireq->options, Z_Options_negotiationModel);
            oi_unit->which = Z_OtherInfo_externallyDefinedInfo;
            oi_unit->information.externallyDefinedInfo =
                yaz_set_proposal_charneg_list(c->odr_out, " ",
                                              c->charset, c->lang, 1);
        }
    }
    assert(apdu);
    return send_APDU(c, apdu);
}

zoom_ret ZOOM_connection_Z3950_send_search(ZOOM_connection c)
{
    ZOOM_resultset r;
    int lslb, ssub, mspn;
    const char *syntax;
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_searchRequest);
    Z_SearchRequest *search_req = apdu->u.searchRequest;
    const char *elementSetName;
    const char *smallSetElementSetName;
    const char *mediumSetElementSetName;
    const char *facets;

    assert(c->tasks);
    assert(c->tasks->which == ZOOM_TASK_SEARCH);

    r = c->tasks->u.search.resultset;

    yaz_log(c->log_details, "%p ZOOM_connection_send_search set=%p", c, r);

    elementSetName =
        ZOOM_options_get(r->options, "elementSetName");
    smallSetElementSetName  =
        ZOOM_options_get(r->options, "smallSetElementSetName");
    mediumSetElementSetName =
        ZOOM_options_get(r->options, "mediumSetElementSetName");

    if (!smallSetElementSetName)
        smallSetElementSetName = elementSetName;

    if (!mediumSetElementSetName)
        mediumSetElementSetName = elementSetName;

    facets = ZOOM_options_get(r->options, "facets");
    if (facets) {
        Z_FacetList *facet_list = yaz_pqf_parse_facet_list(c->odr_out, facets);
        if (facet_list) {
            Z_OtherInformation **oi;
            yaz_oi_APDU(apdu, &oi);
            if (facet_list) {
                yaz_oi_set_facetlist(oi, c->odr_out, facet_list);
            }
        }
    }

    assert(r);
    assert(r->query);

    /* prepare query for the search request */
    search_req->query = r->query->z_query;
    if (!search_req->query)
    {
        ZOOM_set_error(c, ZOOM_ERROR_INVALID_QUERY, 0);
        return zoom_complete;
    }
    if (r->query->z_query->which == Z_Query_type_1 || 
        r->query->z_query->which == Z_Query_type_101)
    {
        const char *cp = ZOOM_options_get(r->options, "rpnCharset");
        if (cp)
        {
            yaz_iconv_t cd = yaz_iconv_open(cp, "UTF-8");
            if (cd)
            {
                int r;
                search_req->query = yaz_copy_Z_Query(search_req->query,
                                                     c->odr_out);
                
                r = yaz_query_charset_convert_rpnquery_check(
                    search_req->query->u.type_1,
                    c->odr_out, cd);
                yaz_iconv_close(cd);
                if (r)
                {  /* query could not be char converted */
                    ZOOM_set_error(c, ZOOM_ERROR_INVALID_QUERY, 0);
                    return zoom_complete;
                }
            }
        }
    }
    search_req->databaseNames = r->databaseNames;
    search_req->num_databaseNames = r->num_databaseNames;

    /* get syntax (no need to provide unless piggyback is in effect) */
    syntax = c->tasks->u.search.syntax;

    lslb = ZOOM_options_get_int(r->options, "largeSetLowerBound", -1);
    ssub = ZOOM_options_get_int(r->options, "smallSetUpperBound", -1);
    mspn = ZOOM_options_get_int(r->options, "mediumSetPresentNumber", -1);
    if (lslb != -1 && ssub != -1 && mspn != -1)
    {
        /* So're a Z39.50 expert? Let's hope you don't do sort */
        *search_req->largeSetLowerBound = lslb;
        *search_req->smallSetUpperBound = ssub;
        *search_req->mediumSetPresentNumber = mspn;
    }
    else if (c->tasks->u.search.start == 0 && c->tasks->u.search.count > 0
             && r->piggyback && !r->r_sort_spec && !r->schema)
    {
        /* Regular piggyback - do it unless we're going to do sort */
        *search_req->largeSetLowerBound = 2000000000;
        *search_req->smallSetUpperBound = 1;
        *search_req->mediumSetPresentNumber = 
            r->step>0 ? r->step : c->tasks->u.search.count;
    }
    else
    {
        /* non-piggyback. Need not provide elementsets or syntaxes .. */
        smallSetElementSetName = 0;
        mediumSetElementSetName = 0;
        syntax = 0;
    }
    if (smallSetElementSetName && *smallSetElementSetName)
    {
        Z_ElementSetNames *esn = (Z_ElementSetNames *)
            odr_malloc(c->odr_out, sizeof(*esn));
        
        esn->which = Z_ElementSetNames_generic;
        esn->u.generic = odr_strdup(c->odr_out, smallSetElementSetName);
        search_req->smallSetElementSetNames = esn;
    }
    if (mediumSetElementSetName && *mediumSetElementSetName)
    {
        Z_ElementSetNames *esn =(Z_ElementSetNames *)
            odr_malloc(c->odr_out, sizeof(*esn));
        
        esn->which = Z_ElementSetNames_generic;
        esn->u.generic = odr_strdup(c->odr_out, mediumSetElementSetName);
        search_req->mediumSetElementSetNames = esn;
    }
    if (syntax)
        search_req->preferredRecordSyntax =
            zoom_yaz_str_to_z3950oid(c, CLASS_RECSYN, syntax);
    
    if (!r->setname)
    {
        if (c->support_named_resultsets)
        {
            char setname[14];
            int ord;
            /* find the lowest unused ordinal so that we re-use
               result sets on the server. */
            for (ord = 1; ; ord++)
            {
#if ZOOM_RESULT_LISTS
                ZOOM_resultsets rsp;
                sprintf(setname, "%d", ord);
                for (rsp = c->resultsets; rsp; rsp = rsp->next)
                    if (rsp->resultset->setname && !strcmp(rsp->resultset->setname, setname))
                        break;
                if (!rsp)
                    break;
#else
                ZOOM_resultset rp;
                sprintf(setname, "%d", ord);
                for (rp = c->resultsets; rp; rp = rp->next)
                    if (rp->setname && !strcmp(rp->setname, setname))
                        break;
                if (!rp)
                    break;
#endif

            }
            r->setname = xstrdup(setname);
            yaz_log(c->log_details, "%p ZOOM_connection_send_search: "
                    "allocating set %s", c, r->setname);
        }
        else
        {
            yaz_log(c->log_details, "%p ZOOM_connection_send_search: using "
                    "default set", c);
            r->setname = xstrdup("default");
        }
        ZOOM_options_set(r->options, "setname", r->setname);
    }
    search_req->resultSetName = odr_strdup(c->odr_out, r->setname);
    return send_APDU(c, apdu);
}

zoom_ret ZOOM_connection_Z3950_send_scan(ZOOM_connection c)
{
    ZOOM_scanset scan;
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_scanRequest);
    Z_ScanRequest *req = apdu->u.scanRequest;

    yaz_log(c->log_details, "%p send_scan", c);
    if (!c->tasks)
        return zoom_complete;
    assert (c->tasks->which == ZOOM_TASK_SCAN);
    scan = c->tasks->u.scan.scan;

    /* Z39.50 scan can only carry RPN */
    if (scan->query->z_query->which == Z_Query_type_1 ||
        scan->query->z_query->which == Z_Query_type_101)
    {
        Z_RPNQuery *rpn = scan->query->z_query->u.type_1;
        const char *cp = ZOOM_options_get(scan->options, "rpnCharset");
        if (cp)
        {
            yaz_iconv_t cd = yaz_iconv_open(cp, "UTF-8");
            if (cd)
            {
                rpn = yaz_copy_z_RPNQuery(rpn, c->odr_out);

                yaz_query_charset_convert_rpnquery(
                    rpn, c->odr_out, cd);
                yaz_iconv_close(cd);
            }
        }
        req->attributeSet = rpn->attributeSetId;
        if (!req->attributeSet)
            req->attributeSet = odr_oiddup(c->odr_out, yaz_oid_attset_bib_1);
        if (rpn->RPNStructure->which == Z_RPNStructure_simple &&
            rpn->RPNStructure->u.simple->which == Z_Operand_APT)
        {
            req->termListAndStartPoint =
                rpn->RPNStructure->u.simple->u.attributesPlusTerm;
        }
        else
        {
            ZOOM_set_error(c, ZOOM_ERROR_INVALID_QUERY, 0);
            return zoom_complete;
        }
    }
    else
    {
        ZOOM_set_error(c, ZOOM_ERROR_UNSUPPORTED_QUERY, 0);
        return zoom_complete;
    }

    *req->numberOfTermsRequested =
        ZOOM_options_get_int(scan->options, "number", 20);

    req->preferredPositionInResponse =
        odr_intdup(c->odr_out,
                   ZOOM_options_get_int(scan->options, "position", 1));

    req->stepSize =
        odr_intdup(c->odr_out,
                   ZOOM_options_get_int(scan->options, "stepSize", 0));
    
    req->databaseNames = scan->databaseNames;
    req->num_databaseNames = scan->num_databaseNames;

    return send_APDU(c, apdu);
}

ZOOM_API(void)
    ZOOM_package_send(ZOOM_package p, const char *type)
{
    Z_APDU *apdu = 0;
    ZOOM_connection c;
    if (!p)
        return;
    c = p->connection;
    odr_reset(p->odr_out);
    xfree(p->buf_out);
    p->buf_out = 0;
    if (!strcmp(type, "itemorder"))
    {
        apdu = create_es_package(p, yaz_oid_extserv_item_order);
        if (apdu)
        {
            Z_External *r = (Z_External *) odr_malloc(p->odr_out, sizeof(*r));
            
            r->direct_reference = 
                odr_oiddup(p->odr_out, yaz_oid_extserv_item_order);
            r->descriptor = 0;
            r->which = Z_External_itemOrder;
            r->indirect_reference = 0;
            r->u.itemOrder = encode_item_order(p);

            apdu->u.extendedServicesRequest->taskSpecificParameters = r;
        }
    }
    else if (!strcmp(type, "create"))  /* create database */
    {
        apdu = create_admin_package(p, Z_ESAdminOriginPartToKeep_create,
                                    0, 0);
    }   
    else if (!strcmp(type, "drop"))  /* drop database */
    {
        apdu = create_admin_package(p, Z_ESAdminOriginPartToKeep_drop,
                                    0, 0);
    }
    else if (!strcmp(type, "commit"))  /* commit changes */
    {
        apdu = create_admin_package(p, Z_ESAdminOriginPartToKeep_commit,
                                    0, 0);
    }
    else if (!strcmp(type, "update")) /* update record(s) */
    {
        apdu = create_update_package(p);
    }
    else if (!strcmp(type, "xmlupdate"))
    {
        apdu = create_xmlupdate_package(p);
    }
    if (apdu)
    {
        if (encode_APDU(p->connection, apdu, p->odr_out) == 0)
        {
            char *buf;

            ZOOM_task task = ZOOM_connection_add_task(c, ZOOM_TASK_PACKAGE);
            task->u.package = p;
            buf = odr_getbuf(p->odr_out, &p->len_out, 0);
            p->buf_out = (char *) xmalloc(p->len_out);
            memcpy(p->buf_out, buf, p->len_out);
            
            (p->refcount)++;
            if (!c->async)
            {
                while (ZOOM_event(1, &c))
                    ;
            }
        }
    }
}


static void handle_Z3950_records(ZOOM_connection c, Z_Records *sr,
                                 int present_phase);

static void response_default_diag(ZOOM_connection c, Z_DefaultDiagFormat *r)
{
    char oid_name_buf[OID_STR_MAX];
    const char *oid_name;
    char *addinfo = 0;

    oid_name = yaz_oid_to_string_buf(r->diagnosticSetId, 0, oid_name_buf);
    switch (r->which)
    {
    case Z_DefaultDiagFormat_v2Addinfo:
        addinfo = r->u.v2Addinfo;
        break;
    case Z_DefaultDiagFormat_v3Addinfo:
        addinfo = r->u.v3Addinfo;
        break;
    }
    xfree(c->addinfo);
    c->addinfo = 0;
    ZOOM_set_dset_error(c, *r->condition, oid_name, addinfo, 0);
}

static void response_diag(ZOOM_connection c, Z_DiagRec *p)
{
    if (p->which != Z_DiagRec_defaultFormat)
        ZOOM_set_error(c, ZOOM_ERROR_DECODE, 0);
    else
        response_default_diag(c, p->u.defaultFormat);
}

static int es_response_taskpackage_update(ZOOM_connection c,
		Z_IUUpdateTaskPackage *utp)
{
	if (utp && utp->targetPart)
	{
		Z_IUTargetPart *targetPart = utp->targetPart;
		switch ( *targetPart->updateStatus ) {
			case Z_IUTargetPart_success:
				ZOOM_options_set(c->tasks->u.package->options,"updateStatus", "success");
				break;
			case Z_IUTargetPart_partial:
				ZOOM_options_set(c->tasks->u.package->options,"updateStatus", "partial");
				break;
			case Z_IUTargetPart_failure:
				ZOOM_options_set(c->tasks->u.package->options,"updateStatus", "failure");
				if (targetPart->globalDiagnostics && targetPart->num_globalDiagnostics > 0)
					response_diag(c, targetPart->globalDiagnostics[0]);
				break;
		}
		// NOTE: Individual record status, surrogate diagnostics, and supplemental diagnostics ARE NOT REPORTED.
	}
    return 1;
}

static int es_response_taskpackage(ZOOM_connection c,
                                   Z_TaskPackage *taskPackage)
{
	// targetReference
	Odr_oct *id = taskPackage->targetReference;
	if (id)
		ZOOM_options_setl(c->tasks->u.package->options,
							"targetReference", (char*) id->buf, id->len);
	
	// taskStatus
	switch ( *taskPackage->taskStatus ) {
		case Z_TaskPackage_pending:
			ZOOM_options_set(c->tasks->u.package->options,"taskStatus", "pending");
			break;
		case Z_TaskPackage_active:
			ZOOM_options_set(c->tasks->u.package->options,"taskStatus", "active");
			break;
		case Z_TaskPackage_complete:
			ZOOM_options_set(c->tasks->u.package->options,"taskStatus", "complete");
			break;
		case Z_TaskPackage_aborted:
			ZOOM_options_set(c->tasks->u.package->options,"taskStatus", "aborted");
			if ( taskPackage->num_packageDiagnostics && taskPackage->packageDiagnostics )
				response_diag(c, taskPackage->packageDiagnostics[0]);
			break;
	}
	
	// taskSpecificParameters
	// NOTE: Only Update implemented, no others.
	if ( taskPackage->taskSpecificParameters->which == Z_External_update ) {
			Z_IUUpdateTaskPackage *utp = taskPackage->taskSpecificParameters->u.update->u.taskPackage;
			es_response_taskpackage_update(c, utp);
	}
	return 1;
}


static int handle_Z3950_es_response(ZOOM_connection c,
                                    Z_ExtendedServicesResponse *res)
{
    if (!c->tasks || c->tasks->which != ZOOM_TASK_PACKAGE)
        return 0;
    switch (*res->operationStatus) {
        case Z_ExtendedServicesResponse_done:
            ZOOM_options_set(c->tasks->u.package->options,"operationStatus", "done");
            break;
        case Z_ExtendedServicesResponse_accepted:
            ZOOM_options_set(c->tasks->u.package->options,"operationStatus", "accepted");
            break;
        case Z_ExtendedServicesResponse_failure:
            ZOOM_options_set(c->tasks->u.package->options,"operationStatus", "failure");
            if (res->diagnostics && res->num_diagnostics > 0)
                response_diag(c, res->diagnostics[0]);
            break;
    }
    if (res->taskPackage &&
        res->taskPackage->which == Z_External_extendedService)
    {
        Z_TaskPackage *taskPackage = res->taskPackage->u.extendedService;
        es_response_taskpackage(c, taskPackage);
    }
    if (res->taskPackage && 
        res->taskPackage->which == Z_External_octet)
    {
        Odr_oct *doc = res->taskPackage->u.octet_aligned;
        ZOOM_options_setl(c->tasks->u.package->options,
                          "xmlUpdateDoc", (char*) doc->buf, doc->len);
    }
    return 1;
}

static void interpret_init_diag(ZOOM_connection c,
                                Z_DiagnosticFormat *diag)
{
    if (diag->num > 0)
    {
        Z_DiagnosticFormat_s *ds = diag->elements[0];
        if (ds->which == Z_DiagnosticFormat_s_defaultDiagRec)
            response_default_diag(c, ds->u.defaultDiagRec);
    }
}


static void interpret_otherinformation_field(ZOOM_connection c,
                                             Z_OtherInformation *ui)
{
    int i;
    for (i = 0; i < ui->num_elements; i++)
    {
        Z_OtherInformationUnit *unit = ui->list[i];
        if (unit->which == Z_OtherInfo_externallyDefinedInfo &&
            unit->information.externallyDefinedInfo &&
            unit->information.externallyDefinedInfo->which ==
            Z_External_diag1) 
        {
            interpret_init_diag(c, unit->information.externallyDefinedInfo->u.diag1);
        } 
    }
}

static char *get_term_cstr(ODR odr, Z_Term *term) {

    switch (term->which) {
    case Z_Term_general:
            return odr_strdupn(odr, (const char *) term->u.general->buf, (size_t) term->u.general->len);
        break;
    case Z_Term_characterString:
        return odr_strdup(odr, term->u.characterString);
    }
    return 0;
}

static ZOOM_facet_field get_zoom_facet_field(ODR odr, Z_FacetField *facet) {
    int term_index;
    struct yaz_facet_attr attr_values;
    ZOOM_facet_field facet_field = odr_malloc(odr, sizeof(*facet_field));
    yaz_facet_attr_init(&attr_values);
    yaz_facet_attr_get_z_attributes(facet->attributes, &attr_values);
    facet_field->facet_name = odr_strdup(odr, attr_values.useattr);
    facet_field->num_terms = facet->num_terms;
    yaz_log(YLOG_DEBUG, "ZOOM_facet_field %s %d terms %d", attr_values.useattr, attr_values.limit, facet->num_terms);
    facet_field->facet_terms = odr_malloc(odr, facet_field->num_terms * sizeof(*facet_field->facet_terms));
    for (term_index = 0 ; term_index < facet->num_terms; term_index++) {
        Z_FacetTerm *facetTerm = facet->terms[term_index];
        facet_field->facet_terms[term_index].frequency = *facetTerm->count;
        facet_field->facet_terms[term_index].term = get_term_cstr(odr, facetTerm->term);
        yaz_log(YLOG_DEBUG, "    term[%d] %s %d",
                term_index, facet_field->facet_terms[term_index].term, facet_field->facet_terms[term_index].frequency);
    }
    return facet_field;
}

static void handle_facet_result(ZOOM_connection c, ZOOM_resultset r,
                                Z_OtherInformation *o)
{
    int i;
    for (i = 0; o && i < o->num_elements; i++)
    {
        if (o->list[i]->which == Z_OtherInfo_externallyDefinedInfo)
        {
            Z_External *ext = o->list[i]->information.externallyDefinedInfo;
            if (ext->which == Z_External_userFacets)
            {
                int j;
                Z_FacetList *fl = ext->u.facetList;
                r->num_facets   = fl->num;
                yaz_log(YLOG_DEBUG, "Facets found: %d", fl->num);
                r->facets       =  odr_malloc(r->odr, r->num_facets * sizeof(*r->facets));
                r->facets_names =  odr_malloc(r->odr, r->num_facets * sizeof(*r->facets_names));
                for (j = 0; j < fl->num; j++)
                {
                    r->facets[j] = get_zoom_facet_field(r->odr, fl->elements[j]);
                    if (!r->facets[j])
                        yaz_log(YLOG_DEBUG, "Facet field missing on index %d !", j);
                    r->facets_names[j] = (char *) ZOOM_facet_field_name(r->facets[j]);
                }
            }
        }
    }
}

static void handle_queryExpressionTerm(ZOOM_options opt, const char *name,
                                       Z_Term *term)
{
    switch (term->which)
    {
    case Z_Term_general:
        ZOOM_options_setl(opt, name,
                          (const char *)(term->u.general->buf), 
                          term->u.general->len);
        break;
    case Z_Term_characterString:
        ZOOM_options_set(opt, name, term->u.characterString);
        break;
    case Z_Term_numeric:
        ZOOM_options_set_int(opt, name, *term->u.numeric);
        break;
    }
}

static void handle_queryExpression(ZOOM_options opt, const char *name,
                                   Z_QueryExpression *exp)
{
    char opt_name[80];
    
    switch (exp->which)
    {
    case Z_QueryExpression_term:
        if (exp->u.term && exp->u.term->queryTerm)
        {
            sprintf(opt_name, "%s.term", name);
            handle_queryExpressionTerm(opt, opt_name, exp->u.term->queryTerm);
        }
        break;
    case Z_QueryExpression_query:
        break;
    }
}


static void handle_search_result(ZOOM_connection c, ZOOM_resultset resultset,
                                Z_OtherInformation *o)
{
    int i;
    for (i = 0; o && i < o->num_elements; i++)
    {
        if (o->list[i]->which == Z_OtherInfo_externallyDefinedInfo)
        {
            Z_External *ext = o->list[i]->information.externallyDefinedInfo;
            
            if (ext->which == Z_External_searchResult1)
            {
                int j;
                Z_SearchInfoReport *sr = ext->u.searchResult1;
                
                if (sr->num)
                    ZOOM_options_set_int(
                        resultset->options, "searchresult.size", sr->num);

                for (j = 0; j < sr->num; j++)
                {
                    Z_SearchInfoReport_s *ent =
                        ext->u.searchResult1->elements[j];
                    char pref[80];
                    
                    sprintf(pref, "searchresult.%d", j);

                    if (ent->subqueryId)
                    {
                        char opt_name[80];
                        sprintf(opt_name, "%s.id", pref);
                        ZOOM_options_set(resultset->options, opt_name,
                                         ent->subqueryId);
                    }
                    if (ent->subqueryExpression)
                    {
                        char opt_name[80];
                        sprintf(opt_name, "%s.subquery", pref);
                        handle_queryExpression(resultset->options, opt_name,
                                               ent->subqueryExpression);
                    }
                    if (ent->subqueryInterpretation)
                    {
                        char opt_name[80];
                        sprintf(opt_name, "%s.interpretation", pref);
                        handle_queryExpression(resultset->options, opt_name,
                                               ent->subqueryInterpretation);
                    }
                    if (ent->subqueryRecommendation)
                    {
                        char opt_name[80];
                        sprintf(opt_name, "%s.recommendation", pref);
                        handle_queryExpression(resultset->options, opt_name,
                                               ent->subqueryRecommendation);
                    }
                    if (ent->subqueryCount)
                    {
                        char opt_name[80];
                        sprintf(opt_name, "%s.count", pref);
                        ZOOM_options_set_int(resultset->options, opt_name,
                                             *ent->subqueryCount);
                    }                                             
                }
            }
        }
    }
}

static void handle_Z3950_search_response(ZOOM_connection c,
                                         Z_SearchResponse *sr)
{
    ZOOM_resultset resultset;
    ZOOM_Event event;

    if (!c->tasks || c->tasks->which != ZOOM_TASK_SEARCH)
        return ;

    event = ZOOM_Event_create(ZOOM_EVENT_RECV_SEARCH);
    ZOOM_connection_put_event(c, event);

    resultset = c->tasks->u.search.resultset;

    if (sr->resultSetStatus)
    {
        ZOOM_options_set_int(resultset->options, "resultSetStatus",
                             *sr->resultSetStatus);
    }
    if (sr->presentStatus)
    {
        ZOOM_options_set_int(resultset->options, "presentStatus",
                             *sr->presentStatus);
    }
    handle_search_result(c, resultset, sr->additionalSearchInfo);

    handle_facet_result(c, resultset, sr->additionalSearchInfo);

    resultset->size = *sr->resultCount;
    handle_Z3950_records(c, sr->records, 0);
}

static void handle_Z3950_sort_response(ZOOM_connection c, Z_SortResponse *res)
{
    if (res->diagnostics && res->num_diagnostics > 0)
        response_diag(c, res->diagnostics[0]);
}

static int handle_Z3950_scan_response(ZOOM_connection c, Z_ScanResponse *res)
{
    NMEM nmem = odr_extract_mem(c->odr_in);
    ZOOM_scanset scan;

    if (!c->tasks || c->tasks->which != ZOOM_TASK_SCAN)
        return 0;
    scan = c->tasks->u.scan.scan;

    if (res->entries && res->entries->nonsurrogateDiagnostics)
        response_diag(c, res->entries->nonsurrogateDiagnostics[0]);
    scan->scan_response = res;
    scan->srw_scan_response = 0;
    nmem_transfer(odr_getmem(scan->odr), nmem);
    if (res->stepSize)
        ZOOM_options_set_int(scan->options, "stepSize", *res->stepSize);
    if (res->positionOfTerm)
        ZOOM_options_set_int(scan->options, "position", *res->positionOfTerm);
    if (res->scanStatus)
        ZOOM_options_set_int(scan->options, "scanStatus", *res->scanStatus);
    if (res->numberOfEntriesReturned)
        ZOOM_options_set_int(scan->options, "number",
                             *res->numberOfEntriesReturned);
    nmem_destroy(nmem);
    return 1;
}

static void handle_Z3950_records(ZOOM_connection c, Z_Records *sr,
                                 int present_phase)
{
    ZOOM_resultset resultset;
    int *start, *count;
    const char *syntax = 0, *elementSetName = 0;

    if (!c->tasks)
        return ;
    switch (c->tasks->which)
    {
    case ZOOM_TASK_SEARCH:
        resultset = c->tasks->u.search.resultset;
        start = &c->tasks->u.search.start;
        count = &c->tasks->u.search.count;
        syntax = c->tasks->u.search.syntax;
        elementSetName = c->tasks->u.search.elementSetName;
        break;
    case ZOOM_TASK_RETRIEVE:
        resultset = c->tasks->u.retrieve.resultset;        
        start = &c->tasks->u.retrieve.start;
        count = &c->tasks->u.retrieve.count;
        syntax = c->tasks->u.retrieve.syntax;
        elementSetName = c->tasks->u.retrieve.elementSetName;
        break;
    default:
        return;
    }
    if (sr && sr->which == Z_Records_NSD)
        response_default_diag(c, sr->u.nonSurrogateDiagnostic);
    else if (sr && sr->which == Z_Records_multipleNSD)
    {
        if (sr->u.multipleNonSurDiagnostics->num_diagRecs >= 1)
            response_diag(c, sr->u.multipleNonSurDiagnostics->diagRecs[0]);
        else
            ZOOM_set_error(c, ZOOM_ERROR_DECODE, 0);
    }
    else 
    {
        if (*count + *start > resultset->size)
            *count = resultset->size - *start;
        if (*count < 0)
            *count = 0;
        if (sr && sr->which == Z_Records_DBOSD)
        {
            int i;
            NMEM nmem = odr_extract_mem(c->odr_in);
            Z_NamePlusRecordList *p =
                sr->u.databaseOrSurDiagnostics;
            for (i = 0; i<p->num_records; i++)
            {
                ZOOM_record_cache_add(resultset, p->records[i], i + *start,
                                      syntax, elementSetName,
                                      elementSetName, 0);
            }
            *count -= i;
            if (*count < 0)
                *count = 0;
            *start += i;
            yaz_log(c->log_details, 
                    "handle_records resultset=%p start=%d count=%d",
                    resultset, *start, *count);

            /* transfer our response to search_nmem .. we need it later */
            nmem_transfer(odr_getmem(resultset->odr), nmem);
            nmem_destroy(nmem);
            if (present_phase && p->num_records == 0)
            {
                /* present response and we didn't get any records! */
                Z_NamePlusRecord *myrec = 
                    zget_surrogateDiagRec(
                        resultset->odr, 0, 
                        YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS,
                        "ZOOM C generated. Present phase and no records");
                ZOOM_record_cache_add(resultset, myrec, *start,
                                      syntax, elementSetName, 0, 0);
            }
        }
        else if (present_phase)
        {
            /* present response and we didn't get any records! */
            Z_NamePlusRecord *myrec = 
                zget_surrogateDiagRec(
                    resultset->odr, 0,
                    YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS,
                    "ZOOM C generated: Present response and no records");
            ZOOM_record_cache_add(resultset, myrec, *start,
                                  syntax, elementSetName, 0, 0);
        }
    }
}

static void handle_Z3950_present_response(ZOOM_connection c,
                                          Z_PresentResponse *pr)
{
    handle_Z3950_records(c, pr->records, 1);
}

static void set_init_option(const char *name, void *clientData)
{
    ZOOM_connection c = (ZOOM_connection) clientData;
    char buf[80];

    sprintf(buf, "init_opt_%.70s", name);
    ZOOM_connection_option_set(c, buf, "1");
}


zoom_ret send_Z3950_sort(ZOOM_connection c, ZOOM_resultset resultset)
{
    if (c->error)
        resultset->r_sort_spec = 0;
    if (resultset->r_sort_spec)
    {
        Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_sortRequest);
        Z_SortRequest *req = apdu->u.sortRequest;
        
        req->num_inputResultSetNames = 1;
        req->inputResultSetNames = (Z_InternationalString **)
            odr_malloc(c->odr_out, sizeof(*req->inputResultSetNames));
        req->inputResultSetNames[0] =
            odr_strdup(c->odr_out, resultset->setname);
        req->sortedResultSetName = odr_strdup(c->odr_out, resultset->setname);
        req->sortSequence = resultset->r_sort_spec;
        resultset->r_sort_spec = 0;
        return send_APDU(c, apdu);
    }
    return zoom_complete;
}

zoom_ret send_Z3950_present(ZOOM_connection c)
{
    Z_APDU *apdu = 0;
    Z_PresentRequest *req = 0;
    int i = 0;
    const char *syntax = 0;
    const char *elementSetName = 0;
    ZOOM_resultset  resultset;
    int *start, *count;

    if (!c->tasks)
    {
        yaz_log(c->log_details, "%p send_present no tasks", c);
        return zoom_complete;
    }
    
    switch (c->tasks->which)
    {
    case ZOOM_TASK_SEARCH:
        resultset = c->tasks->u.search.resultset;
        start = &c->tasks->u.search.start;
        count = &c->tasks->u.search.count;
        syntax = c->tasks->u.search.syntax;
        elementSetName = c->tasks->u.search.elementSetName;
        break;
    case ZOOM_TASK_RETRIEVE:
        resultset = c->tasks->u.retrieve.resultset;
        start = &c->tasks->u.retrieve.start;
        count = &c->tasks->u.retrieve.count;
        syntax = c->tasks->u.retrieve.syntax;
        elementSetName = c->tasks->u.retrieve.elementSetName;
        break;
    default:
        return zoom_complete;
    }
    yaz_log(c->log_details, "%p send_present start=%d count=%d",
            c, *start, *count);

    if (*start < 0 || *count < 0 || *start + *count > resultset->size)
    {
        ZOOM_set_dset_error(c, YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE, "Bib-1",
                       "", 0);
    }
    if (c->error)                  /* don't continue on error */
        return zoom_complete;
    yaz_log(c->log_details, "send_present resultset=%p start=%d count=%d",
            resultset, *start, *count);

    for (i = 0; i < *count; i++)
    {
        ZOOM_record rec =
            ZOOM_record_cache_lookup(resultset, i + *start,
                                     syntax, elementSetName);
        if (!rec)
            break;
        else
        {
            ZOOM_Event event = ZOOM_Event_create(ZOOM_EVENT_RECV_RECORD);
            ZOOM_connection_put_event(c, event);
        }
    }
    *start += i;
    *count -= i;

    if (*count == 0)
    {
        yaz_log(c->log_details, "%p send_present skip=%d no more to fetch", c, i);
        return zoom_complete;
    }

    apdu = zget_APDU(c->odr_out, Z_APDU_presentRequest);
    req = apdu->u.presentRequest;

    if (i)
        yaz_log(c->log_details, "%p send_present skip=%d", c, i);

    *req->resultSetStartPoint = *start + 1;

    if (resultset->step > 0 && resultset->step < *count)
        *req->numberOfRecordsRequested = resultset->step;
    else
        *req->numberOfRecordsRequested = *count;
    
    if (*req->numberOfRecordsRequested + *start > resultset->size)
        *req->numberOfRecordsRequested = resultset->size - *start;
    assert(*req->numberOfRecordsRequested > 0);

    if (syntax && *syntax)
        req->preferredRecordSyntax =
            zoom_yaz_str_to_z3950oid(c, CLASS_RECSYN, syntax);

    if (resultset->schema && *resultset->schema)
    {
        Z_RecordComposition *compo = (Z_RecordComposition *)
            odr_malloc(c->odr_out, sizeof(*compo));

        req->recordComposition = compo;
        compo->which = Z_RecordComp_complex;
        compo->u.complex = (Z_CompSpec *)
            odr_malloc(c->odr_out, sizeof(*compo->u.complex));
        compo->u.complex->selectAlternativeSyntax = (bool_t *) 
            odr_malloc(c->odr_out, sizeof(bool_t));
        *compo->u.complex->selectAlternativeSyntax = 0;

        compo->u.complex->generic = (Z_Specification *)
            odr_malloc(c->odr_out, sizeof(*compo->u.complex->generic));

        compo->u.complex->generic->which = Z_Schema_oid;
        compo->u.complex->generic->schema.oid = (Odr_oid *)
            zoom_yaz_str_to_z3950oid (c, CLASS_SCHEMA, resultset->schema);

        if (!compo->u.complex->generic->schema.oid)
        {
            /* OID wasn't a schema! Try record syntax instead. */

            compo->u.complex->generic->schema.oid = (Odr_oid *)
                zoom_yaz_str_to_z3950oid (c, CLASS_RECSYN, resultset->schema);
        }
        if (elementSetName && *elementSetName)
        {
            compo->u.complex->generic->elementSpec = (Z_ElementSpec *)
                odr_malloc(c->odr_out, sizeof(Z_ElementSpec));
            compo->u.complex->generic->elementSpec->which =
                Z_ElementSpec_elementSetName;
            compo->u.complex->generic->elementSpec->u.elementSetName =
                odr_strdup(c->odr_out, elementSetName);
        }
        else
            compo->u.complex->generic->elementSpec = 0;
        compo->u.complex->num_dbSpecific = 0;
        compo->u.complex->dbSpecific = 0;
        compo->u.complex->num_recordSyntax = 0;
        compo->u.complex->recordSyntax = 0;
    }
    else if (elementSetName && *elementSetName)
    {
        Z_ElementSetNames *esn = (Z_ElementSetNames *)
            odr_malloc(c->odr_out, sizeof(*esn));
        Z_RecordComposition *compo = (Z_RecordComposition *)
            odr_malloc(c->odr_out, sizeof(*compo));
        
        esn->which = Z_ElementSetNames_generic;
        esn->u.generic = odr_strdup(c->odr_out, elementSetName);
        compo->which = Z_RecordComp_simple;
        compo->u.simple = esn;
        req->recordComposition = compo;
    }
    req->resultSetId = odr_strdup(c->odr_out, resultset->setname);
    return send_APDU(c, apdu);
}

static zoom_ret send_Z3950_sort_present(ZOOM_connection c)
{
    zoom_ret r = zoom_complete;

    if (c->tasks && c->tasks->which == ZOOM_TASK_SEARCH)
        r = send_Z3950_sort(c, c->tasks->u.search.resultset);
    if (r == zoom_complete)
        r = send_Z3950_present(c);
    return r;
}

void ZOOM_handle_Z3950_apdu(ZOOM_connection c, Z_APDU *apdu)
{
    Z_InitResponse *initrs;
    
    ZOOM_connection_set_mask(c, 0);
    yaz_log(c->log_details, "%p handle_Z3950_apdu apdu->which=%d",
            c, apdu->which);
    switch (apdu->which)
    {
    case Z_APDU_initResponse:
        yaz_log(c->log_api, "%p handle_Z3950_apdu: Received Init response", c);
        initrs = apdu->u.initResponse;
        ZOOM_connection_option_set(c, "serverImplementationId",
                                   initrs->implementationId ?
                                   initrs->implementationId : "");
        ZOOM_connection_option_set(c, "serverImplementationName",
                                   initrs->implementationName ?
                                   initrs->implementationName : "");
        ZOOM_connection_option_set(c, "serverImplementationVersion",
                                   initrs->implementationVersion ?
                                   initrs->implementationVersion : "");
        /* Set the three old options too, for old applications */
        ZOOM_connection_option_set(c, "targetImplementationId",
                                   initrs->implementationId ?
                                   initrs->implementationId : "");
        ZOOM_connection_option_set(c, "targetImplementationName",
                                   initrs->implementationName ?
                                   initrs->implementationName : "");
        ZOOM_connection_option_set(c, "targetImplementationVersion",
                                   initrs->implementationVersion ?
                                   initrs->implementationVersion : "");

        /* Make initrs->options available as ZOOM-level options */
        yaz_init_opt_decode(initrs->options, set_init_option, (void*) c);

        if (!*initrs->result)
        {
            Z_External *uif = initrs->userInformationField;

            ZOOM_set_error(c, ZOOM_ERROR_INIT, 0); /* default error */

            if (uif && uif->which == Z_External_userInfo1)
                interpret_otherinformation_field(c, uif->u.userInfo1);
        }
        else
        {
            char *cookie =
                yaz_oi_get_string_oid(&apdu->u.initResponse->otherInfo,
                                      yaz_oid_userinfo_cookie, 1, 0);
            xfree(c->cookie_in);
            c->cookie_in = 0;
            if (cookie)
                c->cookie_in = xstrdup(cookie);
            if (ODR_MASK_GET(initrs->options, Z_Options_namedResultSets) &&
                ODR_MASK_GET(initrs->protocolVersion, Z_ProtocolVersion_3))
                c->support_named_resultsets = 1;
            if (c->tasks)
            {
                assert(c->tasks->which == ZOOM_TASK_CONNECT);
                ZOOM_connection_remove_task(c);
            }
            ZOOM_connection_exec_task(c);
        }
        if (ODR_MASK_GET(initrs->options, Z_Options_negotiationModel))
        {
            NMEM tmpmem = nmem_create();
            Z_CharSetandLanguageNegotiation *p =
                yaz_get_charneg_record(initrs->otherInfo);
            
            if (p)
            {
                char *charset = NULL, *lang = NULL;
                int sel;
                
                yaz_get_response_charneg(tmpmem, p, &charset, &lang, &sel);
                yaz_log(c->log_details, "%p handle_Z3950_apdu target accepted: "
                        "charset %s, language %s, select %d",
                        c,
                        charset ? charset : "none", lang ? lang : "none", sel);
                if (charset)
                    ZOOM_connection_option_set(c, "negotiation-charset",
                                               charset);
                if (lang)
                    ZOOM_connection_option_set(c, "negotiation-lang",
                                               lang);

                ZOOM_connection_option_set(
                    c,  "negotiation-charset-in-effect-for-records",
                    (sel != 0) ? "1" : "0");
                nmem_destroy(tmpmem);
            }
        }       
        break;
    case Z_APDU_searchResponse:
        yaz_log(c->log_api, "%p handle_Z3950_apdu Search response", c);
        handle_Z3950_search_response(c, apdu->u.searchResponse);
        if (send_Z3950_sort_present(c) == zoom_complete)
            ZOOM_connection_remove_task(c);
        break;
    case Z_APDU_presentResponse:
        yaz_log(c->log_api, "%p handle_Z3950_apdu Present response", c);
        handle_Z3950_present_response(c, apdu->u.presentResponse);
        if (send_Z3950_present(c) == zoom_complete)
            ZOOM_connection_remove_task(c);
        break;
    case Z_APDU_sortResponse:
        yaz_log(c->log_api, "%p handle_Z3950_apdu Sort response", c);
        handle_Z3950_sort_response(c, apdu->u.sortResponse);
        if (send_Z3950_present(c) == zoom_complete)
            ZOOM_connection_remove_task(c);
        break;
    case Z_APDU_scanResponse:
        yaz_log(c->log_api, "%p handle_Z3950_apdu Scan response", c);
        handle_Z3950_scan_response(c, apdu->u.scanResponse);
        ZOOM_connection_remove_task(c);
        break;
    case Z_APDU_extendedServicesResponse:
        yaz_log(c->log_api, "%p handle_Z3950_apdu Extended Services response", c);
        handle_Z3950_es_response(c, apdu->u.extendedServicesResponse);
        ZOOM_connection_remove_task(c);
        break;
    case Z_APDU_close:
        yaz_log(c->log_api, "%p handle_Z3950_apdu Close PDU", c);
        if (!ZOOM_test_reconnect(c))
        {
            ZOOM_set_error(c, ZOOM_ERROR_CONNECTION_LOST, c->host_port);
            ZOOM_connection_close(c);
        }
        break;
    default:
        yaz_log(c->log_api, "%p Received unknown PDU", c);
        ZOOM_set_error(c, ZOOM_ERROR_DECODE, 0);
        ZOOM_connection_close(c);
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

