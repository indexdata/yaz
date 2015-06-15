/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-sru.c
 * \brief Implements ZOOM SRU
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>
#include <errno.h>
#include "zoom-p.h"

#include <yaz/log.h>
#include <yaz/pquery.h>

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
    const char *database =  ZOOM_options_get(c->options, "databaseName");

    gdu = z_get_HTTP_Request_uri(c->odr_out, c->host_port,
                                 database, c->proxy_mode);

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
    return ZOOM_send_GDU(c, gdu);
}
#endif

#if YAZ_HAVE_XML2
static Z_SRW_PDU *ZOOM_srw_get_pdu(ZOOM_connection c, int type)
{
    Z_SRW_PDU *sr = yaz_srw_get_pdu(c->odr_out, type, c->sru_version);
    if (c->url_authentication && c->user)
    {
        Z_SRW_extra_arg **ea = &sr->extra_args;
        while (*ea)
            ea = &(*ea)->next;
        *ea = (Z_SRW_extra_arg *) odr_malloc(c->odr_out, sizeof(**ea));
        (*ea)->name = "x-username";
        (*ea)->value = c->user;
        ea = &(*ea)->next;
        if (c->password)
        {
            *ea = (Z_SRW_extra_arg *) odr_malloc(c->odr_out, sizeof(**ea));
            (*ea)->name = "x-password";
            (*ea)->value = c->password;
            ea = &(*ea)->next;
        }
        *ea = 0;
    }
    else
    {
        sr->username = c->user;
        sr->password = c->password;
    }
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
        sr->u.scan_request->queryType = "cql";
        sr->u.scan_request->scanClause =
            odr_strdup(c->odr_out, ZOOM_query_get_query_string(scan->query));
    }
    else if (z_query->which == Z_Query_type_1
             || z_query->which == Z_Query_type_101)
    {
        sr->u.scan_request->queryType = "pqf";
        sr->u.scan_request->scanClause =
            odr_strdup(c->odr_out, ZOOM_query_get_query_string(scan->query));
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
    int i;
    int *start, *count;
    ZOOM_resultset resultset = 0;
    Z_SRW_PDU *sr = 0;
    const char *option_val = 0;
    const char *schema = 0;
    Z_Query *z_query;
    Z_FacetList *facet_list = 0;

    if (c->error)                  /* don't continue on error */
        return zoom_complete;
    assert(c->tasks);
    if (c->tasks->which != ZOOM_TASK_SEARCH)
        return zoom_complete;

    resultset = c->tasks->u.search.resultset;

    ZOOM_memcached_search(c, resultset);

    if (!resultset->setname)
        resultset->setname = odr_strdup(resultset->odr, "default");
    ZOOM_options_set(resultset->options, "setname", resultset->setname);
    start = &c->tasks->u.search.start;
    count = &c->tasks->u.search.count;
    if (resultset->req_facets)
        facet_list = yaz_pqf_parse_facet_list(c->odr_out,
                                              resultset->req_facets);
    schema = c->tasks->u.search.schema;

    if (resultset->live_set)
    {
        if (*start >= resultset->size)
            return zoom_complete;
        if (*start + *count > resultset->size)
            *count = resultset->size - *start;
    }
    for (i = 0; i < *count; i++)
    {
        ZOOM_record rec =
            ZOOM_record_cache_lookup(resultset, i + *start,
                                     c->tasks->u.search.syntax,
                                     c->tasks->u.search.elementSetName,
                                     schema);
        if (!rec)
            break;
    }
    *start += i;
    *count -= i;

    if (*count == 0 && resultset->live_set)
        return zoom_complete;

    assert(resultset->query);

    sr = ZOOM_srw_get_pdu(c, Z_SRW_searchRetrieve_request);
    z_query = ZOOM_query_get_Z_Query(resultset->query);

    if (z_query->which == Z_Query_type_104
        && z_query->u.type_104->which == Z_External_CQL)
    {
        sr->u.request->queryType = "cql";
        sr->u.request->query = z_query->u.type_104->u.cql;
    }
    else if (z_query->which == Z_Query_type_1 && z_query->u.type_1)
    {
        sr->u.request->queryType = "pqf";
        sr->u.request->query =
            odr_strdup(c->odr_out,
                       ZOOM_query_get_query_string(resultset->query));
    }
    else
    {
        ZOOM_set_error(c, ZOOM_ERROR_UNSUPPORTED_QUERY, 0);
        return zoom_complete;
    }

    option_val = ZOOM_query_get_sru11(resultset->query);
    if (option_val)
    {
        sr->u.request->sort_type = Z_SRW_sort_type_sort;
        sr->u.request->sort.sortKeys = odr_strdup(c->odr_out, option_val);
    }
    sr->u.request->startRecord = odr_intdup(c->odr_out, *start + 1);
    sr->u.request->maximumRecords = odr_intdup(
        c->odr_out, (resultset->step > 0 && resultset->step < *count) ?
        resultset->step : *count);
    sr->u.request->recordSchema = odr_strdup_null(c->odr_out, schema);
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
    int *start, *count;
    int i;
    NMEM nmem;
    ZOOM_Event event;
    const char *syntax, *elementSetName, *schema;

    if (!c->tasks)
        return zoom_complete;

    if (c->tasks->which != ZOOM_TASK_SEARCH)
        return zoom_complete;

    resultset = c->tasks->u.search.resultset;
    start = &c->tasks->u.search.start;
    count = &c->tasks->u.search.count;
    syntax = c->tasks->u.search.syntax;
    elementSetName = c->tasks->u.search.elementSetName;
    schema = c->tasks->u.search.schema;

    if (resultset->live_set == 0)
    {
        event = ZOOM_Event_create(ZOOM_EVENT_RECV_SEARCH);
        ZOOM_connection_put_event(c, event);
    }
    if (res->facetList)
        ZOOM_handle_facet_list(resultset, res->facetList);

    resultset->size = 0;

    if (res->resultSetId)
        ZOOM_resultset_option_set(resultset, "resultSetId", res->resultSetId);

    yaz_log(c->log_details, "%p handle_srw_response got SRW response OK", c);

    if (res->num_diagnostics > 0)
    {
        resultset->live_set = 2;
        set_SRU_error(c, &res->diagnostics[0]);
    }
    else
    {
        if (res->numberOfRecords)
        {
            Z_OtherInformation *oi = 0;
            if (res->facetList)
            {
                ODR o = c->odr_in;
                Z_External *ext = (Z_External *)
                    odr_malloc(o, sizeof(*ext));

                ext->which = Z_External_userFacets;
                ext->u.facetList = res->facetList;
                ext->direct_reference =
                    odr_oiddup(o, yaz_oid_userinfo_facet_1);
                ext->indirect_reference = 0;
                ext->descriptor = 0;
                oi = (Z_OtherInformation *) odr_malloc(o, sizeof(*oi));
                oi->num_elements = 1;
                oi->list = (Z_OtherInformationUnit **)
                    odr_malloc(o, sizeof(*oi->list));
                oi->list[0] = (Z_OtherInformationUnit *)
                    odr_malloc(o, sizeof(**oi->list));
                oi->list[0]->category = 0;
                oi->list[0]->which = Z_OtherInfo_externallyDefinedInfo;
                oi->list[0]->information.externallyDefinedInfo = ext;
            }
            resultset->size = *res->numberOfRecords;
            ZOOM_memcached_hitcount(c, resultset, oi,
                                    res->resultCountPrecision ?
                                    res->resultCountPrecision : "exact");
        }
        resultset->live_set = 2;
        if (res->suggestions)
            ZOOM_resultset_option_set(resultset, "suggestions",
                                      res->suggestions);
        for (i = 0; i < res->num_records; i++)
        {
            int pos = c->tasks->u.search.start + i;
            Z_SRW_record *sru_rec;
            Z_SRW_diagnostic *diag = 0;
            int num_diag;

            /* only trust recordPosition if >= calculated position */
            if (res->records[i].recordPosition &&
                *res->records[i].recordPosition >= pos + 1)
                pos = *res->records[i].recordPosition - 1;

            if (!ZOOM_record_cache_lookup(resultset,
                                          pos,
                                          syntax, elementSetName, schema))
            {
                Z_NamePlusRecord *npr = (Z_NamePlusRecord *)
                    odr_malloc(c->odr_in, sizeof(Z_NamePlusRecord));
                sru_rec = &res->records[i];

                npr->databaseName = 0;
                npr->which = Z_NamePlusRecord_databaseRecord;
                npr->u.databaseRecord = (Z_External *)
                    odr_malloc(c->odr_in, sizeof(Z_External));
                npr->u.databaseRecord->descriptor = 0;
                npr->u.databaseRecord->direct_reference =
                    odr_oiddup(c->odr_in, yaz_oid_recsyn_xml);
                npr->u.databaseRecord->indirect_reference = 0;
                npr->u.databaseRecord->which = Z_External_octet;

                npr->u.databaseRecord->u.octet_aligned =
                    odr_create_Odr_oct(c->odr_in,
                                       sru_rec->recordData_buf,
                                   sru_rec->recordData_len);
                if (sru_rec->recordSchema
                    && !strcmp(sru_rec->recordSchema,
                               "info:srw/schema/1/diagnostics-v1.1"))
                {
                    sru_decode_surrogate_diagnostics(sru_rec->recordData_buf,
                                                     sru_rec->recordData_len,
                                                     &diag, &num_diag,
                                                     resultset->odr);
                }
                ZOOM_record_cache_add(resultset, npr,
                                      pos, syntax, elementSetName,
                                      schema, diag);
            }
        }
        *count -= i;
        if (*count < 0)
            *count = 0;
        *start += i;
        nmem = odr_extract_mem(c->odr_in);
        nmem_transfer(odr_getmem(resultset->odr), nmem);
        nmem_destroy(nmem);

        return ZOOM_connection_srw_send_search(c);
    }
    return zoom_complete;
}
#endif

#if YAZ_HAVE_XML2
static zoom_ret handle_srw_scan_response(ZOOM_connection c,
                                         Z_SRW_scanResponse *res)
{
    NMEM nmem = odr_extract_mem(c->odr_in);
    ZOOM_scanset scan;

    if (!c->tasks || c->tasks->which != ZOOM_TASK_SCAN)
        return zoom_complete;
    scan = c->tasks->u.scan.scan;

    if (res->num_diagnostics > 0)
        set_SRU_error(c, &res->diagnostics[0]);

    scan->scan_response = 0;
    scan->srw_scan_response = res;
    nmem_transfer(odr_getmem(scan->odr), nmem);

    ZOOM_options_set_int(scan->options, "number", res->num_terms);
    nmem_destroy(nmem);
    return zoom_complete;
}
#endif

int ZOOM_handle_sru(ZOOM_connection c, Z_HTTP_Response *hres,
                    zoom_ret *cret, char **addinfo)
{
#if YAZ_HAVE_XML2
    int ret = 0;

    /* not redirect (normal response) */
    if (!yaz_srw_check_content_type(hres))
    {
        *addinfo = "content-type";
        ret = -1;
    }
    else if (c->sru_mode == zoom_sru_solr)
    {
        Z_SRW_PDU *sr;
        ret = yaz_solr_decode_response(c->odr_in, hres, &sr);
        if (ret == 0)
        {
            if (sr->which == Z_SRW_searchRetrieve_response)
                *cret = handle_srw_response(c, sr->u.response);
            else if (sr->which == Z_SRW_scan_response)
                *cret = handle_srw_scan_response(c, sr->u.scan_response);
        }
    }
    else
    {
        Z_SOAP *soap_package = 0;
        ODR o = c->odr_in;
        Z_SOAP_Handler soap_handlers[4] = {
            {YAZ_XMLNS_SRU_v1_response, 0, (Z_SOAP_fun) yaz_srw_codec},
            {YAZ_XMLNS_SRU_v2_mask, 0, (Z_SOAP_fun) yaz_srw_codec},
            {"searchRetrieveResponse", 0, (Z_SOAP_fun) yaz_srw_codec},
            {0, 0, 0}
        };
        ret = z_soap_codec(o, &soap_package,
                           &hres->content_buf, &hres->content_len,
                           soap_handlers);
        if (!ret && soap_package->which == Z_SOAP_generic)
        {
            Z_SRW_PDU *sr = (Z_SRW_PDU*) soap_package->u.generic->p;

            ZOOM_options_set(c->options, "sru_version", sr->srw_version);
            ZOOM_options_setl(c->options, "sru_extra_response_data",
                              sr->extraResponseData_buf, sr->extraResponseData_len);
            if (sr->which == Z_SRW_searchRetrieve_response)
                *cret = handle_srw_response(c, sr->u.response);
            else if (sr->which == Z_SRW_scan_response)
                *cret = handle_srw_scan_response(c, sr->u.scan_response);
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
        {
            size_t max_chars = 1000;
            size_t sz = hres->content_len;
            if (sz > max_chars - 1)
                sz = max_chars;
            *addinfo = odr_malloc(c->odr_in, sz + 4);
            memcpy(*addinfo, hres->content_buf, sz);
            if (sz == max_chars)
                strcpy(*addinfo + sz, "...");
            else
                strcpy(*addinfo + sz, "");
            ret = -1;
        }
    }
    return ret;
#else
    return -1;
#endif
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

