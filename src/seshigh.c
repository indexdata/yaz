/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file seshigh.c
 * \brief Implements GFS session logic.
 *
 * Frontend server logic.
 *
 * This code receives incoming APDUs, and handles client requests by means
 * of the backend API.
 *
 * Some of the code is getting quite involved, compared to simpler servers -
 * primarily because it is asynchronous both in the communication with
 * the user and the backend. We think the complexity will pay off in
 * the form of greater flexibility when more asynchronous facilities
 * are implemented.
 *
 * Memory management has become somewhat involved. In the simple case, where
 * only one PDU is pending at a time, it will simply reuse the same memory,
 * once it has found its working size. When we enable multiple concurrent
 * operations, perhaps even with multiple parallel calls to the backend, it
 * will maintain a pool of buffers for encoding and decoding, trying to
 * minimize memory allocation/deallocation during normal operation.
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef WIN32
#include <io.h>
#include <sys/stat.h>
#define S_ISREG(x) (x & _S_IFREG)
#include <process.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

#include <yaz/facet.h>
#include <yaz/xmalloc.h>
#include <yaz/comstack.h>
#include "eventl.h"
#include "session.h"
#include "mime.h"
#include <yaz/proto.h>
#include <yaz/oid_db.h>
#include <yaz/log.h>
#include <yaz/logrpn.h>
#include <yaz/querytowrbuf.h>
#include <yaz/statserv.h>
#include <yaz/diagbib1.h>
#include <yaz/charneg.h>
#include <yaz/otherinfo.h>
#include <yaz/yaz-util.h>
#include <yaz/pquery.h>
#include <yaz/oid_db.h>
#include <yaz/query-charset.h>
#include <yaz/srw.h>
#include <yaz/backend.h>
#include <yaz/yaz-ccl.h>

static void process_gdu_request(association *assoc, request *req);
static int process_z_request(association *assoc, request *req, char **msg);
static int process_gdu_response(association *assoc, request *req, Z_GDU *res);
static int process_z_response(association *assoc, request *req, Z_APDU *res);
static Z_APDU *process_initRequest(association *assoc, request *reqb);
static Z_External *init_diagnostics(ODR odr, int errcode,
                                    const char *errstring);
static Z_APDU *process_searchRequest(association *assoc, request *reqb);
static Z_APDU *response_searchRequest(association *assoc, request *reqb,
                                      bend_search_rr *bsrr);
static Z_APDU *process_presentRequest(association *assoc, request *reqb);
static Z_APDU *process_scanRequest(association *assoc, request *reqb);
static Z_APDU *process_sortRequest(association *assoc, request *reqb);
static void process_close(association *assoc, request *reqb);
static Z_APDU *process_deleteRequest(association *assoc, request *reqb);
static Z_APDU *process_segmentRequest(association *assoc, request *reqb);
static Z_APDU *process_ESRequest(association *assoc, request *reqb);

/* dynamic logging levels */
static int logbits_set = 0;
static int log_session = 0; /* one-line logs for session */
static int log_sessiondetail = 0; /* more detailed stuff */
static int log_request = 0; /* one-line logs for requests */
static int log_requestdetail = 0;  /* more detailed stuff */

/** get_logbits sets global loglevel bits */
static void get_logbits(void)
{ /* needs to be called after parsing cmd-line args that can set loglevels!*/
    if (!logbits_set)
    {
        logbits_set = 1;
        log_session = yaz_log_module_level("session");
        log_sessiondetail = yaz_log_module_level("sessiondetail");
        log_request = yaz_log_module_level("request");
        log_requestdetail = yaz_log_module_level("requestdetail");
    }
}

static void wr_diag(WRBUF w, int error, const char *addinfo)
{
    wrbuf_printf(w, "ERROR %d+", error);
    wrbuf_puts_replace_char(w, diagbib1_str(error), ' ', '_');
    if (addinfo)
    {
        wrbuf_puts(w, "+");
        wrbuf_puts_replace_char(w, addinfo, ' ', '_');
    }
    wrbuf_puts(w, " ");
}

static int odr_int_to_int(Odr_int v)
{
    if (v >= INT_MAX)
        return INT_MAX;
    else if (v <= INT_MIN)
        return INT_MIN;
    else
        return (int) v;
}

/*
 * Create and initialize a new association-handle.
 *  channel  : iochannel for the current line.
 *  link     : communications channel.
 * Returns: 0 or a new association handle.
 */
association *create_association(IOCHAN channel, COMSTACK link,
                                const char *apdufile)
{
    association *anew;

    if (!logbits_set)
        get_logbits();
    if (!(anew = (association *)xmalloc(sizeof(*anew))))
        return 0;
    anew->init = 0;
    anew->version = 0;
    anew->last_control = 0;
    anew->client_chan = channel;
    anew->client_link = link;
    anew->cs_get_mask = 0;
    anew->cs_put_mask = 0;
    anew->cs_accept_mask = 0;
    if (!(anew->decode = odr_createmem(ODR_DECODE)) ||
        !(anew->encode = odr_createmem(ODR_ENCODE)))
        return 0;
    if (apdufile && *apdufile)
    {
        FILE *f;

        if (!(anew->print = odr_createmem(ODR_PRINT)))
            return 0;
        if (*apdufile == '@')
        {
            odr_setprint(anew->print, yaz_log_file());
        }
        else if (*apdufile != '-')
        {
            char filename[256];
            sprintf(filename, "%.200s.%ld", apdufile, (long)getpid());
            if (!(f = fopen(filename, "w")))
            {
                yaz_log(YLOG_WARN|YLOG_ERRNO, "%s", filename);
                return 0;
            }
            setvbuf(f, 0, _IONBF, 0);
            odr_setprint(anew->print, f);
        }
    }
    else
        anew->print = 0;
    anew->input_buffer = 0;
    anew->input_buffer_len = 0;
    anew->backend = 0;
    anew->state = ASSOC_NEW;
    request_initq(&anew->incoming);
    request_initq(&anew->outgoing);
    anew->proto = cs_getproto(link);
    anew->server = 0;
    return anew;
}

/*
 * Free association and release resources.
 */
void destroy_association(association *h)
{
    statserv_options_block *cb = statserv_getcontrol();
    request *req;

    xfree(h->init);
    odr_destroy(h->decode);
    odr_destroy(h->encode);
    if (h->print)
        odr_destroy(h->print);
    if (h->input_buffer)
    xfree(h->input_buffer);
    if (h->backend)
        (*cb->bend_close)(h->backend);
    while ((req = request_deq(&h->incoming)))
        request_release(req);
    while ((req = request_deq(&h->outgoing)))
        request_release(req);
    request_delq(&h->incoming);
    request_delq(&h->outgoing);
    xfree(h);
    xmalloc_trav("session closed");
}

static void do_close_req(association *a, int reason, char *message,
                         request *req)
{
    Z_APDU *apdu = zget_APDU(a->encode, Z_APDU_close);
    Z_Close *cls = apdu->u.close;

    /* Purge request queue */
    while (request_deq(&a->incoming));
    while (request_deq(&a->outgoing));
    if (a->version >= 3)
    {
        yaz_log(log_requestdetail, "Sending Close PDU, reason=%d, message=%s",
            reason, message ? message : "none");
        *cls->closeReason = reason;
        cls->diagnosticInformation = message;
        process_z_response(a, req, apdu);
        iochan_settimeout(a->client_chan, 20);
    }
    else
    {
        request_release(req);
        yaz_log(log_requestdetail, "v2 client. No Close PDU");
        iochan_setevent(a->client_chan, EVENT_TIMEOUT); /* force imm close */
        a->cs_put_mask = 0;
    }
    a->state = ASSOC_DEAD;
}

static void do_close(association *a, int reason, char *message)
{
    request *req = request_get(&a->outgoing);
    do_close_req(a, reason, message, req);
}


int ir_read(IOCHAN h, int event)
{
    association *assoc = (association *)iochan_getdata(h);
    COMSTACK conn = assoc->client_link;
    request *req;

    if ((assoc->cs_put_mask & EVENT_INPUT) == 0 && (event & assoc->cs_get_mask))
    {
        /* We aren't speaking to this fellow */
        if (assoc->state == ASSOC_DEAD)
        {
            yaz_log(log_session, "Connection closed - end of session");
            cs_close(conn);
            destroy_association(assoc);
            iochan_destroy(h);
            return 0;
        }
        assoc->cs_get_mask = EVENT_INPUT;

        do
        {
            int res = cs_get(conn, &assoc->input_buffer,
                             &assoc->input_buffer_len);
            if (res < 0 && cs_errno(conn) == CSBUFSIZE)
            {
                yaz_log(log_session, "Connection error: %s res=%d",
                        cs_errmsg(cs_errno(conn)), res);
                req = request_get(&assoc->incoming); /* get a new request */
                do_close_req(assoc, Z_Close_protocolError,
                             "Incoming package too large", req);
                return 0;
            }
            else if (res <= 0)
            {
                assoc->state = ASSOC_DEAD;
                yaz_log(log_session, "Connection closed by client");
                return 0;
            }
            else if (res == 1) /* incomplete read - wait for more  */
            {
                if (conn->io_pending & CS_WANT_WRITE)
                    assoc->cs_get_mask |= EVENT_OUTPUT;
                iochan_setflag(h, assoc->cs_get_mask);
                return 0;
            }
            /* we got a complete PDU. Let's decode it */
            yaz_log(YLOG_DEBUG, "Got PDU, %d bytes: lead=%02X %02X %02X", res,
                    assoc->input_buffer[0] & 0xff,
                    assoc->input_buffer[1] & 0xff,
                    assoc->input_buffer[2] & 0xff);
            req = request_get(&assoc->incoming); /* get a new request */
            odr_reset(assoc->decode);
            odr_setbuf(assoc->decode, assoc->input_buffer, res, 0);
            if (!z_GDU(assoc->decode, &req->gdu_request, 0, 0))
            {
                yaz_log(YLOG_WARN, "ODR error on incoming PDU: %s [element %s] "
                        "[near byte %ld] ",
                        odr_errmsg(odr_geterror(assoc->decode)),
                        odr_getelement(assoc->decode),
                        (long) odr_offset(assoc->decode));
                if (assoc->decode->error != OHTTP)
                {
                    yaz_log(YLOG_WARN, "PDU dump:");
                    odr_dumpBER(yaz_log_file(), assoc->input_buffer, res);
                    request_release(req);
                    do_close(assoc, Z_Close_protocolError, "Malformed package");
                }
                else
                {
                    Z_GDU *p = z_get_HTTP_Response(assoc->encode, 400);
                    assoc->state = ASSOC_DEAD;
                    process_gdu_response(assoc, req, p);
                }
                return 0;
            }
            req->request_mem = odr_extract_mem(assoc->decode);
            if (assoc->print)
            {
                if (!z_GDU(assoc->print, &req->gdu_request, 0, 0))
                    yaz_log(YLOG_WARN, "ODR print error: %s",
                            odr_errmsg(odr_geterror(assoc->print)));
                odr_reset(assoc->print);
            }
            request_enq(&assoc->incoming, req);
        }
        while (cs_more(conn));
    }
    return 1;
}

/*
 * This is where PDUs from the client are read and the further
 * processing is initiated. Flow of control moves down through the
 * various process_* functions below, until the encoded result comes back up
 * to the output handler in here.
 *
 *  h     : the I/O channel that has an outstanding event.
 *  event : the current outstanding event.
 */
void ir_session(IOCHAN h, int event)
{
    int res;
    association *assoc = (association *)iochan_getdata(h);
    COMSTACK conn = assoc->client_link;
    request *req;

    assert(h && conn && assoc);
    if (event == EVENT_TIMEOUT)
    {
        if (assoc->state != ASSOC_UP)
        {
            yaz_log(log_session, "Timeout. Closing connection");
            /* do we need to lod this at all */
            cs_close(conn);
            destroy_association(assoc);
            iochan_destroy(h);
        }
        else
        {
            yaz_log(log_sessiondetail, "Timeout. Sending Z39.50 Close");
            do_close(assoc, Z_Close_lackOfActivity, 0);
        }
        return;
    }
    if (event & assoc->cs_accept_mask)
    {
        if (!cs_accept(conn))
        {
            yaz_log(YLOG_WARN, "accept failed");
            destroy_association(assoc);
            iochan_destroy(h);
            return;
        }
        iochan_clearflag(h, EVENT_OUTPUT);
        if (conn->io_pending)
        {   /* cs_accept didn't complete */
            assoc->cs_accept_mask =
                ((conn->io_pending & CS_WANT_WRITE) ? EVENT_OUTPUT : 0) |
                ((conn->io_pending & CS_WANT_READ) ? EVENT_INPUT : 0);

            iochan_setflag(h, assoc->cs_accept_mask);
        }
        else
        {   /* cs_accept completed. Prepare for reading (cs_get) */
            assoc->cs_accept_mask = 0;
            assoc->cs_get_mask = EVENT_INPUT;
            iochan_setflag(h, assoc->cs_get_mask);
        }
        return;
    }
    if (event & assoc->cs_get_mask) /* input */
    {
        if (!ir_read(h, event))
            return;
        req = request_head(&assoc->incoming);
        if (req->state == REQUEST_IDLE)
        {
            request_deq(&assoc->incoming);
            process_gdu_request(assoc, req);
        }
    }
    if (event & assoc->cs_put_mask)
    {
        request *req = request_head(&assoc->outgoing);

        assoc->cs_put_mask = 0;
        yaz_log(YLOG_DEBUG, "ir_session (output)");
        req->state = REQUEST_PENDING;
        switch (res = cs_put(conn, req->response, req->len_response))
        {
        case -1:
            yaz_log(log_sessiondetail, "Connection closed by client");
            cs_close(conn);
            destroy_association(assoc);
            iochan_destroy(h);
            break;
        case 0: /* all sent - release the request structure */
            yaz_log(YLOG_DEBUG, "Wrote PDU, %d bytes", req->len_response);
#if 0
            yaz_log(YLOG_DEBUG, "HTTP out:\n%.*s", req->len_response,
                    req->response);
#endif
            request_deq(&assoc->outgoing);
            request_release(req);
            if (!request_head(&assoc->outgoing))
            {   /* restore mask for cs_get operation ... */
                iochan_clearflag(h, EVENT_OUTPUT|EVENT_INPUT);
                iochan_setflag(h, assoc->cs_get_mask);
                if (assoc->state == ASSOC_DEAD)
                    iochan_setevent(assoc->client_chan, EVENT_TIMEOUT);
            }
            else
            {
                assoc->cs_put_mask = EVENT_OUTPUT;
            }
            break;
        default:
            if (conn->io_pending & CS_WANT_WRITE)
                assoc->cs_put_mask |= EVENT_OUTPUT;
            if (conn->io_pending & CS_WANT_READ)
                assoc->cs_put_mask |= EVENT_INPUT;
            iochan_setflag(h, assoc->cs_put_mask);
        }
    }
    if (event & EVENT_EXCEPT)
    {
        if (assoc->state != ASSOC_DEAD)
            yaz_log(YLOG_WARN, "ir_session (exception)");
        cs_close(conn);
        destroy_association(assoc);
        iochan_destroy(h);
    }
}

static int process_z_request(association *assoc, request *req, char **msg);


static void assoc_init_reset(association *assoc, const char *peer_name1)
{
    const char *peer_name2 = cs_addrstr(assoc->client_link);

    xfree(assoc->init);
    assoc->init = (bend_initrequest *) xmalloc(sizeof(*assoc->init));

    assoc->init->stream = assoc->encode;
    assoc->init->print = assoc->print;
    assoc->init->auth = 0;
    assoc->init->referenceId = 0;
    assoc->init->implementation_version = 0;
    assoc->init->implementation_id = 0;
    assoc->init->implementation_name = 0;
    assoc->init->query_charset = 0;
    assoc->init->records_in_same_charset = 0;
    assoc->init->bend_sort = NULL;
    assoc->init->bend_search = NULL;
    assoc->init->bend_present = NULL;
    assoc->init->bend_esrequest = NULL;
    assoc->init->bend_delete = NULL;
    assoc->init->bend_scan = NULL;
    assoc->init->bend_segment = NULL;
    assoc->init->bend_fetch = NULL;
    assoc->init->bend_explain = NULL;
    assoc->init->bend_srw_scan = NULL;
    assoc->init->bend_srw_update = NULL;
    assoc->init->named_result_sets = 0;

    assoc->init->charneg_request = NULL;
    assoc->init->charneg_response = NULL;

    assoc->init->decode = assoc->decode;

    assoc->init->peer_name = (char *)
        odr_malloc(assoc->encode,
                   (peer_name1 ? strlen(peer_name1) : 0)
                   + 4 + strlen(peer_name2));
    strcpy(assoc->init->peer_name, "");
    if (peer_name1)
    {
        strcat(assoc->init->peer_name, peer_name1);
        strcat(assoc->init->peer_name, ", ");
    }
    strcat(assoc->init->peer_name, peer_name2);

    yaz_log(log_requestdetail, "peer %s", assoc->init->peer_name);
}

static int srw_bend_init(association *assoc, Z_HTTP_Header *headers,
                         Z_SRW_diagnostic **d, int *num, Z_SRW_PDU *sr)
{
    statserv_options_block *cb = statserv_getcontrol();
    if (!assoc->init)
    {
        const char *encoding = "UTF-8";
        Z_External *ce;
        bend_initresult *binitres;

        yaz_log(log_requestdetail, "srw_bend_init config=%s", cb->configname);
        assoc_init_reset(assoc, z_HTTP_header_lookup(headers, "X-Forwarded-For"));

        if (sr->username)
        {
            Z_IdAuthentication *auth = (Z_IdAuthentication *)
                odr_malloc(assoc->decode, sizeof(*auth));
            size_t len;

            len = strlen(sr->username) + 1;
            if (sr->password)
                len += strlen(sr->password) + 2;
            yaz_log(log_requestdetail, "username=%s password-len=%ld",
                    sr->username, (long)
                    (sr->password ? strlen(sr->password) : 0));
            auth->which = Z_IdAuthentication_open;
            auth->u.open = (char *) odr_malloc(assoc->decode, len);
            strcpy(auth->u.open, sr->username);
            if (sr->password && *sr->password)
            {
                strcat(auth->u.open, "/");
                strcat(auth->u.open, sr->password);
            }
            assoc->init->auth = auth;
        }

#if 1
        ce = yaz_set_proposal_charneg(assoc->decode, &encoding, 1, 0, 0, 1);
        assoc->init->charneg_request = ce->u.charNeg3;
#endif
        assoc->backend = 0;
        if (!(binitres = (*cb->bend_init)(assoc->init)))
        {
            assoc->state = ASSOC_DEAD;
            yaz_add_srw_diagnostic(assoc->encode, d, num,
                            YAZ_SRW_AUTHENTICATION_ERROR, 0);
            return 0;
        }
        assoc->backend = binitres->handle;
        assoc->init->auth = 0;
        if (binitres->errcode)
        {
            int srw_code = yaz_diag_bib1_to_srw(binitres->errcode);
            assoc->state = ASSOC_DEAD;
            yaz_add_srw_diagnostic(assoc->encode, d, num, srw_code,
                                   binitres->errstring);
            return 0;
        }
        return 1;
    }
    return 1;
}

static int retrieve_fetch(association *assoc, bend_fetch_rr *rr)
{
#if YAZ_HAVE_XML2
    yaz_record_conv_t rc = 0;
    const char *match_schema = 0;
    Odr_oid *match_syntax = 0;

    if (assoc->server)
    {
        int r;
        const char *input_schema = yaz_get_esn(rr->comp);
        Odr_oid *input_syntax_raw = rr->request_format;

        const char *backend_schema = 0;
        Odr_oid *backend_syntax = 0;

        r = yaz_retrieval_request(assoc->server->retrieval,
                                  input_schema,
                                  input_syntax_raw,
                                  &match_schema,
                                  &match_syntax,
                                  &rc,
                                  &backend_schema,
                                  &backend_syntax);
        if (r == -1) /* error ? */
        {
            const char *details = yaz_retrieval_get_error(
                assoc->server->retrieval);

            rr->errcode = YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS;
            if (details)
                rr->errstring = odr_strdup(rr->stream, details);
            return -1;
        }
        else if (r == 1 || r == 3)
        {
            const char *details = input_schema;
            rr->errcode =
                YAZ_BIB1_SPECIFIED_ELEMENT_SET_NAME_NOT_VALID_FOR_SPECIFIED_;
            if (details)
                rr->errstring = odr_strdup(rr->stream, details);
            return -1;
        }
        else if (r == 2)
        {
            rr->errcode = YAZ_BIB1_RECORD_SYNTAX_UNSUPP;
            if (input_syntax_raw)
            {
                char oidbuf[OID_STR_MAX];
                oid_oid_to_dotstring(input_syntax_raw, oidbuf);
                rr->errstring = odr_strdup(rr->stream, oidbuf);
            }
            return -1;
        }
        if (backend_schema)
        {
            yaz_set_esn(&rr->comp, backend_schema, odr_getmem(rr->stream));
        }
        if (backend_syntax)
            rr->request_format = backend_syntax;
    }
    (*assoc->init->bend_fetch)(assoc->backend, rr);
    if (rc && rr->record && rr->errcode == 0)
    {   /* post conversion must take place .. */
        WRBUF output_record = wrbuf_alloc();
        int r = 1;
        const char *details = 0;
        if (rr->len > 0)
        {
            r = yaz_record_conv_record(rc, rr->record, rr->len, output_record);
            if (r)
                details = yaz_record_conv_get_error(rc);
        }
        else if (rr->len == -1 && rr->output_format &&
                 !oid_oidcmp(rr->output_format, yaz_oid_recsyn_opac))
        {
            r = yaz_record_conv_opac_record(
                rc, (Z_OPACRecord *) rr->record, output_record);
            if (r)
                details = yaz_record_conv_get_error(rc);
        }
        if (r == 0 && match_syntax &&
            !oid_oidcmp(match_syntax, yaz_oid_recsyn_opac))
        {
            yaz_marc_t mt = yaz_marc_create();
            Z_OPACRecord *opac = 0;
            if (yaz_xml_to_opac(mt, wrbuf_buf(output_record),
                                wrbuf_len(output_record),
                                &opac, 0 /* iconv */, rr->stream->mem, 0)
                && opac)
            {
                rr->len = -1;
                rr->record = (char *) opac;
            }
            else
            {
                details = "XML to OPAC conversion failed";
                r = 1;
            }
            yaz_marc_destroy(mt);
        }
        else if (r == 0)
        {
            rr->len = wrbuf_len(output_record);
            rr->record = (char *) odr_malloc(rr->stream, rr->len);
            memcpy(rr->record, wrbuf_buf(output_record), rr->len);
        }
        if (r)
        {
            rr->errcode = YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS;
            rr->surrogate_flag = 1;
            if (details)
                rr->errstring = odr_strdup(rr->stream, details);
        }
        wrbuf_destroy(output_record);
    }
    if (match_syntax)
        rr->output_format = match_syntax;
    if (match_schema)
        rr->schema = odr_strdup(rr->stream, match_schema);
#else
    (*assoc->init->bend_fetch)(assoc->backend, rr);
#endif
    return 0;
}

static int srw_bend_fetch(association *assoc, int pos,
                          Z_SRW_searchRetrieveRequest *srw_req,
                          Z_SRW_record *record,
                          const char **addinfo, int *last_in_set)
{
    bend_fetch_rr rr;
    ODR o = assoc->encode;

    rr.setname = "default";
    rr.number = pos;
    rr.referenceId = 0;
    rr.request_format = odr_oiddup(assoc->decode, yaz_oid_recsyn_xml);

    rr.comp = (Z_RecordComposition *)
            odr_malloc(assoc->decode, sizeof(*rr.comp));
    rr.comp->which = Z_RecordComp_complex;
    rr.comp->u.complex = (Z_CompSpec *)
            odr_malloc(assoc->decode, sizeof(Z_CompSpec));
    rr.comp->u.complex->selectAlternativeSyntax = (bool_t *)
        odr_malloc(assoc->encode, sizeof(bool_t));
    *rr.comp->u.complex->selectAlternativeSyntax = 0;
    rr.comp->u.complex->num_dbSpecific = 0;
    rr.comp->u.complex->dbSpecific = 0;
    rr.comp->u.complex->num_recordSyntax = 0;
    rr.comp->u.complex->recordSyntax = 0;

    rr.comp->u.complex->generic = (Z_Specification *)
            odr_malloc(assoc->decode, sizeof(Z_Specification));

    /* schema uri = recordSchema (or NULL if recordSchema is not given) */
    rr.comp->u.complex->generic->which = Z_Schema_uri;
    rr.comp->u.complex->generic->schema.uri = srw_req->recordSchema;

    /* ESN = recordSchema if recordSchema is present */
    rr.comp->u.complex->generic->elementSpec = 0;
    if (srw_req->recordSchema)
    {
        rr.comp->u.complex->generic->elementSpec =
            (Z_ElementSpec *) odr_malloc(assoc->encode, sizeof(Z_ElementSpec));
        rr.comp->u.complex->generic->elementSpec->which =
            Z_ElementSpec_elementSetName;
        rr.comp->u.complex->generic->elementSpec->u.elementSetName =
            srw_req->recordSchema;
    }

    rr.stream = assoc->encode;
    rr.print = assoc->print;

    rr.basename = 0;
    rr.len = 0;
    rr.record = 0;
    rr.last_in_set = 0;
    rr.errcode = 0;
    rr.errstring = 0;
    rr.surrogate_flag = 0;
    rr.schema = srw_req->recordSchema;

    if (!assoc->init->bend_fetch)
        return 1;

    retrieve_fetch(assoc, &rr);

    *last_in_set = rr.last_in_set;

    if (rr.errcode && rr.surrogate_flag)
    {
        int code = yaz_diag_bib1_to_srw(rr.errcode);
        yaz_mk_sru_surrogate(o, record, pos, code, rr.errstring);
        return 0;
    }
    else if (rr.len >= 0)
    {
        record->recordData_buf = rr.record;
        record->recordData_len = rr.len;
        record->recordPosition = odr_intdup(o, pos);
        record->recordSchema = odr_strdup_null(
            o, rr.schema ? rr.schema : srw_req->recordSchema);
    }
    if (rr.errcode)
    {
        *addinfo = rr.errstring;
        return rr.errcode;
    }
    return 0;
}

static int cql2pqf(ODR odr, const char *cql, cql_transform_t ct,
                   Z_Query *query_result, char **sortkeys_p)
{
    /* have a CQL query and  CQL to PQF transform .. */
    CQL_parser cp = cql_parser_create();
    int r;
    int srw_errcode = 0;
    const char *add = 0;
    WRBUF rpn_buf = wrbuf_alloc();

    *sortkeys_p = 0;
    r = cql_parser_string(cp, cql);
    if (r)
    {
        srw_errcode = YAZ_SRW_QUERY_SYNTAX_ERROR;
    }
    if (!r)
    {
        struct cql_node *cn = cql_parser_result(cp);

        /* Syntax OK */
        r = cql_transform(ct, cn, wrbuf_vp_puts, rpn_buf);
        if (r)
            srw_errcode = cql_transform_error(ct, &add);
        else
        {
            char out[100];
            int r = cql_sortby_to_sortkeys_buf(cn, out, sizeof(out)-1);

            if (r == 0)
            {
                if (*out)
                    yaz_log(log_requestdetail, "srw_sortKeys '%s'", out);
                *sortkeys_p = odr_strdup(odr, out);
            }
            else
            {
                yaz_log(log_requestdetail, "failed to create srw_sortKeys");
                srw_errcode = YAZ_SRW_UNSUPP_SORT_TYPE;
            }
        }
    }
    if (!r)
    {
        /* Syntax & transform OK. */
        /* Convert PQF string to Z39.50 to RPN query struct */
        YAZ_PQF_Parser pp = yaz_pqf_create();
        Z_RPNQuery *rpnquery = yaz_pqf_parse(pp, odr, wrbuf_cstr(rpn_buf));
        if (!rpnquery)
        {
            size_t off;
            const char *pqf_msg;
            int code = yaz_pqf_error(pp, &pqf_msg, &off);
            yaz_log(YLOG_WARN, "PQF Parser Error %s (code %d)",
                    pqf_msg, code);
            srw_errcode = YAZ_SRW_QUERY_SYNTAX_ERROR;
        }
        else
        {
            query_result->which = Z_Query_type_1;
            query_result->u.type_1 = rpnquery;
        }
        yaz_pqf_destroy(pp);
    }
    cql_parser_destroy(cp);
    wrbuf_destroy(rpn_buf);
    return srw_errcode;
}

static int cql2pqf_scan(ODR odr, const char *cql, cql_transform_t ct,
                        Z_AttributesPlusTerm *result)
{
    Z_Query query;
    Z_RPNQuery *rpn;
    char *sortkeys = 0;
    int srw_error = cql2pqf(odr, cql, ct, &query, &sortkeys);
    if (srw_error)
        return srw_error;
    if (query.which != Z_Query_type_1 && query.which != Z_Query_type_101)
        return YAZ_SRW_QUERY_SYNTAX_ERROR; /* bad query type */
    rpn = query.u.type_1;
    if (!rpn->RPNStructure)
        return YAZ_SRW_QUERY_SYNTAX_ERROR; /* must be structure */
    if (rpn->RPNStructure->which != Z_RPNStructure_simple)
        return YAZ_SRW_QUERY_SYNTAX_ERROR; /* must be simple */
    if (rpn->RPNStructure->u.simple->which != Z_Operand_APT)
        return YAZ_SRW_QUERY_SYNTAX_ERROR; /* must be be attributes + term */
    memcpy(result, rpn->RPNStructure->u.simple->u.attributesPlusTerm,
           sizeof(*result));
    return 0;
}


static int ccl2pqf(ODR odr, const Odr_oct *ccl, CCL_bibset bibset,
                   bend_search_rr *bsrr)
{
    char *ccl0;
    struct ccl_rpn_node *node;
    int errcode, pos;

    ccl0 = odr_strdupn(odr, (char*) ccl->buf, ccl->len);
    if ((node = ccl_find_str(bibset, ccl0, &errcode, &pos)) == 0)
    {
        bsrr->errstring = (char*) ccl_err_msg(errcode);
        return YAZ_SRW_QUERY_SYNTAX_ERROR;    /* Query syntax error */
    }

    bsrr->query->which = Z_Query_type_1;
    bsrr->query->u.type_1 = ccl_rpn_query(odr, node);
    return 0;
}

static void srw_bend_search(association *assoc,
                            Z_HTTP_Header *headers,
                            Z_SRW_PDU *sr,
                            Z_SRW_PDU *res,
                            int *http_code)
{
    Z_SRW_searchRetrieveResponse *srw_res = res->u.response;
    int srw_error = 0;
    Z_External *ext;
    Z_SRW_searchRetrieveRequest *srw_req = sr->u.request;

    *http_code = 200;
    yaz_log(log_requestdetail, "Got SRW SearchRetrieveRequest");
    srw_bend_init(assoc, headers,
                  &srw_res->diagnostics, &srw_res->num_diagnostics, sr);
    if (srw_res->num_diagnostics == 0 && assoc->init)
    {
        bend_search_rr rr;
        rr.setname = "default";
        rr.replace_set = 1;
        rr.num_bases = 1;
        rr.basenames = &srw_req->database;
        rr.referenceId = 0;
        rr.srw_sortKeys = 0;
        rr.srw_setname = 0;
        rr.srw_setnameIdleTime = 0;
        rr.estimated_hit_count = 0;
        rr.partial_resultset = 0;
        rr.query = (Z_Query *) odr_malloc(assoc->decode, sizeof(*rr.query));
        rr.query->u.type_1 = 0;
        rr.extra_args = sr->extra_args;
        rr.extra_response_data = 0;
        rr.present_number = srw_req->maximumRecords ?
            *srw_req->maximumRecords : 0;

        if (!srw_req->queryType || !strcmp(srw_req->queryType, "cql"))
        {
            if (assoc->server && assoc->server->cql_transform)
            {
                int srw_errcode = cql2pqf(assoc->encode, srw_req->query,
                                          assoc->server->cql_transform,
                                          rr.query,
                                          &rr.srw_sortKeys);

                if (srw_errcode)
                {
                    yaz_add_srw_diagnostic(assoc->encode,
                                           &srw_res->diagnostics,
                                           &srw_res->num_diagnostics,
                                           srw_errcode, 0);
                }
            }
            else
            {
                /* CQL query to backend. Wrap it - Z39.50 style */
                ext = (Z_External *) odr_malloc(assoc->decode, sizeof(*ext));
                ext->direct_reference = odr_getoidbystr(assoc->decode,
                                                        "1.2.840.10003.16.2");
                ext->indirect_reference = 0;
                ext->descriptor = 0;
                ext->which = Z_External_CQL;
                ext->u.cql = srw_req->query;

                rr.query->which = Z_Query_type_104;
                rr.query->u.type_104 =  ext;
            }
        }
        else if (!strcmp(srw_req->queryType, "pqf"))
        {
            Z_RPNQuery *RPNquery;
            YAZ_PQF_Parser pqf_parser;

            pqf_parser = yaz_pqf_create();

            RPNquery = yaz_pqf_parse(pqf_parser, assoc->decode, srw_req->query);
            if (!RPNquery)
            {
                const char *pqf_msg;
                size_t off;
                int code = yaz_pqf_error(pqf_parser, &pqf_msg, &off);
                yaz_log(log_requestdetail, "Parse error %d %s near offset %ld",
                        code, pqf_msg, (long) off);
                srw_error = YAZ_SRW_QUERY_SYNTAX_ERROR;
            }

            rr.query->which = Z_Query_type_1;
            rr.query->u.type_1 =  RPNquery;

            yaz_pqf_destroy(pqf_parser);
        }
        else
        {
            yaz_add_srw_diagnostic(assoc->encode, &srw_res->diagnostics,
                                   &srw_res->num_diagnostics,
                                   YAZ_SRW_UNSUPP_QUERY_TYPE, 0);
        }
        if (rr.query->u.type_1)
        {
            rr.stream = assoc->encode;
            rr.decode = assoc->decode;
            rr.print = assoc->print;
            if (srw_req->sort.sortKeys)
                rr.srw_sortKeys = odr_strdup(assoc->encode,
                                             srw_req->sort.sortKeys);
            rr.association = assoc;
            rr.hits = 0;
            rr.errcode = 0;
            rr.errstring = 0;
            rr.search_info = 0;
            rr.search_input = 0;

            if (srw_req->facetList)
                yaz_oi_set_facetlist(&rr.search_input, assoc->encode,
                                     srw_req->facetList);

            yaz_log_zquery_level(log_requestdetail,rr.query);

            (assoc->init->bend_search)(assoc->backend, &rr);
            if (rr.errcode)
            {
                if (rr.errcode == YAZ_BIB1_DATABASE_UNAVAILABLE)
                {
                    *http_code = 404;
                }
                else
                {
                    srw_error = yaz_diag_bib1_to_srw(rr.errcode);
                    yaz_add_srw_diagnostic(assoc->encode,
                                           &srw_res->diagnostics,
                                           &srw_res->num_diagnostics,
                                           srw_error, rr.errstring);
                }
            }
            else
            {
                int number = srw_req->maximumRecords ?
                    odr_int_to_int(*srw_req->maximumRecords) : 0;
                int start = srw_req->startRecord ?
                    odr_int_to_int(*srw_req->startRecord) : 1;

                yaz_log(log_requestdetail, "Request to pack %d+%d out of "
                        ODR_INT_PRINTF,
                        start, number, rr.hits);

                srw_res->numberOfRecords = odr_intdup(assoc->encode, rr.hits);
		if (rr.srw_setname)
                {
                    srw_res->resultSetId =
                        odr_strdup(assoc->encode, rr.srw_setname );
                    srw_res->resultSetIdleTime =
                        odr_intdup(assoc->encode, *rr.srw_setnameIdleTime );
		}

                srw_res->facetList = yaz_oi_get_facetlist(&rr.search_info);
                if (start > rr.hits || start < 1)
                {
                    /* if hits<=0 and start=1 we don't return a diagnostic */
                    if (start != 1)
                        yaz_add_srw_diagnostic(
                            assoc->encode,
                            &srw_res->diagnostics, &srw_res->num_diagnostics,
                            YAZ_SRW_FIRST_RECORD_POSITION_OUT_OF_RANGE, 0);
                }
                else if (number > 0)
                {
                    int i;
                    int ok = 1;
                    if (start + number > rr.hits)
                        number = odr_int_to_int(rr.hits) - start + 1;

                    /* Call bend_present if defined */
                    if (assoc->init->bend_present)
                    {
                        bend_present_rr *bprr = (bend_present_rr*)
                            odr_malloc(assoc->decode, sizeof(*bprr));
                        bprr->setname = "default";
                        bprr->start = start;
                        bprr->number = number;
                        if (srw_req->recordSchema)
                        {
                            bprr->comp = (Z_RecordComposition *) odr_malloc(assoc->decode,
                                                                            sizeof(*bprr->comp));
                            bprr->comp->which = Z_RecordComp_simple;
                            bprr->comp->u.simple = (Z_ElementSetNames *)
                                odr_malloc(assoc->decode, sizeof(Z_ElementSetNames));
                            bprr->comp->u.simple->which = Z_ElementSetNames_generic;
                            bprr->comp->u.simple->u.generic = srw_req->recordSchema;
                        }
                        else
                        {
                            bprr->comp = 0;
                        }
                        bprr->stream = assoc->encode;
                        bprr->referenceId = 0;
                        bprr->print = assoc->print;
                        bprr->association = assoc;
                        bprr->errcode = 0;
                        bprr->errstring = NULL;
                        (*assoc->init->bend_present)(assoc->backend, bprr);

                        if (bprr->errcode)
                        {
                            srw_error = yaz_diag_bib1_to_srw(bprr->errcode);
                            yaz_add_srw_diagnostic(assoc->encode,
                                                   &srw_res->diagnostics,
                                                   &srw_res->num_diagnostics,
                                                   srw_error, bprr->errstring);
                            ok = 0;
                        }
                    }

                    if (ok)
                    {
                        int j = 0;
                        int packing = Z_SRW_recordPacking_string;
                        if (srw_req->recordPacking)
                        {
                            packing =
                                yaz_srw_str_to_pack(srw_req->recordPacking);
                            if (packing == -1)
                                packing = Z_SRW_recordPacking_string;
                        }
                        srw_res->records = (Z_SRW_record *)
                            odr_malloc(assoc->encode,
                                       number * sizeof(*srw_res->records));

                        srw_res->extra_records = (Z_SRW_extra_record **)
                            odr_malloc(assoc->encode,
                                       number*sizeof(*srw_res->extra_records));

                        for (i = 0; i<number; i++)
                        {
                            int errcode;
                            int last_in_set = 0;
                            const char *addinfo = 0;

                            srw_res->records[j].recordPacking = packing;
                            srw_res->records[j].recordData_buf = 0;
                            srw_res->extra_records[j] = 0;
                            yaz_log(YLOG_DEBUG, "srw_bend_fetch %d", i+start);
                            errcode = srw_bend_fetch(assoc, i+start, srw_req,
                                                     srw_res->records + j,
                                                     &addinfo, &last_in_set);
                            if (errcode)
                            {
                                yaz_add_srw_diagnostic(assoc->encode,
                                                       &srw_res->diagnostics,
                                                       &srw_res->num_diagnostics,
                                                       yaz_diag_bib1_to_srw(errcode),
                                                       addinfo);

                                break;
                            }
                            if (srw_res->records[j].recordData_buf)
                                j++;
                            if (last_in_set)
                                break;
                        }
                        srw_res->num_records = j;
                        if (!j)
                            srw_res->records = 0;
                    }
                }
                if (rr.extra_response_data)
                {
                    res->extraResponseData_buf = rr.extra_response_data;
                    res->extraResponseData_len = strlen(rr.extra_response_data);
                }
                if (strcmp(res->srw_version, "2.") > 0)
                {
                    if (rr.estimated_hit_count)
                        srw_res->resultCountPrecision =
                            odr_strdup(assoc->encode, "estimate");
                    else if (rr.partial_resultset)
                        srw_res->resultCountPrecision =
                            odr_strdup(assoc->encode, "minimum");
                    else
                        srw_res->resultCountPrecision =
                            odr_strdup(assoc->encode, "exact");
                }
                else if (rr.estimated_hit_count || rr.partial_resultset)
                {
                    yaz_add_srw_diagnostic(
                        assoc->encode,
                        &srw_res->diagnostics,
                        &srw_res->num_diagnostics,
                        YAZ_SRW_RESULT_SET_CREATED_WITH_VALID_PARTIAL_RESULTS_AVAILABLE,
                        0);
                }
            }
        }
    }
    if (log_request)
    {
        WRBUF wr = wrbuf_alloc();

        wrbuf_printf(wr, "SRWSearch %s ", srw_req->database);
        if (srw_res->num_diagnostics)
            wrbuf_printf(wr, "ERROR %s", srw_res->diagnostics[0].uri);
        else if (*http_code != 200)
            wrbuf_printf(wr, "ERROR info:http/%d", *http_code);
        else if (srw_res->numberOfRecords)
        {
            wrbuf_printf(wr, "OK " ODR_INT_PRINTF,
                         (srw_res->numberOfRecords ?
                          *srw_res->numberOfRecords : 0));
        }
        wrbuf_printf(wr, " %s " ODR_INT_PRINTF "+%d",
                     (srw_res->resultSetId ?
                      srw_res->resultSetId : "-"),
                     (srw_req->startRecord ? *srw_req->startRecord : 1),
                     srw_res->num_records);
        yaz_log(log_request, "%s %s: %s", wrbuf_cstr(wr), srw_req->queryType,
                srw_req->query);
        wrbuf_destroy(wr);
    }
}

static char *srw_bend_explain_default(bend_explain_rr *rr)
{
#if YAZ_HAVE_XML2
    xmlNodePtr ptr = (xmlNode *) rr->server_node_ptr;
    if (!ptr)
        return 0;
    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
        if (ptr->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *) ptr->name, "explain"))
        {
            int len;
            xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
            xmlChar *buf_out;
            char *content;

            ptr = xmlCopyNode(ptr, 1);

            xmlDocSetRootElement(doc, ptr);

            xmlDocDumpMemory(doc, &buf_out, &len);
            content = (char*) odr_malloc(rr->stream, 1+len);
            memcpy(content, buf_out, len);
            content[len] = '\0';

            xmlFree(buf_out);
            xmlFreeDoc(doc);
            rr->explain_buf = content;
            return 0;
        }
    }
#endif
    return 0;
}

static void srw_bend_explain(association *assoc,
                             Z_HTTP_Header *headers,
                             Z_SRW_PDU *sr,
                             Z_SRW_explainResponse *srw_res,
                             int *http_code)
{
    Z_SRW_explainRequest *srw_req = sr->u.explain_request;
    yaz_log(log_requestdetail, "Got SRW ExplainRequest");
    srw_bend_init(assoc, headers,
                  &srw_res->diagnostics, &srw_res->num_diagnostics, sr);
    if (!assoc->init && srw_res->num_diagnostics == 0)
        *http_code = 404;
    if (assoc->init)
    {
        bend_explain_rr rr;

        rr.stream = assoc->encode;
        rr.decode = assoc->decode;
        rr.print = assoc->print;
        rr.explain_buf = 0;
        rr.database = srw_req->database;
        if (assoc->server)
            rr.server_node_ptr = assoc->server->server_node_ptr;
        else
            rr.server_node_ptr = 0;
        rr.schema = "http://explain.z3950.org/dtd/2.0/";
        if (assoc->init->bend_explain)
            (*assoc->init->bend_explain)(assoc->backend, &rr);
        else
            srw_bend_explain_default(&rr);

        if (rr.explain_buf)
        {
            int packing = Z_SRW_recordPacking_string;
            if (srw_req->recordPacking)
            {
                packing =
                    yaz_srw_str_to_pack(srw_req->recordPacking);
                if (packing == -1)
                    packing = Z_SRW_recordPacking_string;
            }
            srw_res->record.recordSchema = rr.schema;
            srw_res->record.recordPacking = packing;
            srw_res->record.recordData_buf = rr.explain_buf;
            srw_res->record.recordData_len = strlen(rr.explain_buf);
            srw_res->record.recordPosition = 0;
            *http_code = 200;
        }
    }
}

static void srw_bend_scan(association *assoc,
                          Z_HTTP_Header *headers,
                          Z_SRW_PDU *sr,
                          Z_SRW_PDU *res,
                          int *http_code)
{
    Z_SRW_scanRequest *srw_req = sr->u.scan_request;
    Z_SRW_scanResponse *srw_res = res->u.scan_response;
    yaz_log(log_requestdetail, "Got SRW ScanRequest");

    *http_code = 200;
    srw_bend_init(assoc, headers,
                  &srw_res->diagnostics, &srw_res->num_diagnostics, sr);
    if (srw_res->num_diagnostics == 0 && assoc->init)
    {
        int step_size = 0;
        struct scan_entry *save_entries;

        bend_scan_rr *bsrr = (bend_scan_rr *)
            odr_malloc(assoc->encode, sizeof(*bsrr));
        bsrr->num_bases = 1;
        bsrr->basenames = &srw_req->database;

        bsrr->num_entries = srw_req->maximumTerms ?
            odr_int_to_int(*srw_req->maximumTerms) : 10;
        bsrr->term_position = srw_req->responsePosition ?
            odr_int_to_int(*srw_req->responsePosition) : 1;

        bsrr->errcode = 0;
        bsrr->errstring = 0;
        bsrr->referenceId = 0;
        bsrr->stream = assoc->encode;
        bsrr->print = assoc->print;
        bsrr->step_size = &step_size;
        bsrr->entries = 0;
        bsrr->setname = 0;
        bsrr->extra_args = sr->extra_args;
        bsrr->extra_response_data = 0;

        if (bsrr->num_entries > 0)
        {
            int i;
            bsrr->entries = (struct scan_entry *)
                odr_malloc(assoc->decode, sizeof(*bsrr->entries) *
                           bsrr->num_entries);
            for (i = 0; i<bsrr->num_entries; i++)
            {
                bsrr->entries[i].term = 0;
                bsrr->entries[i].occurrences = 0;
                bsrr->entries[i].errcode = 0;
                bsrr->entries[i].errstring = 0;
                bsrr->entries[i].display_term = 0;
            }
        }
        save_entries = bsrr->entries;  /* save it so we can compare later */

        if (srw_req->queryType && !strcmp(srw_req->queryType, "pqf") &&
            assoc->init->bend_scan)
        {
            YAZ_PQF_Parser pqf_parser = yaz_pqf_create();

            bsrr->term = yaz_pqf_scan(pqf_parser, assoc->decode,
                                      &bsrr->attributeset,
                                      srw_req->scanClause);
            yaz_pqf_destroy(pqf_parser);
            bsrr->scanClause = 0;
            ((int (*)(void *, bend_scan_rr *))
             (*assoc->init->bend_scan))(assoc->backend, bsrr);
        }
        else if ((!srw_req->queryType || !strcmp(srw_req->queryType, "cql"))
                 && assoc->init->bend_scan && assoc->server
                 && assoc->server->cql_transform)
        {
            int srw_error;
            bsrr->scanClause = 0;
            bsrr->attributeset = 0;
            bsrr->term = (Z_AttributesPlusTerm *)
                odr_malloc(assoc->decode, sizeof(*bsrr->term));
            srw_error = cql2pqf_scan(assoc->encode,
                                     srw_req->scanClause,
                                     assoc->server->cql_transform,
                                     bsrr->term);
            if (srw_error)
                yaz_add_srw_diagnostic(assoc->encode, &srw_res->diagnostics,
                                       &srw_res->num_diagnostics,
                                       srw_error, 0);
            else
            {
                ((int (*)(void *, bend_scan_rr *))
                 (*assoc->init->bend_scan))(assoc->backend, bsrr);
            }
        }
        else if ((!srw_req->queryType || !strcmp(srw_req->queryType, "cql"))
                 && assoc->init->bend_srw_scan)
        {
            bsrr->term = 0;
            bsrr->attributeset = 0;
            bsrr->scanClause = srw_req->scanClause;
            ((int (*)(void *, bend_scan_rr *))
             (*assoc->init->bend_srw_scan))(assoc->backend, bsrr);
        }
        else
        {
            yaz_add_srw_diagnostic(assoc->encode, &srw_res->diagnostics,
                                   &srw_res->num_diagnostics,
                                   YAZ_SRW_UNSUPP_OPERATION, "scan");
        }
        if (bsrr->extra_response_data)
        {
            res->extraResponseData_buf = bsrr->extra_response_data;
            res->extraResponseData_len = strlen(bsrr->extra_response_data);
        }
        if (bsrr->errcode)
        {
            int srw_error;
            if (bsrr->errcode == YAZ_BIB1_DATABASE_UNAVAILABLE)
            {
                *http_code = 404;
                return;
            }
            srw_error = yaz_diag_bib1_to_srw(bsrr->errcode);

            yaz_add_srw_diagnostic(assoc->encode, &srw_res->diagnostics,
                                   &srw_res->num_diagnostics,
                                   srw_error, bsrr->errstring);
        }
        else if (srw_res->num_diagnostics == 0 && bsrr->num_entries)
        {
            int i;
            srw_res->terms = (Z_SRW_scanTerm*)
                odr_malloc(assoc->encode, sizeof(*srw_res->terms) *
                           bsrr->num_entries);

            srw_res->num_terms =  bsrr->num_entries;
            for (i = 0; i<bsrr->num_entries; i++)
            {
                Z_SRW_scanTerm *t = srw_res->terms + i;
                t->value = odr_strdup(assoc->encode, bsrr->entries[i].term);
                t->numberOfRecords =
                    odr_intdup(assoc->encode, bsrr->entries[i].occurrences);
                t->displayTerm = 0;
                if (save_entries == bsrr->entries &&
                    bsrr->entries[i].display_term)
                {
                    /* the entries was _not_ set by the handler. So it's
                       safe to test for new member display_term. It is
                       NULL'ed by us.
                    */
                    t->displayTerm = odr_strdup(assoc->encode,
                                                bsrr->entries[i].display_term);
                }
                t->whereInList = 0;
            }
        }
    }
    if (log_request)
    {
        WRBUF wr = wrbuf_alloc();
        wrbuf_printf(wr, "SRWScan %s ", srw_req->database);

        if (srw_res->num_diagnostics)
            wrbuf_printf(wr, "ERROR %s - ", srw_res->diagnostics[0].uri);
        else if (srw_res->num_terms)
            wrbuf_printf(wr, "OK %d - ", srw_res->num_terms);
        else
            wrbuf_printf(wr, "OK - - ");

        wrbuf_printf(wr, ODR_INT_PRINTF "+" ODR_INT_PRINTF " ",
                     (srw_req->responsePosition ?
                      *srw_req->responsePosition : 1),
                     (srw_req->maximumTerms ?
                      *srw_req->maximumTerms : 1));
        /* there is no step size in SRU/W ??? */
        wrbuf_printf(wr, "%s: %s ", srw_req->queryType, srw_req->scanClause);
        yaz_log(log_request, "%s ", wrbuf_cstr(wr) );
        wrbuf_destroy(wr);
    }

}

static void srw_bend_update(association *assoc,
                            Z_HTTP_Header *headers,
			    Z_SRW_PDU *sr,
			    Z_SRW_updateResponse *srw_res,
			    int *http_code)
{
    Z_SRW_updateRequest *srw_req = sr->u.update_request;
    yaz_log(log_session, "SRWUpdate action=%s", srw_req->operation);
    yaz_log(YLOG_DEBUG, "num_diag = %d", srw_res->num_diagnostics );
    srw_bend_init(assoc, headers,
                  &srw_res->diagnostics, &srw_res->num_diagnostics, sr);
    if (!assoc->init && srw_res->num_diagnostics == 0)
        *http_code = 404;
    if (assoc->init)
    {
	bend_update_rr rr;
        Z_SRW_extra_record *extra = srw_req->extra_record;

	rr.stream = assoc->encode;
	rr.print = assoc->print;
        rr.num_bases = 1;
        rr.basenames = &srw_req->database;
	rr.operation = srw_req->operation;
	rr.operation_status = "failed";
	rr.record_id = 0;
        rr.record_versions = 0;
        rr.num_versions = 0;
        rr.record_packing = "string";
	rr.record_schema = 0;
        rr.record_data = 0;
        rr.extra_record_data = 0;
        rr.extra_request_data = 0;
        rr.extra_response_data = 0;
        rr.uri = 0;
        rr.message = 0;
        rr.details = 0;

	*http_code = 200;
        if (rr.operation == 0)
        {
            yaz_add_sru_update_diagnostic(
                assoc->encode, &srw_res->diagnostics,
                &srw_res->num_diagnostics,
                YAZ_SRU_UPDATE_MISSING_MANDATORY_ELEMENT_RECORD_REJECTED,
                "action" );
            return;
        }
        yaz_log(YLOG_DEBUG, "basename = %s", rr.basenames[0] );
        yaz_log(YLOG_DEBUG, "Operation = %s", rr.operation );
	if (!strcmp( rr.operation, "delete"))
        {
            if (srw_req->record && !srw_req->record->recordSchema)
            {
                rr.record_schema = odr_strdup(
                    assoc->encode,
                    srw_req->record->recordSchema);
            }
            if (srw_req->record)
            {
                rr.record_data = odr_strdupn(
                    assoc->encode,
                    srw_req->record->recordData_buf,
                    srw_req->record->recordData_len );
            }
            if (extra && extra->extraRecordData_len)
            {
                rr.extra_record_data = odr_strdupn(
                    assoc->encode,
                    extra->extraRecordData_buf,
                    extra->extraRecordData_len );
            }
            if (srw_req->recordId)
                rr.record_id = srw_req->recordId;
            else if (extra && extra->recordIdentifier)
                rr.record_id = extra->recordIdentifier;
	}
	else if (!strcmp(rr.operation, "replace"))
        {
            if (srw_req->recordId)
                rr.record_id = srw_req->recordId;
            else if (extra && extra->recordIdentifier)
                rr.record_id = extra->recordIdentifier;
            else
            {
                yaz_add_sru_update_diagnostic(
                    assoc->encode, &srw_res->diagnostics,
                    &srw_res->num_diagnostics,
                    YAZ_SRU_UPDATE_MISSING_MANDATORY_ELEMENT_RECORD_REJECTED,
                    "recordIdentifier");
            }
            if (!srw_req->record)
            {
                yaz_add_sru_update_diagnostic(
                    assoc->encode, &srw_res->diagnostics,
                    &srw_res->num_diagnostics,
                    YAZ_SRU_UPDATE_MISSING_MANDATORY_ELEMENT_RECORD_REJECTED,
                    "record");
            }
            else
            {
                if (srw_req->record->recordSchema)
                    rr.record_schema = odr_strdup(
                        assoc->encode, srw_req->record->recordSchema);
                if (srw_req->record->recordData_len )
                {
                    rr.record_data = odr_strdupn(assoc->encode,
                                                 srw_req->record->recordData_buf,
                                                 srw_req->record->recordData_len );
                }
                else
                {
                    yaz_add_sru_update_diagnostic(
                        assoc->encode, &srw_res->diagnostics,
                        &srw_res->num_diagnostics,
                        YAZ_SRU_UPDATE_MISSING_MANDATORY_ELEMENT_RECORD_REJECTED,
                        "recordData" );
                }
            }
            if (extra && extra->extraRecordData_len)
            {
                rr.extra_record_data = odr_strdupn(
                    assoc->encode,
                    extra->extraRecordData_buf,
                    extra->extraRecordData_len );
            }
	}
	else if (!strcmp(rr.operation, "insert"))
        {
            if (srw_req->recordId)
                rr.record_id = srw_req->recordId;
            else if (extra)
                rr.record_id = extra->recordIdentifier;

            if (srw_req->record)
            {
                if (srw_req->record->recordSchema)
                    rr.record_schema = odr_strdup(
                        assoc->encode, srw_req->record->recordSchema);

                if (srw_req->record->recordData_len)
                    rr.record_data = odr_strdupn(
                        assoc->encode,
                        srw_req->record->recordData_buf,
                        srw_req->record->recordData_len );
            }
            if (extra && extra->extraRecordData_len)
            {
                rr.extra_record_data = odr_strdupn(
                    assoc->encode,
                    extra->extraRecordData_buf,
                    extra->extraRecordData_len );
            }
	}
	else
            yaz_add_sru_update_diagnostic(assoc->encode, &srw_res->diagnostics,
                                          &srw_res->num_diagnostics,
                                          YAZ_SRU_UPDATE_INVALID_ACTION,
                                          rr.operation );

        if (srw_req->record)
        {
            const char *pack_str =
                yaz_srw_pack_to_str(srw_req->record->recordPacking);
            if (pack_str)
                rr.record_packing = odr_strdup(assoc->encode, pack_str);
        }

        if (srw_req->num_recordVersions)
        {
            rr.record_versions = srw_req->recordVersions;
            rr.num_versions = srw_req->num_recordVersions;
        }
        if (srw_req->extraRequestData_len)
        {
            rr.extra_request_data = odr_strdupn(assoc->encode,
                                                srw_req->extraRequestData_buf,
                                                srw_req->extraRequestData_len );
        }
        if (srw_res->num_diagnostics == 0)
        {
            if ( assoc->init->bend_srw_update)
                (*assoc->init->bend_srw_update)(assoc->backend, &rr);
            else
                yaz_add_sru_update_diagnostic(
                    assoc->encode, &srw_res->diagnostics,
                    &srw_res->num_diagnostics,
                    YAZ_SRU_UPDATE_UNSPECIFIED_DATABASE_ERROR,
                    "No Update backend handler");
        }

        if (rr.uri)
            yaz_add_srw_diagnostic_uri(assoc->encode,
                                       &srw_res->diagnostics,
                                       &srw_res->num_diagnostics,
                                       rr.uri,
                                       rr.message,
                                       rr.details);
	srw_res->recordId = rr.record_id;
	srw_res->operationStatus = rr.operation_status;
	srw_res->recordVersions = rr.record_versions;
	srw_res->num_recordVersions = rr.num_versions;
        if (srw_res->extraResponseData_len)
        {
            srw_res->extraResponseData_buf = rr.extra_response_data;
            srw_res->extraResponseData_len = strlen(rr.extra_response_data);
        }
	if (srw_res->num_diagnostics == 0 && rr.record_data)
        {
            srw_res->record = yaz_srw_get_record(assoc->encode);
            srw_res->record->recordSchema = rr.record_schema;
            if (rr.record_packing)
            {
                int pack = yaz_srw_str_to_pack(rr.record_packing);

                if (pack == -1)
                {
                    pack = Z_SRW_recordPacking_string;
                    yaz_log(YLOG_WARN, "Back packing %s from backend",
                            rr.record_packing);
                }
                srw_res->record->recordPacking = pack;
            }
            srw_res->record->recordData_buf = rr.record_data;
            srw_res->record->recordData_len = strlen(rr.record_data);
            if (rr.extra_record_data)
            {
                Z_SRW_extra_record *ex =
                    yaz_srw_get_extra_record(assoc->encode);
                srw_res->extra_record = ex;
                ex->extraRecordData_buf = rr.extra_record_data;
                ex->extraRecordData_len = strlen(rr.extra_record_data);
            }
        }
    }
}

/* check if path is OK (1); BAD (0) */
static int check_path(const char *path)
{
    if (*path != '/')
        return 0;
    if (strstr(path, ".."))
        return 0;
    return 1;
}

static char *read_file(const char *fname, ODR o, long *sz)
{
    struct stat stat_buf;
    FILE *inf;
    char *buf = 0;

    if (stat(fname, &stat_buf) == -1)
    {
        yaz_log(YLOG_LOG|YLOG_ERRNO, "stat %s", fname);
        return 0;
    }
    if (!S_ISREG(stat_buf.st_mode))
    {
        yaz_log(YLOG_LOG, "Is not a regular file: %s", fname);
        return 0;
    }
    inf = fopen(fname, "rb");
    if (!inf)
    {
        yaz_log(YLOG_LOG|YLOG_ERRNO, "fopen %s", fname);
        return 0;
    }
    if (fseek(inf, 0L, SEEK_END) < 0)
        yaz_log(YLOG_LOG|YLOG_ERRNO, "fseek %s", fname);
    else if ((*sz = ftell(inf))  == -1L)
        yaz_log(YLOG_LOG|YLOG_ERRNO, "ftell %s", fname);
    else
    {
        rewind(inf);
        buf = (char *) odr_malloc(o, *sz);
        if (fread(buf, 1, *sz, inf) != *sz)
            yaz_log(YLOG_WARN|YLOG_ERRNO, "short read %s", fname);
    }
    fclose(inf);
    return buf;
}

static void process_http_request(association *assoc, request *req)
{
    Z_HTTP_Request *hreq = req->gdu_request->u.HTTP_Request;
    ODR o = assoc->encode;
    int r = 2;  /* 2=NOT TAKEN, 1=TAKEN, 0=SOAP TAKEN */
    Z_SRW_PDU *sr = 0;
    Z_SOAP *soap_package = 0;
    Z_GDU *p = 0;
    char *charset = 0;
    Z_HTTP_Response *hres = 0;
    int keepalive = 1;
    const char *stylesheet = 0; /* for now .. set later */
    Z_SRW_diagnostic *diagnostic = 0;
    int num_diagnostic = 0;
    const char *host = z_HTTP_header_lookup(hreq->headers, "Host");

    yaz_log(log_request, "%s %s HTTP/%s", hreq->method, hreq->path, hreq->version);
    if (!control_association(assoc, host, 0))
    {
        p = z_get_HTTP_Response(o, 404);
        r = 1;
    }
    if (r == 2 && assoc->server && assoc->server->docpath
        && hreq->path[0] == '/'
        &&
        /* check if path is a proper prefix of documentroot */
        strncmp(hreq->path+1, assoc->server->docpath,
                strlen(assoc->server->docpath))
        == 0)
    {
        if (!check_path(hreq->path))
        {
            yaz_log(YLOG_LOG, "File %s access forbidden", hreq->path+1);
            p = z_get_HTTP_Response(o, 403);
        }
        else
        {
            long content_size = 0;
            char *content_buf = read_file(hreq->path+1, o, &content_size);
            if (!content_buf)
            {
                p = z_get_HTTP_Response(o, 404);
            }
            else
            {
                const char *ctype = 0;
                yaz_mime_types types = yaz_mime_types_create();

                yaz_mime_types_add(types, "xsl", "application/xml");
                yaz_mime_types_add(types, "xml", "application/xml");
                yaz_mime_types_add(types, "css", "text/css");
                yaz_mime_types_add(types, "html", "text/html");
                yaz_mime_types_add(types, "htm", "text/html");
                yaz_mime_types_add(types, "txt", "text/plain");
                yaz_mime_types_add(types, "js", "application/x-javascript");

                yaz_mime_types_add(types, "gif", "image/gif");
                yaz_mime_types_add(types, "png", "image/png");
                yaz_mime_types_add(types, "jpg", "image/jpeg");
                yaz_mime_types_add(types, "jpeg", "image/jpeg");

                ctype = yaz_mime_lookup_fname(types, hreq->path);
                if (!ctype)
                {
                    yaz_log(YLOG_LOG, "No mime type for %s", hreq->path+1);
                    p = z_get_HTTP_Response(o, 404);
                }
                else
                {
                    p = z_get_HTTP_Response(o, 200);
                    hres = p->u.HTTP_Response;
                    hres->content_buf = content_buf;
                    hres->content_len = content_size;
                    z_HTTP_header_add(o, &hres->headers, "Content-Type", ctype);
                }
                yaz_mime_types_destroy(types);
            }
        }
        r = 1;
    }

    if (r == 2)
    {
        r = yaz_srw_decode(hreq, &sr, &soap_package, assoc->decode, &charset);
        yaz_log(YLOG_DEBUG, "yaz_srw_decode returned %d", r);
    }
    if (r == 2)  /* not taken */
    {
        r = yaz_sru_decode(hreq, &sr, &soap_package, assoc->decode, &charset,
                           &diagnostic, &num_diagnostic);
        yaz_log(YLOG_DEBUG, "yaz_sru_decode returned %d", r);
    }
    if (r == 0)  /* decode SRW/SRU OK .. */
    {
        int http_code = 200;
        if (sr->which == Z_SRW_searchRetrieve_request)
        {
            Z_SRW_PDU *res =
                yaz_srw_get_pdu_e(assoc->encode, Z_SRW_searchRetrieve_response,
                                  sr);
            stylesheet = sr->u.request->stylesheet;
            if (num_diagnostic)
            {
                res->u.response->diagnostics = diagnostic;
                res->u.response->num_diagnostics = num_diagnostic;
            }
            else
            {
                srw_bend_search(assoc, hreq->headers, sr, res, &http_code);
            }
            if (http_code == 200)
                soap_package->u.generic->p = res;
        }
        else if (sr->which == Z_SRW_explain_request)
        {
            Z_SRW_PDU *res = yaz_srw_get_pdu_e(o, Z_SRW_explain_response, sr);
            stylesheet = sr->u.explain_request->stylesheet;
            if (num_diagnostic)
            {
                res->u.explain_response->diagnostics = diagnostic;
                res->u.explain_response->num_diagnostics = num_diagnostic;
            }
            srw_bend_explain(assoc, hreq->headers,
                             sr, res->u.explain_response, &http_code);
            if (http_code == 200)
                soap_package->u.generic->p = res;
        }
        else if (sr->which == Z_SRW_scan_request)
        {
            Z_SRW_PDU *res = yaz_srw_get_pdu_e(o, Z_SRW_scan_response, sr);
            stylesheet = sr->u.scan_request->stylesheet;
            if (num_diagnostic)
            {
                res->u.scan_response->diagnostics = diagnostic;
                res->u.scan_response->num_diagnostics = num_diagnostic;
            }
            srw_bend_scan(assoc, hreq->headers, sr, res, &http_code);
            if (http_code == 200)
                soap_package->u.generic->p = res;
        }
        else if (sr->which == Z_SRW_update_request)
        {
            Z_SRW_PDU *res = yaz_srw_get_pdu(o, Z_SRW_update_response,
                                             sr->srw_version);
            yaz_log(YLOG_DEBUG, "handling SRW UpdateRequest");
            if (num_diagnostic)
            {
                res->u.update_response->diagnostics = diagnostic;
                res->u.update_response->num_diagnostics = num_diagnostic;
            }
            yaz_log(YLOG_DEBUG, "num_diag = %d", res->u.update_response->num_diagnostics );
            srw_bend_update(assoc, hreq->headers,
                            sr, res->u.update_response, &http_code);
            if (http_code == 200)
                soap_package->u.generic->p = res;
        }
        else
        {
            yaz_log(log_request, "SOAP ERROR");
            /* FIXME - what error, what query */
            http_code = 500;
            z_soap_error(assoc->encode, soap_package,
                         "SOAP-ENV:Client", "Bad method", 0);
        }
        if (http_code == 200 || http_code == 500)
        {
            static Z_SOAP_Handler soap_handlers[4] = {
#if YAZ_HAVE_XML2
                {YAZ_XMLNS_SRU_v1_1, 0, (Z_SOAP_fun) yaz_srw_codec},
                {YAZ_XMLNS_SRU_v1_0, 0, (Z_SOAP_fun) yaz_srw_codec},
                {YAZ_XMLNS_UPDATE_v0_9, 0, (Z_SOAP_fun) yaz_ucp_codec},
#endif
                {0, 0, 0}
            };
            char ctype[80];
            p = z_get_HTTP_Response(o, 200);
            hres = p->u.HTTP_Response;

            if (!stylesheet && assoc->server)
                stylesheet = assoc->server->stylesheet;

            /* empty stylesheet means NO stylesheet */
            if (stylesheet && *stylesheet == '\0')
                stylesheet = 0;

            z_soap_codec_enc_xsl(assoc->encode, &soap_package,
                                 &hres->content_buf, &hres->content_len,
                                 soap_handlers, charset, stylesheet);
            hres->code = http_code;

            strcpy(ctype, "text/xml");
            z_HTTP_header_add(o, &hres->headers, "Content-Type", ctype);
        }
        else
            p = z_get_HTTP_Response(o, http_code);
    }

    if (p == 0)
        p = z_get_HTTP_Response(o, 500);
    hres = p->u.HTTP_Response;
    if (!strcmp(hreq->version, "1.0"))
    {
        const char *v = z_HTTP_header_lookup(hreq->headers, "Connection");
        if (v && !strcmp(v, "Keep-Alive"))
            keepalive = 1;
        else
            keepalive = 0;
        hres->version = "1.0";
    }
    else
    {
        const char *v = z_HTTP_header_lookup(hreq->headers, "Connection");
        if (v && !strcmp(v, "close"))
            keepalive = 0;
        else
            keepalive = 1;
        hres->version = "1.1";
    }
    if (!keepalive || !assoc->last_control->keepalive)
    {
        z_HTTP_header_add(o, &hres->headers, "Connection", "close");
        assoc->state = ASSOC_DEAD;
        assoc->cs_get_mask = 0;
    }
    else
    {
        int t;
        const char *alive = z_HTTP_header_lookup(hreq->headers, "Keep-Alive");

        if (alive && yaz_isdigit(*(const unsigned char *) alive))
            t = atoi(alive);
        else
            t = 15;
        if (t < 0 || t > 3600)
            t = 3600;
        iochan_settimeout(assoc->client_chan,t);
        z_HTTP_header_add(o, &hres->headers, "Connection", "Keep-Alive");
    }
    process_gdu_response(assoc, req, p);
}

static void process_gdu_request(association *assoc, request *req)
{
    if (req->gdu_request->which == Z_GDU_Z3950)
    {
        char *msg = 0;
        req->apdu_request = req->gdu_request->u.z3950;
        if (process_z_request(assoc, req, &msg) < 0)
            do_close_req(assoc, Z_Close_systemProblem, msg, req);
    }
    else if (req->gdu_request->which == Z_GDU_HTTP_Request)
        process_http_request(assoc, req);
    else
    {
        do_close_req(assoc, Z_Close_systemProblem, "bad protocol packet", req);
    }
}

/*
 * Initiate request processing.
 */
static int process_z_request(association *assoc, request *req, char **msg)
{
    Z_APDU *res;
    int retval;

    *msg = "Unknown Error";
    assert(req && req->state == REQUEST_IDLE);
    if (req->apdu_request->which != Z_APDU_initRequest && !assoc->init)
    {
        *msg = "Missing InitRequest";
        return -1;
    }
    switch (req->apdu_request->which)
    {
    case Z_APDU_initRequest:
        res = process_initRequest(assoc, req); break;
    case Z_APDU_searchRequest:
        res = process_searchRequest(assoc, req); break;
    case Z_APDU_presentRequest:
        res = process_presentRequest(assoc, req); break;
    case Z_APDU_scanRequest:
        if (assoc->init->bend_scan)
            res = process_scanRequest(assoc, req);
        else
        {
            *msg = "Cannot handle Scan APDU";
            return -1;
        }
        break;
    case Z_APDU_extendedServicesRequest:
        if (assoc->init->bend_esrequest)
            res = process_ESRequest(assoc, req);
        else
        {
            *msg = "Cannot handle Extended Services APDU";
            return -1;
        }
        break;
    case Z_APDU_sortRequest:
        if (assoc->init->bend_sort)
            res = process_sortRequest(assoc, req);
        else
        {
            *msg = "Cannot handle Sort APDU";
            return -1;
        }
        break;
    case Z_APDU_close:
        process_close(assoc, req);
        return 0;
    case Z_APDU_deleteResultSetRequest:
        if (assoc->init->bend_delete)
            res = process_deleteRequest(assoc, req);
        else
        {
            *msg = "Cannot handle Delete APDU";
            return -1;
        }
        break;
    case Z_APDU_segmentRequest:
        if (assoc->init->bend_segment)
        {
            res = process_segmentRequest(assoc, req);
        }
        else
        {
            *msg = "Cannot handle Segment APDU";
            return -1;
        }
        break;
    case Z_APDU_triggerResourceControlRequest:
        return 0;
    default:
        *msg = "Bad APDU received";
        return -1;
    }
    if (res)
    {
        yaz_log(YLOG_DEBUG, "  result immediately available");
        retval = process_z_response(assoc, req, res);
    }
    else
    {
        yaz_log(YLOG_DEBUG, "  result unavailable");
        retval = -1;
    }
    return retval;
}

/*
 * Encode response, and transfer the request structure to the outgoing queue.
 */
static int process_gdu_response(association *assoc, request *req, Z_GDU *res)
{
    odr_setbuf(assoc->encode, req->response, req->size_response, 1);

    if (assoc->print)
    {
        if (!z_GDU(assoc->print, &res, 0, 0))
            yaz_log(YLOG_WARN, "ODR print error: %s",
                odr_errmsg(odr_geterror(assoc->print)));
        odr_reset(assoc->print);
    }
    if (!z_GDU(assoc->encode, &res, 0, 0))
    {
        yaz_log(YLOG_WARN, "ODR error when encoding PDU: %s [element %s]",
                odr_errmsg(odr_geterror(assoc->decode)),
                odr_getelement(assoc->decode));
        return -1;
    }
    req->response = odr_getbuf(assoc->encode, &req->len_response,
        &req->size_response);
    odr_setbuf(assoc->encode, 0, 0, 0); /* don'txfree if we abort later */
    odr_reset(assoc->encode);
    req->state = REQUEST_IDLE;
    request_enq(&assoc->outgoing, req);
    /* turn the work over to the ir_session handler */
    iochan_setflag(assoc->client_chan, EVENT_OUTPUT);
    assoc->cs_put_mask = EVENT_OUTPUT;
    /* Is there more work to be done? give that to the input handler too */
    for (;;)
    {
        req = request_head(&assoc->incoming);
        if (req && req->state == REQUEST_IDLE)
        {
            request_deq(&assoc->incoming);
            process_gdu_request(assoc, req);
        }
        else
            break;
    }
    return 0;
}

/*
 * Encode response, and transfer the request structure to the outgoing queue.
 */
static int process_z_response(association *assoc, request *req, Z_APDU *res)
{
    Z_GDU *gres = (Z_GDU *) odr_malloc(assoc->encode, sizeof(*gres));
    gres->which = Z_GDU_Z3950;
    gres->u.z3950 = res;

    return process_gdu_response(assoc, req, gres);
}

static char *get_vhost(Z_OtherInformation *otherInfo)
{
    return yaz_oi_get_string_oid(&otherInfo, yaz_oid_userinfo_proxy, 1, 0);
}

/*
 * Handle init request.
 * At the moment, we don't check the options
 * anywhere else in the code - we just try not to do anything that would
 * break a naive client. We'll toss 'em into the association block when
 * we need them there.
 */
static Z_APDU *process_initRequest(association *assoc, request *reqb)
{
    Z_InitRequest *req = reqb->apdu_request->u.initRequest;
    Z_APDU *apdu = zget_APDU(assoc->encode, Z_APDU_initResponse);
    Z_InitResponse *resp = apdu->u.initResponse;
    bend_initresult *binitres;
    char options[140];
    statserv_options_block *cb = 0;  /* by default no control for backend */

    if (control_association(assoc, get_vhost(req->otherInfo), 1))
        cb = statserv_getcontrol();  /* got control block for backend */

    if (cb && assoc->backend)
        (*cb->bend_close)(assoc->backend);

    yaz_log(log_requestdetail, "Got initRequest");
    if (req->implementationId)
        yaz_log(log_requestdetail, "Id:        %s",
                req->implementationId);
    if (req->implementationName)
        yaz_log(log_requestdetail, "Name:      %s",
                req->implementationName);
    if (req->implementationVersion)
        yaz_log(log_requestdetail, "Version:   %s",
                req->implementationVersion);

    assoc_init_reset(assoc, 
                     yaz_oi_get_string_oid(&req->otherInfo,
                                           yaz_oid_userinfo_client_ip, 1, 0));
    assoc->init->auth = req->idAuthentication;
    assoc->init->referenceId = req->referenceId;

    if (ODR_MASK_GET(req->options, Z_Options_negotiationModel))
    {
        Z_CharSetandLanguageNegotiation *negotiation =
            yaz_get_charneg_record (req->otherInfo);
        if (negotiation &&
            negotiation->which == Z_CharSetandLanguageNegotiation_proposal)
            assoc->init->charneg_request = negotiation;
    }

    /* by default named_result_sets is 0 .. Enable it if client asks for it. */
    if (ODR_MASK_GET(req->options, Z_Options_namedResultSets))
        assoc->init->named_result_sets = 1;

    assoc->backend = 0;
    if (cb)
    {
        if (req->implementationVersion)
            yaz_log(log_requestdetail, "Config:    %s",
                    cb->configname);

        iochan_settimeout(assoc->client_chan, cb->idle_timeout);

        /* we have a backend control block, so call that init function */
        if (!(binitres = (*cb->bend_init)(assoc->init)))
        {
            yaz_log(YLOG_WARN, "Bad response from backend.");
            return 0;
        }
        assoc->backend = binitres->handle;
    }
    else
    {
        /* no backend. return error */
        binitres = (bend_initresult *)
            odr_malloc(assoc->encode, sizeof(*binitres));
        binitres->errstring = 0;
        binitres->errcode = YAZ_BIB1_PERMANENT_SYSTEM_ERROR;
        iochan_settimeout(assoc->client_chan, 10);
    }
    if ((assoc->init->bend_sort))
        yaz_log(YLOG_DEBUG, "Sort handler installed");
    if ((assoc->init->bend_search))
        yaz_log(YLOG_DEBUG, "Search handler installed");
    if ((assoc->init->bend_present))
        yaz_log(YLOG_DEBUG, "Present handler installed");
    if ((assoc->init->bend_esrequest))
        yaz_log(YLOG_DEBUG, "ESRequest handler installed");
    if ((assoc->init->bend_delete))
        yaz_log(YLOG_DEBUG, "Delete handler installed");
    if ((assoc->init->bend_scan))
        yaz_log(YLOG_DEBUG, "Scan handler installed");
    if ((assoc->init->bend_segment))
        yaz_log(YLOG_DEBUG, "Segment handler installed");

    resp->referenceId = req->referenceId;
    *options = '\0';
    /* let's tell the client what we can do */
    if (ODR_MASK_GET(req->options, Z_Options_search))
    {
        ODR_MASK_SET(resp->options, Z_Options_search);
        strcat(options, "srch");
    }
    if (ODR_MASK_GET(req->options, Z_Options_present))
    {
        ODR_MASK_SET(resp->options, Z_Options_present);
        strcat(options, " prst");
    }
    if (ODR_MASK_GET(req->options, Z_Options_delSet) &&
        assoc->init->bend_delete)
    {
        ODR_MASK_SET(resp->options, Z_Options_delSet);
        strcat(options, " del");
    }
    if (ODR_MASK_GET(req->options, Z_Options_extendedServices) &&
        assoc->init->bend_esrequest)
    {
        ODR_MASK_SET(resp->options, Z_Options_extendedServices);
        strcat(options, " extendedServices");
    }
    if (ODR_MASK_GET(req->options, Z_Options_namedResultSets)
        && assoc->init->named_result_sets)
    {
        ODR_MASK_SET(resp->options, Z_Options_namedResultSets);
        strcat(options, " namedresults");
    }
    if (ODR_MASK_GET(req->options, Z_Options_scan) && assoc->init->bend_scan)
    {
        ODR_MASK_SET(resp->options, Z_Options_scan);
        strcat(options, " scan");
    }
    if (ODR_MASK_GET(req->options, Z_Options_concurrentOperations))
    {
        ODR_MASK_SET(resp->options, Z_Options_concurrentOperations);
        strcat(options, " concurrop");
    }
    if (ODR_MASK_GET(req->options, Z_Options_sort) && assoc->init->bend_sort)
    {
        ODR_MASK_SET(resp->options, Z_Options_sort);
        strcat(options, " sort");
    }

    if (ODR_MASK_GET(req->options, Z_Options_negotiationModel))
    {
        Z_OtherInformationUnit *p0;

        if (!assoc->init->charneg_response)
        {
            if (assoc->init->query_charset)
            {
                assoc->init->charneg_response = yaz_set_response_charneg(
                    assoc->encode, assoc->init->query_charset, 0,
                    assoc->init->records_in_same_charset);
            }
            else
            {
                yaz_log(YLOG_WARN, "default query_charset not defined by backend");
            }
        }
        if (assoc->init->charneg_response
            && (p0=yaz_oi_update(&resp->otherInfo, assoc->encode, NULL, 0, 0)))
        {
            p0->which = Z_OtherInfo_externallyDefinedInfo;
            p0->information.externallyDefinedInfo =
                assoc->init->charneg_response;
            ODR_MASK_SET(resp->options, Z_Options_negotiationModel);
            strcat(options, " negotiation");
        }
    }
    if (ODR_MASK_GET(req->options, Z_Options_triggerResourceCtrl))
        ODR_MASK_SET(resp->options, Z_Options_triggerResourceCtrl);

    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_1))
    {
        ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_1);
        assoc->version = 1; /* 1 & 2 are equivalent */
    }
    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_2))
    {
        ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_2);
        assoc->version = 2;
    }
    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_3))
    {
        ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_3);
        assoc->version = 3;
    }

    yaz_log(log_requestdetail, "Negotiated to v%d: %s", assoc->version, options);

    if (*req->maximumRecordSize < assoc->maximumRecordSize)
        assoc->maximumRecordSize = odr_int_to_int(*req->maximumRecordSize);

    if (*req->preferredMessageSize < assoc->preferredMessageSize)
        assoc->preferredMessageSize = odr_int_to_int(*req->preferredMessageSize);

    resp->preferredMessageSize =
        odr_intdup(assoc->encode, assoc->preferredMessageSize);
    resp->maximumRecordSize =
        odr_intdup(assoc->encode, assoc->maximumRecordSize);

    resp->implementationId = odr_prepend(assoc->encode,
                assoc->init->implementation_id,
                resp->implementationId);

    resp->implementationVersion = odr_prepend(assoc->encode,
                assoc->init->implementation_version,
                resp->implementationVersion);

    resp->implementationName = odr_prepend(assoc->encode,
                assoc->init->implementation_name,
                odr_prepend(assoc->encode, "GFS", resp->implementationName));

    if (binitres->errcode)
    {
        assoc->state = ASSOC_DEAD;
        resp->userInformationField =
            init_diagnostics(assoc->encode, binitres->errcode,
                             binitres->errstring);
        *resp->result = 0;
    }
    else
        assoc->state = ASSOC_UP;

    if (log_request)
    {
        if (!req->idAuthentication)
            yaz_log(log_request, "Auth none");
        else if (req->idAuthentication->which == Z_IdAuthentication_open)
        {
            const char *open = req->idAuthentication->u.open;
            const char *slash = strchr(open, '/');
            int len;
            if (slash)
                len = slash - open;
            else
                len = strlen(open);
            yaz_log(log_request, "Auth open %.*s", len, open);
        }
        else if (req->idAuthentication->which == Z_IdAuthentication_idPass)
        {
            const char *user = req->idAuthentication->u.idPass->userId;
            const char *group = req->idAuthentication->u.idPass->groupId;
            yaz_log(log_request, "Auth idPass %s %s",
                    user ? user : "-", group ? group : "-");
        }
        else if (req->idAuthentication->which
                 == Z_IdAuthentication_anonymous)
        {
            yaz_log(log_request, "Auth anonymous");
        }
        else
        {
            yaz_log(log_request, "Auth other");
        }
    }
    if (log_request)
    {
        WRBUF wr = wrbuf_alloc();
        wrbuf_printf(wr, "Init ");
        if (binitres->errcode)
            wrbuf_printf(wr, "ERROR %d", binitres->errcode);
        else
            wrbuf_printf(wr, "OK -");
        wrbuf_printf(wr, " ID:%s Name:%s Version:%s",
                     (req->implementationId ? req->implementationId :"-"),
                     (req->implementationName ?
                      req->implementationName : "-"),
                     (req->implementationVersion ?
                      req->implementationVersion : "-")
            );
        yaz_log(log_request, "%s", wrbuf_cstr(wr));
        wrbuf_destroy(wr);
    }
    return apdu;
}

/*
 * Set the specified `errcode' and `errstring' into a UserInfo-1
 * external to be returned to the client in accordance with Z35.90
 * Implementor Agreement 5 (Returning diagnostics in an InitResponse):
 *      http://www.loc.gov/z3950/agency/agree/initdiag.html
 */
static Z_External *init_diagnostics(ODR odr, int error, const char *addinfo)
{
    yaz_log(log_requestdetail, "[%d] %s%s%s", error, diagbib1_str(error),
        addinfo ? " -- " : "", addinfo ? addinfo : "");
    return zget_init_diagnostics(odr, error, addinfo);
}

/*
 * nonsurrogate diagnostic record.
 */
static Z_Records *diagrec(association *assoc, int error, char *addinfo)
{
    Z_Records *rec = (Z_Records *) odr_malloc(assoc->encode, sizeof(*rec));

    yaz_log(log_requestdetail, "[%d] %s%s%s", error, diagbib1_str(error),
            addinfo ? " -- " : "", addinfo ? addinfo : "");

    rec->which = Z_Records_NSD;
    rec->u.nonSurrogateDiagnostic = zget_DefaultDiagFormat(assoc->encode,
                                                           error, addinfo);
    return rec;
}

/*
 * surrogate diagnostic.
 */
static Z_NamePlusRecord *surrogatediagrec(association *assoc,
                                          const char *dbname,
                                          int error, const char *addinfo)
{
    yaz_log(log_requestdetail, "[%d] %s%s%s", error, diagbib1_str(error),
            addinfo ? " -- " : "", addinfo ? addinfo : "");
    return zget_surrogateDiagRec(assoc->encode, dbname, error, addinfo);
}

static Z_Records *pack_records(association *a, char *setname, Odr_int start,
                               Odr_int *num, Z_RecordComposition *comp,
                               Odr_int *next, Odr_int *pres,
                               Z_ReferenceId *referenceId,
                               Odr_oid *oid, int *errcode)
{
    int recno;
    Odr_int dumped_records = 0;
    int toget = odr_int_to_int(*num);
    Z_Records *records =
        (Z_Records *) odr_malloc(a->encode, sizeof(*records));
    Z_NamePlusRecordList *reclist =
        (Z_NamePlusRecordList *) odr_malloc(a->encode, sizeof(*reclist));

    records->which = Z_Records_DBOSD;
    records->u.databaseOrSurDiagnostics = reclist;
    reclist->num_records = 0;

    if (toget < 0)
        return diagrec(a, YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE, 0);
    else if (toget == 0)
        reclist->records = odr_nullval();
    else
        reclist->records = (Z_NamePlusRecord **)
            odr_malloc(a->encode, sizeof(*reclist->records) * toget);

    *pres = Z_PresentStatus_success;
    *num = 0;
    *next = 0;

    yaz_log(log_requestdetail, "Request to pack " ODR_INT_PRINTF "+%d %s", start, toget, setname);
    yaz_log(log_requestdetail, "pms=%d, mrs=%d", a->preferredMessageSize,
        a->maximumRecordSize);
    for (recno = odr_int_to_int(start); reclist->num_records < toget; recno++)
    {
        bend_fetch_rr freq;
        Z_NamePlusRecord *thisrec;
        Odr_int this_length = 0;
        /*
         * we get the number of bytes allocated on the stream before any
         * allocation done by the backend - this should give us a reasonable
         * idea of the total size of the data so far.
         */
        Odr_int total_length = odr_total(a->encode) - dumped_records;
        freq.errcode = 0;
        freq.errstring = 0;
        freq.basename = 0;
        freq.len = 0;
        freq.record = 0;
        freq.last_in_set = 0;
        freq.setname = setname;
        freq.surrogate_flag = 0;
        freq.number = recno;
        freq.comp = comp;
        freq.request_format = oid;
        freq.output_format = 0;
        freq.stream = a->encode;
        freq.print = a->print;
        freq.referenceId = referenceId;
        freq.schema = 0;

        retrieve_fetch(a, &freq);

        *next = freq.last_in_set ? 0 : recno + 1;

        if (freq.errcode)
        {
            if (!freq.surrogate_flag) /* non-surrogate diagnostic i.e. global */
            {
                char s[20];
                *pres = Z_PresentStatus_failure;
                /* for 'present request out of range',
                   set addinfo to record position if not set */
                if (freq.errcode == YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE  &&
                                freq.errstring == 0)
                {
                    sprintf(s, "%d", recno);
                    freq.errstring = s;
                }
                if (errcode)
                    *errcode = freq.errcode;
                return diagrec(a, freq.errcode, freq.errstring);
            }
            reclist->records[reclist->num_records] =
                surrogatediagrec(a, freq.basename, freq.errcode,
                                 freq.errstring);
            reclist->num_records++;
            continue;
        }
        if (freq.record == 0)  /* no error and no record ? */
        {
            *pres = Z_PresentStatus_partial_4;
            *next = 0;   /* signal end-of-set and stop */
            break;
        }
        if (freq.len >= 0)
            this_length = freq.len;
        else
            this_length = odr_total(a->encode) - total_length - dumped_records;
        yaz_log(log_requestdetail, "  fetched record, len=" ODR_INT_PRINTF
                " total=" ODR_INT_PRINTF " dumped=" ODR_INT_PRINTF,
                this_length, total_length, dumped_records);
        if (a->preferredMessageSize > 0 &&
            this_length + total_length > a->preferredMessageSize)
        {
            /* record is small enough, really */
            if (this_length <= a->preferredMessageSize && recno > start)
            {
                yaz_log(log_requestdetail, "  Dropped last normal-sized record");
                *pres = Z_PresentStatus_partial_2;
                if (*next > 0)
                    (*next)--;
                break;
            }
            /* record can only be fetched by itself */
            if (this_length < a->maximumRecordSize)
            {
                yaz_log(log_requestdetail, "  Record > prefmsgsz");
                if (toget > 1)
                {
                    yaz_log(YLOG_DEBUG, "  Dropped it");
                    reclist->records[reclist->num_records] =
                         surrogatediagrec(
                             a, freq.basename,
                             YAZ_BIB1_RECORD_EXCEEDS_PREFERRED_MESSAGE_SIZE, 0);
                    reclist->num_records++;
                    dumped_records += this_length;
                    continue;
                }
            }
            else /* too big entirely */
            {
                yaz_log(log_requestdetail, "Record > maxrcdsz "
                        "this=" ODR_INT_PRINTF " max=%d",
                        this_length, a->maximumRecordSize);
                reclist->records[reclist->num_records] =
                    surrogatediagrec(
                        a, freq.basename,
                        YAZ_BIB1_RECORD_EXCEEDS_MAXIMUM_RECORD_SIZE, 0);
                reclist->num_records++;
                dumped_records += this_length;
                continue;
            }
        }

        if (!(thisrec = (Z_NamePlusRecord *)
              odr_malloc(a->encode, sizeof(*thisrec))))
            return 0;
        thisrec->databaseName = odr_strdup_null(a->encode, freq.basename);
        thisrec->which = Z_NamePlusRecord_databaseRecord;

        if (!freq.output_format)
        {
            yaz_log(YLOG_WARN, "bend_fetch output_format not set");
            return 0;
        }
        thisrec->u.databaseRecord = z_ext_record_oid(
            a->encode, freq.output_format, freq.record, freq.len);
        if (!thisrec->u.databaseRecord)
            return 0;
        reclist->records[reclist->num_records] = thisrec;
        reclist->num_records++;
        if (freq.last_in_set)
            break;
    }
    *num = reclist->num_records;
    return records;
}

static Z_APDU *process_searchRequest(association *assoc, request *reqb)
{
    Z_SearchRequest *req = reqb->apdu_request->u.searchRequest;
    bend_search_rr *bsrr =
        (bend_search_rr *)nmem_malloc(reqb->request_mem, sizeof(*bsrr));

    yaz_log(log_requestdetail, "Got SearchRequest.");
    bsrr->association = assoc;
    bsrr->referenceId = req->referenceId;
    bsrr->srw_sortKeys = 0;
    bsrr->srw_setname = 0;
    bsrr->srw_setnameIdleTime = 0;
    bsrr->estimated_hit_count = 0;
    bsrr->partial_resultset = 0;
    bsrr->extra_args = 0;
    bsrr->extra_response_data = 0;

    yaz_log(log_requestdetail, "ResultSet '%s'", req->resultSetName);
    if (req->databaseNames)
    {
        int i;
        for (i = 0; i < req->num_databaseNames; i++)
            yaz_log(log_requestdetail, "Database '%s'", req->databaseNames[i]);
    }

    yaz_log_zquery_level(log_requestdetail,req->query);

    if (assoc->init->bend_search)
    {
        bsrr->setname = req->resultSetName;
        bsrr->replace_set = *req->replaceIndicator;
        bsrr->num_bases = req->num_databaseNames;
        bsrr->basenames = req->databaseNames;
        bsrr->query = req->query;
        bsrr->stream = assoc->encode;
        nmem_transfer(odr_getmem(bsrr->stream), reqb->request_mem);
        bsrr->decode = assoc->decode;
        bsrr->print = assoc->print;
        bsrr->hits = 0;
        bsrr->errcode = 0;
        bsrr->errstring = NULL;
        bsrr->search_info = NULL;
        bsrr->search_input = req->additionalSearchInfo;
        if (!bsrr->search_input)
            bsrr->search_input = req->otherInfo;
        bsrr->present_number = *req->mediumSetPresentNumber;

        if (assoc->server && assoc->server->client_query_charset &&
            req->query->which == Z_Query_type_1)
        {
            yaz_iconv_t cd =
                yaz_iconv_open("UTF-8", assoc->server->client_query_charset);
            if (cd)
            {
                yaz_query_charset_convert_rpnquery(req->query->u.type_1,
                                                   assoc->decode, cd);
                yaz_iconv_close(cd);
            }
        }

        if (assoc->server && assoc->server->cql_transform
            && req->query->which == Z_Query_type_104
            && req->query->u.type_104->which == Z_External_CQL)
        {
            /* have a CQL query and a CQL to PQF transform .. */
            int srw_errcode =
                cql2pqf(bsrr->stream, req->query->u.type_104->u.cql,
                        assoc->server->cql_transform, bsrr->query,
                        &bsrr->srw_sortKeys);
            if (srw_errcode)
                bsrr->errcode = yaz_diag_srw_to_bib1(srw_errcode);
        }

        if (assoc->server && assoc->server->ccl_transform
            && req->query->which == Z_Query_type_2) /*CCL*/
        {
            /* have a CCL query and a CCL to PQF transform .. */
            int srw_errcode =
                ccl2pqf(bsrr->stream, req->query->u.type_2,
                        assoc->server->ccl_transform, bsrr);
            if (srw_errcode)
                bsrr->errcode = yaz_diag_srw_to_bib1(srw_errcode);
        }

        if (!bsrr->errcode)
            (assoc->init->bend_search)(assoc->backend, bsrr);
    }
    else
    {
        /* FIXME - make a diagnostic for it */
        yaz_log(YLOG_WARN,"Search not supported ?!?!");
    }
    return response_searchRequest(assoc, reqb, bsrr);
}

/*
 * Prepare a searchresponse based on the backend results. We probably want
 * to look at making the fetching of records nonblocking as well, but
 * so far, we'll keep things simple.
 * If bsrt is null, that means we're called in response to a communications
 * event, and we'll have to get the response for ourselves.
 */
static Z_APDU *response_searchRequest(association *assoc, request *reqb,
                                      bend_search_rr *bsrt)
{
    Z_SearchRequest *req = reqb->apdu_request->u.searchRequest;
    Z_APDU *apdu = (Z_APDU *)odr_malloc(assoc->encode, sizeof(*apdu));
    Z_SearchResponse *resp = (Z_SearchResponse *)
        odr_malloc(assoc->encode, sizeof(*resp));
    Odr_int *nulint = odr_intdup(assoc->encode, 0);
    Odr_int *next = odr_intdup(assoc->encode, 0);
    Odr_int *none = odr_intdup(assoc->encode, Z_SearchResponse_none);
    Odr_int returnedrecs = 0;

    apdu->which = Z_APDU_searchResponse;
    apdu->u.searchResponse = resp;
    resp->referenceId = req->referenceId;
    resp->additionalSearchInfo = 0;
    resp->otherInfo = 0;
    if (!bsrt)
    {
        yaz_log(YLOG_FATAL, "Bad result from backend");
        return 0;
    }
    else if (bsrt->errcode)
    {
        resp->records = diagrec(assoc, bsrt->errcode, bsrt->errstring);
        resp->resultCount = nulint;
        resp->numberOfRecordsReturned = nulint;
        resp->nextResultSetPosition = nulint;
        resp->searchStatus = odr_booldup(assoc->encode, 0);
        resp->resultSetStatus = none;
        resp->presentStatus = 0;
    }
    else
    {
        bool_t *sr = odr_booldup(assoc->encode, 1);
        Odr_int *toget = odr_intdup(assoc->encode, 0);
        Z_RecordComposition comp, *compp = 0;

        yaz_log(log_requestdetail, "resultCount: " ODR_INT_PRINTF, bsrt->hits);

        resp->records = 0;
        resp->resultCount = &bsrt->hits;

        comp.which = Z_RecordComp_simple;
        /* how many records does the user agent want, then? */
        if (bsrt->hits < 0)
            *toget = 0;
        else if (bsrt->hits <= *req->smallSetUpperBound)
        {
            *toget = bsrt->hits;
            if ((comp.u.simple = req->smallSetElementSetNames))
                compp = &comp;
        }
        else if (bsrt->hits < *req->largeSetLowerBound)
        {
            *toget = *req->mediumSetPresentNumber;
            if (*toget > bsrt->hits)
                *toget = bsrt->hits;
            if ((comp.u.simple = req->mediumSetElementSetNames))
                compp = &comp;
        }
        else
            *toget = 0;

        if (*toget && !resp->records)
        {
            Odr_int *presst = odr_intdup(assoc->encode, 0);
            /* Call bend_present if defined */
            if (assoc->init->bend_present)
            {
                bend_present_rr *bprr = (bend_present_rr *)
                    nmem_malloc(reqb->request_mem, sizeof(*bprr));
                bprr->setname = req->resultSetName;
                bprr->start = 1;
                bprr->number = odr_int_to_int(*toget);
                bprr->format = req->preferredRecordSyntax;
                bprr->comp = compp;
                bprr->referenceId = req->referenceId;
                bprr->stream = assoc->encode;
                bprr->print = assoc->print;
                bprr->association = assoc;
                bprr->errcode = 0;
                bprr->errstring = NULL;
                (*assoc->init->bend_present)(assoc->backend, bprr);

                if (bprr->errcode)
                {
                    resp->records = diagrec(assoc, bprr->errcode, bprr->errstring);
                    *resp->presentStatus = Z_PresentStatus_failure;
                }
            }

            if (!resp->records)
                resp->records = pack_records(
                    assoc, req->resultSetName, 1,
                    toget, compp, next, presst, req->referenceId,
                    req->preferredRecordSyntax, NULL);
            if (!resp->records)
                return 0;
            resp->numberOfRecordsReturned = toget;
            returnedrecs = *toget;
            resp->presentStatus = presst;
        }
        else
        {
            if (*resp->resultCount)
                *next = 1;
            resp->numberOfRecordsReturned = nulint;
            resp->presentStatus = 0;
        }
        resp->nextResultSetPosition = next;
        resp->searchStatus = sr;
        resp->resultSetStatus = 0;
        if (bsrt->estimated_hit_count)
        {
            resp->resultSetStatus = odr_intdup(assoc->encode,
                                               Z_SearchResponse_estimate);
        }
        else if (bsrt->partial_resultset)
        {
            resp->resultSetStatus = odr_intdup(assoc->encode,
                                               Z_SearchResponse_subset);
        }
    }
    resp->additionalSearchInfo = bsrt->search_info;

    if (log_request)
    {
        int i;
        WRBUF wr = wrbuf_alloc();

        for (i = 0 ; i < req->num_databaseNames; i++)
        {
            if (i)
                wrbuf_printf(wr, "+");
            wrbuf_puts(wr, req->databaseNames[i]);
        }
        wrbuf_printf(wr, " ");

        if (bsrt->errcode)
            wrbuf_printf(wr, "ERROR %d", bsrt->errcode);
        else
            wrbuf_printf(wr, "OK " ODR_INT_PRINTF, bsrt->hits);
        wrbuf_printf(wr, " %s 1+" ODR_INT_PRINTF " ",
                     req->resultSetName, returnedrecs);
        yaz_query_to_wrbuf(wr, req->query);

        yaz_log(log_request, "Search %s", wrbuf_cstr(wr));
        wrbuf_destroy(wr);
    }
    return apdu;
}

/*
 * Maybe we got a little over-friendly when we designed bend_fetch to
 * get only one record at a time. Some backends can optimise multiple-record
 * fetches, and at any rate, there is some overhead involved in
 * all that selecting and hopping around. Problem is, of course, that the
 * frontend can't know ahead of time how many records it'll need to
 * fill the negotiated PDU size. Annoying. Segmentation or not, Z/SR
 * is downright lousy as a bulk data transfer protocol.
 *
 * To start with, we'll do the fetching of records from the backend
 * in one operation: To save some trips in and out of the event-handler,
 * and to simplify the interface to pack_records. At any rate, asynch
 * operation is more fun in operations that have an unpredictable execution
 * speed - which is normally more true for search than for present.
 */
static Z_APDU *process_presentRequest(association *assoc, request *reqb)
{
    Z_PresentRequest *req = reqb->apdu_request->u.presentRequest;
    Z_APDU *apdu;
    Z_PresentResponse *resp;
    Odr_int *next;
    Odr_int *num;
    int errcode = 0;

    yaz_log(log_requestdetail, "Got PresentRequest.");

    resp = (Z_PresentResponse *)odr_malloc(assoc->encode, sizeof(*resp));
    resp->records = 0;
    resp->presentStatus = odr_intdup(assoc->encode, 0);
    if (assoc->init->bend_present)
    {
        bend_present_rr *bprr = (bend_present_rr *)
            nmem_malloc(reqb->request_mem, sizeof(*bprr));
        bprr->setname = req->resultSetId;
        bprr->start = odr_int_to_int(*req->resultSetStartPoint);
        bprr->number = odr_int_to_int(*req->numberOfRecordsRequested);
        bprr->format = req->preferredRecordSyntax;
        bprr->comp = req->recordComposition;
        bprr->referenceId = req->referenceId;
        bprr->stream = assoc->encode;
        bprr->print = assoc->print;
        bprr->association = assoc;
        bprr->errcode = 0;
        bprr->errstring = NULL;
        (*assoc->init->bend_present)(assoc->backend, bprr);

        if (bprr->errcode)
        {
            resp->records = diagrec(assoc, bprr->errcode, bprr->errstring);
            *resp->presentStatus = Z_PresentStatus_failure;
            errcode = bprr->errcode;
        }
    }
    apdu = (Z_APDU *)odr_malloc(assoc->encode, sizeof(*apdu));
    next = odr_intdup(assoc->encode, 0);
    num = odr_intdup(assoc->encode, 0);

    apdu->which = Z_APDU_presentResponse;
    apdu->u.presentResponse = resp;
    resp->referenceId = req->referenceId;
    resp->otherInfo = 0;

    if (!resp->records)
    {
        *num = *req->numberOfRecordsRequested;
        resp->records =
            pack_records(assoc, req->resultSetId, *req->resultSetStartPoint,
                         num, req->recordComposition, next,
                         resp->presentStatus,
                         req->referenceId, req->preferredRecordSyntax,
                         &errcode);
    }
    if (log_request)
    {
        WRBUF wr = wrbuf_alloc();
        wrbuf_printf(wr, "Present ");

        if (*resp->presentStatus == Z_PresentStatus_failure)
            wrbuf_printf(wr, "ERROR %d ", errcode);
        else if (*resp->presentStatus == Z_PresentStatus_success)
            wrbuf_printf(wr, "OK -  ");
        else
            wrbuf_printf(wr, "Partial " ODR_INT_PRINTF " - ",
                         *resp->presentStatus);

        wrbuf_printf(wr, " %s " ODR_INT_PRINTF "+" ODR_INT_PRINTF " ",
                req->resultSetId, *req->resultSetStartPoint,
                *req->numberOfRecordsRequested);
        yaz_log(log_request, "%s", wrbuf_cstr(wr) );
        wrbuf_destroy(wr);
    }
    if (!resp->records)
        return 0;
    resp->numberOfRecordsReturned = num;
    resp->nextResultSetPosition = next;

    return apdu;
}

/*
 * Scan was implemented rather in a hurry, and with support for only the basic
 * elements of the service in the backend API. Suggestions are welcome.
 */
static Z_APDU *process_scanRequest(association *assoc, request *reqb)
{
    Z_ScanRequest *req = reqb->apdu_request->u.scanRequest;
    Z_APDU *apdu = (Z_APDU *)odr_malloc(assoc->encode, sizeof(*apdu));
    Z_ScanResponse *res = (Z_ScanResponse *)
        odr_malloc(assoc->encode, sizeof(*res));
    Odr_int *scanStatus = odr_intdup(assoc->encode, Z_Scan_failure);
    Odr_int *numberOfEntriesReturned = odr_intdup(assoc->encode, 0);
    Z_ListEntries *ents = (Z_ListEntries *)
        odr_malloc(assoc->encode, sizeof(*ents));
    Z_DiagRecs *diagrecs_p = NULL;
    bend_scan_rr *bsrr = (bend_scan_rr *)
        odr_malloc(assoc->encode, sizeof(*bsrr));
    struct scan_entry *save_entries;
    int step_size = 0;

    yaz_log(log_requestdetail, "Got ScanRequest");

    apdu->which = Z_APDU_scanResponse;
    apdu->u.scanResponse = res;
    res->referenceId = req->referenceId;

    /* if step is absent, set it to 0 */
    if (req->stepSize)
        step_size = odr_int_to_int(*req->stepSize);

    res->stepSize = 0;
    res->scanStatus = scanStatus;
    res->numberOfEntriesReturned = numberOfEntriesReturned;
    res->positionOfTerm = 0;
    res->entries = ents;
    ents->num_entries = 0;
    ents->entries = NULL;
    ents->num_nonsurrogateDiagnostics = 0;
    ents->nonsurrogateDiagnostics = NULL;
    res->attributeSet = 0;
    res->otherInfo = 0;

    if (req->databaseNames)
    {
        int i;
        for (i = 0; i < req->num_databaseNames; i++)
            yaz_log(log_requestdetail, "Database '%s'", req->databaseNames[i]);
    }
    bsrr->scanClause = 0;
    bsrr->errcode = 0;
    bsrr->errstring = 0;
    bsrr->num_bases = req->num_databaseNames;
    bsrr->basenames = req->databaseNames;
    bsrr->num_entries = odr_int_to_int(*req->numberOfTermsRequested);
    bsrr->term = req->termListAndStartPoint;
    bsrr->referenceId = req->referenceId;
    bsrr->stream = assoc->encode;
    bsrr->print = assoc->print;
    bsrr->step_size = &step_size;
    bsrr->setname = yaz_oi_get_string_oid(&req->otherInfo,
                                          yaz_oid_userinfo_scan_set, 1, 0);
    bsrr->entries = 0;
    bsrr->extra_args = 0;
    bsrr->extra_response_data = 0;
    /* For YAZ 2.0 and earlier it was the backend handler that
       initialized entries (member display_term did not exist)
       YAZ 2.0 and later sets 'entries'  and initialize all members
       including 'display_term'. If YAZ 2.0 or later sees that
       entries was modified - we assume that it is an old handler and
       that 'display_term' is _not_ set.
    */
    if (bsrr->num_entries > 0)
    {
        int i;
        bsrr->entries = (struct scan_entry *)
            odr_malloc(assoc->decode, sizeof(*bsrr->entries) *
                       bsrr->num_entries);
        for (i = 0; i<bsrr->num_entries; i++)
        {
            bsrr->entries[i].term = 0;
            bsrr->entries[i].occurrences = 0;
            bsrr->entries[i].errcode = 0;
            bsrr->entries[i].errstring = 0;
            bsrr->entries[i].display_term = 0;
        }
    }
    save_entries = bsrr->entries;  /* save it so we can compare later */

    bsrr->attributeset = req->attributeSet;
    log_scan_term_level(log_requestdetail, req->termListAndStartPoint,
                        bsrr->attributeset);
    bsrr->term_position = req->preferredPositionInResponse ?
        odr_int_to_int(*req->preferredPositionInResponse) : 1;

    ((int (*)(void *, bend_scan_rr *))
     (*assoc->init->bend_scan))(assoc->backend, bsrr);

    if (bsrr->errcode)
        diagrecs_p = zget_DiagRecs(assoc->encode,
                                   bsrr->errcode, bsrr->errstring);
    else
    {
        int i;
        Z_Entry **tab = (Z_Entry **)
            odr_malloc(assoc->encode, sizeof(*tab) * bsrr->num_entries);

        if (bsrr->status == BEND_SCAN_PARTIAL)
            *scanStatus = Z_Scan_partial_5;
        else
            *scanStatus = Z_Scan_success;
        res->stepSize = odr_intdup(assoc->encode, step_size);
        ents->entries = tab;
        ents->num_entries = bsrr->num_entries;
        res->numberOfEntriesReturned = odr_intdup(assoc->encode,
                                                   ents->num_entries);
        res->positionOfTerm = odr_intdup(assoc->encode, bsrr->term_position);
        for (i = 0; i < bsrr->num_entries; i++)
        {
            Z_Entry *e;
            Z_TermInfo *t;

            tab[i] = e = (Z_Entry *)odr_malloc(assoc->encode, sizeof(*e));
            if (bsrr->entries[i].occurrences >= 0)
            {
                e->which = Z_Entry_termInfo;
                e->u.termInfo = t = (Z_TermInfo *)
                    odr_malloc(assoc->encode, sizeof(*t));
                t->suggestedAttributes = 0;
                t->displayTerm = 0;
                if (save_entries == bsrr->entries &&
                    bsrr->entries[i].display_term)
                {
                    /* the entries was _not_ set by the handler. So it's
                       safe to test for new member display_term. It is
                       NULL'ed by us.
                    */
                    t->displayTerm = odr_strdup(assoc->encode,
                                                bsrr->entries[i].display_term);
                }
                t->alternativeTerm = 0;
                t->byAttributes = 0;
                t->otherTermInfo = 0;
                t->globalOccurrences = &bsrr->entries[i].occurrences;
                t->term = (Z_Term *)
                    odr_malloc(assoc->encode, sizeof(*t->term));
                t->term->which = Z_Term_general;
                t->term->u.general =
                    odr_create_Odr_oct(assoc->encode,
                                       bsrr->entries[i].term,
                                       strlen(bsrr->entries[i].term));
                yaz_log(YLOG_DEBUG, "  term #%d: '%s' (" ODR_INT_PRINTF ")", i,
                         bsrr->entries[i].term, bsrr->entries[i].occurrences);
            }
            else
            {
                Z_DiagRecs *drecs = zget_DiagRecs(assoc->encode,
                                                  bsrr->entries[i].errcode,
                                                  bsrr->entries[i].errstring);
                assert(drecs->num_diagRecs == 1);
                e->which = Z_Entry_surrogateDiagnostic;
                assert(drecs->diagRecs[0]);
                e->u.surrogateDiagnostic = drecs->diagRecs[0];
            }
        }
    }
    if (diagrecs_p)
    {
        ents->num_nonsurrogateDiagnostics = diagrecs_p->num_diagRecs;
        ents->nonsurrogateDiagnostics = diagrecs_p->diagRecs;
    }
    if (log_request)
    {
        int i;
        WRBUF wr = wrbuf_alloc();
        wrbuf_printf(wr, "Scan ");
        for (i = 0 ; i < req->num_databaseNames; i++)
        {
            if (i)
                wrbuf_printf(wr, "+");
            wrbuf_puts(wr, req->databaseNames[i]);
        }

        wrbuf_printf(wr, " ");

        if (bsrr->errcode)
            wr_diag(wr, bsrr->errcode, bsrr->errstring);
        else
            wrbuf_printf(wr, "OK");

        wrbuf_printf(wr, " " ODR_INT_PRINTF " - " ODR_INT_PRINTF "+"
                     ODR_INT_PRINTF "+" ODR_INT_PRINTF,
                     res->numberOfEntriesReturned ?
                     *res->numberOfEntriesReturned : 0,
                     (req->preferredPositionInResponse ?
                      *req->preferredPositionInResponse : 1),
                     *req->numberOfTermsRequested,
                     (res->stepSize ? *res->stepSize : 1));

        if (bsrr->setname)
            wrbuf_printf(wr, "+%s", bsrr->setname);

        wrbuf_printf(wr, " ");
        yaz_scan_to_wrbuf(wr, req->termListAndStartPoint,
                          bsrr->attributeset);
        yaz_log(log_request, "%s", wrbuf_cstr(wr) );
        wrbuf_destroy(wr);
    }
    return apdu;
}

static Z_APDU *process_sortRequest(association *assoc, request *reqb)
{
    int i;
    Z_SortRequest *req = reqb->apdu_request->u.sortRequest;
    Z_SortResponse *res = (Z_SortResponse *)
        odr_malloc(assoc->encode, sizeof(*res));
    bend_sort_rr *bsrr = (bend_sort_rr *)
        odr_malloc(assoc->encode, sizeof(*bsrr));

    Z_APDU *apdu = (Z_APDU *)odr_malloc(assoc->encode, sizeof(*apdu));

    yaz_log(log_requestdetail, "Got SortRequest.");

    bsrr->num_input_setnames = req->num_inputResultSetNames;
    for (i=0;i<req->num_inputResultSetNames;i++)
        yaz_log(log_requestdetail, "Input resultset: '%s'",
                req->inputResultSetNames[i]);
    bsrr->input_setnames = req->inputResultSetNames;
    bsrr->referenceId = req->referenceId;
    bsrr->output_setname = req->sortedResultSetName;
    yaz_log(log_requestdetail, "Output resultset: '%s'",
                req->sortedResultSetName);
    bsrr->sort_sequence = req->sortSequence;
       /*FIXME - dump those sequences too */
    bsrr->stream = assoc->encode;
    bsrr->print = assoc->print;

    bsrr->sort_status = Z_SortResponse_failure;
    bsrr->errcode = 0;
    bsrr->errstring = 0;

    (*assoc->init->bend_sort)(assoc->backend, bsrr);

    res->referenceId = bsrr->referenceId;
    res->sortStatus = odr_intdup(assoc->encode, bsrr->sort_status);
    res->resultSetStatus = 0;
    if (bsrr->errcode)
    {
        Z_DiagRecs *dr = zget_DiagRecs(assoc->encode,
                                       bsrr->errcode, bsrr->errstring);
        res->diagnostics = dr->diagRecs;
        res->num_diagnostics = dr->num_diagRecs;
    }
    else
    {
        res->num_diagnostics = 0;
        res->diagnostics = 0;
    }
    res->resultCount = 0;
    res->otherInfo = 0;

    apdu->which = Z_APDU_sortResponse;
    apdu->u.sortResponse = res;
    if (log_request)
    {
        WRBUF wr = wrbuf_alloc();
        wrbuf_printf(wr, "Sort ");
        if (bsrr->errcode)
            wrbuf_printf(wr, " ERROR %d", bsrr->errcode);
        else
            wrbuf_printf(wr,  "OK -");
        wrbuf_printf(wr, " (");
        for (i = 0; i<req->num_inputResultSetNames; i++)
        {
            if (i)
                wrbuf_printf(wr, "+");
            wrbuf_puts(wr, req->inputResultSetNames[i]);
        }
        wrbuf_printf(wr, ")->%s ",req->sortedResultSetName);

        yaz_log(log_request, "%s", wrbuf_cstr(wr) );
        wrbuf_destroy(wr);
    }
    return apdu;
}

static Z_APDU *process_deleteRequest(association *assoc, request *reqb)
{
    int i;
    Z_DeleteResultSetRequest *req =
        reqb->apdu_request->u.deleteResultSetRequest;
    Z_DeleteResultSetResponse *res = (Z_DeleteResultSetResponse *)
        odr_malloc(assoc->encode, sizeof(*res));
    bend_delete_rr *bdrr = (bend_delete_rr *)
        odr_malloc(assoc->encode, sizeof(*bdrr));
    Z_APDU *apdu = (Z_APDU *)odr_malloc(assoc->encode, sizeof(*apdu));

    yaz_log(log_requestdetail, "Got DeleteRequest.");

    bdrr->num_setnames = req->num_resultSetList;
    bdrr->setnames = req->resultSetList;
    for (i = 0; i<req->num_resultSetList; i++)
        yaz_log(log_requestdetail, "resultset: '%s'",
                req->resultSetList[i]);
    bdrr->stream = assoc->encode;
    bdrr->print = assoc->print;
    bdrr->function = odr_int_to_int(*req->deleteFunction);
    bdrr->referenceId = req->referenceId;
    bdrr->statuses = 0;
    if (bdrr->num_setnames > 0)
    {
        bdrr->statuses = (int*)
            odr_malloc(assoc->encode, sizeof(*bdrr->statuses) *
                       bdrr->num_setnames);
        for (i = 0; i < bdrr->num_setnames; i++)
            bdrr->statuses[i] = 0;
    }
    (*assoc->init->bend_delete)(assoc->backend, bdrr);

    res->referenceId = req->referenceId;

    res->deleteOperationStatus = odr_intdup(assoc->encode,bdrr->delete_status);

    res->deleteListStatuses = 0;
    if (bdrr->num_setnames > 0)
    {
        int i;
        res->deleteListStatuses = (Z_ListStatuses *)
            odr_malloc(assoc->encode, sizeof(*res->deleteListStatuses));
        res->deleteListStatuses->num = bdrr->num_setnames;
        res->deleteListStatuses->elements =
            (Z_ListStatus **)
            odr_malloc(assoc->encode,
                        sizeof(*res->deleteListStatuses->elements) *
                        bdrr->num_setnames);
        for (i = 0; i<bdrr->num_setnames; i++)
        {
            res->deleteListStatuses->elements[i] =
                (Z_ListStatus *)
                odr_malloc(assoc->encode,
                            sizeof(**res->deleteListStatuses->elements));
            res->deleteListStatuses->elements[i]->status =
                odr_intdup(assoc->encode, bdrr->statuses[i]);
            res->deleteListStatuses->elements[i]->id =
                odr_strdup(assoc->encode, bdrr->setnames[i]);
        }
    }
    res->numberNotDeleted = 0;
    res->bulkStatuses = 0;
    res->deleteMessage = 0;
    res->otherInfo = 0;

    apdu->which = Z_APDU_deleteResultSetResponse;
    apdu->u.deleteResultSetResponse = res;
    if (log_request)
    {
        WRBUF wr = wrbuf_alloc();
        wrbuf_printf(wr, "Delete ");
        if (bdrr->delete_status)
            wrbuf_printf(wr, "ERROR %d", bdrr->delete_status);
        else
            wrbuf_printf(wr, "OK -");
        for (i = 0; i<req->num_resultSetList; i++)
            wrbuf_printf(wr, " %s ", req->resultSetList[i]);
        yaz_log(log_request, "%s", wrbuf_cstr(wr) );
        wrbuf_destroy(wr);
    }
    return apdu;
}

static void process_close(association *assoc, request *reqb)
{
    Z_Close *req = reqb->apdu_request->u.close;
    static char *reasons[] =
    {
        "finished",
        "shutdown",
        "systemProblem",
        "costLimit",
        "resources",
        "securityViolation",
        "protocolError",
        "lackOfActivity",
        "peerAbort",
        "unspecified"
    };

    yaz_log(log_requestdetail, "Got Close, reason %s, message %s",
        reasons[*req->closeReason], req->diagnosticInformation ?
        req->diagnosticInformation : "NULL");
    if (assoc->version < 3) /* to make do_force respond with close */
        assoc->version = 3;
    do_close_req(assoc, Z_Close_finished,
                 "Association terminated by client", reqb);
    yaz_log(log_request,"Close OK");
}

static Z_APDU *process_segmentRequest(association *assoc, request *reqb)
{
    bend_segment_rr req;

    req.segment = reqb->apdu_request->u.segmentRequest;
    req.stream = assoc->encode;
    req.decode = assoc->decode;
    req.print = assoc->print;
    req.association = assoc;

    (*assoc->init->bend_segment)(assoc->backend, &req);

    return 0;
}

static Z_APDU *process_ESRequest(association *assoc, request *reqb)
{
    bend_esrequest_rr esrequest;
    char oidname_buf[OID_STR_MAX];
    const char *ext_name = "unknown";

    Z_ExtendedServicesRequest *req =
        reqb->apdu_request->u.extendedServicesRequest;
    Z_External *ext =
        reqb->apdu_request->u.extendedServicesRequest->taskSpecificParameters;
    Z_APDU *apdu = zget_APDU(assoc->encode, Z_APDU_extendedServicesResponse);

    Z_ExtendedServicesResponse *resp = apdu->u.extendedServicesResponse;

    esrequest.esr = reqb->apdu_request->u.extendedServicesRequest;
    esrequest.stream = assoc->encode;
    esrequest.decode = assoc->decode;
    esrequest.print = assoc->print;
    esrequest.errcode = 0;
    esrequest.errstring = NULL;
    esrequest.association = assoc;
    esrequest.taskPackage = 0;
    esrequest.taskPackageExt = 0;
    esrequest.referenceId = req->referenceId;

    if (ext)
    {
        oid_class oclass;
        ext_name = yaz_oid_to_string_buf(ext->direct_reference, &oclass,
                                         oidname_buf);
    }

    (*assoc->init->bend_esrequest)(assoc->backend, &esrequest);

    resp->referenceId = req->referenceId;

    if (esrequest.errcode == -1)
    {
        /* Backend service indicates request will be processed */
        yaz_log(log_request, "Extended Service: %s (accepted)", ext_name);
        *resp->operationStatus = Z_ExtendedServicesResponse_accepted;
    }
    else if (esrequest.errcode == 0)
    {
        /* Backend service indicates request will be processed */
        yaz_log(log_request, "Extended Service: %s (done)", ext_name);
        *resp->operationStatus = Z_ExtendedServicesResponse_done;
    }
    else if (esrequest.errcode == -2)
    {
        Z_DiagRecs *diagRecs =
            zget_DiagRecs(assoc->encode, 401, esrequest.errstring);
        yaz_log(log_request, "Extended Service: %s (401)", ext_name);
        *resp->operationStatus = Z_ExtendedServicesResponse_done;
        resp->num_diagnostics = diagRecs->num_diagRecs;
        resp->diagnostics = diagRecs->diagRecs;
        if (log_request)
        {
            WRBUF wr = wrbuf_alloc();
            wrbuf_diags(wr, resp->num_diagnostics, resp->diagnostics);
            yaz_log(log_request, "EsRequest %s", wrbuf_cstr(wr) );
            wrbuf_destroy(wr);
        }
    }
    else
    {
        Z_DiagRecs *diagRecs =
            zget_DiagRecs(assoc->encode, esrequest.errcode,
                          esrequest.errstring);
        /* Backend indicates error, request will not be processed */
        yaz_log(log_request, "Extended Service: %s (failed)", ext_name);
        *resp->operationStatus = Z_ExtendedServicesResponse_failure;
        resp->num_diagnostics = diagRecs->num_diagRecs;
        resp->diagnostics = diagRecs->diagRecs;
        if (log_request)
        {
            WRBUF wr = wrbuf_alloc();
            wrbuf_diags(wr, resp->num_diagnostics, resp->diagnostics);
            yaz_log(log_request, "EsRequest %s", wrbuf_cstr(wr) );
            wrbuf_destroy(wr);
        }

    }
    /* Do something with the members of bend_extendedservice */
    resp->taskPackage = esrequest.taskPackageExt;
    if (esrequest.taskPackage)
    {
        resp->taskPackage = z_ext_record_oid(
            assoc->encode, yaz_oid_recsyn_extended,
            (const char *)  esrequest.taskPackage, -1);
    }
    yaz_log(YLOG_DEBUG,"Send the result apdu");
    return apdu;
}

int bend_assoc_is_alive(bend_association assoc)
{
    if (assoc->state == ASSOC_DEAD)
        return 0; /* already marked as dead. Don't check I/O chan anymore */

    return iochan_is_alive(assoc->client_chan);
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

