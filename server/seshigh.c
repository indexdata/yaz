/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: seshigh.c,v $
 * Revision 1.21  1995-05-02 08:53:19  quinn
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <comstack.h>
#include <eventl.h>
#include <session.h>
#include <proto.h>
#include <oid.h>
#include <log.h>

#include <backend.h>

#define MAXRECORDSIZE 1024*1024*5 /* should be configurable, of course */

static int process_apdu(IOCHAN chan);
static int process_initRequest(IOCHAN client, Z_InitRequest *req);
static int process_searchRequest(IOCHAN client, Z_SearchRequest *req);
static int process_presentRequest(IOCHAN client, Z_PresentRequest *req);
static int process_scanRequest(IOCHAN client, Z_ScanRequest *req);

extern int dynamic;
extern char *apdufile;

static FILE *apduf = 0; /* for use in static mode */

/*
 * Create a new association-handle.
 *  channel  : iochannel for the current line.
 *  link     : communications channel.
 * Returns: 0 or a new association handle.
 */
association *create_association(IOCHAN channel, COMSTACK link)
{
    association *new;

    if (!(new = malloc(sizeof(*new))))
    	return 0;
    new->client_chan = channel;
    new->client_link = link;
    if (!(new->decode = odr_createmem(ODR_DECODE)) ||
    	!(new->encode = odr_createmem(ODR_ENCODE)))
	return 0;
    if (apdufile)
    {
    	char filename[256];
	FILE *f;

	if (!(new->print = odr_createmem(ODR_PRINT)))
	    return 0;
	if (*apdufile)
	{
	    strcpy(filename, apdufile);
	    if (!dynamic)
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
    if (cs_getproto(link) == CS_Z3950)
	new->proto = PROTO_Z3950;
    else
	new->proto = PROTO_SR;
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
    free(h);
}

/*
 * process events on the association. Called when the event loop detects an
 * event.
 *  h : the I/O channel that has an outstanding event.
 *  event : the current outstanding event.
 */
void ir_session(IOCHAN h, int event)
{
    int res;
    association *assoc = iochan_getdata(h);
    COMSTACK conn = assoc->client_link;

    if (event == EVENT_INPUT)
    {
	assert(assoc && conn);
	res = cs_get(conn, &assoc->input_buffer, &assoc->input_buffer_len);
	switch (res)
	{
	    case 0: case -1: /* connection closed by peer */
	    	logf(LOG_LOG, "Connection closed by client");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
		return;
	    case 1:  /* incomplete read */
		return;
	    default: /* data! */
		assoc->input_apdu_len = res;
		if (process_apdu(h) < 0)
		{
		    cs_close(conn);
		    destroy_association(assoc);
		    iochan_destroy(h);
		}
		else if (cs_more(conn)) /* arrange to be called again */
		    iochan_setevent(h, EVENT_INPUT);
	}
    }
    else if (event == EVENT_OUTPUT)
    {
    	switch (res = cs_put(conn, assoc->encode_buffer, assoc->encoded_len))
	{
	    case -1:
	    	logf(LOG_LOG, "Connection closed by client");
		cs_close(conn);
		destroy_association(assoc);
		iochan_destroy(h);
	    case 0: /* all sent */
	    	iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset */
		break;
	    case 1: /* partial send */
	    	break; /* we'll get called again */
	}
    }
    else if (event == EVENT_EXCEPT)
    {
	logf(LOG_LOG, "Exception on line");
	cs_close(conn);
	destroy_association(assoc);
	iochan_destroy(h);
    }
}

/*
 * Process the current outstanding APDU.
 */
static int process_apdu(IOCHAN chan)
{
    Z_APDU *apdu;
    int res;
    association *assoc = iochan_getdata(chan);

    odr_setbuf(assoc->decode, assoc->input_buffer, assoc->input_apdu_len);
    if (!z_APDU(assoc->decode, &apdu, 0))
    {
    	logf(LOG_WARN, "ODR error: %s",
	    odr_errlist[odr_geterror(assoc->decode)]);
	return -1;
    }
    if (assoc->print && !z_APDU(assoc->print, &apdu, 0))
    {
    	logf(LOG_WARN, "ODR print error: %s", 
	    odr_errlist[odr_geterror(assoc->print)]);
	return -1;
    }
    switch (apdu->which)
    {
    	case Z_APDU_initRequest:
	    res = process_initRequest(chan, apdu->u.initRequest); break;
	case Z_APDU_searchRequest:
	    res = process_searchRequest(chan, apdu->u.searchRequest); break;
	case Z_APDU_presentRequest:
	    res = process_presentRequest(chan, apdu->u.presentRequest); break;
	case Z_APDU_scanRequest:
	    res = process_scanRequest(chan, apdu->u.scanRequest); break;
	default:
	    logf(LOG_WARN, "Bad APDU");
	    return -1;
    }
    odr_reset(assoc->decode); /* release incoming APDU */
    odr_reset(assoc->encode); /* release stuff alloced before encoding */
    return res;
}

static int process_initRequest(IOCHAN client, Z_InitRequest *req)
{
    Z_APDU apdu, *apdup;
    Z_InitResponse resp;
    bool_t result = 1;
    association *assoc = iochan_getdata(client);
    bend_initrequest binitreq;
    bend_initresult *binitres;
    Odr_bitmask options, protocolVersion;

    logf(LOG_LOG, "Got initRequest");
    if (req->implementationId)
    	logf(LOG_LOG, "Id:        %s", req->implementationId);
    if (req->implementationName)
    	logf(LOG_LOG, "Name:      %s", req->implementationName);
    if (req->implementationVersion)
    	logf(LOG_LOG, "Version:   %s", req->implementationVersion);

    binitreq.configname = "default-config";
    if (!(binitres = bend_init(&binitreq)) || binitres->errcode)
    {
    	logf(LOG_WARN, "Negative response from backend");
    	return -1;
    }

    assoc->backend = binitres->handle;
    apdup = &apdu;
    apdu.which = Z_APDU_initResponse;
    apdu.u.initResponse = &resp;
    resp.referenceId = req->referenceId;
    ODR_MASK_ZERO(&options);
    if (ODR_MASK_GET(req->options, Z_Options_search))
    	ODR_MASK_SET(&options, Z_Options_search);
    if (ODR_MASK_GET(req->options, Z_Options_present))
    	ODR_MASK_SET(&options, Z_Options_present);
#if 0
    if (ODR_MASK_GET(req->options, Z_Options_delSet))
    	ODR_MASK_SET(&options, Z_Options_delSet);
#endif
    if (ODR_MASK_GET(req->options, Z_Options_namedResultSets))
    	ODR_MASK_SET(&options, Z_Options_namedResultSets);
    if (ODR_MASK_GET(req->options, Z_Options_scan))
    	ODR_MASK_SET(&options, Z_Options_scan);
    resp.options = &options;
    ODR_MASK_ZERO(&protocolVersion);
    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_1))
    	ODR_MASK_SET(&protocolVersion, Z_ProtocolVersion_1);
    if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_2))
    	ODR_MASK_SET(&protocolVersion, Z_ProtocolVersion_2);
    resp.protocolVersion = &protocolVersion;
    assoc->maximumRecordSize = *req->maximumRecordSize;
    if (assoc->maximumRecordSize > MAXRECORDSIZE)
    	assoc->maximumRecordSize = MAXRECORDSIZE;
    assoc->preferredMessageSize = *req->preferredMessageSize;
    if (assoc->preferredMessageSize > assoc->maximumRecordSize)
    	assoc->preferredMessageSize = assoc->maximumRecordSize;
    resp.preferredMessageSize = &assoc->preferredMessageSize;
    resp.maximumRecordSize = &assoc->maximumRecordSize;
    resp.result = &result;
    resp.implementationId = "YAZ";
#if 0
    resp.implementationName = "Index Data/YAZ Generic Frontend Server";
#endif
    resp.implementationName = "High Level API Server aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbdddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddduuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    resp.implementationVersion = "$Revision: 1.21 $";
    resp.userInformationField = 0;
    if (!z_APDU(assoc->encode, &apdup, 0))
    {
	logf(LOG_FATAL, "ODR error encoding initres: %s",
	    odr_errlist[odr_geterror(assoc->encode)]);
	return -1;
    }
    assoc->encode_buffer = odr_getbuf(assoc->encode, &assoc->encoded_len);
    odr_reset(assoc->encode);
    iochan_setflags(client, EVENT_OUTPUT | EVENT_EXCEPT);
    return 0;
}

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
		    *pres = Z_PRES_PARTIAL_2;
		    break;
		}
	    }
	    else /* too big entirely */
	    {
	    	logf(LOG_DEBUG, "Record > maxrcdsz");
		reclist.records[reclist.num_records] =
		    surrogatediagrec(a->proto, fres->basename, 17, 0);
		reclist.num_records++;
		*pres = Z_PRES_PARTIAL_2;
		break;
	    }
	}
	if (!(thisrec = odr_malloc(a->encode, sizeof(*thisrec))))
	    return 0;
	if (!(thisrec->databaseName = odr_malloc(a->encode,
	    strlen(fres->basename) + 1)))
	    return 0;
	strcpy(thisrec->databaseName, fres->basename);
	thisrec->which = Z_NamePlusRecord_databaseRecord;
	if (!(thisrec->u.databaseRecord = thisext =  odr_malloc(a->encode,
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
	(*num)++;
	*next = fres->last_in_set ? 0 : recno + 1;
    }
    return &records;
}

static int process_searchRequest(IOCHAN client, Z_SearchRequest *req)
{
    Z_APDU apdu, *apdup;
    Z_SearchResponse resp;
    association *assoc = iochan_getdata(client);
    int nulint = 0;
    bool_t sr = 1;
    bend_searchrequest bsrq;
    bend_searchresult *bsrt;
    oident *oent;
    int next = 0;
    static int none = Z_RES_NONE;

    logf(LOG_LOG, "Got SearchRequest.");
    apdup = &apdu;
    apdu.which = Z_APDU_searchResponse;
    apdu.u.searchResponse = &resp;
    resp.referenceId = req->referenceId;

    resp.records = 0;
    if (req->query->which == Z_Query_type_1)
    {
    	Z_RPNQuery *q = req->query->u.type_1;

    	if (!(oent = oid_getentbyoid(q->attributeSetId)) ||
	    oent->class != CLASS_ATTSET ||
	    oent->value != VAL_BIB1)
	{
	    resp.records = diagrec(assoc->proto, 121, 0);
	    resp.resultCount = &nulint;
	    resp.numberOfRecordsReturned = &nulint;
	    resp.nextResultSetPosition = &nulint;
	    resp.searchStatus = &none;
	    resp.resultSetStatus = 0;
	    resp.presentStatus = 0;
	}
    }
    if (!resp.records)
    {
    	int toget;
	Z_ElementSetNames *setnames;
	int presst = 0;

	bsrq.setname = req->resultSetName;
	bsrq.replace_set = *req->replaceIndicator;
	bsrq.num_bases = req->num_databaseNames;
	bsrq.basenames = req->databaseNames;
	bsrq.query = req->query;

	if (!(bsrt = bend_search(assoc->backend, &bsrq, 0)))
	    return -1;
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
		    return -1;
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
    }

    if (!z_APDU(assoc->encode, &apdup, 0))
    {
	logf(LOG_FATAL, "ODR error encoding searchres: %s",
	    odr_errlist[odr_geterror(assoc->encode)]);
	return -1;
    }
    assoc->encode_buffer = odr_getbuf(assoc->encode, &assoc->encoded_len);
    odr_reset(assoc->encode);
    iochan_setflags(client, EVENT_OUTPUT | EVENT_EXCEPT);
    return 0;
}

static int process_presentRequest(IOCHAN client, Z_PresentRequest *req)
{
    Z_APDU apdu, *apdup;
    Z_PresentResponse resp;
    association *assoc = iochan_getdata(client);
    int presst, next, num;

    logf(LOG_LOG, "Got PresentRequest.");
    apdup = &apdu;
    apdu.which = Z_APDU_presentResponse;
    apdu.u.presentResponse = &resp;
    resp.referenceId = req->referenceId;

    num = *req->numberOfRecordsRequested;
    resp.records = pack_records(assoc, req->resultSetId,
	*req->resultSetStartPoint, &num, req->elementSetNames, &next, &presst);
    if (!resp.records)
    	return -1;
    resp.numberOfRecordsReturned = &num;
    resp.presentStatus = &presst;
    resp.nextResultSetPosition = &next;

    if (!z_APDU(assoc->encode, &apdup, 0))
    {
	logf(LOG_FATAL, "ODR error encoding initres: %s",
	    odr_errlist[odr_geterror(assoc->encode)]);
	return -1;
    }
    assoc->encode_buffer = odr_getbuf(assoc->encode, &assoc->encoded_len);
    odr_reset(assoc->encode);
    iochan_setflags(client, EVENT_OUTPUT | EVENT_EXCEPT);
    return 0;
}

static int process_scanRequest(IOCHAN client, Z_ScanRequest *req)
{
    association *assoc = iochan_getdata(client);
    Z_APDU apdu, *apdup;
    Z_ScanResponse res;
    bend_scanrequest srq;
    bend_scanresult *srs;
    int scanStatus = Z_Scan_failure;
    int numberOfEntriesReturned = 0;
    oident *attent;
    Z_ListEntries ents;
#define SCAN_MAX_ENTRIES 200
    Z_Entry *tab[SCAN_MAX_ENTRIES];

    apdup = &apdu;
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
    if (!z_APDU(assoc->encode, &apdup, 0))
    {
	logf(LOG_FATAL, "ODR error encoding scanres: %s",
	    odr_errlist[odr_geterror(assoc->encode)]);
	return -1;
    }
#if 0
    odr_reset(assoc->print);
    if (!z_APDU(assoc->print, &apdup, 0))
    {
	logf(LOG_FATAL, "ODR error priniting scanres: %s",
	    odr_errlist[odr_geterror(assoc->encode)]);
	return -1;
    }
#endif
    assoc->encode_buffer = odr_getbuf(assoc->encode, &assoc->encoded_len);
    odr_reset(assoc->encode);
    iochan_setflags(client, EVENT_OUTPUT | EVENT_EXCEPT);
    return 0;
}
