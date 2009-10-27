/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/** \file
 * \brief yaz-ztest Generic Frontend Server
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <yaz/log.h>
#include <yaz/backend.h>
#include <yaz/ill.h>
#include <yaz/diagbib1.h>

#include "ztest.h"

static int log_level=0;
static int log_level_set=0;

int ztest_search(void *handle, bend_search_rr *rr);
int ztest_sort(void *handle, bend_sort_rr *rr);
int ztest_present(void *handle, bend_present_rr *rr);
int ztest_esrequest(void *handle, bend_esrequest_rr *rr);
int ztest_delete(void *handle, bend_delete_rr *rr);

/** \brief use term value as hit count 
    \param s RPN structure
    \return >= 0: search term number or -1: not found
   
    Traverse RPN tree 'in order' and use term value as hit count.
    Only terms  that looks a numeric is used.. Returns -1 if
    no sub tree has a hit count term
*/
static int get_term_hit(Z_RPNStructure *s)
{
    int h = -1;
    switch(s->which)
    {
    case Z_RPNStructure_simple:
        if (s->u.simple->which == Z_Operand_APT)
        {
            Z_AttributesPlusTerm *apt = s->u.simple->u.attributesPlusTerm;
            if (apt->term->which == Z_Term_general)
            {
                Odr_oct *oct = apt->term->u.general;
                if (oct->len > 0 && oct->buf[0] >= '0' && oct->buf[0] <= '9')
                    h = atoi_n((const char *) oct->buf, oct->len);
            }
        }
        break;
    case Z_RPNStructure_complex:
        h = get_term_hit(s->u.complex->s1);
        if (h == -1)
            h = get_term_hit(s->u.complex->s2);
        break;
    }
    return h;
}

/** \brief gets hit count for numeric terms in RPN queries
    \param q RPN Query
    \return number of hits (random or number for term)
    
    This is just for testing.. A real database of course uses
    the content of a database to establish a value.. In our case, we
    have a way to trigger a certain hit count. Good for testing of
    client applications etc
*/
static int get_hit_count(Z_Query *q)
{
    int h = -1;
    if (q->which == Z_Query_type_1 || q->which == Z_Query_type_101)
        h = get_term_hit(q->u.type_1->RPNStructure);
    if (h == -1)
        h = rand() % 24;
    return h;
}

/** \brief checks if it's a dummy Slow database..
    \param basename database name to check
    \param association backend association (or NULL if not available)
    \retval 1 is slow database
    \retval 0 is not a slow database

    The Slow database is for testing.. It allows us to simulate
    a slow server...
*/
static int check_slow(const char *basename, bend_association association)
{
    if (strncmp(basename, "Slow", 4) == 0)
    {
#if HAVE_UNISTD_H
        int i, w = 3;
        if (basename[4])
            sscanf(basename+4, "%d", &w);
        /* wait up to 3 seconds and check if connection is still alive */
        for (i = 0; i < w; i++)
        {
            if (association && !bend_assoc_is_alive(association))
            {
                yaz_log(YLOG_LOG, "search aborted");
                break;
            }
            sleep(1);
        }
#endif
        return 1;
    }
    return 0;
}

int ztest_search(void *handle, bend_search_rr *rr)
{
    if (rr->num_bases != 1)
    {
        rr->errcode = YAZ_BIB1_COMBI_OF_SPECIFIED_DATABASES_UNSUPP;
        return 0;
    }
    /* Throw Database unavailable if other than Default or Slow */
    if (!yaz_matchstr(rr->basenames[0], "Default"))
        ;  /* Default is OK in our test */
    else if (check_slow(rr->basenames[0], rr->association))
    {
        rr->estimated_hit_count = 1;
    }
    else
    {
        rr->errcode = YAZ_BIB1_DATABASE_UNAVAILABLE;
        rr->errstring = rr->basenames[0];
        return 0;
    }

    if (rr->extra_args)
    {
        Z_SRW_extra_arg *a;
        WRBUF response_xml = wrbuf_alloc();
        wrbuf_puts(response_xml, "<extra>");
        for (a = rr->extra_args; a; a = a->next)
        {
            wrbuf_puts(response_xml, "<extra name=\"");
            wrbuf_xmlputs(response_xml, a->name);
            wrbuf_puts(response_xml, "\"");
            if (a->value)
            {
                wrbuf_puts(response_xml, " value=\"");
                wrbuf_xmlputs(response_xml, a->value);
                wrbuf_puts(response_xml, "\"");
            }
            wrbuf_puts(response_xml, "/>");
        }
        wrbuf_puts(response_xml, "</extra>");
        rr->extra_response_data =
            odr_strdup(rr->stream, wrbuf_cstr(response_xml));
        wrbuf_destroy(response_xml);
    }
    rr->hits = get_hit_count(rr->query);
    return 0;
}


/* this huge function handles extended services */
int ztest_esrequest(void *handle, bend_esrequest_rr *rr)
{
    /* user-defined handle - created in bend_init */
    int *counter = (int*) handle;  

    yaz_log(log_level, "ESRequest no %d", *counter);

    (*counter)++;

    if (rr->esr->packageName)
        yaz_log(log_level, "packagename: %s", rr->esr->packageName);
    yaz_log(log_level, "Waitaction: " ODR_INT_PRINTF, *rr->esr->waitAction);


    yaz_log(log_level, "function: " ODR_INT_PRINTF, *rr->esr->function);

    if (!rr->esr->taskSpecificParameters)
    {
        yaz_log(log_level, "No task specific parameters");
    }
    else if (rr->esr->taskSpecificParameters->which == Z_External_itemOrder)
    {
        Z_ItemOrder *it = rr->esr->taskSpecificParameters->u.itemOrder;
        yaz_log(log_level, "Received ItemOrder");
        if (it->which == Z_IOItemOrder_esRequest)
        {
            Z_IORequest *ir = it->u.esRequest;
            Z_IOOriginPartToKeep *k = ir->toKeep;
            Z_IOOriginPartNotToKeep *n = ir->notToKeep;
            const char *xml_in_response = 0;
            
            if (k && k->contact)
            {
                if (k->contact->name)
                    yaz_log(log_level, "contact name %s", k->contact->name);
                if (k->contact->phone)
                    yaz_log(log_level, "contact phone %s", k->contact->phone);
                if (k->contact->email)
                    yaz_log(log_level, "contact email %s", k->contact->email);
            }
            if (k->addlBilling)
            {
                yaz_log(log_level, "Billing info (not shown)");
            }
            
            if (n->resultSetItem)
            {
                yaz_log(log_level, "resultsetItem");
                yaz_log(log_level, "setId: %s", n->resultSetItem->resultSetId);
                yaz_log(log_level, "item: " ODR_INT_PRINTF, *n->resultSetItem->item);
            }
            if (n->itemRequest)
            {
                Z_External *r = (Z_External*) n->itemRequest;
                ILL_ItemRequest *item_req = 0;
                ILL_APDU *ill_apdu = 0;
                if (r->direct_reference)
                {
                    char oid_name_str[OID_STR_MAX];
                    oid_class oclass;
                    const char *oid_name = 
                        yaz_oid_to_string_buf(r->direct_reference,
                                              &oclass, oid_name_str);
                    if (oid_name)
                        yaz_log(log_level, "OID %s", oid_name);
                    if (!oid_oidcmp(r->direct_reference, yaz_oid_recsyn_xml))
                    {
                        yaz_log(log_level, "ILL XML request");
                        if (r->which == Z_External_octet)
                            yaz_log(log_level, "%.*s",
                                    r->u.octet_aligned->len,
                                    r->u.octet_aligned->buf); 
                        xml_in_response = "<dummy>x</dummy>";
                    }
                    if (!oid_oidcmp(r->direct_reference, 
                                    yaz_oid_general_isoill_1))
                    {
                        yaz_log(log_level, "Decode ItemRequest begin");
                        if (r->which == ODR_EXTERNAL_single)
                        {
                            odr_setbuf(rr->decode,
                                       (char *) r->u.single_ASN1_type->buf,
                                       r->u.single_ASN1_type->len, 0);
                            
                            if (!ill_ItemRequest(rr->decode, &item_req, 0, 0))
                            {
                                yaz_log(log_level,
                                        "Couldn't decode ItemRequest %s near %ld",
                                        odr_errmsg(odr_geterror(rr->decode)),
                                        (long) odr_offset(rr->decode));
                            }
                            else
                                yaz_log(log_level, "Decode ItemRequest OK");
                            if (rr->print)
                            {
                                ill_ItemRequest(rr->print, &item_req, 0,
                                                "ItemRequest");
                                odr_reset(rr->print);
                            }
                        }
                        if (!item_req && r->which == ODR_EXTERNAL_single)
                        {
                            yaz_log(log_level, "Decode ILL APDU begin");
                            odr_setbuf(rr->decode,
                                       (char*) r->u.single_ASN1_type->buf,
                                       r->u.single_ASN1_type->len, 0);
                            
                            if (!ill_APDU(rr->decode, &ill_apdu, 0, 0))
                            {
                                yaz_log(log_level,
                                        "Couldn't decode ILL APDU %s near %ld",
                                        odr_errmsg(odr_geterror(rr->decode)),
                                        (long) odr_offset(rr->decode));
                                yaz_log(log_level, "PDU dump:");
                                odr_dumpBER(yaz_log_file(),
                                            (char *) r->u.single_ASN1_type->buf,
                                            r->u.single_ASN1_type->len);
                            }
                            else
                                yaz_log(log_level, "Decode ILL APDU OK");
                            if (rr->print)
                            {
                                ill_APDU(rr->print, &ill_apdu, 0,
                                         "ILL APDU");
                                odr_reset(rr->print);
                            }
                        }
                    }
                }
                if (item_req)
                {
                    yaz_log(log_level, "ILL protocol version = "
                            ODR_INT_PRINTF,
                            *item_req->protocol_version_num);
                }
            }
            if (k)
            {

                Z_External *ext = (Z_External *)
                    odr_malloc(rr->stream, sizeof(*ext));
                Z_IUOriginPartToKeep *keep = (Z_IUOriginPartToKeep *)
                    odr_malloc(rr->stream, sizeof(*keep));
                Z_IOTargetPart *targetPart = (Z_IOTargetPart *)
                    odr_malloc(rr->stream, sizeof(*targetPart));

                rr->taskPackage = (Z_TaskPackage *)
                    odr_malloc(rr->stream, sizeof(*rr->taskPackage));
                rr->taskPackage->packageType =
                    odr_oiddup(rr->stream, rr->esr->packageType);
                rr->taskPackage->packageName = 0;
                rr->taskPackage->userId = 0;
                rr->taskPackage->retentionTime = 0;
                rr->taskPackage->permissions = 0;
                rr->taskPackage->description = 0;
                rr->taskPackage->targetReference = (Odr_oct *)
                    odr_malloc(rr->stream, sizeof(Odr_oct));
                rr->taskPackage->targetReference->buf =
                    (unsigned char *) odr_strdup(rr->stream, "911");
                rr->taskPackage->targetReference->len =
                    rr->taskPackage->targetReference->size =
                    strlen((char *) (rr->taskPackage->targetReference->buf));
                rr->taskPackage->creationDateTime = 0;
                rr->taskPackage->taskStatus = odr_intdup(rr->stream, 0);
                rr->taskPackage->packageDiagnostics = 0;
                rr->taskPackage->taskSpecificParameters = ext;

                ext->direct_reference =
                    odr_oiddup(rr->stream, rr->esr->packageType);
                ext->indirect_reference = 0;
                ext->descriptor = 0;
                ext->which = Z_External_itemOrder;
                ext->u.itemOrder = (Z_ItemOrder *)
                    odr_malloc(rr->stream, sizeof(*ext->u.update));
                ext->u.itemOrder->which = Z_IOItemOrder_taskPackage;
                ext->u.itemOrder->u.taskPackage =  (Z_IOTaskPackage *)
                    odr_malloc(rr->stream, sizeof(Z_IOTaskPackage));
                ext->u.itemOrder->u.taskPackage->originPart = k;
                ext->u.itemOrder->u.taskPackage->targetPart = targetPart;

                if (xml_in_response)
                    targetPart->itemRequest =
                        z_ext_record_xml(rr->stream, xml_in_response,
                                         strlen(xml_in_response));
                else
                    targetPart->itemRequest = 0;
                    
                targetPart->statusOrErrorReport = 0;
                targetPart->auxiliaryStatus = 0;
            }
        }
    }
    else if (rr->esr->taskSpecificParameters->which == Z_External_update)
    {
        Z_IUUpdate *up = rr->esr->taskSpecificParameters->u.update;
        yaz_log(log_level, "Received DB Update");
        if (up->which == Z_IUUpdate_esRequest)
        {
            Z_IUUpdateEsRequest *esRequest = up->u.esRequest;
            Z_IUOriginPartToKeep *toKeep = esRequest->toKeep;
            Z_IUSuppliedRecords *notToKeep = esRequest->notToKeep;
            
            yaz_log(log_level, "action");
            if (toKeep->action)
            {
                switch (*toKeep->action)
                {
                case Z_IUOriginPartToKeep_recordInsert:
                    yaz_log(log_level, " recordInsert");
                    break;
                case Z_IUOriginPartToKeep_recordReplace:
                    yaz_log(log_level, " recordReplace");
                    break;
                case Z_IUOriginPartToKeep_recordDelete:
                    yaz_log(log_level, " recordDelete");
                    break;
                case Z_IUOriginPartToKeep_elementUpdate:
                    yaz_log(log_level, " elementUpdate");
                    break;
                case Z_IUOriginPartToKeep_specialUpdate:
                    yaz_log(log_level, " specialUpdate");
                    break;
                default:
                    yaz_log(log_level, " unknown (" ODR_INT_PRINTF ")",
                            *toKeep->action);
                }
            }
            if (toKeep->databaseName)
            {
                yaz_log(log_level, "database: %s", toKeep->databaseName);
                if (!strcmp(toKeep->databaseName, "fault"))
                {
                    rr->errcode = YAZ_BIB1_DATABASE_UNAVAILABLE;
                    rr->errstring = toKeep->databaseName;
                }
                if (!strcmp(toKeep->databaseName, "accept"))
                    rr->errcode = -1;
            }
            if (toKeep)
            {
                Z_External *ext = (Z_External *)
                    odr_malloc(rr->stream, sizeof(*ext));
                Z_IUOriginPartToKeep *keep = (Z_IUOriginPartToKeep *)
                    odr_malloc(rr->stream, sizeof(*keep));
                Z_IUTargetPart *targetPart = (Z_IUTargetPart *)
                    odr_malloc(rr->stream, sizeof(*targetPart));

                rr->taskPackage = (Z_TaskPackage *)
                    odr_malloc(rr->stream, sizeof(*rr->taskPackage));
                rr->taskPackage->packageType =
                    odr_oiddup(rr->stream, rr->esr->packageType);
                rr->taskPackage->packageName = 0;
                rr->taskPackage->userId = 0;
                rr->taskPackage->retentionTime = 0;
                rr->taskPackage->permissions = 0;
                rr->taskPackage->description = 0;
                rr->taskPackage->targetReference = (Odr_oct *)
                    odr_malloc(rr->stream, sizeof(Odr_oct));
                rr->taskPackage->targetReference->buf =
                    (unsigned char *) odr_strdup(rr->stream, "123");
                rr->taskPackage->targetReference->len =
                    rr->taskPackage->targetReference->size =
                    strlen((char *) (rr->taskPackage->targetReference->buf));
                rr->taskPackage->creationDateTime = 0;
                rr->taskPackage->taskStatus = odr_intdup(rr->stream, 0);
                rr->taskPackage->packageDiagnostics = 0;
                rr->taskPackage->taskSpecificParameters = ext;

                ext->direct_reference =
                    odr_oiddup(rr->stream, rr->esr->packageType);
                ext->indirect_reference = 0;
                ext->descriptor = 0;
                ext->which = Z_External_update;
                ext->u.update = (Z_IUUpdate *)
                    odr_malloc(rr->stream, sizeof(*ext->u.update));
                ext->u.update->which = Z_IUUpdate_taskPackage;
                ext->u.update->u.taskPackage =  (Z_IUUpdateTaskPackage *)
                    odr_malloc(rr->stream, sizeof(Z_IUUpdateTaskPackage));
                ext->u.update->u.taskPackage->originPart = keep;
                ext->u.update->u.taskPackage->targetPart = targetPart;

                keep->action = odr_intdup(rr->stream, *toKeep->action);
                keep->databaseName =
                    odr_strdup(rr->stream, toKeep->databaseName);
                keep->schema = 0;
                keep->elementSetName = 0;
                keep->actionQualifier = 0;

                targetPart->updateStatus = odr_intdup(rr->stream, 1);
                targetPart->num_globalDiagnostics = 0;
                targetPart->globalDiagnostics = (Z_DiagRec **) odr_nullval();
                targetPart->num_taskPackageRecords = 1;
                targetPart->taskPackageRecords = 
                    (Z_IUTaskPackageRecordStructure **)
                    odr_malloc(rr->stream,
                               sizeof(Z_IUTaskPackageRecordStructure *));
                targetPart->taskPackageRecords[0] =
                    (Z_IUTaskPackageRecordStructure *)
                    odr_malloc(rr->stream,
                               sizeof(Z_IUTaskPackageRecordStructure));
                
                targetPart->taskPackageRecords[0]->which =
                    Z_IUTaskPackageRecordStructure_record;
                targetPart->taskPackageRecords[0]->u.record = 
                    z_ext_record_sutrs(rr->stream, "test", 4);
                targetPart->taskPackageRecords[0]->correlationInfo = 0; 
                targetPart->taskPackageRecords[0]->recordStatus =
                    odr_intdup(rr->stream,
                               Z_IUTaskPackageRecordStructure_success);  
                targetPart->taskPackageRecords[0]->num_supplementalDiagnostics
                    = 0;

                targetPart->taskPackageRecords[0]->supplementalDiagnostics = 0;
            }
            if (notToKeep)
            {
                int i;
                for (i = 0; i < notToKeep->num; i++)
                {
                    Z_External *rec = notToKeep->elements[i]->record;

                    if (rec->direct_reference)
                    {
                        char oid_name_str[OID_STR_MAX];
                        const char *oid_name 
                            = oid_name = yaz_oid_to_string_buf(
                                rec->direct_reference, 0,
                                oid_name_str);
                        if (oid_name)
                            yaz_log(log_level, "record %d type %s", i,
                                    oid_name);
                    }
                    switch (rec->which)
                    {
                    case Z_External_sutrs:
                        if (rec->u.octet_aligned->len > 170)
                            yaz_log(log_level, "%d bytes:\n%.168s ...",
                                    rec->u.sutrs->len,
                                    rec->u.sutrs->buf);
                        else
                            yaz_log(log_level, "%d bytes:\n%s",
                                    rec->u.sutrs->len,
                                    rec->u.sutrs->buf);
                        break;
                    case Z_External_octet        :
                        if (rec->u.octet_aligned->len > 170)
                            yaz_log(log_level, "%d bytes:\n%.168s ...",
                                    rec->u.octet_aligned->len,
                                    rec->u.octet_aligned->buf);
                        else
                            yaz_log(log_level, "%d bytes\n%s",
                                    rec->u.octet_aligned->len,
                                    rec->u.octet_aligned->buf);
                    }
                }
            }
        }
    }
    return 0;
}

/* result set delete */
int ztest_delete(void *handle, bend_delete_rr *rr)
{
    if (rr->num_setnames == 1 && !strcmp(rr->setnames[0], "1"))
        rr->delete_status = Z_DeleteStatus_success;
    else
        rr->delete_status = Z_DeleteStatus_resultSetDidNotExist;
    return 0;
}

/* Our sort handler really doesn't sort... */
int ztest_sort(void *handle, bend_sort_rr *rr)
{
    rr->errcode = 0;
    rr->sort_status = Z_SortResponse_success;
    return 0;
}


/* present request handler */
int ztest_present(void *handle, bend_present_rr *rr)
{
    return 0;
}

/* retrieval of a single record (present, and piggy back search) */
int ztest_fetch(void *handle, bend_fetch_rr *r)
{
    char *cp;
    const Odr_oid *oid = r->request_format;

    r->last_in_set = 0;
    r->basename = "Default";
    r->output_format = r->request_format;

    if (!oid || yaz_oid_is_iso2709(oid))
    {
        cp = dummy_marc_record(r->number, r->stream);
        if (!cp)
        {
            r->errcode = YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE;
            return 0;
        }
        else
        {
            r->len = strlen(cp);
            r->record = cp;
            r->output_format = odr_oiddup(r->stream, yaz_oid_recsyn_usmarc);
        }
    }
    else if (!oid_oidcmp(oid, yaz_oid_recsyn_opac))
    {
        cp = dummy_marc_record(r->number, r->stream);
        if (!cp)
        {
            r->errcode = YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE;
            return 0;
        }
        r->record = (char *) dummy_opac(r->number, r->stream, cp);
        r->len = -1;
    }
    else if (!oid_oidcmp(oid, yaz_oid_recsyn_sutrs))
    {
        /* this section returns a small record */
        char buf[100];
        
        sprintf(buf, "This is dummy SUTRS record number %d\n", r->number);

        r->len = strlen(buf);
        r->record = (char *) odr_malloc(r->stream, r->len+1);
        strcpy(r->record, buf);
    }
    else if (!oid_oidcmp(oid, yaz_oid_recsyn_grs_1))
    {
        r->len = -1;
        r->record = (char*) dummy_grs_record(r->number, r->stream);
        if (!r->record)
        {
            r->errcode = YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE;
            return 0;
        }
    }
    else if (!oid_oidcmp(oid, yaz_oid_recsyn_postscript))
    {
        char fname[20];
        FILE *f;
        long size;

        sprintf(fname, "part.%d.ps", r->number);
        f = fopen(fname, "rb");
        if (!f)
        {
            r->errcode = YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE;
            return 0;
        }
        fseek(f, 0L, SEEK_END);
        size = ftell(f);
        if (size <= 0 || size >= 5000000)
        {
            r->errcode = YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS;
        }
        fseek(f, 0L, SEEK_SET);
        r->record = (char*) odr_malloc(r->stream, size);
        r->len = size;
        if (fread(r->record, size, 1, f) != 1)
        {
            r->errcode = YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS;
        }
        fclose(f);
    }
    else if (!oid_oidcmp(oid, yaz_oid_recsyn_xml))
    {
        if ((cp = dummy_xml_record(r->number, r->stream)))
        {
            r->len = strlen(cp);
            r->record = cp;
        }
        else 
        {
            r->errcode = YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE;
            r->surrogate_flag = 1;
            return 0;
        }
    }
    else
    {
        char buf[OID_STR_MAX];
        r->errcode = YAZ_BIB1_RECORD_SYNTAX_UNSUPP;
        r->errstring = odr_strdup(r->stream, oid_oid_to_dotstring(oid, buf));
        return 0;
    }
    r->errcode = 0;
    return 0;
}

/*
 * silly dummy-scan what reads words from a file.
 */
int ztest_scan(void *handle, bend_scan_rr *q)
{
    static FILE *f = 0;
    static struct scan_entry list[200];
    static char entries[200][80];
    int hits[200];
    char term[80], *p;
    int i, pos;
    int term_position_req = q->term_position;
    int num_entries_req = q->num_entries;

    /* Throw Database unavailable if other than Default or Slow */
    if (!yaz_matchstr(q->basenames[0], "Default"))
        ;  /* Default is OK in our test */
    else if (check_slow(q->basenames[0], 0 /* no assoc for scan */))
        ;
    else
    {
        q->errcode = YAZ_BIB1_DATABASE_UNAVAILABLE;
        q->errstring = q->basenames[0];
        return 0;
    }

    q->errcode = 0;
    q->errstring = 0;
    q->entries = list;
    q->status = BEND_SCAN_SUCCESS;
    if (!f && !(f = fopen("dummy-words", "r")))
    {
        perror("dummy-words");
        exit(1);
    }
    if (q->num_entries > 200)
    {
        q->errcode = YAZ_BIB1_RESOURCES_EXHAUSTED_NO_RESULTS_AVAILABLE;
        return 0;
    }
    if (q->term)
    {
        int len;
        if (q->term->term->which != Z_Term_general)
        {
            q->errcode = YAZ_BIB1_TERM_TYPE_UNSUPP;
            return 0;
        }
        if (*q->step_size != 0)
        {
            q->errcode = YAZ_BIB1_ONLY_ZERO_STEP_SIZE_SUPPORTED_FOR_SCAN;
            return 0;
        }
        len = q->term->term->u.general->len;
        if (len >= sizeof(term))
            len = sizeof(term)-1;
        memcpy(term, q->term->term->u.general->buf, len);
        term[len] = '\0';
    }
    else if (q->scanClause)
    {
        strncpy(term, q->scanClause, sizeof(term)-1);
        term[sizeof(term)-1] = '\0';
    }
    else
        strcpy(term, "0");

    for (p = term; *p; p++)
        if (islower(*(unsigned char *) p))
            *p = toupper(*p);

    fseek(f, 0, SEEK_SET);
    q->num_entries = 0;

    for (i = 0, pos = 0; fscanf(f, " %79[^:]:%d", entries[pos], &hits[pos]) == 2;
         i++, pos < 199 ? pos++ : (pos = 0))
    {
        if (!q->num_entries && strcmp(entries[pos], term) >= 0) /* s-point fnd */
        {
            if ((q->term_position = term_position_req) > i + 1)
            {
                q->term_position = i + 1;
                q->status = BEND_SCAN_PARTIAL;
            }
            for (; q->num_entries < q->term_position; q->num_entries++)
            {
                int po;

                po = pos - q->term_position + q->num_entries+1; /* find pos */
                if (po < 0)
                    po += 200;

                if (!strcmp(term, "SD") && q->num_entries == 2)
                {
                    list[q->num_entries].term = entries[pos];
                    list[q->num_entries].occurrences = -1;
                    list[q->num_entries].errcode =
                        YAZ_BIB1_SCAN_UNSUPP_VALUE_OF_POSITION_IN_RESPONSE;
                    list[q->num_entries].errstring = "SD for Scan Term";
                }
                else
                {
                    list[q->num_entries].term = entries[po];
                    list[q->num_entries].occurrences = hits[po];
                }
            }
        }
        else if (q->num_entries)
        {
            list[q->num_entries].term = entries[pos];
            list[q->num_entries].occurrences = hits[pos];
            q->num_entries++;
        }
        if (q->num_entries >= num_entries_req)
            break;
    }
    if (feof(f))
        q->status = BEND_SCAN_PARTIAL;
    return 0;
}

int ztest_explain(void *handle, bend_explain_rr *rr)
{
    if (rr->database && !strcmp(rr->database, "Default"))
    {
        rr->explain_buf = "<explain>\n"
            "\t<serverInfo>\n"
            "\t\t<host>localhost</host>\n"
            "\t\t<port>210</port>\n"
            "\t</serverInfo>\n"
            "</explain>\n";
    }
    return 0;
}

int ztest_update(void *handle, bend_update_rr *rr)
{
    rr->operation_status = "success";
    return 0;
}

bend_initresult *bend_init(bend_initrequest *q)
{
    bend_initresult *r = (bend_initresult *)
        odr_malloc(q->stream, sizeof(*r));
    int *counter = (int *) xmalloc(sizeof(int));

    if (!log_level_set)
    {
        log_level=yaz_log_module_level("ztest");
        log_level_set=1;
    }

    *counter = 0;
    r->errcode = 0;
    r->errstring = 0;
    r->handle = counter;         /* user handle, in this case a simple int */
    q->bend_sort = ztest_sort;              /* register sort handler */
    q->bend_search = ztest_search;          /* register search handler */
    q->bend_present = ztest_present;        /* register present handle */
    q->bend_esrequest = ztest_esrequest;
    q->bend_delete = ztest_delete;
    q->bend_fetch = ztest_fetch;
    q->bend_scan = ztest_scan;
#if 0
    q->bend_explain = ztest_explain;
#endif
    q->bend_srw_scan = ztest_scan;
    q->bend_srw_update = ztest_update;

    q->query_charset = "ISO-8859-1";
    q->records_in_same_charset = 0;

    return r;
}

void bend_close(void *handle)
{
    xfree(handle);              /* release our user-defined handle */
    return;
}

int main(int argc, char **argv)
{
    return statserv_main(argc, argv, bend_init, bend_close);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

