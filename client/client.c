/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: client.c,v $
 * Revision 1.6  1995-05-31 08:29:21  quinn
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
#include <sys/time.h>
#include <assert.h>

#include <comstack.h>
#include <tcpip.h>
#ifdef USE_XTIMOSI
#include <xmosi.h>
#endif

#include <proto.h>

#include <marcdisp.h>

#ifdef RPN_QUERY
#ifdef PREFIX_QUERY
#include <pquery.h>
#else
#include <yaz-ccl.h>
#endif
#endif

#include "../version.h"

#define C_PROMPT "Z> "

static ODR out, in, print;              /* encoding and decoding streams */
static COMSTACK conn = 0;               /* our z-association */
static Z_IdAuthentication *auth = 0;    /* our current auth definition */
static char database[512] = "Default";  /* Database name */
static int setnumber = 0;               /* current result set number */
static int smallSetUpperBound = 0;
static int largeSetLowerBound = 1;
static int mediumSetPresentNumber = 0;
static int setno = 1;                   /* current set offset */
static int protocol = PROTO_Z3950;      /* current app protocol */
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
    	fprintf(stderr, "cs_put: %s", cs_errlist[cs_errno(conn)]);
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

    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_2);

    req->idAuthentication = auth;

    send_apdu(apdu);
    printf("Sent initrequest.\n");
}

static int process_initResponse(Z_InitResponse *res)
{
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
	if (!odr_external(print, (Odr_external**)&res-> userInformationField,
	    0))
	{
	    odr_perror(print, "Printing userinfo\n");
	    odr_reset(print);
	}
	if (res->userInformationField->which == ODR_EXTERNAL_octet)
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
    printf("Ok!\n");
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

void display_record(Z_DatabaseRecord *p)
{
    Odr_external *r = (Odr_external*) p;

    if (r->direct_reference)
    {
    	oident *ent = oid_getentbyoid(r->direct_reference);

    	printf("Record type: ");
	if (ent)
	    printf("%s\n", ent->desc);
	else if (!odr_oid(print, &r->direct_reference, 0))
    	{
	    odr_perror(print, "print oid");
	    odr_reset(print);
	}
    }
#if 1
    if (r->which == ODR_EXTERNAL_octet && p->u.octet_aligned->len)
    {
#if 1
	marc_display ((char*)p->u.octet_aligned->buf, stdout);
#else
	FILE *ofi = fopen("dump", "a");
	assert(ofi);
	fwrite(p->u.octet_aligned->buf, 1, p->u.octet_aligned->len, ofi);
	fclose(ofi);
	printf("dumped record\n");
#endif
    }
    else
    {
    	printf("Unknown record representation.\n");
    	if (!odr_external(print, &r, 0))
    	{
	    odr_perror(print, "Printing external");
	    odr_reset(print);
	}
    }
#endif
}

static void display_diagrec(Z_DiagRec *p)
{
    oident *ent;

    printf("Diagnostic message from database.\n");
    if (!(ent = oid_getentbyoid(p->diagnosticSetId)) ||
    	ent->class != CLASS_DIAGSET || ent->value != VAL_BIB1)
    	printf("Missing or unknown diagset\n");
    printf("Error condition: %d", *p->condition);
    printf(" -- %s\n", p->addinfo ? p->addinfo : "");
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
    bib1.class = CLASS_ATTSET;
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
    if (res->searchStatus)
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
    send_apdu(apdu);
    printf("Sent presentRequest (%d+%d).\n", setno, nos);
    return 2;
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
    	{"authentication", cmd_authentication, "<ACCTSTRING>"},
	{"lslb", cmd_lslb, "<largeSetLowerBound>"},
	{"ssub", cmd_ssub, "<smallSetUpperBound>"},
	{"mspn", cmd_mspn, "<mediumSetPresentNumber>"},
	{"status", cmd_status, ""},
	{"setnames", cmd_setnames, ""},
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
	    	printf(C_PROMPT);
		continue;
	    }
	    if (res == 1)
		*arg = 0;
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
		    case Z_APDU_presentResponse:
		    	printf("Received presentResponse.\n");
			setno +=
			    *apdu->u.presentResponse->numberOfRecordsReturned;
		    	if (apdu->u.presentResponse->records)
			    display_records(apdu->u.presentResponse->records);
			else
			    printf("No records.\n");
			break;
		    default:
		    	printf("Received unknown APDU type (%d).\n", 
			    apdu->which);
			exit(1);
		}
		printf("Z> ");
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
    return client();
}
