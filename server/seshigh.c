/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: seshigh.c,v $
 * Revision 1.30  1995-06-05 10:53:32  quinn
 * Added a better SCAN.
 *
 * Revision 1.29  1995/06/01  11:25:03  quinn
 * Smallish.
 *
 * Revision 1.28  1995/06/01  11:21:01  quinn
 * Attempting to fix a bug in pack-records. replaced break with continue
 * for large records, according to standard.
 *
 * Revision 1.27  1995/05/29  08:12:06  quinn
 * Moved oid to util
 *
 * Revision 1.26  1995/05/18  13:02:12  quinn
 * Smallish.
 *
 * Revision 1.25  1995/05/17  08:42:26  quinn
 * Transfer auth info to backend. Allow backend to reject init gracefully.
 *
 * Revision 1.24  1995/05/16  08:51:04  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.23  1995/05/15  13:25:10  quinn
 * Fixed memory bug.
 *
 * Revision 1.22  1995/05/15  11:56:39  quinn
 * Asynchronous facilities. Restructuring of seshigh code.
 *
 * Revision 1.21  1995/05/02  08:53:19  quinn
 * Trying in vain to fix comm with ISODE
 *
 * Revision 1.20  1995/04/20  15:13:00  quinn
 * Cosmetic
 *
 * Revision 1.19  1995/04/18  08:15:34  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.18  1995/04/17  11:28:25  quinn
 * Smallish
 *
 * Revision 1.17  1995/04/10  10:23:36  quinn
 * Some work to add scan and other things.
 *
 * Revision 1.16  1995/03/31  09:18:55  quinn
 * Added logging.
 *
 * Revision 1.15  1995/03/30  14:03:23  quinn
 * Added RFC1006 as separate library
 *
 * Revision 1.14  1995/03/30  12:18:17  quinn
 * Fixed bug.
 *
 * Revision 1.13  1995/03/30  09:09:24  quinn
 * Added state-handle and some support for asynchronous activities.
 *
 * Revision 1.12  1995/03/29  15:40:16  quinn
 * Ongoing work. Statserv is now dynamic by default
 *
 * Revision 1.11  1995/03/28  09:16:21  quinn
 * Added record packing to the search request
 *
 * Revision 1.10  1995/03/27  08:34:24  quinn
 * Added dynamic server functionality.
 * Released bindings to session.c (is now redundant)
 *
 * Revision 1.9  1995/03/22  15:01:26  quinn
 * Adjusting record packing.
 *
 * Revision 1.8  1995/03/22  10:13:21  quinn
 * Working on record packer
 *
 * Revision 1.7  1995/03/21  15:53:31  quinn
 * Little changes.
 *
 * Revision 1.6  1995/03/21  12:30:09  quinn
 * Beginning to add support for record packing.
 *
 * Revision 1.5  1995/03/17  10:44:13  quinn
 * Added catch of null-string in makediagrec
 *
 * Revision 1.4  1995/03/17  10:18:08  quinn
 * Added memory management.
 *
 * Revision 1.3  1995/03/16  17:42:39  quinn
 * Little changes
 *
 * Revision 1.2  1995/03/16  13:29:01  quinn
 * Partitioned server.
 *
 * Revision 1.1  1995/03/15  16:02:10  quinn
 * Modded session.c to seshigh.c
 *
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
 * TODOs include (and will be done in order of public interest):
 * 
 * Support for EXPLAIN - provide simple meta-database system.
 * Support for access control.
 * Support for resource control.
 * Support for extended services - primarily Item Order.
 * Rest of Z39.50-1994
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <dmalloc.h>
#include <comstack.h>
#include <eventl.h>
#include <session.h>
#include <proto.h>
#include <oid.h>
#include <log.h>
#include <statserv.h>
#include "../version.h"

#include <backend.h>

static int process_request(association *assoc);
void backend_response(IOCHAN i, int event);
static int process_response(association *assoc, request *req, Z_APDU *res);
static Z_APDU *process_initRequest(association *assoc, request *reqb);
static Z_APDU *process_searchRequest(association *assoc, request *reqb,
    int *fd);
static Z_APDU *response_searchRequest(association *assoc, request *reqb,
    bend_searchresult *bsrt, int *fd);
static Z_APDU *process_presentRequest(association *assoc, request *reqb,
    int *fd);
static Z_APDU *process_scanRequest(association *assoc, request *reqb, int *fd);

static FILE *apduf = 0; /* for use in static mode */
static statserv_options_block *control_block = 0;

/*
 * Create and initialize a new association-handle.
 *  channel  : iochannel for the current line.
 *  link     : communications channel.
 * Returns: 0 or a new association handle.
 */
association *create_association(IOCHAN channel, COMSTACK link)
{
    association *new;

    if (!control_block)
    	control_block = statserv_getcontrol();
    if (!(new = malloc(sizeof(*new))))
    	return 0;
    new->client_chan = channel;
    new->client_link = link;
    if (!(new->decode = odr_createmem(ODR_DECODE)) ||
    	!(new->encode = odr_createmem(ODR_ENCODE)))
	return 0;
    if (*control_block->apdufile)
    {
    	char filename[256];
	FILE *f;

	strcpy(filename, control_block->apdufile);
	if (!(new->print = odr_createmem(ODR_PRINT)))
	    return 0;
	if (*control_block->apdufile != '-')
	{
	    strcpy(filename, control_block->apdufile);
	    if (!control_block->dynamic)
	    {
		if (!apduf)
		{
		    if (!(apduf = fopen(filename, "w")))
		    {
			logf(LOG_WARN|LOG_ERRNO, "%s", filename);
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
		    logf(LOG_WARN|LOG_ERRNO, "%s", filename);
		    return 0;
		}
		setvbuf(f, 0, _IONBF, 0);
	    }
	    odr_setprint(new->print, f);
    	}
    }
    else
    	new->print = 0;
    new->input_buffer = 0;
    new->input_buffer_len = 0;
    new->backend = 0;
    new->rejected = 0;
    request_initq(&new->incoming);
    request_initq(&new->outgoing);
    new->proto = cs_getproto(link);
    return new;
}

/*
 * Free association and release resources.
 */
void destroy_association(association *h)
{
    odr_destroy(h->decode);
    odr_destroy(h->encode);
    if (h->print)
	odr_destroy(h->print);
    if (h->input_buffer)
    	free(h->input_buffer);
    if (h->backend)
    	bend_close(h->backend);
    while (request_deq(&h->incoming));
    while (request_deq(&h->outgoing));
    free(h);
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
    association *assoc = iochan_getdata(h);
    COMSTACK conn = assoc->client_link;
    request *req;

    assert(h && conn && assoc);
    if (event & EVENT_INPUT || event & EVENT_WORK) /* input */
    {
    	if (event & EVENT_INPUT)
	{
	    logf(LOG_DEBUG, "ir_session (input)");
	    assert(assoc && conn);
	    /* We aren't speaking to this fellow */
	    if (assoc->rejected)
	    {
		logf(LOG_LOG, "Closed connection after reject");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		return;
	    }
	    if ((res = cs_get(conn, &assoc->input_buffer,
		&assoc->input_buffer_len)) <= 0)
	    {
		logf(LOG_LOG, "Connection closed by client");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		return;
	    }
	    else if (res == 1) /* incomplete read - wait for more  */
		return;
	    if (cs_more(conn)) /* more stuff - call us again later, please */
		iochan_setevent(h, EVENT_INPUT);
	    	
	    /* we got a complete PDU. Let's decode it */
	    req = request_get(); /* get a new request structure */
	    odr_reset(assoc->decode);
	    odr_setbuf(assoc->decode, assoc->input_buffer, res, 0);
	    if (!z_APDU(assoc->decode, &req->request, 0))
	    {
		logf(LOG_WARN, "ODR error: %s",
		    odr_errlist[odr_geterror(assoc->decode)]);
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		return;
	    }
	    req->request_mem = odr_extract_mem(assoc->decode);
	    if (assoc->print && !z_APDU(assoc->print, &req->request, 0))
	    {
		logf(LOG_WARN, "ODR print error: %s", 
		    odr_errlist[odr_geterror(assoc->print)]);
		odr_reset(assoc->print);
	    }
	    request_enq(&assoc->incoming, req);
	}

	/* can we do something yet? */
	req = request_head(&assoc->incoming);
	if (req->state == REQUEST_IDLE)
	    if (process_request(assoc) < 0)
	    {
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
	    }
    }
    if (event & EVENT_OUTPUT)
    {
    	request *req = request_head(&assoc->outgoing);

	logf(LOG_DEBUG, "ir_session (output)");
	req->state = REQUEST_PENDING;
    	switch (res = cs_put(conn, req->response, req->len_response))
	{
	    case -1:
	    	logf(LOG_LOG, "Connection closed by client");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		break;
	    case 0: /* all sent - release the request structure */
		odr_release_mem(req->request_mem);
		request_deq(&assoc->outgoing);
		request_release(req);
		if (!request_head(&assoc->outgoing))
		    iochan_clearflag(h, EVENT_OUTPUT);
		break;
	    /* value of 1 -- partial send -- is simply ignored */
	}
    }
    if (event & EVENT_EXCEPT)
    {
	logf(LOG_DEBUG, "ir_session (exception)");
	cs_close(conn);
	destroy_association(assoc);
	iochan_destroy(h);
    }
}

/*
 * Initiate request processing.
 */
static int process_request(association *assoc)
{
    request *req = request_head(&assoc->incoming);
    int fd = -1;
    Z_APDU *res;

    logf(LOG_DEBUG, "process_request");
    assert(req && req->state == REQUEST_IDLE);
    switch (req->request->which)
    {
    	case Z_APDU_initRequest:
	    res = process_initRequest(assoc, req); break;
	case Z_APDU_searchRequest:
	    res = process_searchRequest(assoc, req, &fd); break;
	case Z_APDU_presentRequest:
	    res = process_presentRequest(assoc, req, &fd); break;
	case Z_APDU_scanRequest:
	    res = process_scanRequest(assoc, req, &fd); break;
	default:
	    logf(LOG_WARN, "Bad APDU received");
	    return -1;
    }
    if (res)
    {
    	logf(LOG_DEBUG, "  result immediately available");
    	return process_response(assoc, req, res);
    }
    else if (fd < 0)
    {
    	logf(LOG_WARN, "   bad result");
    	return -1;
    }
    else /* no result yet - one will be provided later */
    {
    	IOCHAN chan;

	/* Set up an I/O handler for the fd supplied by the backend */

	logf(LOG_DEBUG, "   establishing handler for result");
	req->state = REQUEST_PENDING;
	if (!(chan = iochan_create(fd, backend_response, EVENT_INPUT)))
	    abort();
	iochan_setdata(chan, assoc);
	return 0;
    }
}

/*
 * Handle message from the backend.
 */
void backend_response(IOCHAN i, int event)
{
    association *assoc = iochan_getdata(i);
    request *req = request_head(&assoc->incoming);
    Z_APDU *res;
    int fd;

    logf(LOG_DEBUG, "backend_response");
    assert(assoc && req && req->state != REQUEST_IDLE);
    /* determine what it is we're waiting for */
    switch (req->request->which)
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
	    logf(LOG_WARN, "Serious programmer's lapse or bug");
	    abort();
    }
    if ((res && process_response(assoc, req, res) < 0) || fd < 0)
    {
	logf(LOG_LOG, "Fatal error when talking to backend");
	cs_close(assoc->client_link);
	destroy_association(assoc);
	iochan_destroy(assoc->client_chan);
	iochan_destroy(i);
	return;
    }
    else if (!res) /* no result yet - try again later */
    {
    	logf(LOG_DEBUG, "   no result yet");
    	iochan_setfd(i, fd); /* in case fd has changed */
    }
}

/*
 * Encode response, and transfer the request structure to the outgoing queue.
 */
static int process_response(association *assoc, request *req, Z_APDU *res)
{
    odr_setbuf(assoc->encode, req->response, req->size_response, 1);
    if (!z_APDU(assoc->encode, &res, 0))
    {
    	logf(LOG_WARN, "ODR error when encoding response: %s",
	    odr_errlist[odr_geterror(assoc->decode)]);
	return -1;
    }
    req->response = odr_getbuf(assoc->encode, &req->len_response,
	&req->size_response);
    odr_setbuf(assoc->encode, 0, 0, 0); /* don't free if we abort later */
    odr_reset(assoc->encode);
    if (assoc->print && !z_APDU(assoc->print, &res, 0))
    {
	logf(LOG_WARN, "ODR print error: %s", 
	    odr_errlist[odr_geterror(assoc->print)]);
	odr_reset(assoc->print);
    }
    /* change this when we make the backend reentrant */
    assert(req == request_head(&assoc->incoming));
    req->state = REQUEST_IDLE;
    request_deq(&assoc->incoming);
    request_enq(&assoc->outgoing, req);
    /* turn the work over to the ir_session handler */
    iochan_setflag(assoc->client_chan, EVENT_OUTPUT);
    /* Is there more work to be done? give that to the input handler too */
    if (request_head(&assoc->incoming))
    	iochan_setevent(assoc->client_chan, EVENT_WORK);
    return 0;
}

/*
 * Handle init request.
 * At the moment, we don't check the protocol version or the options
 * anywhere else in the code - we just try not to do anything that would
 * break a naive client. We'll toss 'em into the association block when
 * we need them there.
 */
static Z_APDU *process_initRequest(association *assoc, request *reqb)
{
    Z_InitRequest *req = reqb->request->u.initRequest;
    Z_APDU *apdu = zget_APDU(assoc->encode, Z_APDU_initResponse);
    Z_InitResponse *resp = apdu->u.initResponse;
    bend_initrequest binitreq;
    bend_initresult *binitres;

    logf(LOG_LOG, "Got initRequest");
    if (req->implementationId)
    	logf(LOG_LOG, "Id:        %s", req->implementationId);
    if (req->implementationName)
    	logf(LOG_LOG, "Name:      %s", req->implementationName);
    if (req->implementationVersion)
    	logf(LOG_LOG, "Version:   %s", req->implementationVersion);

    binitreq.configname = "default-config";
    binitreq.auth = req->idAuthentication;
    if (!(binitres = bend_init(&binitreq)))
    {
    	logf(LOG_WARN, "Bad response from backend.");
    	return 0;
    }

    assoc->backend = binitres->handle;
    resp->referenceId = req->referenceId;
    /* let's tell the client what we can do */
    if (ODR_MASK_GET(req->options, Z_Options_search))
    	ODR_MASK_SET(resp->options, Z_Options_search);
    if (ODR_MASK_GET(req->options, Z_Options_present))
    	ODR_MASK_SET(resp->options, Z_Options_present);
#if 0
    if (ODR_MASK_GET(req->options, Z_Options_delSet))
    	ODR_MASK_SET(&options, Z_Options_delSet);
#endif
    if (ODR_MASK_GET(req->options, Z_Options_namedResultSets))
    	ODR_MASK_SET(resp->options, Z_Options_namedResultSets);
    if (ODR_MASK_GET(req->options, Z_Options_scan))
    	ODR_MASK_SET(resp->options, Z_Options_scan);
    if (ODR_MASK_GET(req->options, Z_Options_concurrentOperations))
    	ODR_MASK_SET(resp->options, Z_Options_concurrentOperations);

    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_1))
    	ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_1);
    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_2))
    	ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_2);
    assoc->maximumRecordSize = *req->maximumRecordSize;
    if (assoc->maximumRecordSize > control_block->maxrecordsize)
    	assoc->maximumRecordSize = control_block->maxrecordsize;
    assoc->preferredMessageSize = *req->preferredMessageSize;
    if (assoc->preferredMessageSize > assoc->maximumRecordSize)
    	assoc->preferredMessageSize = assoc->maximumRecordSize;
    resp->preferredMessageSize = &assoc->preferredMessageSize;
    resp->maximumRecordSize = &assoc->maximumRecordSize;
    resp->implementationName = "Index Data/YAZ Generic Frontend Server";
    if (binitres->errcode)
    {
    	logf(LOG_LOG, "Connection rejected by backend.");
    	*resp->result = 0;
	assoc->rejected = 1;
    }
    return apdu;
}

/*
 * These functions should be merged.
 */

/*
 * nonsurrogate diagnostic record.
 */
static Z_Records *diagrec(oid_proto proto, int error, char *addinfo)
{
    static Z_Records rec;
    oident bib1;
    static Z_DiagRec dr;
    static int err;

    bib1.proto = proto;
    bib1.class = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    logf(LOG_DEBUG, "Diagnostic: %d -- %s", error, addinfo ? addinfo :
	"NULL");
    err = error;
    rec.which = Z_Records_NSD;
    rec.u.nonSurrogateDiagnostic = &dr;
    dr.diagnosticSetId = oid_getoidbyent(&bib1);
    dr.condition = &err;
    dr.addinfo = addinfo ? addinfo : "";
    return &rec;
}

/*
 * surrogate diagnostic.
 */
static Z_NamePlusRecord *surrogatediagrec(oid_proto proto, char *dbname,
					    int error, char *addinfo)
{
    static Z_NamePlusRecord rec;
    static Z_DiagRec dr;
    static int err;
    oident bib1;

    bib1.proto = proto;
    bib1.class = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    logf(LOG_DEBUG, "SurrogateDiagnotic: %d -- %s", error, addinfo);
    err = error;
    rec.databaseName = dbname;
    rec.which = Z_NamePlusRecord_surrogateDiagnostic;
    rec.u.surrogateDiagnostic = &dr;
    dr.diagnosticSetId = oid_getoidbyent(&bib1);
    dr.condition = &err;
    dr.addinfo = addinfo ? addinfo : "";
    return &rec;
}

/*
 * multiple nonsurrogate diagnostics.
 */
static Z_DiagRecs *diagrecs(oid_proto proto, int error, char *addinfo)
{
    static Z_DiagRecs recs;
    static Z_DiagRec *recp[1], rec;
    static int err;
    oident bib1;

    logf(LOG_DEBUG, "DiagRecs: %d -- %s", error, addinfo);
    bib1.proto = proto;
    bib1.class = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    err = error;
    recs.num_diagRecs = 1;
    recs.diagRecs = recp;
    recp[0] = &rec;
    rec.diagnosticSetId = oid_getoidbyent(&bib1);
    rec.condition = &err;
    rec.addinfo = addinfo ? addinfo : "";
    return &recs;
}

#define MAX_RECORDS 256

static Z_Records *pack_records(association *a, char *setname, int start,
				int *num, Z_ElementSetNames *esn,
				int *next, int *pres)
{
    int recno, total_length = 0, toget = *num;
    static Z_Records records;
    static Z_NamePlusRecordList reclist;
    static Z_NamePlusRecord *list[MAX_RECORDS];
    oident recform;
    Odr_oid *oid;

    records.which = Z_Records_DBOSD;
    records.u.databaseOrSurDiagnostics = &reclist;
    reclist.num_records = 0;
    reclist.records = list;
    *pres = Z_PRES_SUCCESS;
    *num = 0;
    *next = 0;

    recform.proto = a->proto;
    recform.class = CLASS_RECSYN;
    recform.value = VAL_USMARC;
    if (!(oid = odr_oiddup(a->encode, oid_getoidbyent(&recform))))
    	return 0;

    logf(LOG_DEBUG, "Request to pack %d+%d", start, toget);
    logf(LOG_DEBUG, "pms=%d, mrs=%d", a->preferredMessageSize,
    	a->maximumRecordSize);
    for (recno = start; reclist.num_records < toget; recno++)
    {
    	bend_fetchrequest freq;
	bend_fetchresult *fres;
	Z_NamePlusRecord *thisrec;
	Z_DatabaseRecord *thisext;

	if (reclist.num_records == MAX_RECORDS - 1)
	{
	    *pres = Z_PRES_PARTIAL_2;
	    break;
	}
	freq.setname = setname;
	freq.number = recno;
	if (!(fres = bend_fetch(a->backend, &freq, 0)))
	{
	    *pres = Z_PRES_FAILURE;
	    return diagrec(a->proto, 2, "Backend interface problem");
	}
	/* backend should be able to signal whether error is system-wide
	   or only pertaining to current record */
	if (fres->errcode)
	{
	    *pres = Z_PRES_FAILURE;
	    return diagrec(a->proto, fres->errcode, fres->errstring);
	}
	logf(LOG_DEBUG, "  fetched record, len=%d, total=%d",
	    fres->len, total_length);
	if (fres->len + total_length > a->preferredMessageSize)
	{
	    /* record is small enough, really */
	    if (fres->len <= a->preferredMessageSize)
	    {
	    	logf(LOG_DEBUG, "  Dropped last normal-sized record");
		*pres = Z_PRES_PARTIAL_2;
		break;
	    }
	    /* record can only be fetched by itself */
	    if (fres->len < a->maximumRecordSize)
	    {
	    	logf(LOG_DEBUG, "  Record > prefmsgsz");
	    	if (toget > 1)
		{
		    logf(LOG_DEBUG, "  Dropped it");
		    reclist.records[reclist.num_records] =
		   	 surrogatediagrec(a->proto, fres->basename, 16, 0);
		    reclist.num_records++;
		    total_length += 10; /* totally arbitrary */
		    continue;
		}
	    }
	    else /* too big entirely */
	    {
	    	logf(LOG_DEBUG, "Record > maxrcdsz");
		reclist.records[reclist.num_records] =
		    surrogatediagrec(a->proto, fres->basename, 17, 0);
		reclist.num_records++;
		total_length += 10; /* totally arbitrary */
		continue;
	    }
	}
	if (!(thisrec = odr_malloc(a->encode, sizeof(*thisrec))))
	    return 0;
	if (!(thisrec->databaseName = odr_malloc(a->encode,
	    strlen(fres->basename) + 1)))
	    return 0;
	strcpy(thisrec->databaseName, fres->basename);
	thisrec->which = Z_NamePlusRecord_databaseRecord;
	if (!(thisrec->u.databaseRecord = thisext = odr_malloc(a->encode,
	    sizeof(Z_DatabaseRecord))))
	    return 0;
	thisext->direct_reference = oid; /* should be OID for current MARC */
	thisext->indirect_reference = 0;
	thisext->descriptor = 0;
	thisext->which = ODR_EXTERNAL_octet;
	if (!(thisext->u.octet_aligned = odr_malloc(a->encode,
	    sizeof(Odr_oct))))
	    return 0;
	if (!(thisext->u.octet_aligned->buf = odr_malloc(a->encode, fres->len)))
	    return 0;
	memcpy(thisext->u.octet_aligned->buf, fres->record, fres->len);
	thisext->u.octet_aligned->len = thisext->u.octet_aligned->size =
	    fres->len;
	reclist.records[reclist.num_records] = thisrec;
	reclist.num_records++;
	total_length += fres->len;
	*next = fres->last_in_set ? 0 : recno + 1;
    }
    *num = reclist.num_records;
    return &records;
}

static Z_APDU *process_searchRequest(association *assoc, request *reqb,
    int *fd)
{
    Z_SearchRequest *req = reqb->request->u.searchRequest;
    bend_searchrequest bsrq;
    bend_searchresult *bsrt;

    logf(LOG_LOG, "Got SearchRequest.");

    bsrq.setname = req->resultSetName;
    bsrq.replace_set = *req->replaceIndicator;
    bsrq.num_bases = req->num_databaseNames;
    bsrq.basenames = req->databaseNames;
    bsrq.query = req->query;

    if (!(bsrt = bend_search(assoc->backend, &bsrq, fd)))
	return 0;
    return response_searchRequest(assoc, reqb, bsrt, fd);
}

bend_searchresult *bend_searchresponse(void *handle) {return 0;}

/*
 * Prepare a searchresponse based on the backend results. We probably want
 * to look at making the fetching of records nonblocking as well, but
 * so far, we'll keep things simple.
 * If bsrt is null, that means we're called in response to a communications
 * event, and we'll have to get the response for ourselves.
 */
static Z_APDU *response_searchRequest(association *assoc, request *reqb,
    bend_searchresult *bsrt, int *fd)
{
    Z_SearchRequest *req = reqb->request->u.searchRequest;
    static Z_APDU apdu;
    static Z_SearchResponse resp;
    static int nulint = 0;
    static bool_t sr = 1;
    static int next = 0;
    static int none = Z_RES_NONE;

    apdu.which = Z_APDU_searchResponse;
    apdu.u.searchResponse = &resp;
    resp.referenceId = req->referenceId;
    *fd = -1;
    if (!bsrt && !(bsrt = bend_searchresponse(assoc->backend)))
    {
    	logf(LOG_FATAL, "Bad result from backend");
    	return 0;
    }
    else if (bsrt->errcode)
    {
	resp.records = diagrec(assoc->proto, bsrt->errcode,
	    bsrt->errstring);
	resp.resultCount = &nulint;
	resp.numberOfRecordsReturned = &nulint;
	resp.nextResultSetPosition = &nulint;
	resp.searchStatus = &nulint;
	resp.resultSetStatus = &none;
	resp.presentStatus = 0;
    }
    else
    {
    	int toget;
	Z_ElementSetNames *setnames;
	int presst = 0;

	resp.records = 0;
	resp.resultCount = &bsrt->hits;

	/* how many records does the user agent want, then? */
	if (bsrt->hits <= *req->smallSetUpperBound)
	{
	    toget = bsrt->hits;
	    setnames = req->smallSetElementSetNames;
	}
	else if (bsrt->hits < *req->largeSetLowerBound)
	{
	    toget = *req->mediumSetPresentNumber;
	    if (toget > bsrt->hits)
		toget = bsrt->hits;
	    setnames = req->mediumSetElementSetNames;
	}
	else
	    toget = 0;

	if (toget && !resp.records)
	{
	    resp.records = pack_records(assoc, req->resultSetName, 1,
		&toget, setnames, &next, &presst);
	    if (!resp.records)
		return 0;
	    resp.numberOfRecordsReturned = &toget;
	    resp.nextResultSetPosition = &next;
	    resp.searchStatus = &sr;
	    resp.resultSetStatus = 0;
	    resp.presentStatus = &presst;
	}
	else
	{
	    resp.numberOfRecordsReturned = &nulint;
	    resp.nextResultSetPosition = &next;
	    resp.searchStatus = &sr;
	    resp.resultSetStatus = 0;
	    resp.presentStatus = 0;
	}
    }
    return &apdu;
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
    Z_PresentRequest *req = reqb->request->u.presentRequest;
    static Z_APDU apdu;
    static Z_PresentResponse resp;
    static int presst, next, num;

    logf(LOG_LOG, "Got PresentRequest.");
    apdu.which = Z_APDU_presentResponse;
    apdu.u.presentResponse = &resp;
    resp.referenceId = req->referenceId;

    num = *req->numberOfRecordsRequested;
    resp.records = pack_records(assoc, req->resultSetId,
	*req->resultSetStartPoint, &num, req->elementSetNames, &next, &presst);
    if (!resp.records)
    	return 0;
    resp.numberOfRecordsReturned = &num;
    resp.presentStatus = &presst;
    resp.nextResultSetPosition = &next;

    return &apdu;
}

/*
 * Scan was implemented rather in a hurry, and with support for only the basic
 * elements of the service in the backend API. Suggestions are welcome.
 */
static Z_APDU *process_scanRequest(association *assoc, request *reqb, int *fd)
{
    Z_ScanRequest *req = reqb->request->u.scanRequest;
    static Z_APDU apdu;
    static Z_ScanResponse res;
    static int scanStatus = Z_Scan_failure;
    static int numberOfEntriesReturned = 0;
    oident *attent;
    static Z_ListEntries ents;
#define SCAN_MAX_ENTRIES 200
    static Z_Entry *tab[SCAN_MAX_ENTRIES];
    bend_scanrequest srq;
    bend_scanresult *srs;

    logf(LOG_LOG, "Got scanrequest");
    apdu.which = Z_APDU_scanResponse;
    apdu.u.scanResponse = &res;
    res.referenceId = req->referenceId;
    res.stepSize = 0;
    res.scanStatus = &scanStatus;
    res.numberOfEntriesReturned = &numberOfEntriesReturned;
    res.positionOfTerm = 0;
    res.entries = &ents;
    ents.which = Z_ListEntries_nonSurrogateDiagnostics;
    res.attributeSet = 0;

    if (req->attributeSet && (!(attent = oid_getentbyoid(req->attributeSet)) ||
    	attent->class != CLASS_ATTSET || attent->value != VAL_BIB1))
	ents.u.nonSurrogateDiagnostics = diagrecs(assoc->proto, 121, 0);
    else if (req->stepSize && *req->stepSize > 0)
    	ents.u.nonSurrogateDiagnostics = diagrecs(assoc->proto, 205, 0);
    else
    {
	srq.num_bases = req->num_databaseNames;
	srq.basenames = req->databaseNames;
	srq.num_entries = *req->numberOfTermsRequested;
	srq.term = req->termListAndStartPoint;
	srq.term_position = req->preferredPositionInResponse ?
	    *req->preferredPositionInResponse : 1;
	if (!(srs = bend_scan(assoc->backend, &srq, 0)))
	    ents.u.nonSurrogateDiagnostics = diagrecs(assoc->proto, 2, 0);
	else if (srs->errcode)
	    ents.u.nonSurrogateDiagnostics = diagrecs(assoc->proto,
		srs->errcode, srs->errstring);
	else
	{
	    int i;
	    static Z_Entries list;

	    if (srs->status == BEND_SCAN_PARTIAL)
	    	scanStatus = Z_Scan_partial_5;
	    else
	    	scanStatus = Z_Scan_success;
	    ents.which = Z_ListEntries_entries;
	    ents.u.entries = &list;
	    list.entries = tab;
	    for (i = 0; i < srs->num_entries; i++)
	    {
	    	Z_Entry *e;
		Z_TermInfo *t;
		Odr_oct *o;

		if (i >= SCAN_MAX_ENTRIES)
		{
		    scanStatus = Z_Scan_partial_4;
		    break;
		}
		list.entries[i] = e = odr_malloc(assoc->encode, sizeof(*e));
		e->which = Z_Entry_termInfo;
		e->u.termInfo = t = odr_malloc(assoc->encode, sizeof(*t));
		t->suggestedAttributes = 0;
		t->alternativeTerm = 0;
		t->byAttributes = 0;
		t->globalOccurrences = &srs->entries[i].occurrences;
		t->term = odr_malloc(assoc->encode, sizeof(*t->term));
		t->term->which = Z_Term_general;
		t->term->u.general = o = odr_malloc(assoc->encode,
		    sizeof(Odr_oct));
		o->buf = odr_malloc(assoc->encode, o->len = o->size =
		    strlen(srs->entries[i].term));
		memcpy(o->buf, srs->entries[i].term, o->len);
	    }
	    list.num_entries = i;
	    res.numberOfEntriesReturned = &list.num_entries;
	    res.positionOfTerm = &srs->term_position;
	}
    }

    return &apdu;
}
