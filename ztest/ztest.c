/*
 * Copyright (c) 1995-2000, Index Data.
 * See the file LICENSE for details.
 *
 * NT Service interface by
 *    Chas Woodfield, Fretwell Downing Datasystems.
 *
 * $Log: ztest.c,v $
 * Revision 1.35  2000-11-23 10:58:33  adam
 * SSL comstack support. Separate POSIX thread support library.
 *
 * Revision 1.34  2000/09/04 08:58:15  adam
 * Added prefix yaz_ for most logging utility functions.
 *
 * Revision 1.33  2000/08/10 08:41:26  adam
 * Fixes for ILL.
 *
 * Revision 1.32  2000/04/05 07:39:55  adam
 * Added shared library support (libtool).
 *
 * Revision 1.31  2000/01/31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.30  2000/01/13 23:05:50  adam
 * Fixed tagging for member requester-CHECKED-IN in ILL ASN.1 spec.
 *
 * Revision 1.29  2000/01/12 14:36:07  adam
 * Added printing stream (ODR) for backend functions.
 *
 * Revision 1.28  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 * Revision 1.27  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.26  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.25  1999/06/01 14:29:12  adam
 * Work on Extended Services.
 *
 * Revision 1.24  1999/05/27 13:07:54  adam
 * Fix.
 *
 * Revision 1.23  1999/05/27 13:02:20  adam
 * Assigned OID for old DB Update (VAL_DBUPDATE0).
 *
 * Revision 1.22  1999/05/26 13:49:12  adam
 * DB Update implemented in client (very basic).
 *
 * Revision 1.21  1998/12/15 12:45:42  adam
 * Minor change.
 *
 * Revision 1.20  1998/12/14 14:48:05  adam
 * Fixed memory leak - happened when fetching MARC records.
 *
 * Revision 1.19  1998/10/20 15:16:22  adam
 * Minor change to prevent warning.
 *
 * Revision 1.18  1998/10/20 15:13:45  adam
 * Minor fix regarding output for Item Order.
 *
 * Revision 1.17  1998/10/18 22:33:35  quinn
 * Added diagnostic dump of Item Order Eservice.
 *
 * Revision 1.16  1998/10/15 08:26:23  adam
 * Added type cast to make C++ happy.
 *
 * Revision 1.15  1998/10/13 20:05:57  adam
 * Minor change.
 *
 * Revision 1.14  1998/10/13 16:12:25  adam
 * Added support for Surrogate Diagnostics for Scan Term entries.
 *
 * Revision 1.13  1998/08/19 16:10:09  adam
 * Changed som member names of DeleteResultSetRequest/Response.
 *
 * Revision 1.12  1998/07/20 12:38:44  adam
 * Implemented delete result set service to server API.
 *
 * Revision 1.11  1998/06/09 13:55:08  adam
 * Minor changes.
 *
 * Revision 1.10  1998/05/27 16:55:54  adam
 * Minor changes.
 *
 * Revision 1.9  1998/03/31 11:07:45  adam
 * Furhter work on UNIverse resource report.
 * Added Extended Services handling in frontend server.
 *
 * Revision 1.8  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.7  1998/02/10 11:03:57  adam
 * Added support for extended handlers in backend server interface.
 *
 * Revision 1.6  1998/01/29 13:16:02  adam
 * Added dummy sort in test server.
 *
 * Revision 1.5  1997/11/07 13:31:58  adam
 * Added NT Service name part of statserv_options_block. Moved NT
 * service utility to server library.
 *
 * Revision 1.4  1997/09/17 12:10:43  adam
 * YAZ version 1.4.
 *
 * Revision 1.3  1997/09/09 10:10:20  adam
 * Another MSV5.0 port. Changed projects to include proper
 * library/include paths.
 * Server starts server in test-mode when no options are given.
 *
 * Revision 1.2  1997/09/04 13:50:31  adam
 * Bug fix in ztest.
 *
 */

/*
 * Demonstration of simple server
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <yaz/backend.h>
#include <yaz/log.h>

#ifdef ASN_COMPILED
#include <yaz/ill.h>
#endif

Z_GenericRecord *read_grs1(FILE *f, ODR o);

int ztest_search (void *handle, bend_search_rr *rr);
int ztest_sort (void *handle, bend_sort_rr *rr);
int ztest_present (void *handle, bend_present_rr *rr);
int ztest_esrequest (void *handle, bend_esrequest_rr *rr);
int ztest_delete (void *handle, bend_delete_rr *rr);

int ztest_search (void *handle, bend_search_rr *rr)
{
    rr->hits = rand() % 22;
    return 0;
}

int ztest_present (void *handle, bend_present_rr *rr)
{
    return 0;
}

int ztest_esrequest (void *handle, bend_esrequest_rr *rr)
{
    yaz_log(LOG_LOG, "function: %d", *rr->esr->function);
    if (rr->esr->packageName)
    	yaz_log(LOG_LOG, "packagename: %s", rr->esr->packageName);
    yaz_log(LOG_LOG, "Waitaction: %d", *rr->esr->waitAction);

    if (!rr->esr->taskSpecificParameters)
    {
        yaz_log (LOG_WARN, "No task specific parameters");
    }
    else if (rr->esr->taskSpecificParameters->which == Z_External_itemOrder)
    {
    	Z_ItemOrder *it = rr->esr->taskSpecificParameters->u.itemOrder;
	yaz_log (LOG_LOG, "Received ItemOrder");
	switch (it->which)
	{
#ifdef ASN_COMPILED
	case Z_IOItemOrder_esRequest:
#else
	case Z_ItemOrder_esRequest:
#endif
	{
	    Z_IORequest *ir = it->u.esRequest;
	    Z_IOOriginPartToKeep *k = ir->toKeep;
	    Z_IOOriginPartNotToKeep *n = ir->notToKeep;
	    
	    if (k && k->contact)
	    {
	        if (k->contact->name)
		    yaz_log(LOG_LOG, "contact name %s", k->contact->name);
		if (k->contact->phone)
		    yaz_log(LOG_LOG, "contact phone %s", k->contact->phone);
		if (k->contact->email)
		    yaz_log(LOG_LOG, "contact email %s", k->contact->email);
	    }
	    if (k->addlBilling)
	    {
	        yaz_log(LOG_LOG, "Billing info (not shown)");
	    }
	    
	    if (n->resultSetItem)
	    {
	        yaz_log(LOG_LOG, "resultsetItem");
		yaz_log(LOG_LOG, "setId: %s", n->resultSetItem->resultSetId);
		yaz_log(LOG_LOG, "item: %d", *n->resultSetItem->item);
	    }
#ifdef ASN_COMPILED
	    if (n->itemRequest)
	    {
		Z_External *r = (Z_External*) n->itemRequest;
		ILL_ItemRequest *item_req = 0;
		ILL_Request *ill_req = 0;
		if (r->direct_reference)
		{
		    oident *ent = oid_getentbyoid(r->direct_reference);
		    if (ent)
			yaz_log(LOG_LOG, "OID %s", ent->desc);
		    if (ent && ent->value == VAL_ISO_ILL_1)
		    {
			yaz_log (LOG_LOG, "Decode ItemRequest begin");
			if (r->which == ODR_EXTERNAL_single)
			{
			    odr_setbuf(rr->decode,
				       r->u.single_ASN1_type->buf,
				       r->u.single_ASN1_type->len, 0);
			    
			    if (!ill_ItemRequest (rr->decode, &item_req, 0, 0))
			    {
				yaz_log (LOG_LOG,
                                    "Couldn't decode ItemRequest %s near %d",
                                       odr_errmsg(odr_geterror(rr->decode)),
                                       odr_offset(rr->decode));
#if 0
                                yaz_log(LOG_LOG, "PDU dump:");
                                odr_dumpBER(yaz_log_file(),
                                     r->u.single_ASN1_type->buf,
                                     r->u.single_ASN1_type->len);
#endif
                            }
			    else
			        yaz_log(LOG_LOG, "Decode ItemRequest OK");
			    if (rr->print)
			    {
				ill_ItemRequest (rr->print, &item_req, 0,
                                    "ItemRequest");
				odr_reset (rr->print);
 			    }
			}
			if (!item_req && r->which == ODR_EXTERNAL_single)
			{
			    yaz_log (LOG_LOG, "Decode ILLRequest begin");
			    odr_setbuf(rr->decode,
				       r->u.single_ASN1_type->buf,
				       r->u.single_ASN1_type->len, 0);
			    
			    if (!ill_Request (rr->decode, &ill_req, 0, 0))
			    {
				yaz_log (LOG_LOG,
                                    "Couldn't decode ILLRequest %s near %d",
                                       odr_errmsg(odr_geterror(rr->decode)),
                                       odr_offset(rr->decode));
                                yaz_log(LOG_LOG, "PDU dump:");
                                odr_dumpBER(yaz_log_file(),
                                     r->u.single_ASN1_type->buf,
                                     r->u.single_ASN1_type->len);
                            }
			    else
			        yaz_log(LOG_LOG, "Decode ILLRequest OK");
			    if (rr->print)
                            {
				ill_Request (rr->print, &ill_req, 0,
                                    "ILLRequest");
				odr_reset (rr->print);
			    }
			}
		    }
		}
		if (item_req)
		{
		    yaz_log (LOG_LOG, "ILL protocol version = %d",
			     *item_req->protocol_version_num);
		}
	    }
#endif
	}
	break;
	}
    }
    else if (rr->esr->taskSpecificParameters->which == Z_External_update)
    {
    	Z_IUUpdate *up = rr->esr->taskSpecificParameters->u.update;
	yaz_log (LOG_LOG, "Received DB Update");
	if (up->which == Z_IUUpdate_esRequest)
	{
	    Z_IUUpdateEsRequest *esRequest = up->u.esRequest;
	    Z_IUOriginPartToKeep *toKeep = esRequest->toKeep;
	    Z_IUSuppliedRecords *notToKeep = esRequest->notToKeep;
	    
	    yaz_log (LOG_LOG, "action");
	    if (toKeep->action)
	    {
		switch (*toKeep->action)
		{
		case Z_IUOriginPartToKeep_recordInsert:
		    yaz_log (LOG_LOG, " recordInsert");
		    break;
		case Z_IUOriginPartToKeep_recordReplace:
		    yaz_log (LOG_LOG, " recordUpdate");
		    break;
		case Z_IUOriginPartToKeep_recordDelete:
		    yaz_log (LOG_LOG, " recordDelete");
		    break;
		case Z_IUOriginPartToKeep_elementUpdate:
		    yaz_log (LOG_LOG, " elementUpdate");
		    break;
		case Z_IUOriginPartToKeep_specialUpdate:
		    yaz_log (LOG_LOG, " specialUpdate");
		    break;
		default:
		    yaz_log (LOG_LOG, " unknown (%d)", *toKeep->action);
		}
	    }
	    if (toKeep->databaseName)
	    {
		yaz_log (LOG_LOG, "database: %s", toKeep->databaseName);
		if (!strcmp(toKeep->databaseName, "fault"))
		{
		    rr->errcode = 109;
		    rr->errstring = toKeep->databaseName;
		}
		if (!strcmp(toKeep->databaseName, "accept"))
		    rr->errcode = -1;
	    }
	    if (notToKeep)
	    {
		int i;
		for (i = 0; i < notToKeep->num; i++)
		{
		    Z_External *rec = notToKeep->elements[i]->record;

		    if (rec->direct_reference)
		    {
			struct oident *oident;
			oident = oid_getentbyoid(rec->direct_reference);
			if (oident)
			    yaz_log (LOG_LOG, "record %d type %s", i,
				     oident->desc);
		    }
		    switch (rec->which)
		    {
		    case Z_External_sutrs:
			if (rec->u.octet_aligned->len > 170)
			    yaz_log (LOG_LOG, "%d bytes:\n%.168s ...",
				     rec->u.sutrs->len,
				     rec->u.sutrs->buf);
			else
			    yaz_log (LOG_LOG, "%d bytes:\n%s",
				     rec->u.sutrs->len,
				     rec->u.sutrs->buf);
                        break;
		    case Z_External_octet        :
			if (rec->u.octet_aligned->len > 170)
			    yaz_log (LOG_LOG, "%d bytes:\n%.168s ...",
				     rec->u.octet_aligned->len,
				     rec->u.octet_aligned->buf);
			else
			    yaz_log (LOG_LOG, "%d bytes\n%s",
				     rec->u.octet_aligned->len,
				     rec->u.octet_aligned->buf);
		    }
		}
	    }
	}
    }
    else
    {
        yaz_log (LOG_WARN, "Unknown Extended Service(%d)",
		 rr->esr->taskSpecificParameters->which);
	
    }
    return 0;
}

int ztest_delete (void *handle, bend_delete_rr *rr)
{
    if (rr->num_setnames == 1 && !strcmp (rr->setnames[0], "1"))
	rr->delete_status = Z_DeleteStatus_success;
    else
        rr->delete_status = Z_DeleteStatus_resultSetDidNotExist;
    return 0;
}

/* Our sort handler really doesn't sort... */
int ztest_sort (void *handle, bend_sort_rr *rr)
{
    rr->errcode = 0;
    rr->sort_status = Z_SortStatus_success;
    return 0;
}

static int atoin (const char *buf, int n)
{
    int val = 0;
    while (--n >= 0)
    {
        if (isdigit(*buf))
            val = val*10 + (*buf - '0');
        buf++;
    }
    return val;
}

char *marc_read(FILE *inf, ODR odr)
{
    char length_str[5];
    size_t size;
    char *buf;

    if (fread (length_str, 1, 5, inf) != 5)
        return NULL;
    size = atoin (length_str, 5);
    if (size <= 6)
        return NULL;
    if (!(buf = (char*) odr_malloc (odr, size+1)))
        return NULL;
    if (fread (buf+5, 1, size-5, inf) != (size-5))
    {
        xfree (buf);
        return NULL;
    }
    memcpy (buf, length_str, 5);
    buf[size] = '\0';
    return buf;
}

static char *dummy_database_record (int num, ODR odr)
{
    FILE *inf = fopen ("dummy-records", "r");
    char *buf = 0;

    if (!inf)
	return NULL;
    while (--num >= 0)
    {
	if (num == 98)
	{
	    buf = (char*) odr_malloc(odr, 2101);
	    memset(buf, 'A', 2100);
	    buf[2100] = '\0';
	    break;
	}
	else
	    buf = marc_read (inf, odr);
	if (!num || !buf)
	    break;
    }
    fclose(inf);
    if (num < 0)
    	return 0;
    return buf;
}

static Z_GenericRecord *dummy_grs_record (int num, ODR o)
{
    FILE *f = fopen("dummy-grs", "r");
    char line[512];
    Z_GenericRecord *r = 0;
    int n;

    if (!f)
	return 0;
    while (fgets(line, 512, f))
	if (*line == '#' && sscanf(line, "#%d", &n) == 1 && n == num)
	{
	    r = read_grs1(f, o);
	    break;
	}
    fclose(f);
    return r;
}

int ztest_fetch(void *handle, bend_fetch_rr *r)
{
    char *cp;
    r->errstring = 0;
    r->last_in_set = 0;
    r->basename = "DUMMY";
    r->output_format = r->request_format;  
    if (r->request_format == VAL_SUTRS)
    {
#if 0
/* this section returns a huge record (for testing non-blocking write, etc) */
	r->len = 980000;
	r->record = odr_malloc (r->stream, r->len);
	memset (r->record, 'x', r->len);
#else
/* this section returns a small record */
    	char buf[100];

	sprintf(buf, "This is dummy SUTRS record number %d\n", r->number);

	r->len = strlen(buf);
	r->record = (char *) odr_malloc (r->stream, r->len+1);
	strcpy(r->record, buf);
#endif
    }
    else if (r->request_format == VAL_GRS1)
    {
	r->len = -1;
	r->record = (char*) dummy_grs_record(r->number, r->stream);
	if (!r->record)
	{
	    r->errcode = 13;
	    return 0;
	}
    }
    else if ((cp = dummy_database_record(r->number, r->stream)))
    {
	r->len = strlen(cp);
	r->record = cp;
	r->output_format = VAL_USMARC;
    }
    else
    {
    	r->errcode = 13;
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

    q->errcode = 0;
    q->errstring = 0;
    q->entries = list;
    q->status = BEND_SCAN_SUCCESS;
    if (!f && !(f = fopen("dummy-words", "r")))
    {
	perror("dummy-words");
	exit(1);
    }
    if (q->term->term->which != Z_Term_general)
    {
    	q->errcode = 229; /* unsupported term type */
	return 0;
    }
    if (q->term->term->u.general->len >= 80)
    {
    	q->errcode = 11; /* term too long */
	return 0;
    }
    if (q->num_entries > 200)
    {
    	q->errcode = 31;
	return 0;
    }
    memcpy(term, q->term->term->u.general->buf, q->term->term->u.general->len);
    term[q->term->term->u.general->len] = '\0';
    for (p = term; *p; p++)
    	if (islower(*p))
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

		if (!strcmp (term, "SD") && q->num_entries == 2)
		{
		    list[q->num_entries].term = entries[pos];
		    list[q->num_entries].occurrences = -1;
		    list[q->num_entries].errcode = 233;
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

bend_initresult *bend_init(bend_initrequest *q)
{
    bend_initresult *r = (bend_initresult *) odr_malloc (q->stream, sizeof(*r));
    static char *dummy = "Hej fister";

    r->errcode = 0;
    r->errstring = 0;
    r->handle = dummy;
    q->bend_sort = ztest_sort;              /* register sort handler */
    q->bend_search = ztest_search;          /* register search handler */
    q->bend_present = ztest_present;        /* register present handle */
    q->bend_esrequest = ztest_esrequest;
    q->bend_delete = ztest_delete;
    q->bend_fetch = ztest_fetch;
    q->bend_scan = ztest_scan;
    return r;
}

void bend_close(void *handle)
{
    return;
}

int main(int argc, char **argv)
{
    return statserv_main(argc, argv, bend_init, bend_close);
}
