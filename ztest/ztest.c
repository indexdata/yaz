/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * NT Service interface by
 *    Chas Woodfield, Fretwell Downing Datasystems.
 *
 * $Log: ztest.c,v $
 * Revision 1.23  1999-05-27 13:02:20  adam
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
#include <assert.h>

#include <backend.h>
#include <proto.h>
#include <log.h>

Z_GenericRecord *read_grs1(FILE *f, ODR o);

int ztest_search (void *handle, bend_search_rr *rr);
int ztest_sort (void *handle, bend_sort_rr *rr);
int ztest_present (void *handle, bend_present_rr *rr);
int ztest_esrequest (void *handle, bend_esrequest_rr *rr);
int ztest_delete (void *handle, bend_delete_rr *rr);

bend_initresult *bend_init(bend_initrequest *q)
{
    bend_initresult *r = (bend_initresult *) odr_malloc (q->stream, sizeof(*r));
    static char *dummy = "Hej fister";

    r->errcode = 0;
    r->errstring = 0;
    r->handle = dummy;
    q->bend_sort = ztest_sort;       /* register sort handler */
    q->bend_search = ztest_search;   /* register search handler */
    q->bend_present = ztest_present; /* register present handle */
    q->bend_esrequest = ztest_esrequest;
    q->bend_delete = ztest_delete;
    return r;
}

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
    logf(LOG_LOG, "function: %d", *rr->esr->function);
    if (rr->esr->packageName)
    	logf(LOG_LOG, "packagename: %s", rr->esr->packageName);
    logf(LOG_LOG, "Waitaction: %d", *rr->esr->waitAction);

    if (!rr->esr->taskSpecificParameters)
    {
        logf (LOG_WARN, "No task specific parameters");
    }
    else if (rr->esr->taskSpecificParameters->which == Z_External_itemOrder)
    {
    	Z_ItemOrder *it = rr->esr->taskSpecificParameters->u.itemOrder;
	logf (LOG_LOG, "Received ItemOrder");
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
		    logf(LOG_LOG, "contact name %s", k->contact->name);
		if (k->contact->phone)
		    logf(LOG_LOG, "contact phone %s", k->contact->phone);
		if (k->contact->email)
		    logf(LOG_LOG, "contact email %s", k->contact->email);
	    }
	    if (k->addlBilling)
	    {
	        logf(LOG_LOG, "Billing info (not shown)");
	    }
	    
	    if (n->resultSetItem)
	    {
	        logf(LOG_LOG, "resultsetItem");
		logf(LOG_LOG, "setId: %s", n->resultSetItem->resultSetId);
		logf(LOG_LOG, "item: %d", *n->resultSetItem->item);
	    }
	}
	break;
	}
    }
    else if (rr->esr->taskSpecificParameters->which == Z_External_update)
    {
    	Z_IUUpdate *up = rr->esr->taskSpecificParameters->u.update;
	logf (LOG_LOG, "Received DB Update");
	if (up->which == Z_IUUpdate_esRequest)
	{
	    Z_IUUpdateEsRequest *esRequest = up->u.esRequest;
	    Z_IUOriginPartToKeep *toKeep = esRequest->toKeep;
	    Z_IUSuppliedRecords *notToKeep = esRequest->notToKeep;
	    
	    logf (LOG_LOG, "action");
	    if (toKeep->action)
	    {
		switch (*toKeep->action)
		{
		case Z_IUOriginPartToKeep_recordInsert:
		    logf (LOG_LOG, " recordInsert");
		    break;
		case Z_IUOriginPartToKeep_recordReplace:
		    logf (LOG_LOG, " recordUpdate");
		    break;
		case Z_IUOriginPartToKeep_recordDelete:
		    logf (LOG_LOG, " recordDelete");
		    break;
		case Z_IUOriginPartToKeep_elementUpdate:
		    logf (LOG_LOG, " elementUpdate");
		    break;
		case Z_IUOriginPartToKeep_specialUpdate:
		    logf (LOG_LOG, " specialUpdate");
		    break;
		default:
		    logf (LOG_LOG, " unknown (%d)", *toKeep->action);
		}
	    }
	    logf (LOG_LOG, "database: %s", 
		  (toKeep->databaseName ? toKeep->databaseName : "<null>"));
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
			    logf (LOG_LOG, "record %d type %s", i,
				  oident->desc);
		    }
		    switch (rec->which)
		    {
		    case Z_External_sutrs:
			if (rec->u.octet_aligned->len > 170)
			    logf (LOG_LOG, "%d bytes:\n%.168s ...",
				  rec->u.sutrs->len,
				  rec->u.sutrs->buf);
			else
			    logf (LOG_LOG, "%d bytes:\n%s",
				  rec->u.sutrs->len,
				  rec->u.sutrs->buf);
		    case Z_External_octet        :
			if (rec->u.octet_aligned->len > 170)
			    logf (LOG_LOG, "%d bytes:\n%.168s ...",
				  rec->u.octet_aligned->len,
				  rec->u.octet_aligned->buf);
			else
			    logf (LOG_LOG, "%d bytes\n%s",
				  rec->u.octet_aligned->len,
				  rec->u.octet_aligned->buf);
		    }
		}
	    }
	}
    }
    else
    {
        logf (LOG_WARN, "Unknown Extended Service(%d)",
	      rr->esr->taskSpecificParameters->which);
	
    }
    rr->errcode = 0;
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

/* Obsolete bend_search, never called because handler is registered */
bend_searchresult *bend_search(void *handle, bend_searchrequest *q, int *fd)
{
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
    char *buf;

    if (!inf)
	return NULL;
    while (--num >= 0)
    {
	if (num == 98)
	{
	    buf = (char*) odr_malloc(odr, 2101);
	    assert(buf);
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

bend_fetchresult *bend_fetch(void *handle, bend_fetchrequest *q, int *fd)
{
    bend_fetchresult *r = (bend_fetchresult *)
			odr_malloc (q->stream, sizeof(*r));
    char *cp;
    r->errstring = 0;
    r->last_in_set = 0;
    r->basename = "DUMMY";
    r->format = q->format;  
    if (q->format == VAL_SUTRS)
    {
    	char buf[100];

	sprintf(buf, "This is dummy SUTRS record number %d\n", q->number);
	r->len = strlen(buf);
	r->record = (char *) odr_malloc (q->stream, r->len+1);
	strcpy(r->record, buf);
    }
    else if (q->format == VAL_GRS1)
    {
	r->len = -1;
	r->record = (char*) dummy_grs_record(q->number, q->stream);
	if (!r->record)
	{
	    r->errcode = 13;
	    return r;
	}
    }
    else if ((cp = dummy_database_record(q->number, q->stream)))
    {
	r->len = strlen(cp);
	r->record = cp;
	r->format = VAL_USMARC;
    }
    else
    {
    	r->errcode = 13;
	return r;
    }
    r->errcode = 0;
    return r;
}

/*
 * silly dummy-scan what reads words from a file.
 */
bend_scanresult *bend_scan(void *handle, bend_scanrequest *q, int *fd)
{
    bend_scanresult *r = (bend_scanresult *)
	odr_malloc (q->stream, sizeof(*r));
    static FILE *f = 0;
    static struct scan_entry list[200];
    static char entries[200][80];
    int hits[200];
    char term[80], *p;
    int i, pos;

    r->errcode = 0;
    r->errstring = 0;
    r->entries = list;
    r->status = BEND_SCAN_SUCCESS;
    if (!f && !(f = fopen("dummy-words", "r")))
    {
	perror("dummy-words");
	exit(1);
    }
    if (q->term->term->which != Z_Term_general)
    {
    	r->errcode = 229; /* unsupported term type */
	return r;
    }
    if (q->term->term->u.general->len >= 80)
    {
    	r->errcode = 11; /* term too long */
	return r;
    }
    if (q->num_entries > 200)
    {
    	r->errcode = 31;
	return r;
    }
    memcpy(term, q->term->term->u.general->buf, q->term->term->u.general->len);
    term[q->term->term->u.general->len] = '\0';
    for (p = term; *p; p++)
    	if (islower(*p))
	    *p = toupper(*p);

    fseek(f, 0, SEEK_SET);
    r->num_entries = 0;
    for (i = 0, pos = 0; fscanf(f, " %79[^:]:%d", entries[pos], &hits[pos]) == 2;
	i++, pos < 199 ? pos++ : (pos = 0))
    {
    	if (!r->num_entries && strcmp(entries[pos], term) >= 0) /* s-point fnd */
	{
	    if ((r->term_position = q->term_position) > i + 1)
	    {
	    	r->term_position = i + 1;
		r->status = BEND_SCAN_PARTIAL;
	    }
	    for (; r->num_entries < r->term_position; r->num_entries++)
	    {
	    	int po;

		po = pos - r->term_position + r->num_entries + 1; /* find pos */
		if (po < 0)
		    po += 200;

		if (!strcmp (term, "SD") && r->num_entries == 2)
		{
		    list[r->num_entries].term = entries[pos];
		    list[r->num_entries].occurrences = -1;
		    list[r->num_entries].errcode = 233;
		    list[r->num_entries].errstring = "SD for Scan Term";
		}
		else
		{
		    list[r->num_entries].term = entries[po];
		    list[r->num_entries].occurrences = hits[po];
		}
	    }
	}
	else if (r->num_entries)
	{
	    list[r->num_entries].term = entries[pos];
	    list[r->num_entries].occurrences = hits[pos];
	    r->num_entries++;
	}
	if (r->num_entries >= q->num_entries)
	    break;
    }
    if (feof(f))
    	r->status = BEND_SCAN_PARTIAL;
    return r;
}

void bend_close(void *handle)
{
    return;
}

int main(int argc, char **argv)
{
    return statserv_main(argc, argv);
}
