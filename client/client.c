/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: client.c,v $
 * Revision 1.30  1996-02-12 18:18:09  quinn
 * Fidgeting.
 *
 * Revision 1.29  1996/01/02  08:57:25  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.28  1995/12/14  11:09:31  quinn
 * Added Explain record syntax to the format command.
 *
 * Revision 1.27  1995/12/12  16:37:02  quinn
 * Added destroy element to data1_node.
 *
 * Revision 1.26  1995/12/12  14:11:00  quinn
 * Minimal.
 *
 * Revision 1.25  1995/11/13  09:27:22  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.24  1995/10/30  12:41:13  quinn
 * Added hostname lookup for server.
 *
 * Revision 1.23  1995/10/18  16:12:30  quinn
 * Better diagnostics.
 *
 * Revision 1.22  1995/10/11  14:49:12  quinn
 * Smallish.
 *
 * Revision 1.21  1995/09/29  17:01:47  quinn
 * More Windows work
 *
 * Revision 1.20  1995/08/29  14:24:13  quinn
 * Added second half of close-handshake
 *
 * Revision 1.19  1995/08/29  11:17:28  quinn
 * Added code to receive close
 *
 * Revision 1.18  1995/08/28  12:21:27  quinn
 * Client can now ask for simple element set names.
 *
 * Revision 1.17  1995/08/17  12:45:02  quinn
 * Fixed minor problems with GRS-1. Added support in c&s.
 *
 * Revision 1.16  1995/08/15  12:00:04  quinn
 * Updated External
 *
 * Revision 1.15  1995/06/22  09:28:03  quinn
 * Fixed bug in SUTRS processing.
 *
 * Revision 1.14  1995/06/19  12:37:41  quinn
 * Added BER dumper.
 *
 * Revision 1.13  1995/06/16  10:29:11  quinn
 * *** empty log message ***
 *
 * Revision 1.12  1995/06/15  07:44:57  quinn
 * Moving to v3.
 *
 * Revision 1.11  1995/06/14  15:26:40  quinn
 * *** empty log message ***
 *
 * Revision 1.10  1995/06/06  14:56:58  quinn
 * Better diagnostics.
 *
 * Revision 1.9  1995/06/06  08:15:19  quinn
 * Cosmetic.
 *
 * Revision 1.8  1995/06/05  10:52:22  quinn
 * Added SCAN.
 *
 * Revision 1.7  1995/06/02  09:50:09  quinn
 * Smallish.
 *
 * Revision 1.6  1995/05/31  08:29:21  quinn
 * Nothing significant.
 *
 * Revision 1.5  1995/05/29  08:10:47  quinn
 * Moved oid.c to util.
 *
 * Revision 1.4  1995/05/22  15:30:13  adam
 * Client uses prefix query notation.
 *
 * Revision 1.3  1995/05/22  15:06:53  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/05/22  14:56:40  quinn
 * *** empty log message ***
 *
 * Revision 1.1  1995/05/22  11:30:31  quinn
 * Added prettier client.
 *
 *
 */

/*
 * This is the obligatory little toy client, whose primary purpose is
 * to illustrate the use of the YAZ service-level API.
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef WINDOWS
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <assert.h>
#ifdef _AIX
#include <sys/select.h>
#endif

#include <comstack.h>
#include <tcpip.h>
#ifdef USE_XTIMOSI
#include <xmosi.h>
#endif

#include <proto.h>
#include <marcdisp.h>
#include <diagbib1.h>

#ifdef RPN_QUERY
#ifdef PREFIX_QUERY
#include <pquery.h>
#else
#include <yaz-ccl.h>
#endif
#endif

#define C_PROMPT "Z> "

static ODR out, in, print;              /* encoding and decoding streams */
static COMSTACK conn = 0;               /* our z-association */
static Z_IdAuthentication *auth = 0;    /* our current auth definition */
static char database[512] = "Default";  /* Database name */
static int setnumber = 0;               /* current result set number */
static int smallSetUpperBound = 0;
static int largeSetLowerBound = 1;
static int mediumSetPresentNumber = 0;
static Z_ElementSetNames *elementSetNames = 0; 
static int setno = 1;                   /* current set offset */
static int protocol = PROTO_Z3950;      /* current app protocol */
static int recordsyntax = VAL_USMARC;
static int sent_close = 0;
static ODR_MEM session_mem;                /* memory handle for init-response */
static Z_InitResponse *session = 0;        /* session parameters */
static char last_scan[512] = "0";
static char last_cmd[100] = "?";
static oid_value attributeset = VAL_BIB1;
static FILE *marcdump = 0;
static char marcdump_file[512] = "marc.out";

#ifdef RPN_QUERY
#ifndef PREFIX_QUERY
static CCL_bibset bibset;               /* CCL bibset handle */
#endif
#endif

static void send_apdu(Z_APDU *a)
{
    char *buf;
    int len;

    if (!z_APDU(out, &a, 0))
    {
        odr_perror(out, "Encoding APDU");
        exit(1);
    }
    buf = odr_getbuf(out, &len, 0);
    odr_reset(out); /* release the APDU */
    if (cs_put(conn, buf, len) < 0)
    {
        fprintf(stderr, "cs_put: %s", cs_errmsg(cs_errno(conn)));
        exit(1);
    }
}

/* INIT SERVICE ------------------------------- */

static void send_initRequest()
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_initRequest);
    Z_InitRequest *req = apdu->u.initRequest;

    ODR_MASK_SET(req->options, Z_Options_search);
    ODR_MASK_SET(req->options, Z_Options_present);
    ODR_MASK_SET(req->options, Z_Options_namedResultSets);
    ODR_MASK_SET(req->options, Z_Options_triggerResourceCtrl);
    ODR_MASK_SET(req->options, Z_Options_scan);

    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_2);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_3);

    *req->maximumRecordSize = 1024*1024;

    req->idAuthentication = auth;

    send_apdu(apdu);
    printf("Sent initrequest.\n");
}

static int process_initResponse(Z_InitResponse *res)
{
    /* save session parameters for later use */
    session_mem = odr_extract_mem(in);
    session = res;

    if (!*res->result)
        printf("Connection rejected by target.\n");
    else
        printf("Connection accepted by target.\n");
    if (res->implementationId)
        printf("ID     : %s\n", res->implementationId);
    if (res->implementationName)
        printf("Name   : %s\n", res->implementationName);
    if (res->implementationVersion)
        printf("Version: %s\n", res->implementationVersion);
    if (res->userInformationField)
    {
        printf("UserInformationfield:\n");
        if (!z_External(print, (Z_External**)&res-> userInformationField,
            0))
        {
            odr_perror(print, "Printing userinfo\n");
            odr_reset(print);
        }
        if (res->userInformationField->which == Z_External_octet)
        {
            printf("Guessing visiblestring:\n");
            printf("'%s'\n", res->userInformationField->u. octet_aligned->buf);
        }
    }
    return 0;
}

int cmd_open(char *arg)
{
    void *add;
    char type[100], addr[100];
    CS_TYPE t;

    if (conn)
    {
        printf("Already connected.\n");
        return 0;
    }
    if (!*arg || sscanf(arg, "%[^:]:%s", type, addr) < 2)
    {
        fprintf(stderr, "Usage: open (osi|tcp) ':' [tsel '/']host[':'port]\n");
        return 0;
    }
#ifdef USE_XTIMOSI
    if (!strcmp(type, "osi"))
    {
        if (!(add = mosi_strtoaddr(addr)))
        {
            perror(arg);
            return 0;
        }
        t = mosi_type;
        protocol = PROTO_SR;
    }
    else
#endif
    if (!strcmp(type, "tcp"))
    {
        if (!(add = tcpip_strtoaddr(addr)))
        {
            perror(arg);
            return 0;
        }
        t = tcpip_type;
        protocol = PROTO_Z3950;
    }
    else
    {
        fprintf(stderr, "Bad type: %s\n", type);
        return 0;
    }
    if (!(conn = cs_create(t, 1, protocol)))
    {
        perror("cs_create");
        return 0;
    }
    printf("Connecting...");
    fflush(stdout);
    if (cs_connect(conn, add) < 0)
    {
        perror("connect");
        cs_close(conn);
        conn = 0;
        return 0;
    }
    printf("Ok.\n");
    send_initRequest();
    return 2;
}

int cmd_authentication(char *arg)
{
    static Z_IdAuthentication au;
    static char open[256];

    if (!*arg)
    {
        printf("Auth field set to null\n");
        auth = 0;
        return 1;
    }
    auth = &au;
    au.which = Z_IdAuthentication_open;
    au.u.open = open;
    strcpy(open, arg);
    return 1;
}

/* SEARCH SERVICE ------------------------------ */

static void display_variant(Z_Variant *v, int level)
{
    int i;

    for (i = 0; i < v->num_triples; i++)
    {
	printf("%*sclass=%d,type=%d", level * 4, "", *v->triples[i]->class,
	    *v->triples[i]->type);
	if (v->triples[i]->which == Z_Triple_internationalString)
	    printf(",value=%s\n", v->triples[i]->value.internationalString);
	else
	    printf("\n");
    }
}

static void display_grs1(Z_GenericRecord *r, int level)
{
    int i;

    if (!r)
        return;
    for (i = 0; i < r->num_elements; i++)
    {
        Z_TaggedElement *t;

        printf("%*s", level * 4, "");
        t = r->elements[i];
        printf("(");
        if (t->tagType)
            printf("%d,", *t->tagType);
        else
            printf("?,");
        if (t->tagValue->which == Z_StringOrNumeric_numeric)
            printf("%d) ", *t->tagValue->u.numeric);
        else
            printf("%s) ", t->tagValue->u.string);
        if (t->content->which == Z_ElementData_subtree)
        {
            printf("\n");
            display_grs1(t->content->u.subtree, level+1);
        }
        else if (t->content->which == Z_ElementData_string)
            printf("%s\n", t->content->u.string);
        else if (t->content->which == Z_ElementData_numeric)
	    printf("%d\n", *t->content->u.numeric);
	else if (t->content->which == Z_ElementData_noDataRequested)
	    printf("[No data requested]\n");
	else if (t->content->which == Z_ElementData_elementEmpty)
	    printf("[Element empty]\n");
	else if (t->content->which == Z_ElementData_elementNotThere)
	    printf("[Element not there]\n");
	else
            printf("??????\n");
	if (t->appliedVariant)
	    display_variant(t->appliedVariant, level+1);
	if (t->metaData && t->metaData->supportedVariants)
	{
	    int c;

	    printf("%*s---- variant list\n", (level+1)*4, "");
	    for (c = 0; c < t->metaData->num_supportedVariants; c++)
	    {
		printf("%*svariant #%d\n", (level+1)*4, "", c);
		display_variant(t->metaData->supportedVariants[c], level + 2);
	    }
	}
    }
}

static void display_record(Z_DatabaseRecord *p)
{
    Z_External *r = (Z_External*) p;
    oident *ent = oid_getentbyoid(r->direct_reference);

    if (r->direct_reference)
    {
        printf("Record type: ");
        if (ent)
            printf("%s\n", ent->desc);
        else if (!odr_oid(print, &r->direct_reference, 0))
        {
            odr_perror(print, "print oid");
            odr_reset(print);
        }
    }
    if (r->which == Z_External_octet && p->u.octet_aligned->len)
    {
#if 0
	if (marcdump)
	    fwrite(p->u.octet_aligned->buf, p->u.octet_aligned->len, 1,
		marcdump);
#endif
        static count = 0;
        FILE *f;
        char buf[256];

        sprintf(buf, "marc.%d", count++);
        if (!(f = fopen(buf, "w")))
        {
            perror(buf);
            exit(1);

        }
        fwrite(p->u.octet_aligned->buf, p->u.octet_aligned->len, 1, f);
        fclose(f);


        marc_display ((char*)p->u.octet_aligned->buf, stdout);
    }
    else if (ent->value == VAL_SUTRS)
    {
        if (r->which != Z_External_sutrs)
        {
            printf("Expecting single SUTRS type for SUTRS.\n");
            return;
        }
        printf("%.*s", r->u.sutrs->len, r->u.sutrs->buf);
    }
    else if (ent->value == VAL_GRS1)
    {
        if (r->which != Z_External_grs1)
        {
            printf("Expecting single GRS type for GRS.\n");
            return;
        }
        display_grs1(r->u.grs1, 0);
    }
    else 
    {
        printf("Unknown record representation.\n");
        if (!z_External(print, &r, 0))
        {
            odr_perror(print, "Printing external");
            odr_reset(print);
        }
    }
}

static void display_diagrec(Z_DiagRec *p)
{
    oident *ent;
#ifdef Z_95
    Z_DefaultDiagFormat *r;
#else
    Z_DiagRec *r = p;
#endif

    printf("Diagnostic message from database:\n");
#ifdef Z_95
    if (p->which != Z_DiagRec_defaultFormat)
    {
        printf("Diagnostic record not in default format.\n");
        return;
    }
    else
        r = p->u.defaultFormat;
#endif
    if (!(ent = oid_getentbyoid(r->diagnosticSetId)) ||
        ent->oclass != CLASS_DIAGSET || ent->value != VAL_BIB1)
        printf("Missing or unknown diagset\n");
    printf("    [%d] %s", *r->condition, diagbib1_str(*r->condition));
    if (r->addinfo && *r->addinfo)
        printf(" -- %s\n", r->addinfo);
    else
        printf("\n");
}

static void display_nameplusrecord(Z_NamePlusRecord *p)
{
    if (p->databaseName)
        printf("[%s]", p->databaseName);
    if (p->which == Z_NamePlusRecord_surrogateDiagnostic)
        display_diagrec(p->u.surrogateDiagnostic);
    else
        display_record(p->u.databaseRecord);
}

static void display_records(Z_Records *p)
{
    int i;

    if (p->which == Z_Records_NSD)
        display_diagrec(p->u.nonSurrogateDiagnostic);
    else
    {
        printf("Records: %d\n", p->u.databaseOrSurDiagnostics->num_records);
        for (i = 0; i < p->u.databaseOrSurDiagnostics->num_records; i++)
            display_nameplusrecord(p->u.databaseOrSurDiagnostics->records[i]);
    }
}

static int send_searchRequest(char *arg)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_searchRequest);
    Z_SearchRequest *req = apdu->u.searchRequest;
    char *databaseNames = database;
    Z_Query query;
#ifdef RPN_QUERY
#ifndef PREFIX_QUERY
    struct ccl_rpn_node *rpn;
    int error, pos;
#endif
#endif
    char setstring[100];
#ifdef RPN_QUERY
    Z_RPNQuery *RPNquery;
    oident bib1;
#else
    Odr_oct ccl_query;
#endif

#ifdef RPN_QUERY
#ifndef PREFIX_QUERY
    rpn = ccl_find_str(bibset, arg, &error, &pos);
    if (error)
    {
        printf("CCL ERROR: %s\n", ccl_err_msg(error));
        return 0;
    }
#endif
#endif

    if (!strcmp(arg, "@big")) /* strictly for troublemaking */
    {
        static unsigned char big[2100];
        static Odr_oct bigo;

        /* send a very big referenceid to test transport stack etc. */
        memset(big, 'A', 2100);
        bigo.len = bigo.size = 2100;
        bigo.buf = big;
        req->referenceId = &bigo;
    }

    if (setnumber >= 0)
    {
        sprintf(setstring, "%d", ++setnumber);
        req->resultSetName = setstring;
    }
    *req->smallSetUpperBound = smallSetUpperBound;
    *req->largeSetLowerBound = largeSetLowerBound;
    *req->mediumSetPresentNumber = mediumSetPresentNumber;
    if (smallSetUpperBound > 0 || (largeSetLowerBound > 1 &&
        mediumSetPresentNumber > 0))
    {
        oident prefsyn;

        prefsyn.proto = protocol;
        prefsyn.oclass = CLASS_RECSYN;
        prefsyn.value = recordsyntax;
        req->preferredRecordSyntax = odr_oiddup(out, oid_getoidbyent(&prefsyn));
        req->smallSetElementSetNames =
            req->mediumSetElementSetNames = elementSetNames;
    }
    req->num_databaseNames = 1;
    req->databaseNames = &databaseNames;

    req->query = &query;

#ifdef RPN_QUERY
    query.which = Z_Query_type_1;

#ifndef PREFIX_QUERY
    assert((RPNquery = ccl_rpn_query(rpn)));
#else
    RPNquery = p_query_rpn (out, arg);
    if (!RPNquery)
    {
        printf("Prefix query error\n");
        return 0;
    }
#endif
    bib1.proto = protocol;
    bib1.oclass = CLASS_ATTSET;
    bib1.value = VAL_BIB1;
    RPNquery->attributeSetId = oid_getoidbyent(&bib1);
    query.u.type_1 = RPNquery;
#else
    query.which = Z_Query_type_2;
    query.u.type_2 = &ccl_query;
    ccl_query.buf = (unsigned char*) arg;
    ccl_query.len = strlen(arg);
#endif

    send_apdu(apdu);
    setno = 1;
    printf("Sent searchRequest.\n");
    return 2;
}

static int process_searchResponse(Z_SearchResponse *res)
{
    if (*res->searchStatus)
        printf("Search was a success.\n");
    else
        printf("Search was a bloomin' failure.\n");
    printf("Number of hits: %d, setno %d\n",
        *res->resultCount, setnumber);
    printf("records returned: %d\n",
        *res->numberOfRecordsReturned);
    setno += *res->numberOfRecordsReturned;
    if (res->records)
        display_records(res->records);
    return 0;
}

static int cmd_find(char *arg)
{
    if (!*arg)
    {
        printf("Find what?\n");
        return 0;
    }
    if (!conn)
    {
        printf("Not connected yet\n");
        return 0;
    }
    if (!send_searchRequest(arg))
        return 0;
    return 2;
}

static int cmd_ssub(char *arg)
{
    if (!(smallSetUpperBound = atoi(arg)))
        return 0;
    return 1;
}

static int cmd_lslb(char *arg)
{
    if (!(largeSetLowerBound = atoi(arg)))
        return 0;
    return 1;
}

static int cmd_mspn(char *arg)
{
    if (!(mediumSetPresentNumber = atoi(arg)))
        return 0;
    return 1;
}

static int cmd_status(char *arg)
{
    printf("smallSetUpperBound: %d\n", smallSetUpperBound);
    printf("largeSetLowerBound: %d\n", largeSetLowerBound);
    printf("mediumSetPresentNumber: %d\n", mediumSetPresentNumber);
    return 1;
}

static int cmd_base(char *arg)
{
    if (!*arg)
    {
        printf("Usage: base <database>\n");
        return 0;
    }
    strcpy(database, arg);
    return 1;
}

static int cmd_setnames(char *arg)
{
    if (setnumber < 0)
    {
        printf("Set numbering enabled.\n");
        setnumber = 0;
    }
    else
    {
        printf("Set numbering disabled.\n");
        setnumber = -1;
    }
    return 1;
}

/* PRESENT SERVICE ----------------------------- */

static int send_presentRequest(char *arg)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_presentRequest);
    Z_PresentRequest *req = apdu->u.presentRequest;
    Z_RecordComposition compo;
    oident prefsyn;
    int nos = 1;
    char *p;
    char setstring[100];

    if ((p = strchr(arg, '+')))
    {
        nos = atoi(p + 1);
        *p = 0;
    }
    if (*arg)
        setno = atoi(arg);

    if (setnumber >= 0)
    {
        sprintf(setstring, "%d", setnumber);
        req->resultSetId = setstring;
    }
    req->resultSetStartPoint = &setno;
    req->numberOfRecordsRequested = &nos;
    prefsyn.proto = protocol;
    prefsyn.oclass = CLASS_RECSYN;
    prefsyn.value = recordsyntax;
    req->preferredRecordSyntax = oid_getoidbyent(&prefsyn);
    if (elementSetNames)
    {
        req->recordComposition = &compo;
        compo.which = Z_RecordComp_simple;
        compo.u.simple = elementSetNames;
    }
    send_apdu(apdu);
    printf("Sent presentRequest (%d+%d).\n", setno, nos);
    return 2;
}

void process_close(Z_Close *req)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_close);
    Z_Close *res = apdu->u.close;

    static char *reasons[] =
    {
        "finished",
        "shutdown",
        "system problem",
        "cost limit reached",
        "resources",
        "security violation",
        "protocolError",
        "lack of activity",
        "peer abort",
        "unspecified"
    };

    printf("Reason: %s, message: %s\n", reasons[*req->closeReason],
        req->diagnosticInformation ? req->diagnosticInformation : "NULL");
    if (sent_close)
    {
        printf("Goodbye.\n");
        exit(0);
    }
    *res->closeReason = Z_Close_finished;
    send_apdu(apdu);
    printf("Sent response.\n");
    sent_close = 1;
}

static int cmd_show(char *arg)
{
    if (!send_presentRequest(arg))
        return 0;
    return 2;
}

int cmd_quit(char *arg)
{
    printf("See you later, alligator.\n");
    exit(0);
}

int cmd_cancel(char *arg)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_triggerResourceControlRequest);
    Z_TriggerResourceControlRequest *req =
        apdu->u.triggerResourceControlRequest;
    bool_t false = 0;
    
    if (!session)
    {
        printf("Session not initialized yet\n");
        return 0;
    }
    if (!ODR_MASK_GET(session->options, Z_Options_triggerResourceCtrl))
    {
        printf("Target doesn't support cancel (trigger resource ctrl)\n");
        return 0;
    }
    *req->requestedAction = Z_TriggerResourceCtrl_cancel;
    req->resultSetWanted = &false;

    send_apdu(apdu);
    printf("Sent cancel request\n");
    return 2;
}

int send_scanrequest(char *string, int pp, int num)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_scanRequest);
    Z_ScanRequest *req = apdu->u.scanRequest;
    char *db = database;
    oident attset;

    req->num_databaseNames = 1;
    req->databaseNames = &db;
    attset.proto = protocol;
    attset.oclass = CLASS_ATTSET;
    attset.value = VAL_BIB1;
    req->attributeSet = oid_getoidbyent(&attset);
    req->termListAndStartPoint = p_query_scan(out, string);
    req->numberOfTermsRequested = &num;
    req->preferredPositionInResponse = &pp;
    send_apdu(apdu);
    return 2;
}

void display_term(Z_TermInfo *t)
{
    if (t->term->which == Z_Term_general)
    {
        printf("%.*s (%d)\n", t->term->u.general->len, t->term->u.general->buf,
            t->globalOccurrences ? *t->globalOccurrences : -1);
        sprintf(last_scan, "%.*s", t->term->u.general->len,
            t->term->u.general->buf);
    }
    else
        printf("Term type not general.\n");
}

void process_scanResponse(Z_ScanResponse *res)
{
    int i;

    printf("SCAN: %d entries, position=%d\n", *res->numberOfEntriesReturned,
        *res->positionOfTerm);
    if (*res->scanStatus != Z_Scan_success)
        printf("Scan returned code %d\n", *res->scanStatus);
    if (!res->entries)
        return;
    if (res->entries->which == Z_ListEntries_entries)
    {
        Z_Entries *ent = res->entries->u.entries;

        for (i = 0; i < ent->num_entries; i++)
            if (ent->entries[i]->which == Z_Entry_termInfo)
            {
                printf("%c ", i + 1 == *res->positionOfTerm ? '*' : ' ');
                display_term(ent->entries[i]->u.termInfo);
            }
            else
                display_diagrec(ent->entries[i]->u.surrogateDiagnostic);
    }
    else
        display_diagrec(res->entries->u.nonSurrogateDiagnostics->diagRecs[0]);
}

int cmd_scan(char *arg)
{
    if (!session)
    {
        printf("Session not initialized yet\n");
        return 0;
    }
    if (!ODR_MASK_GET(session->options, Z_Options_scan))
    {
        printf("Target doesn't support scan\n");
        return 0;
    }
    if (*arg)
    {
        if (send_scanrequest(arg, 5, 20) < 0)
            return 0;
    }
    else
        if (send_scanrequest(last_scan, 1, 20) < 0)
            return 0;
    return 2;
}

int cmd_format(char *arg)
{
    if (!arg || !*arg)
    {
        printf("Usage: format <recordsyntax>\n");
        return 0;
    }
    if (!strcmp(arg, "sutrs"))
    {
        printf("Preferred format is SUTRS.\n");
        recordsyntax = VAL_SUTRS;
        return 1;
    }
    else if (!strcmp(arg, "usmarc"))
    {
        printf("Preferred format is USMARC\n");
        recordsyntax = VAL_USMARC;
        return 1;
    }
    else if (!strcmp(arg, "danmarc"))
    {
        printf("Preferred format is DANMARC\n");
        recordsyntax = VAL_DANMARC;
        return 1;
    }
    else if (!strcmp(arg, "grs1"))
    {
        printf("Preferred format is GRS1\n");
        recordsyntax = VAL_GRS1;
        return 1;
    }
    else if (!strcmp(arg, "explain"))
    {
        printf("Preferred format is Explain\n");
        recordsyntax = VAL_EXPLAIN;
        return 1;
    }
    else
    {
        printf("Specify one of {sutrs,usmarc,danmarc,grs1}.\n");
        return 0;
    }
}

int cmd_elements(char *arg)
{
    static Z_ElementSetNames esn;
    static char what[100];

    if (!arg || !*arg)
    {
        printf("Usage: elements <esn>\n");
        return 0;
    }
    strcpy(what, arg);
    esn.which = Z_ElementSetNames_generic;
    esn.u.generic = what;
    elementSetNames = &esn;
    return 1;
}

int cmd_attributeset(char *arg)
{
    char what[100];
    oid_value v;

    if (!arg || !*arg)
    {
	printf("Usage: attributeset <setname>\n");
	return 0;
    }
    sscanf(arg, "%s", what);
    if ((v = oid_getvalbyname(what)) == VAL_NONE)
    {
	printf("Unknown attribute set name\n");
	return 0;
    }
    attributeset = v;
    return 1;
}

int cmd_close(char *arg)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_close);
    Z_Close *req = apdu->u.close;

    *req->closeReason = Z_Close_finished;
    send_apdu(apdu);
    printf("Sent close request.\n");
    sent_close = 1;
    return 2;
}

static void initialize(void)
{
#ifdef RPN_QUERY
#ifndef PREFIX_QUERY
    FILE *inf;
#endif
#endif

    if (!(out = odr_createmem(ODR_ENCODE)) ||
        !(in = odr_createmem(ODR_DECODE)) ||
        !(print = odr_createmem(ODR_PRINT)))
    {
        fprintf(stderr, "failed to allocate ODR streams\n");
        exit(1);
    }
    setvbuf(stdout, 0, _IONBF, 0);

#ifdef RPN_QUERY
#ifndef PREFIX_QUERY
    bibset = ccl_qual_mk (); 
    inf = fopen ("default.bib", "r");
    if (inf)
    {
        ccl_qual_file (bibset, inf);
        fclose (inf);
    }
#endif
#endif
}

static int client(void)
{
    static struct {
        char *cmd;
        int (*fun)(char *arg);
        char *ad;
    } cmd[] = {
        {"open", cmd_open, "('tcp'|'osi')':'[<TSEL>'/']<HOST>[':'<PORT>]"},
        {"quit", cmd_quit, ""},
        {"find", cmd_find, "<CCL-QUERY>"},
        {"base", cmd_base, "<BASE-NAME>"},
        {"show", cmd_show, "<REC#>['+'<#RECS>]"},
        {"scan", cmd_scan, "<TERM>"},
        {"authentication", cmd_authentication, "<ACCTSTRING>"},
        {"lslb", cmd_lslb, "<largeSetLowerBound>"},
        {"ssub", cmd_ssub, "<smallSetUpperBound>"},
        {"mspn", cmd_mspn, "<mediumSetPresentNumber>"},
        {"status", cmd_status, ""},
        {"setnames", cmd_setnames, ""},
        {"cancel", cmd_cancel, ""},
        {"format", cmd_format, "<recordsyntax>"},
        {"elements", cmd_elements, "<elementSetName>"},
        {"close", cmd_close, ""},
	{"attributeset", cmd_attributeset, "<attrset>"},
        {0,0}
    };
    char *netbuffer= 0;
    int netbufferlen = 0;
    int i;
    Z_APDU *apdu;

    while (1)
    {
        int res;
        fd_set input;
        char line[1024], word[1024], arg[1024];

        FD_ZERO(&input);
        FD_SET(0, &input);
        if (conn)
            FD_SET(cs_fileno(conn), &input);
        if ((res = select(20, &input, 0, 0, 0)) < 0)
        {
            perror("select");
            exit(1);
        }
        if (!res)
            continue;
        if (FD_ISSET(0, &input))
        {
            /* quick & dirty way to get a command line. */
            if (!gets(line))
                break;
            if ((res = sscanf(line, "%s %[^;]", word, arg)) <= 0)
            {
                strcpy(word, last_cmd);
                *arg = '\0';
            }
            else if (res == 1)
                *arg = 0;
            strcpy(last_cmd, word);
            for (i = 0; cmd[i].cmd; i++)
                if (!strncmp(cmd[i].cmd, word, strlen(word)))
                {
                    res = (*cmd[i].fun)(arg);
                    break;
                }
            if (!cmd[i].cmd) /* dump our help-screen */
            {
                printf("Unknown command: %s.\n", word);
                printf("Currently recognized commands:\n");
                for (i = 0; cmd[i].cmd; i++)
                    printf("   %s %s\n", cmd[i].cmd, cmd[i].ad);
                res = 1;
            }
            if (res < 2)
                printf(C_PROMPT);
        }
        if (conn && FD_ISSET(cs_fileno(conn), &input))
        {
            do
            {
                if ((res = cs_get(conn, &netbuffer, &netbufferlen)) < 0)
                {
                    perror("cs_get");
                    exit(1);
                }
                if (!res)
                {
                    printf("Target closed connection.\n");
                    exit(1);
                }
                odr_reset(in); /* release APDU from last round */
                odr_setbuf(in, netbuffer, res, 0);
                if (!z_APDU(in, &apdu, 0))
                {
                    odr_perror(in, "Decoding incoming APDU");
		    fprintf(stderr, "[Near %d]\n", odr_offset(in));
                    fprintf(stderr, "Packet dump:\n---------\n");
                    odr_dumpBER(stderr, netbuffer, res);
                    fprintf(stderr, "---------\n");
                    exit(1);
                }
#if 0
                if (!z_APDU(print, &apdu, 0))
                {
                    odr_perror(print, "Failed to print incoming APDU");
                    odr_reset(print);
                    continue;
                }
#endif
                switch(apdu->which)
                {
                    case Z_APDU_initResponse:
                        process_initResponse(apdu->u.initResponse);
                        break;
                    case Z_APDU_searchResponse:
                        process_searchResponse(apdu->u.searchResponse);
                        break;
                    case Z_APDU_scanResponse:
                        process_scanResponse(apdu->u.scanResponse);
                        break;
                    case Z_APDU_presentResponse:
                        printf("Received presentResponse.\n");
                        setno +=
                            *apdu->u.presentResponse->numberOfRecordsReturned;
                        if (apdu->u.presentResponse->records)
                            display_records(apdu->u.presentResponse->records);
                        else
                            printf("No records.\n");
                        break;
                    case Z_APDU_close:
                        printf("Target has closed the association.\n");
                        process_close(apdu->u.close);
                        break;
                    default:
                        printf("Received unknown APDU type (%d).\n", 
                            apdu->which);
                        exit(1);
                }
                printf(C_PROMPT);
                fflush(stdout);
            }
            while (cs_more(conn));
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    initialize();
    if (argc > 1)
        cmd_open(argv[1]);
    else
        printf(C_PROMPT);
    if (*marcdump_file && !(marcdump = fopen(marcdump_file, "a")))
    {
	perror(marcdump_file);
	exit(1);
    }
    return client();
}
