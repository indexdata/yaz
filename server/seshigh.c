/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 *
 * $Log: seshigh.c,v $
 * Revision 1.110  2000-10-02 13:05:32  adam
 * Fixed bug introduced by previous commit.
 *
 * Revision 1.109  2000/10/02 11:07:44  adam
 * Added peer_name member for bend_init handler. Changed the YAZ
 * client so that tcp: can be avoided in target spec.
 *
 * Revision 1.108  2000/09/04 08:58:15  adam
 * Added prefix yaz_ for most logging utility functions.
 *
 * Revision 1.107  2000/08/31 10:20:12  adam
 * Added member request_format and output_format for backend fetch method.
 *
 * Revision 1.106  2000/08/31 09:51:25  adam
 * Added record_syntax member for fetch method (raw OID).
 *
 * Revision 1.105  2000/07/06 10:38:47  adam
 * Enhanced option --enable-tcpd.
 *
 * Revision 1.104  2000/04/05 07:39:55  adam
 * Added shared library support (libtool).
 *
 * Revision 1.103  2000/03/20 19:06:25  adam
 * Added Segment request for fronend server. Work on admin for client.
 *
 * Revision 1.102  2000/03/15 12:59:49  adam
 * Added handle member to statserv_control.
 *
 * Revision 1.101  2000/01/12 14:36:07  adam
 * Added printing stream (ODR) for backend functions.
 *
 * Revision 1.100  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 * Revision 1.99  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.98  1999/11/29 15:12:27  adam
 * Changed the way implementationName - and version is set.
 *
 * Revision 1.96  1999/11/04 14:58:44  adam
 * Added status elements for backend delete result set handler.
 * Updated delete result result set command for client.
 *
 * Revision 1.95  1999/10/11 10:01:24  adam
 * Implemented bend_sort_rr handler for frontend server.
 *
 * Revision 1.94  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.93  1999/07/06 12:17:15  adam
 * Added option -1 that runs server once (for profiling purposes).
 *
 * Revision 1.92  1999/06/17 10:54:45  adam
 * Added facility to specify implementation version - and name
 * for server.
 *
 * Revision 1.91  1999/06/01 14:29:12  adam
 * Work on Extended Services.
 *
 * Revision 1.90  1999/05/27 13:02:20  adam
 * Assigned OID for old DB Update (VAL_DBUPDATE0).
 *
 * Revision 1.89  1999/05/26 15:24:26  adam
 * Fixed minor bugs regarding DB Update (introduced by previous commit).
 *
 * Revision 1.88  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.87  1999/03/31 11:18:25  adam
 * Implemented odr_strdup. Added Reference ID to backend server API.
 *
 * Revision 1.86  1999/02/02 13:57:38  adam
 * Uses preprocessor define WIN32 instead of WINDOWS to build code
 * for Microsoft WIN32.
 *
 * Revision 1.85  1998/11/17 09:52:59  adam
 * Fixed minor bug (introduced by previous commit).
 *
 * Revision 1.84  1998/11/16 16:02:32  adam
 * Added loggin utilies, log_rpn_query and log_scan_term. These used
 * to be part of Zebra.
 *
 * Revision 1.83  1998/11/03 10:09:36  adam
 * Fixed bug regarding YC.
 *
 * Revision 1.82  1998/10/20 14:00:30  quinn
 * Fixed Scan
 *
 * Revision 1.81  1998/10/13 16:12:24  adam
 * Added support for Surrogate Diagnostics for Scan Term entries.
 *
 * Revision 1.80  1998/09/02 12:41:53  adam
 * Added decode stream in bend search structures.
 *
 * Revision 1.79  1998/08/19 16:10:08  adam
 * Changed som member names of DeleteResultSetRequest/Response.
 *
 * Revision 1.78  1998/08/03 10:23:55  adam
 * Fixed bug regarding Options for Sort.
 *
 * Revision 1.77  1998/07/20 12:38:42  adam
 * Implemented delete result set service to server API.
 *
 * Revision 1.76  1998/05/27 16:57:07  adam
 * Support for surrogate diagnostic records added for bend_fetch.
 *
 * Revision 1.75  1998/05/18 10:13:07  adam
 * Fixed call to es_request handler - extra argument was passed.
 *
 * Revision 1.74  1998/03/31 15:13:20  adam
 * Development towards compiled ASN.1.
 *
 * Revision 1.73  1998/03/31 11:07:45  adam
 * Furhter work on UNIverse resource report.
 * Added Extended Services handling in frontend server.
 *
 * Revision 1.72  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.71  1998/02/10 11:03:57  adam
 * Added support for extended handlers in backend server interface.
 *
 * Revision 1.70  1998/01/29 13:15:35  adam
 * Implemented sort for the backend interface.
 *
 * Revision 1.69  1997/09/30 11:48:12  adam
 * Fixed bug introduced by previous commit.
 *
 * Revision 1.68  1997/09/29 13:18:59  adam
 * Added function, oid_ent_to_oid, to replace the function
 * oid_getoidbyent, which is not thread safe.
 *
 * Revision 1.67  1997/09/17 12:10:40  adam
 * YAZ version 1.4.
 *
 * Revision 1.66  1997/09/05 15:26:44  adam
 * Added ODR encode in search and scen bend request structures.
 * Fixed a few enums that caused trouble with C++.
 *
 * Revision 1.65  1997/09/01 08:53:01  adam
 * New windows NT/95 port using MSV5.0. The test server 'ztest' was
 * moved a separate directory. MSV5.0 project server.dsp created.
 * As an option, the server can now operate as an NT service.
 *
 * Revision 1.64  1997/04/30 08:52:11  quinn
 * Null
 *
 * Revision 1.63  1996/10/11  11:57:26  quinn
 * Smallish
 *
 * Revision 1.62  1996/07/06  19:58:35  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.61  1996/06/10  08:56:16  quinn
 * Work on Summary.
 *
 * Revision 1.60  1996/05/30  11:03:10  quinn
 * Fixed NextresultSetPosition bug fixed.
 *
 * Revision 1.59  1996/05/14  09:26:46  quinn
 * Added attribute set to scan backend
 *
 * Revision 1.58  1996/02/20  12:53:04  quinn
 * Chanes to SCAN
 *
 * Revision 1.57  1996/01/02  08:57:47  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.56  1995/12/14  11:09:57  quinn
 * Work on Explain
 *
 * Revision 1.55  1995/11/08  17:41:37  quinn
 * Smallish.
 *
 * Revision 1.54  1995/11/08  15:11:29  quinn
 * Log of close transmit.
 *
 * Revision 1.53  1995/11/01  13:54:58  quinn
 * Minor adjustments
 *
 * Revision 1.52  1995/11/01  12:19:13  quinn
 * Second attempt to fix same bug.
 *
 * Revision 1.50  1995/10/25  16:58:32  quinn
 * Simple.
 *
 * Revision 1.49  1995/10/16  13:51:53  quinn
 * Changes to provide Especs to the backend.
 *
 * Revision 1.48  1995/10/06  08:51:20  quinn
 * Added Write-buffer.
 *
 * Revision 1.47  1995/08/29  14:24:16  quinn
 * Added second half of close-handshake
 *
 * Revision 1.46  1995/08/29  11:17:58  quinn
 * Added code to receive close
 *
 * Revision 1.45  1995/08/21  09:11:00  quinn
 * Smallish fixes to suppport new formats.
 *
 * Revision 1.44  1995/08/17  12:45:25  quinn
 * Fixed minor problems with GRS-1. Added support in c&s.
 *
 * Revision 1.43  1995/08/15  12:00:31  quinn
 * Updated External
 *
 * Revision 1.42  1995/08/15  11:16:50  quinn
 *
 * Revision 1.41  1995/08/02  10:23:06  quinn
 * Smallish
 *
 * Revision 1.40  1995/07/31  14:34:26  quinn
 * Fixed bug in process_searchResponse (numberOfRecordsReturned).
 *
 * Revision 1.39  1995/06/27  13:21:00  quinn
 * SUTRS support
 *
 * Revision 1.38  1995/06/19  12:39:11  quinn
 * Fixed bug in timeout code. Added BER dumper.
 *
 * Revision 1.37  1995/06/16  13:16:14  quinn
 * Fixed Defaultdiagformat.
 *
 * Revision 1.36  1995/06/16  10:31:36  quinn
 * Added session timeout.
 *
 * Revision 1.35  1995/06/15  07:45:14  quinn
 * Moving to v3.
 *
 * Revision 1.34  1995/06/14  15:26:46  quinn
 * *** empty log message ***
 *
 * Revision 1.33  1995/06/06  14:57:05  quinn
 * Better diagnostics.
 *
 * Revision 1.32  1995/06/06  08:41:44  quinn
 * Better diagnostics.
 *
 * Revision 1.31  1995/06/06  08:15:37  quinn
 * Cosmetic.
 *
 * Revision 1.30  1995/06/05  10:53:32  quinn
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
#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif
#include <assert.h>

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

#include <yaz/backend.h>

static int process_request(association *assoc, request *req, char **msg);
void backend_response(IOCHAN i, int event);
static int process_response(association *assoc, request *req, Z_APDU *res);
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

/* Chas: Added in from DALI */
static Z_APDU *process_ESRequest(association *assoc, request *reqb, int *fd);
/* Chas: End of addition from DALI */

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
    anew->client_chan = channel;
    anew->client_link = link;
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
	if (*control_block->apdufile != '-')
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
    if (control_block && control_block->one_shot)
	exit (0);
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
	process_response(a, req, &apdu);
	iochan_settimeout(a->client_chan, 60);
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
    if (event & EVENT_INPUT || event & EVENT_WORK) /* input */
    {
    	if (event & EVENT_INPUT)
	{
	    yaz_log(LOG_DEBUG, "ir_session (input)");
	    assert(assoc && conn);
	    /* We aren't speaking to this fellow */
	    if (assoc->state == ASSOC_DEAD)
	    {
		yaz_log(LOG_LOG, "Closed connection after reject");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		return;
	    }
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
		return;
	    if (cs_more(conn)) /* more stuff - call us again later, please */
		iochan_setevent(h, EVENT_INPUT);
	    	
	    /* we got a complete PDU. Let's decode it */
	    yaz_log(LOG_DEBUG, "Got PDU, %d bytes", res);
	    req = request_get(&assoc->incoming); /* get a new request structure */
	    odr_reset(assoc->decode);
	    odr_setbuf(assoc->decode, assoc->input_buffer, res, 0);
	    if (!z_APDU(assoc->decode, &req->request, 0, 0))
	    {
		yaz_log(LOG_LOG, "ODR error on incoming PDU: %s [near byte %d] ",
		    odr_errmsg(odr_geterror(assoc->decode)),
		    odr_offset(assoc->decode));
		yaz_log(LOG_LOG, "PDU dump:");
		odr_dumpBER(yaz_log_file(), assoc->input_buffer, res);
		do_close(assoc, Z_Close_protocolError, "Malformed package");
		return;
	    }
	    req->request_mem = odr_extract_mem(assoc->decode);
	    if (assoc->print && !z_APDU(assoc->print, &req->request, 0, 0))
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
	    char *msg;
	    request_deq(&assoc->incoming);
	    if (process_request(assoc, req, &msg) < 0)
		do_close_req(assoc, Z_Close_systemProblem, msg, req);
	}
    }
    if (event & EVENT_OUTPUT)
    {
    	request *req = request_head(&assoc->outgoing);

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
		nmem_destroy(req->request_mem);
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
	yaz_log(LOG_DEBUG, "ir_session (exception)");
	cs_close(conn);
	destroy_association(assoc);
	iochan_destroy(h);
    }
}

/*
 * Initiate request processing.
 */
static int process_request(association *assoc, request *req, char **msg)
{
    int fd = -1;
    Z_APDU *res;
    int retval;
    
    *msg = "Unknown Error";
    assert(req && req->state == REQUEST_IDLE);
    if (req->request->which != Z_APDU_initRequest && !assoc->init)
    {
	*msg = "Missing InitRequest";
	return -1;
    }
    switch (req->request->which)
    {
    case Z_APDU_initRequest:
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
    	retval = process_response(assoc, req, res);
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
	    yaz_log(LOG_WARN, "Serious programmer's lapse or bug");
	    abort();
    }
    if ((res && process_response(assoc, req, res) < 0) || fd < 0)
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
static int process_response(association *assoc, request *req, Z_APDU *res)
{
    odr_setbuf(assoc->encode, req->response, req->size_response, 1);
    if (!z_APDU(assoc->encode, &res, 0, 0))
    {
    	yaz_log(LOG_WARN, "ODR error when encoding response: %s",
	    odr_errmsg(odr_geterror(assoc->decode)));
	odr_reset(assoc->encode);
	return -1;
    }
    req->response = odr_getbuf(assoc->encode, &req->len_response,
	&req->size_response);
    odr_setbuf(assoc->encode, 0, 0, 0); /* don'txfree if we abort later */
    if (assoc->print && !z_APDU(assoc->print, &res, 0, 0))
    {
	yaz_log(LOG_WARN, "ODR print error: %s", 
	    odr_errmsg(odr_geterror(assoc->print)));
	odr_reset(assoc->print);
    }
    odr_reset(assoc->encode);
    req->state = REQUEST_IDLE;
    request_enq(&assoc->outgoing, req);
    /* turn the work over to the ir_session handler */
    iochan_setflag(assoc->client_chan, EVENT_OUTPUT);
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
 * Handle init request.
 * At the moment, we don't check the options
 * anywhere else in the code - we just try not to do anything that would
 * break a naive client. We'll toss 'em into the association block when
 * we need them there.
 */
static Z_APDU *process_initRequest(association *assoc, request *reqb)
{
    statserv_options_block *cb = statserv_getcontrol();
    Z_InitRequest *req = reqb->request->u.initRequest;
    Z_APDU *apdu = zget_APDU(assoc->encode, Z_APDU_initResponse);
    Z_InitResponse *resp = apdu->u.initResponse;
    bend_initresult *binitres;
    char options[100];

    xfree (assoc->init);
    assoc->init = xmalloc (sizeof(*assoc->init));

    yaz_log(LOG_LOG, "Got initRequest");
    if (req->implementationId)
    	yaz_log(LOG_LOG, "Id:        %s", req->implementationId);
    if (req->implementationName)
    	yaz_log(LOG_LOG, "Name:      %s", req->implementationName);
    if (req->implementationVersion)
    	yaz_log(LOG_LOG, "Version:   %s", req->implementationVersion);

    assoc->init->stream = assoc->encode;
    assoc->init->print = assoc->print;
    assoc->init->auth = req->idAuthentication;
    assoc->init->referenceId = req->referenceId;
    assoc->init->implementation_version = 0;
    assoc->init->implementation_name = 0;
    assoc->init->bend_sort = NULL;
    assoc->init->bend_search = NULL;
    assoc->init->bend_present = NULL;
    assoc->init->bend_esrequest = NULL;
    assoc->init->bend_delete = NULL;
    assoc->init->bend_scan = NULL;
    assoc->init->bend_segment = NULL;
    assoc->init->bend_fetch = NULL;
    
    assoc->init->peer_name =
	odr_strdup (assoc->encode, cs_addrstr(assoc->client_link));
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
	strcat(options, " concurop");
    }
    if (ODR_MASK_GET(req->options, Z_Options_sort) && assoc->init->bend_sort)
    {
	ODR_MASK_SET(resp->options, Z_Options_sort);
	strcat(options, " sort");
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

    resp->implementationName = "GFS";

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

static void set_addinfo (Z_DefaultDiagFormat *dr, char *addinfo)
{
#if ASN_COMPILED
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = addinfo ? addinfo : "";
#else
    dr->which = Z_DiagForm_v2AddInfo;
    dr->addinfo = addinfo ? addinfo : "";
#endif
}

/*
 * nonsurrogate diagnostic record.
 */
static Z_Records *diagrec(association *assoc, int error, char *addinfo)
{
    int oid[OID_SIZE];
    Z_Records *rec = (Z_Records *)
	odr_malloc (assoc->encode, sizeof(*rec));
    oident bib1;
    int *err = (int *)
	odr_malloc (assoc->encode, sizeof(*err));
    Z_DiagRec *drec = (Z_DiagRec *)
	odr_malloc (assoc->encode, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
	odr_malloc (assoc->encode, sizeof(*dr));

    bib1.proto = assoc->proto;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    yaz_log(LOG_DEBUG, "Diagnostic: %d -- %s", error, addinfo ? addinfo :
	"NULL");
    *err = error;
    rec->which = Z_Records_NSD;
#if ASN_COMPILED
    rec->u.nonSurrogateDiagnostic = dr;
#else
    rec->u.nonSurrogateDiagnostic = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
#endif
    dr->diagnosticSetId =
	odr_oiddup (assoc->encode, oid_ent_to_oid(&bib1, oid));
    dr->condition = err;
    set_addinfo (dr, addinfo);
    return rec;
}

/*
 * surrogate diagnostic.
 */
static Z_NamePlusRecord *surrogatediagrec(association *assoc, char *dbname,
					  int error, char *addinfo)
{
    int oid[OID_SIZE];
    Z_NamePlusRecord *rec = (Z_NamePlusRecord *)
	odr_malloc (assoc->encode, sizeof(*rec));
    int *err = (int *)odr_malloc (assoc->encode, sizeof(*err));
    oident bib1;
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (assoc->encode, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
	odr_malloc (assoc->encode, sizeof(*dr));
    
    bib1.proto = assoc->proto;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    yaz_log(LOG_DEBUG, "SurrogateDiagnotic: %d -- %s", error, addinfo);
    *err = error;
    rec->databaseName = dbname;
    rec->which = Z_NamePlusRecord_surrogateDiagnostic;
    rec->u.surrogateDiagnostic = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
    dr->diagnosticSetId = odr_oiddup (assoc->encode,
                                      oid_ent_to_oid(&bib1, oid));
    dr->condition = err;
    set_addinfo (dr, addinfo);

    return rec;
}

/*
 * multiple nonsurrogate diagnostics.
 */
static Z_DiagRecs *diagrecs(association *assoc, int error, char *addinfo)
{
    int oid[OID_SIZE];
    Z_DiagRecs *recs = (Z_DiagRecs *)odr_malloc (assoc->encode, sizeof(*recs));
    int *err = (int *)odr_malloc (assoc->encode, sizeof(*err));
    oident bib1;
    Z_DiagRec **recp = (Z_DiagRec **)odr_malloc (assoc->encode, sizeof(*recp));
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (assoc->encode, sizeof(*drec));
    Z_DefaultDiagFormat *rec = (Z_DefaultDiagFormat *)odr_malloc (assoc->encode, sizeof(*rec));

    yaz_log(LOG_DEBUG, "DiagRecs: %d -- %s", error, addinfo ? addinfo : "");
    bib1.proto = assoc->proto;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    *err = error;
    recs->num_diagRecs = 1;
    recs->diagRecs = recp;
    recp[0] = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = rec;

    rec->diagnosticSetId = odr_oiddup (assoc->encode,
                                      oid_ent_to_oid(&bib1, oid));
    rec->condition = err;

#ifdef ASN_COMPILED
    rec->which = Z_DefaultDiagFormat_v2Addinfo;
    rec->u.v2Addinfo = addinfo ? addinfo : "";
#else
    rec->which = Z_DiagForm_v2AddInfo;
    rec->addinfo = addinfo ? addinfo : "";
#endif
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

    yaz_log(LOG_LOG, "Request to pack %d+%d", start, toget);
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
		*pres = Z_PRES_FAILURE;
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
	    	yaz_log(LOG_DEBUG, "Record > maxrcdsz");
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
    Z_SearchRequest *req = reqb->request->u.searchRequest;
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
    switch (req->query->which)
    {
    case Z_Query_type_1: case Z_Query_type_101:
	log_rpn_query (req->query->u.type_1);
    }
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
	(assoc->init->bend_search)(assoc->backend, bsrr);
	if (!bsrr->request)
	    return 0;
    }
#if 0
    else
    {
	bend_searchrequest bsrq;
	bend_searchresult *bsrt;

	bsrq.setname = req->resultSetName;
	bsrq.replace_set = *req->replaceIndicator;
	bsrq.num_bases = req->num_databaseNames;
	bsrq.basenames = req->databaseNames;
	bsrq.query = req->query;
        bsrq.referenceId = req->referenceId;
	bsrq.stream = assoc->encode;
	bsrq.decode = assoc->decode;
	bsrq.print = assoc->print;
	if (!(bsrt = bend_search (assoc->backend, &bsrq, fd)))
	    return 0;
	bsrr->hits = bsrt->hits;
	bsrr->errcode = bsrt->errcode;
	bsrr->errstring = bsrt->errstring;
    }
#endif
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
    Z_SearchRequest *req = reqb->request->u.searchRequest;
    Z_APDU *apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));
    Z_SearchResponse *resp = (Z_SearchResponse *)
	odr_malloc (assoc->encode, sizeof(*resp));
    int *nulint = (int *)odr_malloc (assoc->encode, sizeof(*nulint));
    bool_t *sr = (bool_t *)odr_malloc (assoc->encode, sizeof(*sr));
    int *next = (int *)odr_malloc (assoc->encode, sizeof(*next));
    int *none = (int *)odr_malloc (assoc->encode, sizeof(*none));

    *nulint = 0;
    *sr = 1;
    *next = 0;
    *none = Z_RES_NONE;

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
    	int *toget = (int *)odr_malloc (assoc->encode, sizeof(*toget));
        int *presst = (int *)odr_malloc (assoc->encode, sizeof(*presst));
	Z_RecordComposition comp, *compp = 0;

        *toget = 0;
        *presst = 0;
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

	    if (!(prefformat = oid_getentbyoid(req->preferredRecordSyntax)) ||
	    	prefformat->oclass != CLASS_RECSYN)
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
    Z_PresentRequest *req = reqb->request->u.presentRequest;
    oident *prefformat;
    oid_value form;
    Z_APDU *apdu;
    Z_PresentResponse *resp;
    int *presst;
    int *next;
    int *num;

    yaz_log(LOG_LOG, "Got PresentRequest.");

    if (!(prefformat = oid_getentbyoid(req->preferredRecordSyntax)) ||
	prefformat->oclass != CLASS_RECSYN)
	form = VAL_NONE;
    else
	form = prefformat->value;
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
    }
    apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));
    resp = (Z_PresentResponse *)odr_malloc (assoc->encode, sizeof(*resp));
    presst = (int *)odr_malloc (assoc->encode, sizeof(*presst));
    next = (int *)odr_malloc (assoc->encode, sizeof(*next));
    num = (int *)odr_malloc (assoc->encode, sizeof(*num));
    *presst = 0;
    *next = 0;
    *num = *req->numberOfRecordsRequested;
    
    apdu->which = Z_APDU_presentResponse;
    apdu->u.presentResponse = resp;
    resp->referenceId = req->referenceId;
    resp->otherInfo = 0;
    
    resp->records =
	pack_records(assoc, req->resultSetId, *req->resultSetStartPoint,
		     num, req->recordComposition, next, presst, form,
                     req->referenceId, req->preferredRecordSyntax);
    if (!resp->records)
	return 0;
    resp->numberOfRecordsReturned = num;
    resp->presentStatus = presst;
    resp->nextResultSetPosition = next;
    
    return apdu;
}

#if 0
static int bend_default_scan (void *handle, bend_scan_rr *rr)
{
    bend_scanrequest srq;
    bend_scanresult *srs;

    srq.num_bases = rr->num_bases;
    srq.basenames = rr->basenames;
    srq.attributeset = rr->attributeset;
    srq.referenceId = rr->referenceId;
    srq.term = rr->term;
    srq.term_position = rr->term_position;
    srq.num_entries = rr->num_entries;
    srq.stream = rr->stream;
    srq.print = rr->print;
    
    srs = bend_scan(handle, &srq, 0);

    rr->term_position = srs->term_position;
    rr->num_entries = srs->num_entries;
    rr->entries = srs->entries;
    rr->status = srs->status;
    rr->errcode = srs->errcode;
    rr->errstring = srs->errstring;
    return 0;
}
#endif

/*
 * Scan was implemented rather in a hurry, and with support for only the basic
 * elements of the service in the backend API. Suggestions are welcome.
 */
static Z_APDU *process_scanRequest(association *assoc, request *reqb, int *fd)
{
    Z_ScanRequest *req = reqb->request->u.scanRequest;
    Z_APDU *apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));
    Z_ScanResponse *res = (Z_ScanResponse *)
	odr_malloc (assoc->encode, sizeof(*res));
    int *scanStatus = (int *)
	odr_malloc (assoc->encode, sizeof(*scanStatus));
    int *numberOfEntriesReturned = (int *)
	odr_malloc (assoc->encode, sizeof(*numberOfEntriesReturned));
    Z_ListEntries *ents = (Z_ListEntries *)
	odr_malloc (assoc->encode, sizeof(*ents));
    Z_DiagRecs *diagrecs_p = NULL;
    oident *attent;
    oident *attset;

    yaz_log(LOG_LOG, "Got ScanRequest");
    *scanStatus = Z_Scan_failure;
    *numberOfEntriesReturned = 0;

    apdu->which = Z_APDU_scanResponse;
    apdu->u.scanResponse = res;
    res->referenceId = req->referenceId;
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

    if (req->attributeSet && (!(attent = oid_getentbyoid(req->attributeSet)) ||
			      attent->oclass != CLASS_ATTSET
			      || attent->value != VAL_BIB1))
	diagrecs_p = diagrecs(assoc, 121, 0);
    else if (req->stepSize && *req->stepSize > 0)
    	diagrecs_p = diagrecs(assoc, 205, 0);
    else
    {
	bend_scan_rr *bsrr = (bend_scan_rr *)
	    odr_malloc (assoc->encode, sizeof(*bsrr));
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
	if (!(attset = oid_getentbyoid(req->attributeSet)) ||
	    attset->oclass != CLASS_RECSYN)
	    bsrr->attributeset = VAL_NONE;
	else
	    bsrr->attributeset = attset->value;
	log_scan_term (req->termListAndStartPoint, attset->value);
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
    Z_SortRequest *req = reqb->request->u.sortRequest;
    Z_SortResponse *res = (Z_SortResponse *)
	odr_malloc (assoc->encode, sizeof(*res));
    bend_sort_rr *bsrr = (bend_sort_rr *)
	odr_malloc (assoc->encode, sizeof(*bsrr));

    Z_APDU *apdu = (Z_APDU *)odr_malloc (assoc->encode, sizeof(*apdu));

    yaz_log(LOG_LOG, "Got SortRequest.");

#ifdef ASN_COMPILED
    bsrr->num_input_setnames = req->num_inputResultSetNames;
    bsrr->input_setnames = req->inputResultSetNames;
#else
    bsrr->num_input_setnames = req->inputResultSetNames->num_strings;
    bsrr->input_setnames = req->inputResultSetNames->strings;
#endif
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
    res->sortStatus = (int *)
	odr_malloc (assoc->encode, sizeof(*res->sortStatus));
    *res->sortStatus = bsrr->sort_status;
    res->resultSetStatus = 0;
    if (bsrr->errcode)
    {
	Z_DiagRecs *dr = diagrecs (assoc, bsrr->errcode, bsrr->errstring);
#ifdef ASN_COMPILED
	res->diagnostics = dr->diagRecs;
	res->num_diagnostics = dr->num_diagRecs;
#else
	res->diagnostics = dr;
#endif
    }
    else
    {
#ifdef ASN_COMPILED
	res->num_diagnostics = 0;
#endif
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
    Z_DeleteResultSetRequest *req = reqb->request->u.deleteResultSetRequest;
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
	bdrr->statuses = odr_malloc(assoc->encode, sizeof(*bdrr->statuses) *
				    bdrr->num_setnames);
	for (i = 0; i < bdrr->num_setnames; i++)
	    bdrr->statuses[i] = 0;
    }
    (*assoc->init->bend_delete)(assoc->backend, bdrr);
    
    res->referenceId = req->referenceId;

    res->deleteOperationStatus = (int *)
	odr_malloc (assoc->encode, sizeof(*res->deleteOperationStatus));
    *res->deleteOperationStatus = bdrr->delete_status;

    res->deleteListStatuses = 0;
    if (bdrr->num_setnames > 0)
    {
	int i;
	res->deleteListStatuses = odr_malloc(assoc->encode,
					     sizeof(*res->deleteListStatuses));
	res->deleteListStatuses->num = bdrr->num_setnames;
	res->deleteListStatuses->elements =
	    odr_malloc (assoc->encode,
			sizeof(*res->deleteListStatuses->elements) *
			bdrr->num_setnames);
	for (i = 0; i<bdrr->num_setnames; i++)
	{
	    res->deleteListStatuses->elements[i] =
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
    Z_Close *req = reqb->request->u.close;
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
    process_response (a, req, res);
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
    r = process_request (a, req, &msg);
    if (r < 0)
	logf (LOG_WARN, "%s", msg);
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
    bend_segment_rr request;

    request.segment = reqb->request->u.segmentRequest;
    request.stream = assoc->encode;
    request.decode = assoc->decode;
    request.print = assoc->print;
    request.association = assoc;
    
    (*assoc->init->bend_segment)(assoc->backend, &request);

    return 0;
}

static Z_APDU *process_ESRequest(association *assoc, request *reqb, int *fd)
{
    bend_esrequest_rr esrequest;

    Z_ExtendedServicesRequest *req = reqb->request->u.extendedServicesRequest;
    Z_APDU *apdu = zget_APDU(assoc->encode, Z_APDU_extendedServicesResponse);

    Z_ExtendedServicesResponse *resp = apdu->u.extendedServicesResponse;

    yaz_log(LOG_DEBUG,"inside Process esRequest");

    esrequest.esr = reqb->request->u.extendedServicesRequest;
    esrequest.stream = assoc->encode;
    esrequest.decode = assoc->decode;
    esrequest.print = assoc->print;
    esrequest.errcode = 0;
    esrequest.errstring = NULL;
    esrequest.request = reqb;
    esrequest.association = assoc;
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

    yaz_log(LOG_DEBUG,"Send the result apdu");
    return apdu;
}
