/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: seshigh.c,v 1.153 2003-04-18 15:11:04 adam Exp $
 */

/*
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef WIN32
#include <io.h>
#define S_ISREG(x) (x & _S_IFREG)
#include <process.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <assert.h>
#include <ctype.h>

#include <yaz/yconfig.h>
#include <yaz/xmalloc.h>
#include <yaz/comstack.h>
#include "eventl.h"
#include "session.h"
#include <yaz/proto.h>
#include <yaz/oid.h>
#include <yaz/log.h>
#include <yaz/logrpn.h>
#include <yaz/statserv.h>
#include <yaz/diagbib1.h>
#include <yaz/charneg.h>
#include <yaz/otherinfo.h>
#include <yaz/yaz-util.h>
#include <yaz/pquery.h>

#include <yaz/srw.h>
#include <yaz/backend.h>

static void process_gdu_request(association *assoc, request *req);
static int process_z_request(association *assoc, request *req, char **msg);
void backend_response(IOCHAN i, int event);
static int process_gdu_response(association *assoc, request *req, Z_GDU *res);
static int process_z_response(association *assoc, request *req, Z_APDU *res);
static Z_APDU *process_initRequest(association *assoc, request *reqb);
static Z_APDU *process_searchRequest(association *assoc, request *reqb,
    int *fd);
static Z_APDU *response_searchRequest(association *assoc, request *reqb,
    bend_search_rr *bsrr, int *fd);
static Z_APDU *process_presentRequest(association *assoc, request *reqb,
    int *fd);
static Z_APDU *process_scanRequest(association *assoc, request *reqb, int *fd);
static Z_APDU *process_sortRequest(association *assoc, request *reqb, int *fd);
static void process_close(association *assoc, request *reqb);
void save_referenceId (request *reqb, Z_ReferenceId *refid);
static Z_APDU *process_deleteRequest(association *assoc, request *reqb,
    int *fd);
static Z_APDU *process_segmentRequest (association *assoc, request *reqb);

static FILE *apduf = 0; /* for use in static mode */
static statserv_options_block *control_block = 0;

static Z_APDU *process_ESRequest(association *assoc, request *reqb, int *fd);

/*
 * Create and initialize a new association-handle.
 *  channel  : iochannel for the current line.
 *  link     : communications channel.
 * Returns: 0 or a new association handle.
 */
association *create_association(IOCHAN channel, COMSTACK link)
{
    association *anew;

    if (!control_block)
    	control_block = statserv_getcontrol();
    if (!(anew = (association *)xmalloc(sizeof(*anew))))
    	return 0;
    anew->init = 0;
    anew->version = 0;
    anew->client_chan = channel;
    anew->client_link = link;
    anew->cs_get_mask = 0;
    anew->cs_put_mask = 0;
    anew->cs_accept_mask = 0;
    if (!(anew->decode = odr_createmem(ODR_DECODE)) ||
    	!(anew->encode = odr_createmem(ODR_ENCODE)))
	return 0;
    if (*control_block->apdufile)
    {
    	char filename[256];
	FILE *f;

	strcpy(filename, control_block->apdufile);
	if (!(anew->print = odr_createmem(ODR_PRINT)))
	    return 0;
	if (*control_block->apdufile == '@')
        {
	    odr_setprint(anew->print, yaz_log_file());
	}	
	else if (*control_block->apdufile != '-')
	{
	    strcpy(filename, control_block->apdufile);
	    if (!control_block->dynamic)
	    {
		if (!apduf)
		{
		    if (!(apduf = fopen(filename, "w")))
		    {
			yaz_log(LOG_WARN|LOG_ERRNO, "%s", filename);
			return 0;
		    }
		    setvbuf(apduf, 0, _IONBF, 0);
		}
		f = apduf;
	    }
	    else 
	    {
		sprintf(filename + strlen(filename), ".%d", getpid());
		if (!(f = fopen(filename, "w")))
		{
		    yaz_log(LOG_WARN|LOG_ERRNO, "%s", filename);
		    return 0;
		}
		setvbuf(f, 0, _IONBF, 0);
	    }
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
    return anew;
}

/*
 * Free association and release resources.
 */
void destroy_association(association *h)
{
    statserv_options_block *cb = statserv_getcontrol();

    xfree(h->init);
    odr_destroy(h->decode);
    odr_destroy(h->encode);
    if (h->print)
	odr_destroy(h->print);
    if (h->input_buffer)
    xfree(h->input_buffer);
    if (h->backend)
    	(*cb->bend_close)(h->backend);
    while (request_deq(&h->incoming));
    while (request_deq(&h->outgoing));
    request_delq(&h->incoming);
    request_delq(&h->outgoing);
    xfree(h);
    xmalloc_trav("session closed");
    if (control_block && control_block->one_shot)
    {
	exit (0);
    }
}

static void do_close_req(association *a, int reason, char *message,
			 request *req)
{
    Z_APDU apdu;
    Z_Close *cls = zget_Close(a->encode);
    
    /* Purge request queue */
    while (request_deq(&a->incoming));
    while (request_deq(&a->outgoing));
    if (a->version >= 3)
    {
	yaz_log(LOG_LOG, "Sending Close PDU, reason=%d, message=%s",
	    reason, message ? message : "none");
	apdu.which = Z_APDU_close;
	apdu.u.close = cls;
	*cls->closeReason = reason;
	cls->diagnosticInformation = message;
	process_z_response(a, req, &apdu);
	iochan_settimeout(a->client_chan, 20);
    }
    else
    {
	yaz_log(LOG_DEBUG, "v2 client. No Close PDU");
	iochan_setevent(a->client_chan, EVENT_TIMEOUT); /* force imm close */
    }
    a->state = ASSOC_DEAD;
}

static void do_close(association *a, int reason, char *message)
{
    do_close_req (a, reason, message, request_get(&a->outgoing));
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
	    yaz_log(LOG_LOG, "Final timeout - closing connection.");
	    cs_close(conn);
	    destroy_association(assoc);
	    iochan_destroy(h);
	}
	else
	{
	    yaz_log(LOG_LOG, "Session idle too long. Sending close.");
	    do_close(assoc, Z_Close_lackOfActivity, 0);
	}
	return;
    }
    if (event & assoc->cs_accept_mask)
    {
	yaz_log (LOG_DEBUG, "ir_session (accept)");
	if (!cs_accept (conn))
	{
	    yaz_log (LOG_LOG, "accept failed");
	    destroy_association(assoc);
	    iochan_destroy(h);
	}
	iochan_clearflag (h, EVENT_OUTPUT|EVENT_OUTPUT);
	if (conn->io_pending) 
	{   /* cs_accept didn't complete */
	    assoc->cs_accept_mask = 
		((conn->io_pending & CS_WANT_WRITE) ? EVENT_OUTPUT : 0) |
		((conn->io_pending & CS_WANT_READ) ? EVENT_INPUT : 0);

	    iochan_setflag (h, assoc->cs_accept_mask);
	}
	else
	{   /* cs_accept completed. Prepare for reading (cs_get) */
	    assoc->cs_accept_mask = 0;
	    assoc->cs_get_mask = EVENT_INPUT;
	    iochan_setflag (h, assoc->cs_get_mask);
	}
	return;
    }
    if ((event & assoc->cs_get_mask) || (event & EVENT_WORK)) /* input */
    {
    	if ((assoc->cs_put_mask & EVENT_INPUT) == 0 && (event & assoc->cs_get_mask))
	{
	    yaz_log(LOG_DEBUG, "ir_session (input)");
	    /* We aren't speaking to this fellow */
	    if (assoc->state == ASSOC_DEAD)
	    {
		yaz_log(LOG_LOG, "Connection closed - end of session");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		return;
	    }
	    assoc->cs_get_mask = EVENT_INPUT;
	    if ((res = cs_get(conn, &assoc->input_buffer,
		&assoc->input_buffer_len)) <= 0)
	    {
		yaz_log(LOG_LOG, "Connection closed by client");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		return;
	    }
	    else if (res == 1) /* incomplete read - wait for more  */
	    {
		if (conn->io_pending & CS_WANT_WRITE)
		    assoc->cs_get_mask |= EVENT_OUTPUT;
		iochan_setflag(h, assoc->cs_get_mask);
		return;
	    }
	    if (cs_more(conn)) /* more stuff - call us again later, please */
		iochan_setevent(h, EVENT_INPUT);
	    	
	    /* we got a complete PDU. Let's decode it */
	    yaz_log(LOG_DEBUG, "Got PDU, %d bytes: lead=%02X %02X %02X", res,
			    assoc->input_buffer[0] & 0xff,
			    assoc->input_buffer[1] & 0xff,
			    assoc->input_buffer[2] & 0xff);
	    req = request_get(&assoc->incoming); /* get a new request */
	    odr_reset(assoc->decode);
	    odr_setbuf(assoc->decode, assoc->input_buffer, res, 0);
	    if (!z_GDU(assoc->decode, &req->gdu_request, 0, 0))
	    {
		yaz_log(LOG_LOG, "ODR error on incoming PDU: %s [near byte %d] ",
			odr_errmsg(odr_geterror(assoc->decode)),
			odr_offset(assoc->decode));
                if (assoc->decode->error != OHTTP)
                {
                    yaz_log(LOG_LOG, "PDU dump:");
                    odr_dumpBER(yaz_log_file(), assoc->input_buffer, res);
                    do_close(assoc, Z_Close_protocolError, "Malformed package");
                }
                else
                {
                    Z_GDU *p = z_get_HTTP_Response(assoc->encode, 400);
                    assoc->state = ASSOC_DEAD;
                    process_gdu_response(assoc, req, p);
                }
		return;
	    }
	    req->request_mem = odr_extract_mem(assoc->decode);
	    if (assoc->print && !z_GDU(assoc->print, &req->gdu_request, 0, 0))
	    {
		yaz_log(LOG_WARN, "ODR print error: %s", 
		    odr_errmsg(odr_geterror(assoc->print)));
		odr_reset(assoc->print);
	    }
	    request_enq(&assoc->incoming, req);
	}

	/* can we do something yet? */
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
	yaz_log(LOG_DEBUG, "ir_session (output)");
        req->state = REQUEST_PENDING;
    	switch (res = cs_put(conn, req->response, req->len_response))
	{
	case -1:
	    yaz_log(LOG_LOG, "Connection closed by client");
	    cs_close(conn);
	    destroy_association(assoc);
	    iochan_destroy(h);
	    break;
	case 0: /* all sent - release the request structure */
	    yaz_log(LOG_DEBUG, "Wrote PDU, %d bytes", req->len_response);
#if 0
	    yaz_log(LOG_DEBUG, "HTTP out:\n%.*s", req->len_response,
                    req->response);
#endif
	    nmem_destroy(req->request_mem);
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
	yaz_log(LOG_LOG, "ir_session (exception)");
	cs_close(conn);
	destroy_association(assoc);
	iochan_destroy(h);
    }
}

static int process_z_request(association *assoc, request *req, char **msg);

static void assoc_init_reset(association *assoc)
{
    xfree (assoc->init);
    assoc->init = (bend_initrequest *) xmalloc (sizeof(*assoc->init));

    assoc->init->stream = assoc->encode;
    assoc->init->print = assoc->print;
    assoc->init->auth = 0;
    assoc->init->referenceId = 0;
    assoc->init->implementation_version = 0;
    assoc->init->implementation_id = 0;
    assoc->init->implementation_name = 0;
    assoc->init->bend_sort = NULL;
    assoc->init->bend_search = NULL;
    assoc->init->bend_present = NULL;
    assoc->init->bend_esrequest = NULL;
    assoc->init->bend_delete = NULL;
    assoc->init->bend_scan = NULL;
    assoc->init->bend_segment = NULL;
    assoc->init->bend_fetch = NULL;
    assoc->init->bend_explain = NULL;

    assoc->init->charneg_request = NULL;
    assoc->init->charneg_response = NULL;

    assoc->init->decode = assoc->decode;
    assoc->init->peer_name = 
        odr_strdup (assoc->encode, cs_addrstr(assoc->client_link));
}

static int srw_bend_init(association *assoc)
{
    const char *encoding = "UTF-8";
    Z_External *ce;
    bend_initresult *binitres;
    statserv_options_block *cb = statserv_getcontrol();
    
    assoc_init_reset(assoc);

    assoc->maximumRecordSize = 3000000;
    assoc->preferredMessageSize = 3000000;
#if 1
    ce = yaz_set_proposal_charneg(assoc->decode, &encoding, 1, 0, 0, 1);
    assoc->init->charneg_request = ce->u.charNeg3;
#endif
    if (!(binitres = (*cb->bend_init)(assoc->init)))
    {
    	yaz_log(LOG_WARN, "Bad response from backend.");
    	return 0;
    }
    assoc->backend = binitres->handle;
    return 1;
}

static int srw_bend_fetch(association *assoc, int pos,
                          Z_SRW_searchRetrieveRequest *srw_req,
                          Z_SRW_record *record)
{
    bend_fetch_rr rr;
    ODR o = assoc->encode;

    rr.setname = "default";
    rr.number = pos;
    rr.referenceId = 0;
    rr.request_format = VAL_TEXT_XML;
    rr.request_format_raw = yaz_oidval_to_z3950oid(assoc->decode,
                                                   CLASS_TRANSYN,
                                                   VAL_TEXT_XML);
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
    rr.comp->u.complex->generic->which = Z_Schema_uri;
    rr.comp->u.complex->generic->schema.uri = srw_req->recordSchema;
    rr.comp->u.complex->generic->elementSpec = 0;
    
    rr.stream = assoc->encode;
    rr.print = assoc->print;

    rr.basename = 0;
    rr.len = 0;
    rr.record = 0;
    rr.last_in_set = 0;
    rr.output_format = VAL_TEXT_XML;
    rr.output_format_raw = 0;
    rr.errcode = 0;
    rr.errstring = 0;
    rr.surrogate_flag = 0;

    if (!assoc->init->bend_fetch)
        return 1;

    (*assoc->init->bend_fetch)(assoc->backend, &rr);

    if (rr.len >= 0)
    {
        record->recordData_buf = rr.record;
        record->recordData_len = rr.len;
        record->recordPosition = odr_intdup(o, pos);
        record->recordSchema = 0;
        if (srw_req->recordSchema)
            record->recordSchema = odr_strdup(o, srw_req->recordSchema);
    }
    return rr.errcode;
}

static void srw_bend_search(association *assoc, request *req,
                            Z_SRW_searchRetrieveRequest *srw_req,
                            Z_SRW_searchRetrieveResponse *srw_res)
{
    int srw_error = 0;
    bend_search_rr rr;
    Z_External *ext;
    
    yaz_log(LOG_LOG, "Got SRW SearchRetrieveRequest");
    yaz_log(LOG_DEBUG, "srw_bend_search");
    if (!assoc->init)
    {
        yaz_log(LOG_DEBUG, "srw_bend_init");
        if (!srw_bend_init(assoc))
        {
            srw_error = 3;  /* assume Authentication error */

            srw_res->num_diagnostics = 1;
            srw_res->diagnostics = (Z_SRW_diagnostic *)
                odr_malloc(assoc->encode, sizeof(*srw_res->diagnostics));
            srw_res->diagnostics[0].code = 
                odr_intdup(assoc->encode, srw_error);
            srw_res->diagnostics[0].details = 0;
            return;
        }
    }
    
    rr.setname = "default";
    rr.replace_set = 1;
    rr.num_bases = 1;
    rr.basenames = &srw_req->database;
    rr.referenceId = 0;

    rr.query = (Z_Query *) odr_malloc (assoc->decode, sizeof(*rr.query));

    if (srw_req->query_type == Z_SRW_query_type_cql)
    {
        ext = (Z_External *) odr_malloc(assoc->decode, sizeof(*ext));
        ext->direct_reference = odr_getoidbystr(assoc->decode, 
                                                "1.2.840.10003.16.2");
        ext->indirect_reference = 0;
        ext->descriptor = 0;
        ext->which = Z_External_CQL;
        ext->u.cql = srw_req->query.cql;

        rr.query->which = Z_Query_type_104;
        rr.query->u.type_104 =  ext;
    }
    else if (srw_req->query_type == Z_SRW_query_type_pqf)
    {
        Z_RPNQuery *RPNquery;
        YAZ_PQF_Parser pqf_parser;

        pqf_parser = yaz_pqf_create ();

        RPNquery = yaz_pqf_parse (pqf_parser, assoc->decode,
                                  srw_req->query.pqf);
        if (!RPNquery)
        {
            const char *pqf_msg;
            size_t off;
            int code = yaz_pqf_error (pqf_parser, &pqf_msg, &off);
            yaz_log(LOG_LOG, "%*s^\n", off+4, "");
            yaz_log(LOG_LOG, "Bad PQF: %s (code %d)\n", pqf_msg, code);
            
            srw_error = 10;
        }

        rr.query->which = Z_Query_type_1;
        rr.query->u.type_1 =  RPNquery;

        yaz_pqf_destroy (pqf_parser);
    }
    else
        srw_error = 11;

    if (!srw_error && srw_req->sort_type != Z_SRW_sort_type_none)
        srw_error = 80;

    if (!srw_error && !assoc->init->bend_search)
        srw_error = 1;

    if (srw_error)
    {
        yaz_log(LOG_DEBUG, "srw_bend_search returned SRW error %d", srw_error);
        srw_res->num_diagnostics = 1;
        srw_res->diagnostics = (Z_SRW_diagnostic *)
	    odr_malloc(assoc->encode, sizeof(*srw_res->diagnostics));
        srw_res->diagnostics[0].code = 
            odr_intdup(assoc->encode, srw_error);
        srw_res->diagnostics[0].details = 0;
        return;
    }
    
    rr.stream = assoc->encode;
    rr.decode = assoc->decode;
    rr.print = assoc->print;
    rr.request = req;
    rr.association = assoc;
    rr.fd = 0;
    rr.hits = 0;
    rr.errcode = 0;
    rr.errstring = 0;
    rr.search_info = 0;
    yaz_log_zquery(rr.query);
    (assoc->init->bend_search)(assoc->backend, &rr);
    srw_res->numberOfRecords = odr_intdup(assoc->encode, rr.hits);
    if (rr.errcode)
    {
        yaz_log(LOG_DEBUG, "bend_search returned Bib-1 code %d", rr.errcode);
        srw_res->num_diagnostics = 1;
        srw_res->diagnostics = (Z_SRW_diagnostic *)
	    odr_malloc(assoc->encode, sizeof(*srw_res->diagnostics));
        srw_res->diagnostics[0].code = 
            odr_intdup(assoc->encode, 
                       yaz_diag_bib1_to_srw (rr.errcode));
        srw_res->diagnostics[0].details = rr.errstring;
        yaz_log(LOG_DEBUG, "srw_bend_search returned SRW error %d",
                *srw_res->diagnostics[0].code);
                
    }
    else
    {
        srw_res->numberOfRecords = odr_intdup(assoc->encode, rr.hits);
        if (srw_req->maximumRecords && *srw_req->maximumRecords > 0)
        {
            int number = *srw_req->maximumRecords;
            int start = 1;
            int i;

            if (srw_req->startRecord)
                start = *srw_req->startRecord;

            yaz_log(LOG_DEBUG, "srw_bend_search. start=%d max=%d",
                    start, *srw_req->maximumRecords);

            if (start <= rr.hits)
            {
                int j = 0;
                int packing = Z_SRW_recordPacking_string;
                if (start + number > rr.hits)
                    number = rr.hits - start + 1;
                if (srw_req->recordPacking && 
                    !strcmp(srw_req->recordPacking, "xml"))
                    packing = Z_SRW_recordPacking_XML;
                srw_res->records = (Z_SRW_record *)
                    odr_malloc(assoc->encode,
                               number * sizeof(*srw_res->records));
                for (i = 0; i<number; i++)
                {
                    int errcode;
                    
                    srw_res->records[j].recordPacking = packing;
                    srw_res->records[j].recordData_buf = 0;
                    yaz_log(LOG_DEBUG, "srw_bend_fetch %d", i+start);
                    errcode = srw_bend_fetch(assoc, i+start, srw_req,
                                             srw_res->records + j);
                    if (errcode)
                    {
                        srw_res->num_diagnostics = 1;
                        srw_res->diagnostics = (Z_SRW_diagnostic *)
                            odr_malloc(assoc->encode, 
                                       sizeof(*srw_res->diagnostics));
                        srw_res->diagnostics[0].code = 
                            odr_intdup(assoc->encode, 
                                       yaz_diag_bib1_to_srw (errcode));
                        srw_res->diagnostics[0].details = rr.errstring;
                        break;
                    }
                    if (srw_res->records[j].recordData_buf)
                        j++;
                }
                srw_res->num_records = j;
                if (!j)
                    srw_res->records = 0;
            }
        }
    }
}


static void srw_bend_explain(association *assoc, request *req,
                             Z_SRW_explainRequest *srw_req,
                             Z_SRW_explainResponse *srw_res)
{
    yaz_log(LOG_LOG, "Got SRW ExplainRequest");
    if (!assoc->init)
    {
        yaz_log(LOG_DEBUG, "srw_bend_init");
        if (!srw_bend_init(assoc))
            return;
    }
    if (assoc->init && assoc->init->bend_explain)
    {
        bend_explain_rr rr;

        rr.stream = assoc->encode;
        rr.decode = assoc->decode;
        rr.print = assoc->print;
        rr.explain_buf = 0;
        (*assoc->init->bend_explain)(assoc->backend, &rr);
        if (rr.explain_buf)
        {
            srw_res->explainData_buf = rr.explain_buf;
            srw_res->explainData_len = strlen(rr.explain_buf);
        }
    }
}

static int hex_digit (int ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a'+10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A'+10;
    return 0;
}

static char *uri_val(const char *path, const char *name, ODR o)
{
    size_t nlen = strlen(name);
    if (*path != '?')
        return 0;
    path++;
    while (path && *path)
    {
        const char *p1 = strchr(path, '=');
        if (!p1)
            break;
        if (p1 - path == nlen && !memcmp(path, name, nlen))
        {
            size_t i = 0;
            char *ret;
            
            path = p1 + 1;
            p1 = strchr(path, '&');
            if (!p1)
                p1 = strlen(path) + path;
            ret = odr_malloc(o, p1 - path + 1);
            while (*path && *path != '&')
            {
                if (*path == '+')
                {
                    ret[i++] = ' ';
                    path++;
                }
                else if (*path == '%' && path[1] && path[2])
                {
                    ret[i++] = hex_digit (path[1])*16 + hex_digit (path[2]);
                    path = path + 3;
                }
                else
                    ret[i++] = *path++;
            }
            ret[i] = '\0';
            return ret;
        }
        path = strchr(p1, '&');
        if (path)
            path++;
    }
    return 0;
}

void uri_val_int(const char *path, const char *name, ODR o, int **intp)
{
    const char *v = uri_val(path, name, o);
    if (v)
        *intp = odr_intdup(o, atoi(v));
}

static void process_http_request(association *assoc, request *req)
{
    Z_HTTP_Request *hreq = req->gdu_request->u.HTTP_Request;
    ODR o = assoc->encode;
    Z_GDU *p = 0;
    Z_HTTP_Response *hres = 0;
    int keepalive = 1;

    if (!strcmp(hreq->method, "GET"))
    {
        char *charset = 0;
        int ret = -1;
        Z_SOAP *soap_package = 0;
        char *db = "Default";
        const char *p0 = hreq->path, *p1;
        static Z_SOAP_Handler soap_handlers[2] = {
#if HAVE_XML2
            {"http://www.loc.gov/zing/srw/v1.0/", 0,
             (Z_SOAP_fun) yaz_srw_codec},
#endif
            {0, 0, 0}
        };
        
        if (*p0 == '/')
            p0++;
        p1 = strchr(p0, '?');
        if (!p1)
            p1 = p0 + strlen(p0);
        if (p1 != p0)
        {
            db = odr_malloc(assoc->decode, p1 - p0 + 1);
            memcpy (db, p0, p1 - p0);
            db[p1 - p0] = '\0';
        }
#if HAVE_XML2
        if (p1 && *p1 == '?' && p1[1])
        {
            Z_SRW_PDU *res = yaz_srw_get(o, Z_SRW_searchRetrieve_response);
            Z_SRW_PDU *sr = yaz_srw_get(o, Z_SRW_searchRetrieve_request);
            char *query = uri_val(p1, "query", o);
            char *pQuery = uri_val(p1, "pQuery", o);
            char *sortKeys = uri_val(p1, "sortKeys", o);
            
            if (query)
            {
                sr->u.request->query_type = Z_SRW_query_type_cql;
                sr->u.request->query.cql = query;
            }
            if (pQuery)
            {
                sr->u.request->query_type = Z_SRW_query_type_pqf;
                sr->u.request->query.pqf = pQuery;
            }
            if (sortKeys)
            {
                sr->u.request->sort_type = Z_SRW_sort_type_sort;
                sr->u.request->sort.sortKeys = sortKeys;
            }
            sr->u.request->recordSchema = uri_val(p1, "recordSchema", o);
            sr->u.request->recordPacking = uri_val(p1, "recordPacking", o);
            if (!sr->u.request->recordPacking)
                sr->u.request->recordPacking = "xml";
            uri_val_int(p1, "maximumRecords", o, 
                        &sr->u.request->maximumRecords);
            uri_val_int(p1, "startRecord", o,
                        &sr->u.request->startRecord);
            if (sr->u.request->startRecord)
                yaz_log(LOG_LOG, "startRecord=%d", *sr->u.request->startRecord);
            sr->u.request->database = db;
            srw_bend_search(assoc, req, sr->u.request, res->u.response);
            
            soap_package = odr_malloc(o, sizeof(*soap_package));
            soap_package->which = Z_SOAP_generic;

            soap_package->u.generic =
                odr_malloc(o, sizeof(*soap_package->u.generic));

            soap_package->u.generic->p = res;
            soap_package->u.generic->ns = soap_handlers[0].ns;
            soap_package->u.generic->no = 0;
            
            soap_package->ns = "SRU";

            p = z_get_HTTP_Response(o, 200);
            hres = p->u.HTTP_Response;

            ret = z_soap_codec_enc(assoc->encode, &soap_package,
                                   &hres->content_buf, &hres->content_len,
                                   soap_handlers, charset);
            if (!charset)
                z_HTTP_header_add(o, &hres->headers, "Content-Type", "text/xml");
            else
            {
                char ctype[60];
                strcpy(ctype, "text/xml; charset=");
                strcat(ctype, charset);
                z_HTTP_header_add(o, &hres->headers, "Content-Type", ctype);
            }

        }
        else
        {
            Z_SRW_PDU *res = yaz_srw_get(o, Z_SRW_explain_response);
            Z_SRW_PDU *sr = yaz_srw_get(o, Z_SRW_explain_request);

            srw_bend_explain(assoc, req, sr->u.explain_request,
                            res->u.explain_response);

            if (res->u.explain_response->explainData_buf)
            {
                soap_package = odr_malloc(o, sizeof(*soap_package));
                soap_package->which = Z_SOAP_generic;
                
                soap_package->u.generic =
                    odr_malloc(o, sizeof(*soap_package->u.generic));
                
                soap_package->u.generic->p = res;
                soap_package->u.generic->ns = soap_handlers[0].ns;
                soap_package->u.generic->no = 0;
                
                soap_package->ns = "SRU";
                
                p = z_get_HTTP_Response(o, 200);
                hres = p->u.HTTP_Response;
                
                ret = z_soap_codec_enc(assoc->encode, &soap_package,
                                       &hres->content_buf, &hres->content_len,
                                       soap_handlers, charset);
                if (!charset)
                    z_HTTP_header_add(o, &hres->headers, "Content-Type", "text/xml");
                else
                {
                    char ctype[60];
                    strcpy(ctype, "text/xml; charset=");
                    strcat(ctype, charset);
                    z_HTTP_header_add(o, &hres->headers, "Content-Type",
                                      ctype);
                }
            }
        }
#endif
#ifdef DOCDIR
	if (strlen(hreq->path) >= 5 && strlen(hreq->path) < 80 &&
			 !memcmp(hreq->path, "/doc/", 5))
        {
	    FILE *f;
            char fpath[120];

	    strcpy(fpath, DOCDIR);
	    strcat(fpath, hreq->path+4);
	    f = fopen(fpath, "rb");
	    if (f) {
                struct stat sbuf;
                if (fstat(fileno(f), &sbuf) || !S_ISREG(sbuf.st_mode))
                {
                    fclose(f);
                    f = 0;
                }
            }
            if (f)
            {
		long sz;
		fseek(f, 0L, SEEK_END);
		sz = ftell(f);
		if (sz >= 0 && sz < 500000)
		{
		    const char *ctype = "application/octet-stream";
		    const char *cp;

                    p = z_get_HTTP_Response(o, 200);
                    hres = p->u.HTTP_Response;
		    hres->content_buf = (char *) odr_malloc(o, sz + 1);
		    hres->content_len = sz;
		    fseek(f, 0L, SEEK_SET);
		    fread(hres->content_buf, 1, sz, f);
		    if ((cp = strrchr(fpath, '.'))) {
			cp++;
			if (!strcmp(cp, "png"))
			    ctype = "image/png";
			else if (!strcmp(cp, "gif"))
			    ctype = "image/gif";
			else if (!strcmp(cp, "xml"))
			    ctype = "text/xml";
			else if (!strcmp(cp, "html"))
			    ctype = "text/html";
		    }
                    z_HTTP_header_add(o, &hres->headers, "Content-Type", ctype);
		}
		fclose(f);
	    }
	}
#endif

#if 0
	if (!strcmp(hreq->path, "/")) 
        {
#ifdef DOCDIR
            struct stat sbuf;
#endif
            const char *doclink = "";
            p = z_get_HTTP_Response(o, 200);
            hres = p->u.HTTP_Response;
            hres->content_buf = (char *) odr_malloc(o, 400);
#ifdef DOCDIR
            if (stat(DOCDIR "/yaz.html", &sbuf) == 0 && S_ISREG(sbuf.st_mode))
                doclink = "<P><A HREF=\"/doc/yaz.html\">Documentation</A></P>";
#endif
            sprintf (hres->content_buf, 
                     "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
                     "<HTML>\n"
                     " <HEAD>\n"
                     "  <TITLE>YAZ " YAZ_VERSION "</TITLE>\n"
                     " </HEAD>\n"
                     " <BODY>\n"
                     "  <P><A HREF=\"http://www.indexdata.dk/yaz/\">YAZ</A> " 
                     YAZ_VERSION "</P>\n"
                     "%s"
                     " </BODY>\n"
                     "</HTML>\n", doclink);
            hres->content_len = strlen(hres->content_buf);
            z_HTTP_header_add(o, &hres->headers, "Content-Type", "text/html");
        }
#endif

        if (!p)
        {
            p = z_get_HTTP_Response(o, 404);
        }
    }
    else if (!strcmp(hreq->method, "POST"))
    {
        const char *content_type = z_HTTP_header_lookup(hreq->headers,
                                                        "Content-Type");
        if (content_type && !yaz_strcmp_del("text/xml", content_type, "; "))
        {
            Z_SOAP *soap_package = 0;
            int ret = -1;
            int http_code = 500;
            const char *charset_p = 0;
            char *charset = 0;

            static Z_SOAP_Handler soap_handlers[2] = {
#if HAVE_XML2
                {"http://www.loc.gov/zing/srw/v1.0/", 0,
                 (Z_SOAP_fun) yaz_srw_codec},
#endif
                {0, 0, 0}
            };
            if ((charset_p = strstr(content_type, "; charset=")))
            {
                int i = 0;
                charset_p += 10;
                while (i < 20 && charset_p[i] &&
                       !strchr("; \n\r", charset_p[i]))
                    i++;
                charset = odr_malloc(assoc->encode, i+1);
                memcpy(charset, charset_p, i);
                charset[i] = '\0';
                yaz_log(LOG_LOG, "SOAP encoding %s", charset);
            }
            ret = z_soap_codec(assoc->decode, &soap_package, 
                               &hreq->content_buf, &hreq->content_len,
                               soap_handlers);
#if HAVE_XML2
            if (!ret && soap_package->which == Z_SOAP_generic &&
                soap_package->u.generic->no == 0)
            {
                /* SRW package */
                Z_SRW_PDU *sr = soap_package->u.generic->p;
                
                if (sr->which == Z_SRW_searchRetrieve_request)
                {
                    Z_SRW_PDU *res =
                        yaz_srw_get(assoc->encode,
                                    Z_SRW_searchRetrieve_response);

                    if (!sr->u.request->database)
                    {
                        const char *p0 = hreq->path, *p1;
                        if (*p0 == '/')
                            p0++;
                        p1 = strchr(p0, '?');
                        if (!p1)
                            p1 = p0 + strlen(p0);
                        if (p1 != p0)
                        {
                            sr->u.request->database =
                                odr_malloc(assoc->decode, p1 - p0 + 1);
                            memcpy (sr->u.request->database, p0, p1 - p0);
                            sr->u.request->database[p1 - p0] = '\0';
                        }
                        else
                            sr->u.request->database = "Default";
                    }
                    srw_bend_search(assoc, req, sr->u.request,
                                    res->u.response);
                    
                    soap_package->u.generic->p = res;
                    http_code = 200;
                }
                else if (sr->which == Z_SRW_explain_request)
                {
                    Z_SRW_PDU *res =
                        yaz_srw_get(assoc->encode, Z_SRW_explain_response);

                    srw_bend_explain(assoc, req, sr->u.explain_request,
                                     res->u.explain_response);
                    if (!res->u.explain_response->explainData_buf)
                    {
                        z_soap_error(assoc->encode, soap_package,
                                     "SOAP-ENV:Client", "Explain Not Supported", 0);
                    }
                    else
                    {
                        soap_package->u.generic->p = res;
                        http_code = 200;
                    }
                }
                else
                {
                    z_soap_error(assoc->encode, soap_package,
                                 "SOAP-ENV:Client", "Bad method", 0); 
                }
            }
#endif
            p = z_get_HTTP_Response(o, 200);
            hres = p->u.HTTP_Response;
            ret = z_soap_codec_enc(assoc->encode, &soap_package,
                                   &hres->content_buf, &hres->content_len,
                                   soap_handlers, charset);
            hres->code = http_code;
            if (!charset)
                z_HTTP_header_add(o, &hres->headers, "Content-Type", "text/xml");
            else
            {
                char ctype[60];
                strcpy(ctype, "text/xml; charset=");
                strcat(ctype, charset);
                z_HTTP_header_add(o, &hres->headers, "Content-Type", ctype);
            }
        }
        if (!p) /* still no response ? */
            p = z_get_HTTP_Response(o, 500);
    }
    else
    {
        p = z_get_HTTP_Response(o, 405);
        hres = p->u.HTTP_Response;

        z_HTTP_header_add(o, &hres->headers, "Allow", "GET, POST");
    }
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
    if (!keepalive)
    {
        z_HTTP_header_add(o, &hres->headers, "Connection", "close");
        assoc->state = ASSOC_DEAD;
    }
    else
    {
        int t;
        const char *alive = z_HTTP_header_lookup(hreq->headers, "Keep-Alive");

        if (alive && isdigit(*alive))
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
    int fd = -1;
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
        iochan_settimeout(assoc->client_chan,
                          statserv_getcontrol()->idle_timeout * 60);
	res = process_initRequest(assoc, req); break;
    case Z_APDU_searchRequest:
	res = process_searchRequest(assoc, req, &fd); break;
    case Z_APDU_presentRequest:
	res = process_presentRequest(assoc, req, &fd); break;
    case Z_APDU_scanRequest:
	if (assoc->init->bend_scan)
	    res = process_scanRequest(assoc, req, &fd);
	else
	{
	    *msg = "Cannot handle Scan APDU";
	    return -1;
	}
	break;
    case Z_APDU_extendedServicesRequest:
	if (assoc->init->bend_esrequest)
	    res = process_ESRequest(assoc, req, &fd);
	else
	{
	    *msg = "Cannot handle Extended Services APDU";
	    return -1;
	}
	break;
    case Z_APDU_sortRequest:
	if (assoc->init->bend_sort)
	    res = process_sortRequest(assoc, req, &fd);
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
	    res = process_deleteRequest(assoc, req, &fd);
	else
	{
	    *msg = "Cannot handle Delete APDU";
	    return -1;
	}
	break;
    case Z_APDU_segmentRequest:
	if (assoc->init->bend_segment)
	{
	    res = process_segmentRequest (assoc, req);
	}
	else
	{
	    *msg = "Cannot handle Segment APDU";
	    return -1;
	}
	break;
    default:
	*msg = "Bad APDU received";
	return -1;
    }
    if (res)
    {
    	yaz_log(LOG_DEBUG, "  result immediately available");
    	retval = process_z_response(assoc, req, res);
    }
    else if (fd < 0)
    {
    	yaz_log(LOG_DEBUG, "  result unavailble");
	retval = 0;
    }
    else /* no result yet - one will be provided later */
    {
    	IOCHAN chan;

	/* Set up an I/O handler for the fd supplied by the backend */

	yaz_log(LOG_DEBUG, "   establishing handler for result");
	req->state = REQUEST_PENDING;
	if (!(chan = iochan_create(fd, backend_response, EVENT_INPUT)))
	    abort();
	iochan_setdata(chan, assoc);
	retval = 0;
    }
    return retval;
}

/*
 * Handle message from the backend.
 */
void backend_response(IOCHAN i, int event)
{
    association *assoc = (association *)iochan_getdata(i);
    request *req = request_head(&assoc->incoming);
    Z_APDU *res;
    int fd;

    yaz_log(LOG_DEBUG, "backend_response");
    assert(assoc && req && req->state != REQUEST_IDLE);
    /* determine what it is we're waiting for */
    switch (req->apdu_request->which)
    {
	case Z_APDU_searchRequest:
	    res = response_searchRequest(assoc, req, 0, &fd); break;
#if 0
	case Z_APDU_presentRequest:
	    res = response_presentRequest(assoc, req, 0, &fd); break;
	case Z_APDU_scanRequest:
	    res = response_scanRequest(assoc, req, 0, &fd); break;
#endif
	default:
	    yaz_log(LOG_WARN, "Serious programmer's lapse or bug");
	    abort();
    }
    if ((res && process_z_response(assoc, req, res) < 0) || fd < 0)
    {
	yaz_log(LOG_LOG, "Fatal error when talking to backend");
	do_close(assoc, Z_Close_systemProblem, 0);
	iochan_destroy(i);
	return;
    }
    else if (!res) /* no result yet - try again later */
    {
    	yaz_log(LOG_DEBUG, "   no result yet");
    	iochan_setfd(i, fd); /* in case fd has changed */
    }
}

/*
 * Encode response, and transfer the request structure to the outgoing queue.
 */
static int process_gdu_response(association *assoc, request *req, Z_GDU *res)
{
    odr_setbuf(assoc->encode, req->response, req->size_response, 1);

    if (assoc->print && !z_GDU(assoc->print, &res, 0, 0))
    {
	yaz_log(LOG_WARN, "ODR print error: %s", 
	    odr_errmsg(odr_geterror(assoc->print)));
	odr_reset(assoc->print);
    }
    if (!z_GDU(assoc->encode, &res, 0, 0))
    {
    	yaz_log(LOG_WARN, "ODR error when encoding response: %s",
	    odr_errmsg(odr_geterror(assoc->decode)));
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
#if 1
    if (request_head(&assoc->incoming))
    {
	yaz_log (LOG_DEBUG, "more work to be done");
    	iochan_setevent(assoc->client_chan, EVENT_WORK);
    }
#endif
    return 0;
}

/*
 * Encode response, and transfer the request structure to the outgoing queue.
 */
static int process_z_response(association *assoc, request *req, Z_APDU *res)
{
    Z_GDU *gres = (Z_GDU *) odr_malloc(assoc->encode, sizeof(*res));
    gres->which = Z_GDU_Z3950;
    gres->u.z3950 = res;

    return process_gdu_response(assoc, req, gres);
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
    statserv_options_block *cb = statserv_getcontrol();
    Z_InitRequest *req = reqb->apdu_request->u.initRequest;
    Z_APDU *apdu = zget_APDU(assoc->encode, Z_APDU_initResponse);
    Z_InitResponse *resp = apdu->u.initResponse;
    bend_initresult *binitres;

    char options[140];

    yaz_log(LOG_LOG, "Got initRequest");
    if (req->implementationId)
    	yaz_log(LOG_LOG, "Id:        %s", req->implementationId);
    if (req->implementationName)
    	yaz_log(LOG_LOG, "Name:      %s", req->implementationName);
    if (req->implementationVersion)
    	yaz_log(LOG_LOG, "Version:   %s", req->implementationVersion);

    assoc_init_reset(assoc);

    assoc->init->auth = req->idAuthentication;
    assoc->init->referenceId = req->referenceId;

    if (ODR_MASK_GET(req->options, Z_Options_negotiationModel))
    {
        Z_CharSetandLanguageNegotiation *negotiation =
            yaz_get_charneg_record (req->otherInfo);
        if (negotiation->which == Z_CharSetandLanguageNegotiation_proposal)
            assoc->init->charneg_request = negotiation;
    }
    
    if (!(binitres = (*cb->bend_init)(assoc->init)))
    {
    	yaz_log(LOG_WARN, "Bad response from backend.");
    	return 0;
    }

    assoc->backend = binitres->handle;
    if ((assoc->init->bend_sort))
	yaz_log (LOG_DEBUG, "Sort handler installed");
    if ((assoc->init->bend_search))
	yaz_log (LOG_DEBUG, "Search handler installed");
    if ((assoc->init->bend_present))
	yaz_log (LOG_DEBUG, "Present handler installed");   
    if ((assoc->init->bend_esrequest))
	yaz_log (LOG_DEBUG, "ESRequest handler installed");   
    if ((assoc->init->bend_delete))
	yaz_log (LOG_DEBUG, "Delete handler installed");   
    if ((assoc->init->bend_scan))
	yaz_log (LOG_DEBUG, "Scan handler installed");   
    if ((assoc->init->bend_segment))
	yaz_log (LOG_DEBUG, "Segment handler installed");   
    
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
	strcat (options, " extendedServices");
    }
    if (ODR_MASK_GET(req->options, Z_Options_namedResultSets))
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

    if (ODR_MASK_GET(req->options, Z_Options_negotiationModel)
        && assoc->init->charneg_response)
    {
    	Z_OtherInformation **p;
    	Z_OtherInformationUnit *p0;
    	
    	yaz_oi_APDU(apdu, &p);
    	
    	if ((p0=yaz_oi_update(p, assoc->encode, NULL, 0, 0))) {
            ODR_MASK_SET(resp->options, Z_Options_negotiationModel);
            
            p0->which = Z_OtherInfo_externallyDefinedInfo;
            p0->information.externallyDefinedInfo =
                assoc->init->charneg_response;
        }
	ODR_MASK_SET(resp->options, Z_Options_negotiationModel);
	strcat(options, " negotiation");
    }

    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_1))
    {
    	ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_1);
	assoc->version = 2; /* 1 & 2 are equivalent */
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

    yaz_log(LOG_LOG, "Negotiated to v%d: %s", assoc->version, options);
    assoc->maximumRecordSize = *req->maximumRecordSize;
    if (assoc->maximumRecordSize > control_block->maxrecordsize)
    	assoc->maximumRecordSize = control_block->maxrecordsize;
    assoc->preferredMessageSize = *req->preferredMessageSize;
    if (assoc->preferredMessageSize > assoc->maximumRecordSize)
    	assoc->preferredMessageSize = assoc->maximumRecordSize;

    resp->preferredMessageSize = &assoc->preferredMessageSize;
    resp->maximumRecordSize = &assoc->maximumRecordSize;

    resp->implementationName = "GFS/YAZ";

    if (assoc->init->implementation_id)
    {
	char *nv = (char *)
	    odr_malloc (assoc->encode,
			strlen(assoc->init->implementation_id) + 10 + 
			       strlen(resp->implementationId));
	sprintf (nv, "%s / %s",
		 resp->implementationId, assoc->init->implementation_id);
        resp->implementationId = nv;
    }
    if (assoc->init->implementation_name)
    {
	char *nv = (char *)
	    odr_malloc (assoc->encode,
			strlen(assoc->init->implementation_name) + 10 + 
			       strlen(resp->implementationName));
	sprintf (nv, "%s / %s",
		 resp->implementationName, assoc->init->implementation_name);
        resp->implementationName = nv;
    }
    if (assoc->init->implementation_version)
    {
	char *nv = (char *)
	    odr_malloc (assoc->encode,
			strlen(assoc->init->implementation_version) + 10 + 
			       strlen(resp->implementationVersion));
	sprintf (nv, "YAZ %s / %s",
		 resp->implementationVersion,
		 assoc->init->implementation_version);
        resp->implementationVersion = nv;
    }

    if (binitres->errcode)
    {
    	yaz_log(LOG_LOG, "Connection rejected by backend.");
    	*resp->result = 0;
	assoc->state = ASSOC_DEAD;
    }
    else
	assoc->state = ASSOC_UP;
    return apdu;
}

/*
 * These functions should be merged.
 */

static void set_addinfo (Z_DefaultDiagFormat *dr, char *addinfo, ODR odr)
{
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
}

/*
 * nonsurrogate diagnostic record.
 */
static Z_Records *diagrec(association *assoc, int error, char *addinfo)
{
    Z_Records *rec = (Z_Records *)
	odr_malloc (assoc->encode, sizeof(*rec));
    int *err = odr_intdup(assoc->encode, error);
    Z_DiagRec *drec = (Z_DiagRec *)
	odr_malloc (assoc->encode, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
	odr_malloc (assoc->encode, sizeof(*dr));

    yaz_log(LOG_LOG, "[%d] %s %s%s", error, diagbib1_str(error),
        addinfo ? " -- " : "", addinfo ? addinfo : "");
    rec->which = Z_Records_NSD;
    rec->u.nonSurrogateDiagnostic = dr;
    dr->diagnosticSetId =
	yaz_oidval_to_z3950oid (assoc->encode, CLASS_DIAGSET, VAL_BIB1);
    dr->condition = err;
    set_addinfo (dr, addinfo, assoc->encode);
    return rec;
}

/*
 * surrogate diagnostic.
 */
static Z_NamePlusRecord *surrogatediagrec(association *assoc, char *dbname,
					  int error, char *addinfo)
{
    Z_NamePlusRecord *rec = (Z_NamePlusRecord *)
	odr_malloc (assoc->encode, sizeof(*rec));
    int *err = odr_intdup(assoc->encode, error);
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (assoc->encode, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
	odr_malloc (assoc->encode, sizeof(*dr));
    
    yaz_log(LOG_DEBUG, "SurrogateDiagnotic: %d -- %s", error, addinfo);
    rec->databaseName = dbname;
    rec->which = Z_NamePlusRecord_surrogateDiagnostic;
    rec->u.surrogateDiagnostic = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
    dr->diagnosticSetId =
	yaz_oidval_to_z3950oid (assoc->encode, CLASS_DIAGSET, VAL_BIB1);
    dr->condition = err;
    set_addinfo (dr, addinfo, assoc->encode);

    return rec;
}

/*
 * multiple nonsurrogate diagnostics.
 */
static Z_DiagRecs *diagrecs(association *assoc, int error, char *addinfo)
{
    Z_DiagRecs *recs = (Z_DiagRecs *)odr_malloc (assoc->encode, sizeof(*recs));
    int *err = odr_intdup(assoc->encode, error);
    Z_DiagRec **recp = (Z_DiagRec **)odr_malloc (assoc->encode, sizeof(*recp));
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (assoc->encode, sizeof(*drec));
    Z_DefaultDiagFormat *rec = (Z_DefaultDiagFormat *)
	odr_malloc (assoc->encode, sizeof(*rec));

    yaz_log(LOG_DEBUG, "DiagRecs: %d -- %s", error, addinfo ? addinfo : "");

    recs->num_diagRecs = 1;
    recs->diagRecs = recp;
    recp[0] = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = rec;

    rec->diagnosticSetId =
	yaz_oidval_to_z3950oid (assoc->encode, CLASS_DIAGSET, VAL_BIB1);
    rec->condition = err;

    rec->which = Z_DefaultDiagFormat_v2Addinfo;
    rec->u.v2Addinfo = odr_strdup (assoc->encode, addinfo ? addinfo : "");
    return recs;
}

static Z_Records *pack_records(association *a, char *setname, int start,
			       int *num, Z_RecordComposition *comp,
			       int *next, int *pres, oid_value format,
			       Z_ReferenceId *referenceId,
			       int *oid)
{
    int recno, total_length = 0, toget = *num, dumped_records = 0;
    Z_Records *records =
	(Z_Records *) odr_malloc (a->encode, sizeof(*records));
    Z_NamePlusRecordList *reclist =
	(Z_NamePlusRecordList *) odr_malloc (a->encode, sizeof(*reclist));
    Z_NamePlusRecord **list =
	(Z_NamePlusRecord **) odr_malloc (a->encode, sizeof(*list) * toget);

    records->which = Z_Records_DBOSD;
    records->u.databaseOrSurDiagnostics = reclist;
    reclist->num_records = 0;
    reclist->records = list;
    *pres = Z_PRES_SUCCESS;
    *num = 0;
    *next = 0;

    yaz_log(LOG_LOG, "Request to pack %d+%d+%s", start, toget, setname);
    yaz_log(LOG_DEBUG, "pms=%d, mrs=%d", a->preferredMessageSize,
    	a->maximumRecordSize);
    for (recno = start; reclist->num_records < toget; recno++)
    {
    	bend_fetch_rr freq;
	Z_NamePlusRecord *thisrec;
	int this_length = 0;
	/*
	 * we get the number of bytes allocated on the stream before any
	 * allocation done by the backend - this should give us a reasonable
	 * idea of the total size of the data so far.
	 */
	total_length = odr_total(a->encode) - dumped_records;
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
	freq.request_format = format;
	freq.request_format_raw = oid;
	freq.output_format = format;
	freq.output_format_raw = 0;
	freq.stream = a->encode;
	freq.print = a->print;
	freq.surrogate_flag = 0;
	freq.referenceId = referenceId;
	(*a->init->bend_fetch)(a->backend, &freq);
	/* backend should be able to signal whether error is system-wide
	   or only pertaining to current record */
	if (freq.errcode)
	{
	    if (!freq.surrogate_flag)
	    {
		char s[20];
		*pres = Z_PRES_FAILURE;
		/* for 'present request out of range',
                   set addinfo to record position if not set */
		if (freq.errcode == 13 && freq.errstring == 0)
		{
		    sprintf (s, "%d", recno);
		    freq.errstring = s;
		}
		return diagrec(a, freq.errcode, freq.errstring);
	    }
	    reclist->records[reclist->num_records] =
		surrogatediagrec(a, freq.basename, freq.errcode,
				 freq.errstring);
	    reclist->num_records++;
	    *next = freq.last_in_set ? 0 : recno + 1;
	    continue;
	}
	if (freq.len >= 0)
	    this_length = freq.len;
	else
	    this_length = odr_total(a->encode) - total_length;
	yaz_log(LOG_DEBUG, "  fetched record, len=%d, total=%d",
	    this_length, total_length);
	if (this_length + total_length > a->preferredMessageSize)
	{
	    /* record is small enough, really */
	    if (this_length <= a->preferredMessageSize)
	    {
	    	yaz_log(LOG_DEBUG, "  Dropped last normal-sized record");
		*pres = Z_PRES_PARTIAL_2;
		break;
	    }
	    /* record can only be fetched by itself */
	    if (this_length < a->maximumRecordSize)
	    {
	    	yaz_log(LOG_DEBUG, "  Record > prefmsgsz");
	    	if (toget > 1)
		{
		    yaz_log(LOG_DEBUG, "  Dropped it");
		    reclist->records[reclist->num_records] =
		   	 surrogatediagrec(a, freq.basename, 16, 0);
		    reclist->num_records++;
		    *next = freq.last_in_set ? 0 : recno + 1;
		    dumped_records += this_length;
		    continue;
		}
	    }
	    else /* too big entirely */
	    {
	    	yaz_log(LOG_LOG, "Record > maxrcdsz this=%d max=%d", this_length, a->maximumRecordSize);
		reclist->records[reclist->num_records] =
		    surrogatediagrec(a, freq.basename, 17, 0);
		reclist->num_records++;
		*next = freq.last_in_set ? 0 : recno + 1;
		dumped_records += this_length;
		continue;
	    }
	}

	if (!(thisrec = (Z_NamePlusRecord *)
	      odr_malloc(a->encode, sizeof(*thisrec))))
	    return 0;
	if (!(thisrec->databaseName = (char *)odr_malloc(a->encode,
	    strlen(freq.basename) + 1)))
	    return 0;
	strcpy(thisrec->databaseName, freq.basename);
	thisrec->which = Z_NamePlusRecord_databaseRecord;

	if (freq.output_format_raw)
	{
	    struct oident *ident = oid_getentbyoid(freq.output_format_raw);
	    freq.output_format = ident->value;
	}
	thisrec->u.databaseRecord = z_ext_record(a->encode, freq.output_format,
						 freq.record, freq.len);
	if (!thisrec->u.databaseRecord)
	    return 0;
	reclist->records[reclist->num_records] = thisrec;
	reclist->num_records++;
	*next = freq.last_in_set ? 0 : recno + 1;
    }
    *num = reclist->num_records;
    return records;
}

static Z_APDU *process_searchRequest(association *assoc, request *reqb,
    int *fd)
{
    Z_SearchRequest *req = reqb->apdu_request->u.searchRequest;
    bend_search_rr *bsrr = 
	(bend_search_rr *)nmem_malloc (reqb->request_mem, sizeof(*bsrr));
    
    yaz_log(LOG_LOG, "Got SearchRequest.");
    bsrr->fd = fd;
    bsrr->request = reqb;
    bsrr->association = assoc;
    bsrr->referenceId = req->referenceId;
    save_referenceId (reqb, bsrr->referenceId);

    yaz_log (LOG_LOG, "ResultSet '%s'", req->resultSetName);
    if (req->databaseNames)
    {
	int i;
	for (i = 0; i < req->num_databaseNames; i++)
	    yaz_log (LOG_LOG, "Database '%s'", req->databaseNames[i]);
    }
    yaz_log_zquery(req->query);

    if (assoc->init->bend_search)
    {
	bsrr->setname = req->resultSetName;
	bsrr->replace_set = *req->replaceIndicator;
	bsrr->num_bases = req->num_databaseNames;
	bsrr->basenames = req->databaseNames;
	bsrr->query = req->query;
	bsrr->stream = assoc->encode;
	bsrr->decode = assoc->decode;
	bsrr->print = assoc->print;
	bsrr->errcode = 0;
	bsrr->hits = 0;
	bsrr->errstring = NULL;
        bsrr->search_info = NULL;
	(assoc->init->bend_search)(assoc->backend, bsrr);
	if (!bsrr->request)
	    return 0;
    }
    return response_searchRequest(assoc, reqb, bsrr, fd);
}

int bend_searchresponse(void *handle, bend_search_rr *bsrr) {return 0;}

/*
 * Prepare a searchresponse based on the backend results. We probably want
 * to look at making the fetching of records nonblocking as well, but
 * so far, we'll keep things simple.
 * If bsrt is null, that means we're called in response to a communications
 * event, and we'll have to get the response for ourselves.
 */
static Z_APDU *response_searchRequest(association *assoc, request *reqb,
    bend_search_rr *bsrt, int *fd)
{
    Z_SearchRequest *req = reqb->apdu_request->u.searchRequest;
    Z_APDU *apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));
    Z_SearchResponse *resp = (Z_SearchResponse *)
	odr_malloc (assoc->encode, sizeof(*resp));
    int *nulint = odr_intdup (assoc->encode, 0);
    bool_t *sr = odr_intdup(assoc->encode, 1);
    int *next = odr_intdup(assoc->encode, 0);
    int *none = odr_intdup(assoc->encode, Z_RES_NONE);

    apdu->which = Z_APDU_searchResponse;
    apdu->u.searchResponse = resp;
    resp->referenceId = req->referenceId;
    resp->additionalSearchInfo = 0;
    resp->otherInfo = 0;
    *fd = -1;
    if (!bsrt && !bend_searchresponse(assoc->backend, bsrt))
    {
    	yaz_log(LOG_FATAL, "Bad result from backend");
    	return 0;
    }
    else if (bsrt->errcode)
    {
	resp->records = diagrec(assoc, bsrt->errcode, bsrt->errstring);
	resp->resultCount = nulint;
	resp->numberOfRecordsReturned = nulint;
	resp->nextResultSetPosition = nulint;
	resp->searchStatus = nulint;
	resp->resultSetStatus = none;
	resp->presentStatus = 0;
    }
    else
    {
    	int *toget = odr_intdup(assoc->encode, 0);
        int *presst = odr_intdup(assoc->encode, 0);
	Z_RecordComposition comp, *compp = 0;

        yaz_log (LOG_LOG, "resultCount: %d", bsrt->hits);

	resp->records = 0;
	resp->resultCount = &bsrt->hits;

	comp.which = Z_RecordComp_simple;
	/* how many records does the user agent want, then? */
	if (bsrt->hits <= *req->smallSetUpperBound)
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
	    oident *prefformat;
	    oid_value form;

	    if (!(prefformat = oid_getentbyoid(req->preferredRecordSyntax)))
		form = VAL_NONE;
	    else
	    	form = prefformat->value;
	    resp->records = pack_records(assoc, req->resultSetName, 1,
		toget, compp, next, presst, form, req->referenceId,
					 req->preferredRecordSyntax);
	    if (!resp->records)
		return 0;
	    resp->numberOfRecordsReturned = toget;
	    resp->nextResultSetPosition = next;
	    resp->searchStatus = sr;
	    resp->resultSetStatus = 0;
	    resp->presentStatus = presst;
	}
	else
	{
	    if (*resp->resultCount)
	    	*next = 1;
	    resp->numberOfRecordsReturned = nulint;
	    resp->nextResultSetPosition = next;
	    resp->searchStatus = sr;
	    resp->resultSetStatus = 0;
	    resp->presentStatus = 0;
	}
    }
    resp->additionalSearchInfo = bsrt->search_info;
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
static Z_APDU *process_presentRequest(association *assoc, request *reqb,
				      int *fd)
{
    Z_PresentRequest *req = reqb->apdu_request->u.presentRequest;
    oident *prefformat;
    oid_value form;
    Z_APDU *apdu;
    Z_PresentResponse *resp;
    int *next;
    int *num;

    yaz_log(LOG_LOG, "Got PresentRequest.");

    if (!(prefformat = oid_getentbyoid(req->preferredRecordSyntax)))
	form = VAL_NONE;
    else
	form = prefformat->value;
    resp = (Z_PresentResponse *)odr_malloc (assoc->encode, sizeof(*resp));
    resp->records = 0;
    resp->presentStatus = odr_intdup(assoc->encode, 0);
    if (assoc->init->bend_present)
    {
	bend_present_rr *bprr = (bend_present_rr *)
	    nmem_malloc (reqb->request_mem, sizeof(*bprr));
	bprr->setname = req->resultSetId;
	bprr->start = *req->resultSetStartPoint;
	bprr->number = *req->numberOfRecordsRequested;
	bprr->format = form;
	bprr->comp = req->recordComposition;
	bprr->referenceId = req->referenceId;
	bprr->stream = assoc->encode;
	bprr->print = assoc->print;
	bprr->request = reqb;
	bprr->association = assoc;
	bprr->errcode = 0;
	bprr->errstring = NULL;
	(*assoc->init->bend_present)(assoc->backend, bprr);
	
	if (!bprr->request)
	    return 0;
	if (bprr->errcode)
	{
	    resp->records = diagrec(assoc, bprr->errcode, bprr->errstring);
	    *resp->presentStatus = Z_PRES_FAILURE;
	}
    }
    apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));
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
		     num, req->recordComposition, next, resp->presentStatus,
			 form, req->referenceId, req->preferredRecordSyntax);
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
static Z_APDU *process_scanRequest(association *assoc, request *reqb, int *fd)
{
    Z_ScanRequest *req = reqb->apdu_request->u.scanRequest;
    Z_APDU *apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));
    Z_ScanResponse *res = (Z_ScanResponse *)
	odr_malloc (assoc->encode, sizeof(*res));
    int *scanStatus = odr_intdup(assoc->encode, Z_Scan_failure);
    int *numberOfEntriesReturned = odr_intdup(assoc->encode, 0);
    Z_ListEntries *ents = (Z_ListEntries *)
	odr_malloc (assoc->encode, sizeof(*ents));
    Z_DiagRecs *diagrecs_p = NULL;
    oident *attset;
    bend_scan_rr *bsrr = (bend_scan_rr *)
        odr_malloc (assoc->encode, sizeof(*bsrr));

    yaz_log(LOG_LOG, "Got ScanRequest");

    apdu->which = Z_APDU_scanResponse;
    apdu->u.scanResponse = res;
    res->referenceId = req->referenceId;

    /* if step is absent, set it to 0 */
    res->stepSize = odr_intdup(assoc->encode, 0);
    if (req->stepSize)
	*res->stepSize = *req->stepSize;

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
            yaz_log (LOG_LOG, "Database '%s'", req->databaseNames[i]);
    }
    bsrr->num_bases = req->num_databaseNames;
    bsrr->basenames = req->databaseNames;
    bsrr->num_entries = *req->numberOfTermsRequested;
    bsrr->term = req->termListAndStartPoint;
    bsrr->referenceId = req->referenceId;
    bsrr->stream = assoc->encode;
    bsrr->print = assoc->print;
    bsrr->step_size = res->stepSize;
    if (req->attributeSet &&
        (attset = oid_getentbyoid(req->attributeSet)) &&
        (attset->oclass == CLASS_ATTSET || attset->oclass == CLASS_GENERAL))
        bsrr->attributeset = attset->value;
    else
        bsrr->attributeset = VAL_NONE;
    log_scan_term (req->termListAndStartPoint, bsrr->attributeset);
    bsrr->term_position = req->preferredPositionInResponse ?
        *req->preferredPositionInResponse : 1;
    ((int (*)(void *, bend_scan_rr *))
     (*assoc->init->bend_scan))(assoc->backend, bsrr);
    if (bsrr->errcode)
        diagrecs_p = diagrecs(assoc, bsrr->errcode, bsrr->errstring);
    else
    {
        int i;
        Z_Entry **tab = (Z_Entry **)
            odr_malloc (assoc->encode, sizeof(*tab) * bsrr->num_entries);
        
        if (bsrr->status == BEND_SCAN_PARTIAL)
            *scanStatus = Z_Scan_partial_5;
        else
            *scanStatus = Z_Scan_success;
        ents->entries = tab;
        ents->num_entries = bsrr->num_entries;
        res->numberOfEntriesReturned = &ents->num_entries;	    
        res->positionOfTerm = &bsrr->term_position;
        for (i = 0; i < bsrr->num_entries; i++)
        {
            Z_Entry *e;
            Z_TermInfo *t;
            Odr_oct *o;
            
            tab[i] = e = (Z_Entry *)odr_malloc(assoc->encode, sizeof(*e));
            if (bsrr->entries[i].occurrences >= 0)
            {
                e->which = Z_Entry_termInfo;
                e->u.termInfo = t = (Z_TermInfo *)
                    odr_malloc(assoc->encode, sizeof(*t));
                t->suggestedAttributes = 0;
                t->displayTerm = 0;
                t->alternativeTerm = 0;
                t->byAttributes = 0;
                t->otherTermInfo = 0;
                t->globalOccurrences = &bsrr->entries[i].occurrences;
                t->term = (Z_Term *)
                    odr_malloc(assoc->encode, sizeof(*t->term));
                t->term->which = Z_Term_general;
                t->term->u.general = o =
                    (Odr_oct *)odr_malloc(assoc->encode, sizeof(Odr_oct));
                o->buf = (unsigned char *)
                    odr_malloc(assoc->encode, o->len = o->size =
                               strlen(bsrr->entries[i].term));
                memcpy(o->buf, bsrr->entries[i].term, o->len);
                yaz_log(LOG_DEBUG, "  term #%d: '%s' (%d)", i,
			 bsrr->entries[i].term, bsrr->entries[i].occurrences);
            }
            else
            {
                Z_DiagRecs *drecs = diagrecs (assoc,
                                              bsrr->entries[i].errcode,
                                              bsrr->entries[i].errstring);
                assert (drecs->num_diagRecs == 1);
                e->which = Z_Entry_surrogateDiagnostic;
                assert (drecs->diagRecs[0]);
                e->u.surrogateDiagnostic = drecs->diagRecs[0];
            }
        }
    }
    if (diagrecs_p)
    {
	ents->num_nonsurrogateDiagnostics = diagrecs_p->num_diagRecs;
	ents->nonsurrogateDiagnostics = diagrecs_p->diagRecs;
    }
    return apdu;
}

static Z_APDU *process_sortRequest(association *assoc, request *reqb,
    int *fd)
{
    Z_SortRequest *req = reqb->apdu_request->u.sortRequest;
    Z_SortResponse *res = (Z_SortResponse *)
	odr_malloc (assoc->encode, sizeof(*res));
    bend_sort_rr *bsrr = (bend_sort_rr *)
	odr_malloc (assoc->encode, sizeof(*bsrr));

    Z_APDU *apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));

    yaz_log(LOG_LOG, "Got SortRequest.");

    bsrr->num_input_setnames = req->num_inputResultSetNames;
    bsrr->input_setnames = req->inputResultSetNames;
    bsrr->referenceId = req->referenceId;
    bsrr->output_setname = req->sortedResultSetName;
    bsrr->sort_sequence = req->sortSequence;
    bsrr->stream = assoc->encode;
    bsrr->print = assoc->print;

    bsrr->sort_status = Z_SortStatus_failure;
    bsrr->errcode = 0;
    bsrr->errstring = 0;
    
    (*assoc->init->bend_sort)(assoc->backend, bsrr);
    
    res->referenceId = bsrr->referenceId;
    res->sortStatus = odr_intdup(assoc->encode, bsrr->sort_status);
    res->resultSetStatus = 0;
    if (bsrr->errcode)
    {
	Z_DiagRecs *dr = diagrecs (assoc, bsrr->errcode, bsrr->errstring);
	res->diagnostics = dr->diagRecs;
	res->num_diagnostics = dr->num_diagRecs;
    }
    else
    {
	res->num_diagnostics = 0;
	res->diagnostics = 0;
    }
    res->otherInfo = 0;

    apdu->which = Z_APDU_sortResponse;
    apdu->u.sortResponse = res;
    return apdu;
}

static Z_APDU *process_deleteRequest(association *assoc, request *reqb,
    int *fd)
{
    Z_DeleteResultSetRequest *req =
        reqb->apdu_request->u.deleteResultSetRequest;
    Z_DeleteResultSetResponse *res = (Z_DeleteResultSetResponse *)
	odr_malloc (assoc->encode, sizeof(*res));
    bend_delete_rr *bdrr = (bend_delete_rr *)
	odr_malloc (assoc->encode, sizeof(*bdrr));
    Z_APDU *apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));

    yaz_log(LOG_LOG, "Got DeleteRequest.");

    bdrr->num_setnames = req->num_resultSetList;
    bdrr->setnames = req->resultSetList;
    bdrr->stream = assoc->encode;
    bdrr->print = assoc->print;
    bdrr->function = *req->deleteFunction;
    bdrr->referenceId = req->referenceId;
    bdrr->statuses = 0;
    if (bdrr->num_setnames > 0)
    {
	int i;
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
	    odr_malloc (assoc->encode, 
			sizeof(*res->deleteListStatuses->elements) *
			bdrr->num_setnames);
	for (i = 0; i<bdrr->num_setnames; i++)
	{
	    res->deleteListStatuses->elements[i] =
		(Z_ListStatus *)
		odr_malloc (assoc->encode,
			    sizeof(**res->deleteListStatuses->elements));
	    res->deleteListStatuses->elements[i]->status = bdrr->statuses+i;
	    res->deleteListStatuses->elements[i]->id =
		odr_strdup (assoc->encode, bdrr->setnames[i]);
	    
	}
    }
    res->numberNotDeleted = 0;
    res->bulkStatuses = 0;
    res->deleteMessage = 0;
    res->otherInfo = 0;

    apdu->which = Z_APDU_deleteResultSetResponse;
    apdu->u.deleteResultSetResponse = res;
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

    yaz_log(LOG_LOG, "Got Close, reason %s, message %s",
	reasons[*req->closeReason], req->diagnosticInformation ?
	req->diagnosticInformation : "NULL");
    if (assoc->version < 3) /* to make do_force respond with close */
    	assoc->version = 3;
    do_close_req(assoc, Z_Close_finished,
		 "Association terminated by client", reqb);
}

void save_referenceId (request *reqb, Z_ReferenceId *refid)
{
    if (refid)
    {
	reqb->len_refid = refid->len;
	reqb->refid = (char *)nmem_malloc (reqb->request_mem, refid->len);
	memcpy (reqb->refid, refid->buf, refid->len);
    }
    else
    {
	reqb->len_refid = 0;
	reqb->refid = NULL;
    }
}

void bend_request_send (bend_association a, bend_request req, Z_APDU *res)
{
    process_z_response (a, req, res);
}

bend_request bend_request_mk (bend_association a)
{
    request *nreq = request_get (&a->outgoing);
    nreq->request_mem = nmem_create ();
    return nreq;
}

Z_ReferenceId *bend_request_getid (ODR odr, bend_request req)
{
    Z_ReferenceId *id;
    if (!req->refid)
	return 0;
    id = (Odr_oct *)odr_malloc (odr, sizeof(*odr));
    id->buf = (unsigned char *)odr_malloc (odr, req->len_refid);
    id->len = id->size = req->len_refid;
    memcpy (id->buf, req->refid, req->len_refid);
    return id;
}

void bend_request_destroy (bend_request *req)
{
    nmem_destroy((*req)->request_mem);
    request_release(*req);
    *req = NULL;
}

int bend_backend_respond (bend_association a, bend_request req)
{
    char *msg;
    int r;
    r = process_z_request (a, req, &msg);
    if (r < 0)
	yaz_log (LOG_WARN, "%s", msg);
    return r;
}

void bend_request_setdata(bend_request r, void *p)
{
    r->clientData = p;
}

void *bend_request_getdata(bend_request r)
{
    return r->clientData;
}

static Z_APDU *process_segmentRequest (association *assoc, request *reqb)
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

static Z_APDU *process_ESRequest(association *assoc, request *reqb, int *fd)
{
    bend_esrequest_rr esrequest;

    Z_ExtendedServicesRequest *req =
        reqb->apdu_request->u.extendedServicesRequest;
    Z_APDU *apdu = zget_APDU(assoc->encode, Z_APDU_extendedServicesResponse);

    Z_ExtendedServicesResponse *resp = apdu->u.extendedServicesResponse;

    yaz_log(LOG_DEBUG,"inside Process esRequest");

    esrequest.esr = reqb->apdu_request->u.extendedServicesRequest;
    esrequest.stream = assoc->encode;
    esrequest.decode = assoc->decode;
    esrequest.print = assoc->print;
    esrequest.errcode = 0;
    esrequest.errstring = NULL;
    esrequest.request = reqb;
    esrequest.association = assoc;
    esrequest.taskPackage = 0;
    esrequest.referenceId = req->referenceId;
    
    (*assoc->init->bend_esrequest)(assoc->backend, &esrequest);
    
    /* If the response is being delayed, return NULL */
    if (esrequest.request == NULL)
        return(NULL);

    resp->referenceId = req->referenceId;

    if (esrequest.errcode == -1)
    {
        /* Backend service indicates request will be processed */
        yaz_log(LOG_DEBUG,"Request could be processed...Accepted !");
        *resp->operationStatus = Z_ExtendedServicesResponse_accepted;
    }
    else if (esrequest.errcode == 0)
    {
        /* Backend service indicates request will be processed */
        yaz_log(LOG_DEBUG,"Request could be processed...Done !");
        *resp->operationStatus = Z_ExtendedServicesResponse_done;
    }
    else
    {
	Z_DiagRecs *diagRecs = diagrecs (assoc, esrequest.errcode,
					 esrequest.errstring);

        /* Backend indicates error, request will not be processed */
        yaz_log(LOG_DEBUG,"Request could not be processed...failure !");
        *resp->operationStatus = Z_ExtendedServicesResponse_failure;
	resp->num_diagnostics = diagRecs->num_diagRecs;
	resp->diagnostics = diagRecs->diagRecs;
    }
    /* Do something with the members of bend_extendedservice */
    if (esrequest.taskPackage)
	resp->taskPackage = z_ext_record (assoc->encode, VAL_EXTENDED,
					 (const char *)  esrequest.taskPackage,
					  -1);
    yaz_log(LOG_DEBUG,"Send the result apdu");
    return apdu;
}

