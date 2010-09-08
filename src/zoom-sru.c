/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-sru.c
 * \brief Implements ZOOM SRU
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include "zoom-p.h"

#include <yaz/log.h>
#include <yaz/pquery.h>

void handle_facet_list(ZOOM_resultset r, Z_FacetList *fl);

#if YAZ_HAVE_XML2
static void set_SRU_error(ZOOM_connection c, Z_SRW_diagnostic *d)
{
    const char *uri = d->uri;
    if (uri)
        ZOOM_set_dset_error(c, ZOOM_uri_to_code(uri), uri, d->details, 0);
}
#endif


#if YAZ_HAVE_XML2
static zoom_ret send_srw(ZOOM_connection c, Z_SRW_PDU *sr)
{
    Z_GDU *gdu;
    ZOOM_Event event;
    const char *database =  ZOOM_options_get(c->options, "databaseName");
    char *fdatabase = 0;
    
    if (database)
        fdatabase = yaz_encode_sru_dbpath_odr(c->odr_out, database);
    gdu = z_get_HTTP_Request_host_path(c->odr_out, c->host_port,
                                       fdatabase ? fdatabase : c->path);

    if (c->sru_mode == zoom_sru_get)
    {
        yaz_sru_get_encode(gdu->u.HTTP_Request, sr, c->odr_out, c->charset);
    }
    else if (c->sru_mode == zoom_sru_post)
    {
        yaz_sru_post_encode(gdu->u.HTTP_Request, sr, c->odr_out, c->charset);
    }
    else if (c->sru_mode == zoom_sru_soap)
    {
        yaz_sru_soap_encode(gdu->u.HTTP_Request, sr, c->odr_out, c->charset);
    }
    else if (c->sru_mode == zoom_sru_solr)
    {
        yaz_solr_encode_request(gdu->u.HTTP_Request, sr, c->odr_out, c->charset);
    }
    if (!z_GDU(c->odr_out, &gdu, 0, 0))
        return zoom_complete;
    if (c->odr_print)
        z_GDU(c->odr_print, &gdu, 0, 0);
    c->buf_out = odr_getbuf(c->odr_out, &c->len_out, 0);
        
    event = ZOOM_Event_create(ZOOM_EVENT_SEND_APDU);
    ZOOM_connection_put_event(c, event);
    odr_reset(c->odr_out);
    return ZOOM_send_buf(c);
}
#endif

#if YAZ_HAVE_XML2
static Z_SRW_PDU *ZOOM_srw_get_pdu(ZOOM_connection c, int type)
{
    Z_SRW_PDU *sr = yaz_srw_get_pdu(c->odr_out, type, c->sru_version);
    sr->username = c->user;
    sr->password = c->password;
    return sr;
}
#endif

#if YAZ_HAVE_XML2
zoom_ret ZOOM_connection_srw_send_scan(ZOOM_connection c)
{
    ZOOM_scanset scan;
    Z_SRW_PDU *sr = 0;
    const char *option_val = 0;
    Z_Query *z_query;

    if (!c->tasks)
        return zoom_complete;
    assert (c->tasks->which == ZOOM_TASK_SCAN);
    scan = c->tasks->u.scan.scan;
        
    sr = ZOOM_srw_get_pdu(c, Z_SRW_scan_request);

    z_query = ZOOM_query_get_Z_Query(scan->query);
    /* SRU scan can only carry CQL and PQF */
    if (z_query->which == Z_Query_type_104)
    {
        sr->u.scan_request->query_type = Z_SRW_query_type_cql;
        sr->u.scan_request->scanClause.cql =
            ZOOM_query_get_query_string(scan->query);
    }
    else if (z_query->which == Z_Query_type_1
             || z_query->which == Z_Query_type_101)
    {
        sr->u.scan_request->query_type = Z_SRW_query_type_pqf;
        sr->u.scan_request->scanClause.pqf =
            ZOOM_query_get_query_string(scan->query);
    }
    else
    {
        ZOOM_set_error(c, ZOOM_ERROR_UNSUPPORTED_QUERY, 0);
        return zoom_complete;
    }

    sr->u.scan_request->maximumTerms = odr_intdup(
        c->odr_out, ZOOM_options_get_int(scan->options, "number", 10));
    
    sr->u.scan_request->responsePosition = odr_intdup(
        c->odr_out, ZOOM_options_get_int(scan->options, "position", 1));
    
    option_val = ZOOM_options_get(scan->options, "extraArgs");
    yaz_encode_sru_extra(sr, c->odr_out, option_val);
    return send_srw(c, sr);
}
#else
zoom_ret ZOOM_connection_srw_send_scan(ZOOM_connection c)
{
    return zoom_complete;
}
#endif

#if YAZ_HAVE_XML2
zoom_ret ZOOM_connection_srw_send_search(ZOOM_connection c)
{
    const char *facets = 0;
    int i;
    int *start, *count;
    ZOOM_resultset resultset = 0;
    Z_SRW_PDU *sr = 0;
    const char *option_val = 0;
    Z_Query *z_query;
    Z_FacetList *facet_list = 0;
    if (c->error)                  /* don't continue on error */
        return zoom_complete;
    assert(c->tasks);
    switch(c->tasks->which)
    {
    case ZOOM_TASK_SEARCH:
        resultset = c->tasks->u.search.resultset;
        if (!resultset->setname)
            resultset->setname = xstrdup("default");
        ZOOM_options_set(resultset->options, "setname", resultset->setname);
        start = &c->tasks->u.search.start;
        count = &c->tasks->u.search.count;
        facets = ZOOM_options_get(resultset->options, "facets");
        if (facets) {
            facet_list = yaz_pqf_parse_facet_list(c->odr_out, facets);
        }
        break;
    case ZOOM_TASK_RETRIEVE:
        resultset = c->tasks->u.retrieve.resultset;

        start = &c->tasks->u.retrieve.start;
        count = &c->tasks->u.retrieve.count;

        if (*start >= resultset->size)
            return zoom_complete;
        if (*start + *count > resultset->size)
            *count = resultset->size - *start;

        for (i = 0; i < *count; i++)
        {
            ZOOM_record rec =
                ZOOM_record_cache_lookup(resultset, i + *start,
                                         c->tasks->u.retrieve.syntax,
                                         c->tasks->u.retrieve.elementSetName);
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
            return zoom_complete;
        break;
    default:
        return zoom_complete;
    }
    assert(resultset->query);
        
    sr = ZOOM_srw_get_pdu(c, Z_SRW_searchRetrieve_request);
    z_query = ZOOM_query_get_Z_Query(resultset->query);

    if (z_query->which == Z_Query_type_104
        && z_query->u.type_104->which == Z_External_CQL)
    {
        sr->u.request->query_type = Z_SRW_query_type_cql;
        sr->u.request->query.cql = z_query->u.type_104->u.cql;
    }
    else if (z_query->which == Z_Query_type_1 && z_query->u.type_1)
    {
        sr->u.request->query_type = Z_SRW_query_type_pqf;
        sr->u.request->query.pqf =
            ZOOM_query_get_query_string(resultset->query);
    }
    else
    {
        ZOOM_set_error(c, ZOOM_ERROR_UNSUPPORTED_QUERY, 0);
        return zoom_complete;
    }
    sr->u.request->startRecord = odr_intdup(c->odr_out, *start + 1);
    sr->u.request->maximumRecords = odr_intdup(
        c->odr_out, (resultset->step > 0 && resultset->step < *count) ? 
        resultset->step : *count);
    sr->u.request->recordSchema = resultset->schema;
    sr->u.request->facetList = facet_list;
    
    option_val = ZOOM_resultset_option_get(resultset, "recordPacking");
    if (option_val)
        sr->u.request->recordPacking = odr_strdup(c->odr_out, option_val);

    option_val = ZOOM_resultset_option_get(resultset, "extraArgs");
    yaz_encode_sru_extra(sr, c->odr_out, option_val);
    return send_srw(c, sr);
}
#else
zoom_ret ZOOM_connection_srw_send_search(ZOOM_connection c)
{
    return zoom_complete;
}
#endif

#if YAZ_HAVE_XML2
static zoom_ret handle_srw_response(ZOOM_connection c,
                                    Z_SRW_searchRetrieveResponse *res)
{
    ZOOM_resultset resultset = 0;
    int i;
    NMEM nmem;
    ZOOM_Event event;
    int *start, *count;
    const char *syntax, *elementSetName;

    if (!c->tasks)
        return zoom_complete;

    switch(c->tasks->which)
    {
    case ZOOM_TASK_SEARCH:
        resultset = c->tasks->u.search.resultset;
        start = &c->tasks->u.search.start;
        count = &c->tasks->u.search.count;
        syntax = c->tasks->u.search.syntax;
        elementSetName = c->tasks->u.search.elementSetName;        

        if (!c->tasks->u.search.recv_search_fired)
        {
            event = ZOOM_Event_create(ZOOM_EVENT_RECV_SEARCH);
            ZOOM_connection_put_event(c, event);
            c->tasks->u.search.recv_search_fired = 1;
        }
        if (res->facetList)
            handle_facet_list(resultset, res->facetList);
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

    resultset->size = 0;

    if (res->resultSetId)
        ZOOM_resultset_option_set(resultset, "resultSetId", res->resultSetId);

    yaz_log(c->log_details, "%p handle_srw_response got SRW response OK", c);

    if (res->num_diagnostics > 0)
    {
        set_SRU_error(c, &res->diagnostics[0]);
    }
    else
    {
        if (res->numberOfRecords) {
            resultset->size = *res->numberOfRecords;
        }
        for (i = 0; i<res->num_records; i++)
        {
            int pos;
            Z_SRW_record *sru_rec;
            Z_SRW_diagnostic *diag = 0;
            int num_diag;
            
            Z_NamePlusRecord *npr = (Z_NamePlusRecord *)
                odr_malloc(c->odr_in, sizeof(Z_NamePlusRecord));
            /*
             * TODO This does not work with 0-based recordPositions.
             * We will iterate over one twice
             */
            if (res->records[i].recordPosition && 
                *res->records[i].recordPosition > 0)
                pos = *res->records[i].recordPosition - 1;
            else
                pos = *start + i;

            sru_rec = &res->records[i];
            
            npr->databaseName = 0;
            npr->which = Z_NamePlusRecord_databaseRecord;
            npr->u.databaseRecord = (Z_External *)
                odr_malloc(c->odr_in, sizeof(Z_External));
            npr->u.databaseRecord->descriptor = 0;
            npr->u.databaseRecord->direct_reference =
                odr_oiddup(c->odr_in, yaz_oid_recsyn_xml);
            npr->u.databaseRecord->which = Z_External_octet;
            
            npr->u.databaseRecord->u.octet_aligned = (Odr_oct *)
                odr_malloc(c->odr_in, sizeof(Odr_oct));
            npr->u.databaseRecord->u.octet_aligned->buf = (unsigned char*)
                sru_rec->recordData_buf;
            npr->u.databaseRecord->u.octet_aligned->len = 
                npr->u.databaseRecord->u.octet_aligned->size = 
                sru_rec->recordData_len;
            
            if (sru_rec->recordSchema 
                && !strcmp(sru_rec->recordSchema,
                           "info:srw/schema/1/diagnostics-v1.1"))
            {
                sru_decode_surrogate_diagnostics(sru_rec->recordData_buf,
                                                 sru_rec->recordData_len,
                                                 &diag, &num_diag,
                                                 resultset->odr);
            }
            ZOOM_record_cache_add(resultset, npr, pos, syntax, elementSetName,
                                  sru_rec->recordSchema, diag);
        }
        *count -= i;
        *start += i;
        if (*count + *start > resultset->size)
            *count = resultset->size - *start;
        yaz_log(YLOG_DEBUG, "SRU result set size " ODR_INT_PRINTF " start %d count %d", resultset->size, *start, *count);
        if (*count < 0)
            *count = 0;
        nmem = odr_extract_mem(c->odr_in);
        nmem_transfer(odr_getmem(resultset->odr), nmem);
        nmem_destroy(nmem);

        if (*count > 0)
            return ZOOM_connection_srw_send_search(c);
    }
    return zoom_complete;
}
#endif

#if YAZ_HAVE_XML2
static void handle_srw_scan_response(ZOOM_connection c,
                                     Z_SRW_scanResponse *res)
{
    NMEM nmem = odr_extract_mem(c->odr_in);
    ZOOM_scanset scan;

    if (!c->tasks || c->tasks->which != ZOOM_TASK_SCAN)
        return;
    scan = c->tasks->u.scan.scan;

    if (res->num_diagnostics > 0)
        set_SRU_error(c, &res->diagnostics[0]);

    scan->scan_response = 0;
    scan->srw_scan_response = res;
    nmem_transfer(odr_getmem(scan->odr), nmem);

    ZOOM_options_set_int(scan->options, "number", res->num_terms);
    nmem_destroy(nmem);
}
#endif

int ZOOM_handle_sru(ZOOM_connection c, Z_HTTP_Response *hres,
                    zoom_ret *cret)
{
    int ret = 0;
    const char *addinfo = 0;

    /* not redirect (normal response) */
    if (!yaz_srw_check_content_type(hres))
    {
        addinfo = "content-type";
        ret = -1;
    }
    else if (c->sru_mode == zoom_sru_solr)
    {
        Z_SRW_PDU *sr;
        ret = yaz_solr_decode_response(c->odr_in, hres, &sr);
        if (ret == 0)
            if (sr->which == Z_SRW_searchRetrieve_response)
                *cret = handle_srw_response(c, sr->u.response);
    }
    else
    {
        Z_SOAP *soap_package = 0;
        ODR o = c->odr_in;
        Z_SOAP_Handler soap_handlers[2] = {
            {YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec},
            {0, 0, 0}
        };
        ret = z_soap_codec(o, &soap_package,
                           &hres->content_buf, &hres->content_len,
                           soap_handlers);
        if (!ret && soap_package->which == Z_SOAP_generic &&
            soap_package->u.generic->no == 0)
        {
            Z_SRW_PDU *sr = (Z_SRW_PDU*) soap_package->u.generic->p;
            
            ZOOM_options_set(c->options, "sru_version", sr->srw_version);
            ZOOM_options_setl(c->options, "sru_extra_response_data",
                              sr->extraResponseData_buf, sr->extraResponseData_len);
            if (sr->which == Z_SRW_searchRetrieve_response)
                *cret = handle_srw_response(c, sr->u.response);
            else if (sr->which == Z_SRW_scan_response)
                handle_srw_scan_response(c, sr->u.scan_response);
            else
                ret = -1;
        }
        else if (!ret && (soap_package->which == Z_SOAP_fault
                          || soap_package->which == Z_SOAP_error))
        {
            ZOOM_set_HTTP_error(c, hres->code,
                                soap_package->u.fault->fault_code,
                                soap_package->u.fault->fault_string);
        }
        else
            ret = -1;
    }   
    return ret;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

