/*
 * $Id: zoom-c.c,v 1.24 2002-02-28 13:21:16 adam Exp $
 *
 * ZOOM layer for C, connections, result sets, queries.
 */
#include <assert.h>
#include <yaz/xmalloc.h>
#include <yaz/otherinfo.h>
#include <yaz/log.h>
#include <yaz/pquery.h>
#include <yaz/diagbib1.h>

#include "zoom-p.h"

#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

static ZOOM_Event ZOOM_Event_create (int kind)
{
    ZOOM_Event event = (ZOOM_Event) xmalloc (sizeof(*event));
    event->kind = kind;
    event->next = 0;
    event->prev = 0;
    return event;
}

static void ZOOM_Event_destroy (ZOOM_Event event)
{
    xfree (event);
}

static void ZOOM_connection_put_event (ZOOM_connection c, ZOOM_Event event)
{
    if (c->m_queue_back)
    {
	c->m_queue_back->prev = event;
	assert (c->m_queue_front);
    }
    else
    {
	assert (!c->m_queue_front);
	c->m_queue_front = event;
    }
    event->next = c->m_queue_back;
    event->prev = 0;
    c->m_queue_back = event;
}

static ZOOM_Event ZOOM_connection_get_event(ZOOM_connection c)
{
    ZOOM_Event event = c->m_queue_front;
    if (!event)
	return 0;
    assert (c->m_queue_back);
    c->m_queue_front = event->prev;
    if (c->m_queue_front)
    {
	assert (c->m_queue_back);
	c->m_queue_front->next = 0;
    }
    else
	c->m_queue_back = 0;
    c->last_event = event->kind;
    return event;
}

static void clear_error (ZOOM_connection c)
{

    switch (c->error)
    {
    case ZOOM_ERROR_CONNECT:
    case ZOOM_ERROR_MEMORY:
    case ZOOM_ERROR_DECODE:
    case ZOOM_ERROR_CONNECTION_LOST:
    case ZOOM_ERROR_INIT:
    case ZOOM_ERROR_INTERNAL:
        break;
    default:
        c->error = ZOOM_ERROR_NONE;
        xfree (c->addinfo);
        c->addinfo = 0;
    }
}

ZOOM_task ZOOM_connection_add_task (ZOOM_connection c, int which)
{
    ZOOM_task *taskp = &c->tasks;
    while (*taskp)
	taskp = &(*taskp)->next;
    *taskp = (ZOOM_task) xmalloc (sizeof(**taskp));
    (*taskp)->running = 0;
    (*taskp)->which = which;
    (*taskp)->next = 0;
    clear_error (c);
    return *taskp;
}

void ZOOM_connection_remove_task (ZOOM_connection c)
{
    ZOOM_task task = c->tasks;

    if (task)
    {
	c->tasks = task->next;
	switch (task->which)
	{
	case ZOOM_TASK_SEARCH:
	    ZOOM_resultset_destroy (task->u.search.resultset);
	    break;
	case ZOOM_TASK_RETRIEVE:
	    ZOOM_resultset_destroy (task->u.retrieve.resultset);
	    break;
        case ZOOM_TASK_CONNECT:
            break;
        case ZOOM_TASK_SCAN:
            ZOOM_scanset_destroy (task->u.scan.scan);
            break;
	default:
	    assert (0);
	}
	xfree (task);
    }
}

void ZOOM_connection_remove_tasks (ZOOM_connection c)
{
    while (c->tasks)
	ZOOM_connection_remove_task(c);
}

static ZOOM_record record_cache_lookup (ZOOM_resultset r,
					 int pos,
					 const char *elementSetName);

ZOOM_connection ZOOM_connection_create (ZOOM_options options)
{
    ZOOM_connection c = (ZOOM_connection) xmalloc (sizeof(*c));

    c->cs = 0;
    c->mask = 0;
    c->state = STATE_IDLE;
    c->error = ZOOM_ERROR_NONE;
    c->addinfo = 0;
    c->buf_in = 0;
    c->len_in = 0;
    c->buf_out = 0;
    c->len_out = 0;
    c->resultsets = 0;

    c->options = ZOOM_options_create_with_parent(options);

    c->host_port = 0;
    c->proxy = 0;

    c->cookie_out = 0;
    c->cookie_in = 0;
    c->tasks = 0;

    c->odr_in = odr_createmem (ODR_DECODE);
    c->odr_out = odr_createmem (ODR_ENCODE);

    c->async = 0;
    c->support_named_resultsets = 0;
    c->last_event = ZOOM_EVENT_NONE;

    c->m_queue_front = 0;
    c->m_queue_back = 0;
    return c;
}

/* set database names. Take local databases (if set); otherwise
   take databases given in ZURL (if set); otherwise use Default */
static char **set_DatabaseNames (ZOOM_connection con, ZOOM_options options,
                                 int *num)
{
    char **databaseNames;
    const char *c;
    int no = 2;
    const char *cp = ZOOM_options_get (options, "databaseName");
    
    if (!cp || !*cp)
    {
	cp = strchr (con->host_port, '/');
	if (cp)
	    cp++;
	}
    if (cp)
    {
	c = cp;
	while ((c = strchr(c, '+')))
	{
	    c++;
	    no++;
	}
    }
    else
	cp = "Default";
    databaseNames = (char**)
        odr_malloc (con->odr_out, no * sizeof(*databaseNames));
    no = 0;
    while (*cp)
    {
	c = strchr (cp, '+');
	if (!c)
	    c = cp + strlen(cp);
	else if (c == cp)
	{
	    cp++;
	    continue;
	}
	/* cp ptr to first char of db name, c is char
	   following db name */
	databaseNames[no] = (char*) odr_malloc (con->odr_out, 1+c-cp);
	memcpy (databaseNames[no], cp, c-cp);
	databaseNames[no++][c-cp] = '\0';
	cp = c;
	if (*cp)
	    cp++;
    }
    databaseNames[no] = NULL;
    *num = no;
    return databaseNames;
}

ZOOM_connection ZOOM_connection_new (const char *host, int portnum)
{
    ZOOM_connection c = ZOOM_connection_create (0);

    ZOOM_connection_connect (c, host, portnum);
    return c;
}

void ZOOM_connection_connect(ZOOM_connection c,
			      const char *host, int portnum)
{
    const char *val;
    ZOOM_task task;

    val = ZOOM_options_get (c->options, "proxy");
    if (val && *val)
	c->proxy = xstrdup (val);
    else
	c->proxy = 0;

    if (portnum)
    {
	char hostn[128];
	sprintf (hostn, "%.80s:%d", host, portnum);
	c->host_port = xstrdup(hostn);
    }
    else
	c->host_port = xstrdup(host);

    ZOOM_options_set(c->options, "host", c->host_port);

    c->async = ZOOM_options_get_bool (c->options, "async", 0);
 
    c->error = ZOOM_ERROR_NONE;

    task = ZOOM_connection_add_task (c, ZOOM_TASK_CONNECT);

    if (!c->async)
    {
	while (ZOOM_event (1, &c))
	    ;
    }
}

ZOOM_query ZOOM_query_create(void)
{
    ZOOM_query s = (ZOOM_query) xmalloc (sizeof(*s));

    s->refcount = 1;
    s->query = 0;
    s->sort_spec = 0;
    s->odr = odr_createmem (ODR_ENCODE);

    return s;
}

void ZOOM_query_destroy(ZOOM_query s)
{
    if (!s)
	return;

    (s->refcount)--;
    yaz_log (LOG_DEBUG, "ZOOM_query_destroy count=%d", s->refcount);
    if (s->refcount == 0)
    {
	odr_destroy (s->odr);
	xfree (s);
    }
}

int ZOOM_query_prefix(ZOOM_query s, const char *str)
{
    s->query = (Z_Query *) odr_malloc (s->odr, sizeof(*s->query));
    s->query->which = Z_Query_type_1;
    s->query->u.type_1 =  p_query_rpn(s->odr, PROTO_Z3950, str);
    if (!s->query->u.type_1)
	return -1;
    return 0;
}

int ZOOM_query_sortby(ZOOM_query s, const char *criteria)
{
    s->sort_spec = yaz_sort_spec (s->odr, criteria);
    if (!s->sort_spec)
	return -1;
    return 0;
}

static int do_write(ZOOM_connection c);

void ZOOM_connection_destroy(ZOOM_connection c)
{
    ZOOM_resultset r;
    if (!c)
	return;
    if (c->cs)
	cs_close (c->cs);
    for (r = c->resultsets; r; r = r->next)
	r->connection = 0;

    xfree (c->buf_in);
    xfree (c->addinfo);
    odr_destroy (c->odr_in);
    odr_destroy (c->odr_out);
    ZOOM_options_destroy (c->options);
    ZOOM_connection_remove_tasks (c);
    xfree (c->host_port);
    xfree (c);
}

void ZOOM_resultset_addref (ZOOM_resultset r)
{
    if (r)
	(r->refcount)++;
}
ZOOM_resultset ZOOM_resultset_create ()
{
    ZOOM_resultset r = (ZOOM_resultset) xmalloc (sizeof(*r));

    r->refcount = 1;
    r->size = 0;
    r->odr = odr_createmem (ODR_ENCODE);
    r->start = 0;
    r->piggyback = 1;
    r->setname = 0;
    r->count = 0;
    r->record_cache = 0;
    r->r_sort_spec = 0;
    r->r_query = 0;
    r->search = 0;
    r->connection = 0;
    r->next = 0;
    return r;
}

ZOOM_resultset ZOOM_connection_search_pqf(ZOOM_connection c, const char *q)
{
    ZOOM_resultset r;
    ZOOM_query s = ZOOM_query_create();

    ZOOM_query_prefix (s, q);

    r = ZOOM_connection_search (c, s);
    ZOOM_query_destroy (s);
    return r;
}

ZOOM_resultset ZOOM_connection_search(ZOOM_connection c, ZOOM_query q)
{
    ZOOM_resultset r = ZOOM_resultset_create ();
    ZOOM_task task;
    const char *cp;

    r->r_sort_spec = q->sort_spec;
    r->r_query = q->query;
    r->search = q;

    r->options = ZOOM_options_create_with_parent(c->options);

    r->start = ZOOM_options_get_int(r->options, "start", 0);
    r->count = ZOOM_options_get_int(r->options, "count", 0);
    r->piggyback = ZOOM_options_get_bool (r->options, "piggyback", 1);
    cp = ZOOM_options_get (r->options, "setname");
    if (cp)
        r->setname = xstrdup (cp);
    
    r->connection = c;

    r->next = c->resultsets;
    c->resultsets = r;

    task = ZOOM_connection_add_task (c, ZOOM_TASK_SEARCH);
    task->u.search.resultset = r;
    ZOOM_resultset_addref (r);  

    (q->refcount)++;

    if (!c->async)
    {
	while (ZOOM_event (1, &c))
	    ;
    }
    return r;
}

void ZOOM_resultset_destroy(ZOOM_resultset r)
{
    if (!r)
        return;
    (r->refcount)--;
    yaz_log (LOG_DEBUG, "destroy r = %p count=%d", r, r->refcount);
    if (r->refcount == 0)
    {
        ZOOM_record_cache rc;

        for (rc = r->record_cache; rc; rc = rc->next)
            if (rc->rec.wrbuf_marc)
                wrbuf_free (rc->rec.wrbuf_marc, 1);
	if (r->connection)
	{
	    /* remove ourselves from the resultsets in connection */
	    ZOOM_resultset *rp = &r->connection->resultsets;
	    while (1)
	    {
		assert (*rp);   /* we must be in this list!! */
		if (*rp == r)
		{   /* OK, we're here - take us out of it */
		    *rp = (*rp)->next;
		    break;
		}
		rp = &(*rp)->next;
	    }
	}
	ZOOM_query_destroy (r->search);
	ZOOM_options_destroy (r->options);
	odr_destroy (r->odr);
        xfree (r->setname);
	xfree (r);
    }
}

size_t ZOOM_resultset_size (ZOOM_resultset r)
{
    return r->size;
}

static void do_close (ZOOM_connection c)
{
    if (c->cs)
	cs_close(c->cs);
    c->cs = 0;
    c->mask = 0;
    c->state = STATE_IDLE;
}

static void ZOOM_resultset_retrieve (ZOOM_resultset r,
				      int force_sync, int start, int count)
{
    ZOOM_task task;
    ZOOM_connection c;

    if (!r)
	return;
    c = r->connection;
    if (!c)
	return;
    task = ZOOM_connection_add_task (c, ZOOM_TASK_RETRIEVE);
    task->u.retrieve.resultset = r;
    task->u.retrieve.start = start;
    task->u.retrieve.count = count;

    ZOOM_resultset_addref (r);

    if (!r->connection->async || force_sync)
	while (r->connection && ZOOM_event (1, &r->connection))
	    ;
}

void ZOOM_resultset_records (ZOOM_resultset r, ZOOM_record *recs,
			      size_t start, size_t count)
{
    int force_present = 0;

    if (!r)
	return ;
    if (count && recs)
        force_present = 1;
    ZOOM_resultset_retrieve (r, force_present, start, count);
    if (force_present)
    {
        size_t i;
        for (i = 0; i< count; i++)
            recs[i] = ZOOM_resultset_record_immediate (r, i+start);
    }
}

static int do_connect (ZOOM_connection c)
{
    void *add;
    const char *effective_host;

    if (c->proxy)
	effective_host = c->proxy;
    else
	effective_host = c->host_port;

    yaz_log (LOG_DEBUG, "do_connect host=%s", effective_host);

    assert (!c->cs);
    c->cs = cs_create_host (effective_host, 0, &add);

    if (c->cs)
    {
	int ret = cs_connect (c->cs, add);
	yaz_log (LOG_DEBUG, "cs_connect returned %d", ret);
	if (ret >= 0)
	{
	    c->state = STATE_CONNECTING; 
            c->mask = ZOOM_SELECT_EXCEPT;
            if (c->cs->io_pending & CS_WANT_WRITE)
                c->mask += ZOOM_SELECT_WRITE;
            if (c->cs->io_pending & CS_WANT_READ)
                c->mask += ZOOM_SELECT_READ;
	    return 1;
	}
    }
    c->state = STATE_IDLE;
    c->error = ZOOM_ERROR_CONNECT;
    return 0;
}

int z3950_connection_socket(ZOOM_connection c)
{
    if (c->cs)
	return cs_fileno(c->cs);
    return -1;
}

int z3950_connection_mask(ZOOM_connection c)
{
    if (c->cs)
	return c->mask;
    return 0;
}

static int encode_APDU(ZOOM_connection c, Z_APDU *a, ODR out)
{
    char str[120];

    assert (a);
    sprintf (str, "send_APDU t=%p type=%d", c, a->which);
    if (c->cookie_out)
    {
	Z_OtherInformation **oi;
	yaz_oi_APDU(a, &oi);
	yaz_oi_set_string_oidval(oi, out, VAL_COOKIE, 1, c->cookie_out);
    }
    if (!z_APDU(out, &a, 0, 0))
    {
	FILE *outf = fopen("/tmp/apdu.txt", "w");
	if (outf)
	{
	    ODR odr_pr = odr_createmem(ODR_PRINT);
	    fprintf (outf, "a=%p\n", a);
	    odr_setprint(odr_pr, outf);
	    z_APDU(odr_pr, &a, 0, 0);
	    odr_destroy(odr_pr);
	    fclose (outf);
	}
	c->error = ZOOM_ERROR_ENCODE;
	do_close (c);
	return -1;
    }
    
    return 0;
}

static int send_APDU (ZOOM_connection c, Z_APDU *a)
{
    ZOOM_Event event;
    assert (a);
    if (encode_APDU(c, a, c->odr_out))
	return -1;
    c->buf_out = odr_getbuf(c->odr_out, &c->len_out, 0);
    event = ZOOM_Event_create (ZOOM_EVENT_SEND_APDU);
    ZOOM_connection_put_event (c, event);
    odr_reset(c->odr_out);
    do_write (c);
    return 0;	
}

static int ZOOM_connection_send_init (ZOOM_connection c)
{
    const char *impname;
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_initRequest);
    Z_InitRequest *ireq = apdu->u.initRequest;
    Z_IdAuthentication *auth = (Z_IdAuthentication *)
        odr_malloc(c->odr_out, sizeof(*auth));
    const char *auth_groupId = ZOOM_options_get (c->options, "group");
    const char *auth_userId = ZOOM_options_get (c->options, "user");
    const char *auth_password = ZOOM_options_get (c->options, "pass");
    
    ODR_MASK_SET(ireq->options, Z_Options_search);
    ODR_MASK_SET(ireq->options, Z_Options_present);
    ODR_MASK_SET(ireq->options, Z_Options_scan);
    ODR_MASK_SET(ireq->options, Z_Options_sort);
    ODR_MASK_SET(ireq->options, Z_Options_extendedServices);
    ODR_MASK_SET(ireq->options, Z_Options_namedResultSets);
    
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_2);
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_3);
    
    impname = ZOOM_options_get (c->options, "implementationName");
    ireq->implementationName =
	(char *) odr_malloc (c->odr_out, 15 + (impname ? strlen(impname) : 0));
    strcpy (ireq->implementationName, "");
    if (impname)
    {
	strcat (ireq->implementationName, impname);
	strcat (ireq->implementationName, "/");
    }					       
    strcat (ireq->implementationName, "ZOOM-C/YAZ");
    
    *ireq->maximumRecordSize =
	ZOOM_options_get_int (c->options, "maximumRecordSize", 1024*1024);
    *ireq->preferredMessageSize =
	ZOOM_options_get_int (c->options, "preferredMessageSize", 1024*1024);
    
    if (auth_groupId || auth_password)
    {
	Z_IdPass *pass = (Z_IdPass *) odr_malloc(c->odr_out, sizeof(*pass));
	int i = 0;
	pass->groupId = 0;
	if (auth_groupId && *auth_groupId)
	{
	    pass->groupId = (char *)
                odr_malloc(c->odr_out, strlen(auth_groupId)+1);
	    strcpy(pass->groupId, auth_groupId);
	    i++;
	}
	pass->userId = 0;
	if (auth_userId && *auth_userId)
	{
	    pass->userId = (char *)
                odr_malloc(c->odr_out, strlen(auth_userId)+1);
	    strcpy(pass->userId, auth_userId);
	    i++;
	}
	pass->password = 0;
	if (auth_password && *auth_password)
	{
	    pass->password = (char *)
                odr_malloc(c->odr_out, strlen(auth_password)+1);
	    strcpy(pass->password, auth_password);
	    i++;
	}
	if (i)
	{
	    auth->which = Z_IdAuthentication_idPass;
	    auth->u.idPass = pass;
	    ireq->idAuthentication = auth;
	}
    }
    else if (auth_userId)
    {
	auth->which = Z_IdAuthentication_open;
	auth->u.open = (char *)
            odr_malloc(c->odr_out, strlen(auth_userId)+1);
	strcpy(auth->u.open, auth_userId);
	ireq->idAuthentication = auth;
    }
    if (c->proxy)
	yaz_oi_set_string_oidval(&ireq->otherInfo, c->odr_out,
				 VAL_PROXY, 1, c->host_port);
    assert (apdu);
    send_APDU (c, apdu);
    
    return 0;
}

static int ZOOM_connection_send_search (ZOOM_connection c)
{
    ZOOM_resultset r;
    int lslb, ssub, mspn;
    const char *syntax;
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_searchRequest);
    Z_SearchRequest *search_req = apdu->u.searchRequest;
    const char *elementSetName;
    const char *smallSetElementSetName;
    const char *mediumSetElementSetName;
    const char *schema;

    assert (c->tasks);
    assert (c->tasks->which == ZOOM_TASK_SEARCH);

    r = c->tasks->u.search.resultset;

    elementSetName =
	ZOOM_options_get (r->options, "elementSetName");
    smallSetElementSetName  =
	ZOOM_options_get (r->options, "smallSetElementSetName");
    mediumSetElementSetName =
	ZOOM_options_get (r->options, "mediumSetElementSetName");
    schema =
	ZOOM_options_get (r->options, "schema");

    if (!smallSetElementSetName)
	smallSetElementSetName = elementSetName;

    if (!mediumSetElementSetName)
	mediumSetElementSetName = elementSetName;

    assert (r);
    assert (r->r_query);

    /* prepare query for the search request */
    search_req->query = r->r_query;

    search_req->databaseNames =
	set_DatabaseNames (c, r->options, &search_req->num_databaseNames);

    /* get syntax (no need to provide unless piggyback is in effect) */
    syntax = ZOOM_options_get (r->options, "preferredRecordSyntax");

    lslb = ZOOM_options_get_int (r->options, "largeSetLowerBound", -1);
    ssub = ZOOM_options_get_int (r->options, "smallSetUpperBound", -1);
    mspn = ZOOM_options_get_int (r->options, "mediumSetPresentNumber", -1);
    if (lslb != -1 && ssub != -1 && mspn != -1)
    {
	/* So're a Z39.50 expert? Let's hope you don't do sort */
	*search_req->largeSetLowerBound = lslb;
	*search_req->smallSetUpperBound = ssub;
	*search_req->mediumSetPresentNumber = mspn;
    }
    else if (r->start == 0 && r->count > 0
	     && r->piggyback && !r->r_sort_spec && !schema)
    {
	/* Regular piggyback - do it unless we're going to do sort */
	*search_req->largeSetLowerBound = 2000000000;
	*search_req->smallSetUpperBound = r->count;
	*search_req->mediumSetPresentNumber = r->count;
	smallSetElementSetName = 0;  /* no need to provide this */
    }
    else
    {
	/* non-piggyback. Need not provide elementsets or syntaxes .. */
	smallSetElementSetName = 0;
	mediumSetElementSetName = 0;
	syntax = 0;
    }
    if (smallSetElementSetName && *smallSetElementSetName)
    {
	Z_ElementSetNames *esn = (Z_ElementSetNames *)
            odr_malloc (c->odr_out, sizeof(*esn));
	
	esn->which = Z_ElementSetNames_generic;
	esn->u.generic = odr_strdup (c->odr_out, smallSetElementSetName);
	search_req->smallSetElementSetNames = esn;
    }
    if (mediumSetElementSetName && *mediumSetElementSetName)
    {
	Z_ElementSetNames *esn = (Z_ElementSetNames *)
            odr_malloc (c->odr_out, sizeof(*esn));
	
	esn->which = Z_ElementSetNames_generic;
	esn->u.generic = odr_strdup (c->odr_out, mediumSetElementSetName);
	search_req->mediumSetElementSetNames = esn;
    }
    if (syntax)
	search_req->preferredRecordSyntax =
	    yaz_str_to_z3950oid (c->odr_out, CLASS_RECSYN, syntax);
    
    if (!r->setname)
    {
        if (c->support_named_resultsets)
        {
            char setname[14];
            int ord;
            /* find the lowest unused ordinal so that we re-use
               result sets on the server. */
            for (ord = 1; ; ord++)
            {
                ZOOM_resultset rp;
                sprintf (setname, "%d", ord);
                for (rp = c->resultsets; rp; rp = rp->next)
                    if (rp->setname && !strcmp (rp->setname, setname))
                        break;
                if (!rp)
                    break;
            }
            r->setname = xstrdup (setname);
            yaz_log (LOG_DEBUG, "allocating %s", r->setname);
        }
        else
            r->setname = xstrdup ("default");
        ZOOM_options_set (r->options, "setname", r->setname);
    }
    search_req->resultSetName = odr_strdup(c->odr_out, r->setname);
    /* send search request */
    send_APDU (c, apdu);
    r->r_query = 0;
    return 1;
}

static void response_diag (ZOOM_connection c, Z_DiagRec *p)
{
    Z_DefaultDiagFormat *r;
    char *addinfo = 0;
    
    xfree (c->addinfo);
    c->addinfo = 0;
    if (p->which != Z_DiagRec_defaultFormat)
    {
	c->error = ZOOM_ERROR_DECODE;
	return;
    }
    r = p->u.defaultFormat;
    switch (r->which)
    {
    case Z_DefaultDiagFormat_v2Addinfo:
	addinfo = r->u.v2Addinfo;
	break;
    case Z_DefaultDiagFormat_v3Addinfo:
	addinfo = r->u.v3Addinfo;
	break;
    }
    if (addinfo)
	c->addinfo = xstrdup (addinfo);
    c->error = *r->condition;
}

ZOOM_record ZOOM_record_clone (ZOOM_record srec)
{
    char *buf;
    int size;
    ODR odr_enc;
    ZOOM_record nrec;

    odr_enc = odr_createmem(ODR_ENCODE);
    if (!z_NamePlusRecord (odr_enc, &srec->npr, 0, 0))
	return 0;
    buf = odr_getbuf (odr_enc, &size, 0);
    
    nrec = (ZOOM_record) xmalloc (sizeof(*nrec));
    nrec->odr = odr_createmem(ODR_DECODE);
    nrec->wrbuf_marc = 0;
    odr_setbuf (nrec->odr, buf, size, 0);
    z_NamePlusRecord (nrec->odr, &nrec->npr, 0, 0);
    
    odr_destroy (odr_enc);
    return nrec;
}

ZOOM_record ZOOM_resultset_record_immediate (ZOOM_resultset s,size_t pos)
{
    return record_cache_lookup (s, pos, 0);
}

ZOOM_record ZOOM_resultset_record (ZOOM_resultset r, size_t pos)
{
    ZOOM_resultset_retrieve (r, 1, pos, 1);
    return ZOOM_resultset_record_immediate (r, pos);
}

void ZOOM_record_destroy (ZOOM_record rec)
{
    if (!rec)
	return;
    if (rec->wrbuf_marc)
	wrbuf_free (rec->wrbuf_marc, 1);
    odr_destroy (rec->odr);
    xfree (rec);
}

const char *ZOOM_record_get (ZOOM_record rec, const char *type, int *len)
{
    Z_NamePlusRecord *npr;
    if (!rec)
	return 0;
    npr = rec->npr;
    if (!npr)
	return 0;
    if (!strcmp (type, "database"))
    {
	return npr->databaseName;
    }
    else if (!strcmp (type, "syntax"))
    {
	if (npr->which == Z_NamePlusRecord_databaseRecord)
	{
	    Z_External *r = (Z_External *) npr->u.databaseRecord;
	    oident *ent = oid_getentbyoid(r->direct_reference);
	    if (ent)
		return ent->desc;
	}
	return "none";
    }
    else if (!strcmp (type, "render") && 
             npr->which == Z_NamePlusRecord_databaseRecord)
    {
        Z_External *r = (Z_External *) npr->u.databaseRecord;
        oident *ent = oid_getentbyoid(r->direct_reference);
        
        if (r->which == Z_External_sutrs)
        {
            *len = r->u.sutrs->len;
            return (const char *) r->u.sutrs->buf;
        }
        else if (r->which == Z_External_octet)
        {
            switch (ent->value)
            {
            case VAL_SOIF:
            case VAL_HTML:
            case VAL_SUTRS:
                break;
            case VAL_TEXT_XML:
            case VAL_APPLICATION_XML:
                break;
            default:
                if (!rec->wrbuf_marc)
                    rec->wrbuf_marc = wrbuf_alloc();
                wrbuf_rewind (rec->wrbuf_marc);
                if (yaz_marc_decode ((const char *)
                                     r->u.octet_aligned->buf,
                                     rec->wrbuf_marc, 0,
                                     r->u.octet_aligned->len,
                                     0) > 0)
                {
                    *len = wrbuf_len(rec->wrbuf_marc);
                    return wrbuf_buf(rec->wrbuf_marc);
                }
            }
            *len = r->u.octet_aligned->len;
            return (const char *) r->u.octet_aligned->buf;
        }
        else if (r->which == Z_External_grs1)
        {
            *len = 5;
            return "GRS-1";
        }
	return 0;
    }
    else if (!strcmp (type, "xml") && 
             npr->which == Z_NamePlusRecord_databaseRecord)
    {
        Z_External *r = (Z_External *) npr->u.databaseRecord;
        oident *ent = oid_getentbyoid(r->direct_reference);
        
        if (r->which == Z_External_sutrs)
        {
            *len = r->u.sutrs->len;
            return (const char *) r->u.sutrs->buf;
        }
        else if (r->which == Z_External_octet)
        {
            switch (ent->value)
            {
            case VAL_SOIF:
            case VAL_HTML:
            case VAL_SUTRS:
                break;
            case VAL_TEXT_XML:
            case VAL_APPLICATION_XML:
                break;
            default:
                if (!rec->wrbuf_marc)
                    rec->wrbuf_marc = wrbuf_alloc();
                wrbuf_rewind (rec->wrbuf_marc);
                if (yaz_marc_decode ((const char *)
                                     r->u.octet_aligned->buf,
                                     rec->wrbuf_marc, 0,
                                     r->u.octet_aligned->len,
                                     1) > 0)
                {
                    *len = wrbuf_len(rec->wrbuf_marc);
                    return wrbuf_buf(rec->wrbuf_marc);
                }
            }
            *len = r->u.octet_aligned->len;
            return (const char *) r->u.octet_aligned->buf;
        }
        else if (r->which == Z_External_grs1)
        {
            *len = 5;
            return "GRS-1";
        }
	return 0;
    }
    else if (!strcmp (type, "raw"))
    {
	if (npr->which == Z_NamePlusRecord_databaseRecord)
	{
	    Z_External *r = (Z_External *) npr->u.databaseRecord;
	    
	    if (r->which == Z_External_sutrs)
	    {
		*len = r->u.sutrs->len;
		return (const char *) r->u.sutrs->buf;
	    }
	    else if (r->which == Z_External_octet)
	    {
		*len = r->u.octet_aligned->len;
		return (const char *) r->u.octet_aligned->buf;
	    }
	    else /* grs-1, explain, ... */
	    {
		*len = -1;
                return (const char *) npr->u.databaseRecord;
	    }
	}
	return 0;
    }
    return 0;
}

static void record_cache_add (ZOOM_resultset r,
			      Z_NamePlusRecord *npr,
			      int pos,
			      const char *elementSetName)
{
    ZOOM_record_cache rc;

    for (rc = r->record_cache; rc; rc = rc->next)
    {
	if (pos == rc->pos)
	{
	    if ((!elementSetName && !rc->elementSetName)
		|| (elementSetName && rc->elementSetName &&
		    !strcmp (elementSetName, rc->elementSetName)))
	    {
		/* not destroying rc->npr (it's handled by nmem )*/
		rc->rec.npr = npr;
		/* keeping wrbuf_marc too */
		return;
	    }
	}
    }
    rc = (ZOOM_record_cache) odr_malloc (r->odr, sizeof(*rc));
    rc->rec.npr = npr; 
    rc->rec.odr = 0;
    rc->rec.wrbuf_marc = 0;
    if (elementSetName)
	rc->elementSetName = odr_strdup (r->odr, elementSetName);
    else
	rc->elementSetName = 0;
    rc->pos = pos;
    rc->next = r->record_cache;
    r->record_cache = rc;
}

static ZOOM_record record_cache_lookup (ZOOM_resultset r,
					 int pos,
					 const char *elementSetName)
{
    ZOOM_record_cache rc;

    for (rc = r->record_cache; rc; rc = rc->next)
    {
	if (pos == rc->pos)
	{
	    if ((!elementSetName && !rc->elementSetName)
		|| (elementSetName && rc->elementSetName &&
		    !strcmp (elementSetName, rc->elementSetName)))
		return &rc->rec;
	}
    }
    return 0;
}
					     
static void handle_records (ZOOM_connection c, Z_Records *sr,
			    int present_phase)
{
    ZOOM_resultset resultset;

    if (!c->tasks)
	return ;
    switch (c->tasks->which)
    {
    case ZOOM_TASK_SEARCH:
        resultset = c->tasks->u.search.resultset;
        break;
    case ZOOM_TASK_RETRIEVE:
        resultset = c->tasks->u.retrieve.resultset;        
	break;
    default:
        return;
    }
    if (sr && sr->which == Z_Records_NSD)
    {
	Z_DiagRec dr, *dr_p = &dr;
	dr.which = Z_DiagRec_defaultFormat;
	dr.u.defaultFormat = sr->u.nonSurrogateDiagnostic;
	
	response_diag (c, dr_p);
    }
    else if (sr && sr->which == Z_Records_multipleNSD)
    {
	if (sr->u.multipleNonSurDiagnostics->num_diagRecs >= 1)
	    response_diag(c, sr->u.multipleNonSurDiagnostics->diagRecs[0]);
	else
	    c->error = ZOOM_ERROR_DECODE;
    }
    else 
    {
	if (resultset->count + resultset->start > resultset->size)
	    resultset->count = resultset->size - resultset->start;
	if (resultset->count < 0)
	    resultset->count = 0;
	if (sr && sr->which == Z_Records_DBOSD)
	{
	    int i;
	    NMEM nmem = odr_extract_mem (c->odr_in);
	    Z_NamePlusRecordList *p =
		sr->u.databaseOrSurDiagnostics;
	    for (i = 0; i<p->num_records; i++)
	    {
		record_cache_add (resultset, p->records[i],
				  i+ resultset->start, 0);
	    }
	    /* transfer our response to search_nmem .. we need it later */
	    nmem_transfer (resultset->odr->mem, nmem);
	    nmem_destroy (nmem);
	    if (present_phase && p->num_records == 0)
	    {
		/* present response and we didn't get any records! */
		c->error = ZOOM_ERROR_DECODE;
	    }
	}
	else if (present_phase)
	{
	    /* present response and we didn't get any records! */
	    c->error = ZOOM_ERROR_DECODE;
	}
    }
}

static void handle_present_response (ZOOM_connection c, Z_PresentResponse *pr)
{
    handle_records (c, pr->records, 1);
}

static void handle_search_response (ZOOM_connection c, Z_SearchResponse *sr)
{
    ZOOM_resultset resultset;

    yaz_log (LOG_DEBUG, "got search response");

    if (!c->tasks || c->tasks->which != ZOOM_TASK_SEARCH)
	return ;

    resultset = c->tasks->u.search.resultset;

    resultset->size = *sr->resultCount;
    handle_records (c, sr->records, 0);
}

static void sort_response (ZOOM_connection c, Z_SortResponse *res)
{
    if (res->diagnostics && res->num_diagnostics > 0)
	response_diag (c, res->diagnostics[0]);
}

static int scan_response (ZOOM_connection c, Z_ScanResponse *res)
{
    NMEM nmem = odr_extract_mem (c->odr_in);
    ZOOM_scanset scan;

    if (!c->tasks || c->tasks->which != ZOOM_TASK_SCAN)
        return 0;
    scan = c->tasks->u.scan.scan;

    if (res->entries && res->entries->nonsurrogateDiagnostics)
        response_diag(c, res->entries->nonsurrogateDiagnostics[0]);
    scan->scan_response = res;
    nmem_transfer (scan->odr->mem, nmem);
    if (res->stepSize)
        ZOOM_options_set_int (scan->options, "stepSize", *res->stepSize);
    if (res->positionOfTerm)
        ZOOM_options_set_int (scan->options, "position", *res->positionOfTerm);
    if (res->scanStatus)
        ZOOM_options_set_int (scan->options, "scanStatus", *res->scanStatus);
    if (res->numberOfEntriesReturned)
        ZOOM_options_set_int (scan->options, "number",
                              *res->numberOfEntriesReturned);
    nmem_destroy (nmem);
    return 1;
}

static int send_sort (ZOOM_connection c)
{
    ZOOM_resultset  resultset;

    if (!c->tasks || c->tasks->which != ZOOM_TASK_SEARCH)
	return 0;

    resultset = c->tasks->u.search.resultset;

    if (c->error)
    {
	resultset->r_sort_spec = 0;
	return 0;
    }
    if (resultset->r_sort_spec)
    {
	Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_sortRequest);
	Z_SortRequest *req = apdu->u.sortRequest;
	
	req->num_inputResultSetNames = 1;
	req->inputResultSetNames = (Z_InternationalString **)
	    odr_malloc (c->odr_out, sizeof(*req->inputResultSetNames));
	req->inputResultSetNames[0] =
            odr_strdup (c->odr_out, resultset->setname);
	req->sortedResultSetName = odr_strdup (c->odr_out, resultset->setname);
	req->sortSequence = resultset->r_sort_spec;
	resultset->r_sort_spec = 0;
	send_APDU (c, apdu);
	return 1;
    }
    return 0;
}

static int send_present (ZOOM_connection c)
{
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_presentRequest);
    Z_PresentRequest *req = apdu->u.presentRequest;
    int i = 0;
    const char *syntax = 
	ZOOM_options_get (c->options, "preferredRecordSyntax");
    const char *element =
	ZOOM_options_get (c->options, "elementSetName");
    const char *schema =
	ZOOM_options_get (c->options, "schema");
    ZOOM_resultset  resultset;

    if (!c->tasks)
	return 0;

    switch (c->tasks->which)
    {
    case ZOOM_TASK_SEARCH:
        resultset = c->tasks->u.search.resultset;
        break;
    case ZOOM_TASK_RETRIEVE:
        resultset = c->tasks->u.retrieve.resultset;
        resultset->start = c->tasks->u.retrieve.start;
        resultset->count = c->tasks->u.retrieve.count;

        if (resultset->start >= resultset->size)
            return 0;
        if (resultset->start + resultset->count > resultset->size)
            resultset->count = resultset->size - resultset->start;
	break;
    default:
        return 0;
    }

    if (c->error)                  /* don't continue on error */
	return 0;
    if (resultset->start < 0)
	return 0;
    for (i = 0; i<resultset->count; i++)
    {
	ZOOM_record rec =
	    record_cache_lookup (resultset, i + resultset->start, 0);
	if (!rec)
	    break;
    }
    if (i == resultset->count)
	return 0;

    resultset->start += i;
    resultset->count -= i;
    *req->resultSetStartPoint = resultset->start + 1;
    *req->numberOfRecordsRequested = resultset->count;
    assert (*req->numberOfRecordsRequested > 0);

    if (syntax && *syntax)
	req->preferredRecordSyntax =
	    yaz_str_to_z3950oid (c->odr_out, CLASS_RECSYN, syntax);

    if (schema && *schema)
    {
	Z_RecordComposition *compo = (Z_RecordComposition *)
            odr_malloc (c->odr_out, sizeof(*compo));

        req->recordComposition = compo;
        compo->which = Z_RecordComp_complex;
        compo->u.complex = (Z_CompSpec *)
            odr_malloc(c->odr_out, sizeof(*compo->u.complex));
        compo->u.complex->selectAlternativeSyntax = (bool_t *) 
            odr_malloc(c->odr_out, sizeof(bool_t));
        *compo->u.complex->selectAlternativeSyntax = 0;

        compo->u.complex->generic = (Z_Specification *)
            odr_malloc(c->odr_out, sizeof(*compo->u.complex->generic));

        compo->u.complex->generic->schema = (Odr_oid *)
            yaz_str_to_z3950oid (c->odr_out, CLASS_SCHEMA, schema);

        if (!compo->u.complex->generic->schema)
        {
            /* OID wasn't a schema! Try record syntax instead. */

            compo->u.complex->generic->schema = (Odr_oid *)
                yaz_str_to_z3950oid (c->odr_out, CLASS_RECSYN, schema);
        }
        if (element && *element)
        {
            compo->u.complex->generic->elementSpec = (Z_ElementSpec *)
                odr_malloc(c->odr_out, sizeof(Z_ElementSpec));
            compo->u.complex->generic->elementSpec->which =
                Z_ElementSpec_elementSetName;
            compo->u.complex->generic->elementSpec->u.elementSetName =
                odr_strdup (c->odr_out, element);
        }
        else
            compo->u.complex->generic->elementSpec = 0;
        compo->u.complex->num_dbSpecific = 0;
        compo->u.complex->dbSpecific = 0;
        compo->u.complex->num_recordSyntax = 0;
        compo->u.complex->recordSyntax = 0;
    }
    else if (element && *element)
    {
	Z_ElementSetNames *esn = (Z_ElementSetNames *)
            odr_malloc (c->odr_out, sizeof(*esn));
	Z_RecordComposition *compo = (Z_RecordComposition *)
            odr_malloc (c->odr_out, sizeof(*compo));
	
	esn->which = Z_ElementSetNames_generic;
	esn->u.generic = odr_strdup (c->odr_out, element);
	compo->which = Z_RecordComp_simple;
	compo->u.simple = esn;
	req->recordComposition = compo;
    }
    req->resultSetId = odr_strdup(c->odr_out, resultset->setname);
    send_APDU (c, apdu);
    return 1;
}

ZOOM_scanset ZOOM_connection_scan (ZOOM_connection c, const char *start)
{
    ZOOM_scanset scan = (ZOOM_scanset) xmalloc (sizeof(*scan));

    scan->connection = c;
    scan->odr = odr_createmem (ODR_DECODE);
    scan->options = ZOOM_options_create_with_parent (c->options);
    scan->refcount = 1;
    scan->scan_response = 0;

    if ((scan->termListAndStartPoint =
         p_query_scan(scan->odr, PROTO_Z3950, &scan->attributeSet,
                      start)))
    {
        ZOOM_task task = ZOOM_connection_add_task (c, ZOOM_TASK_SCAN);
        task->u.scan.scan = scan;
        
        (scan->refcount)++;
        if (!c->async)
        {
            while (ZOOM_event (1, &c))
                ;
        }
    }
    return scan;
}

void ZOOM_scanset_destroy (ZOOM_scanset scan)
{
    if (!scan)
        return;
    (scan->refcount)--;
    if (scan->refcount == 0)
    {
        odr_destroy (scan->odr);
        
        ZOOM_options_destroy (scan->options);
        xfree (scan);
    }
}

int send_scan (ZOOM_connection c)
{
    ZOOM_scanset scan;
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_scanRequest);
    Z_ScanRequest *req = apdu->u.scanRequest;
    if (!c->tasks)
        return 0;
    assert (c->tasks->which == ZOOM_TASK_SCAN);
    scan = c->tasks->u.scan.scan;

    req->termListAndStartPoint = scan->termListAndStartPoint;
    req->attributeSet = scan->attributeSet;

    *req->numberOfTermsRequested =
        ZOOM_options_get_int(scan->options, "number", 10);

    req->preferredPositionInResponse =
        odr_intdup (c->odr_out,
                    ZOOM_options_get_int(scan->options, "position", 1));

    req->stepSize =
        odr_intdup (c->odr_out,
                    ZOOM_options_get_int(scan->options, "stepSize", 0));
    
    req->databaseNames = set_DatabaseNames (c, scan->options, 
                                            &req->num_databaseNames);

    send_APDU (c, apdu);

    return 1;
}

size_t ZOOM_scanset_size (ZOOM_scanset scan)
{
    if (!scan || !scan->scan_response || !scan->scan_response->entries)
        return 0;
    return scan->scan_response->entries->num_entries;
}

const char *ZOOM_scanset_term (ZOOM_scanset scan, size_t pos,
                               int *occ, int *len)
{
    const char *term = 0;
    size_t noent = ZOOM_scanset_size (scan);
    Z_ScanResponse *res = scan->scan_response;
    
    *len = 0;
    *occ = 0;
    if (pos >= noent)
        return 0;
    if (res->entries->entries[pos]->which == Z_Entry_termInfo)
    {
        Z_TermInfo *t = res->entries->entries[pos]->u.termInfo;
        
        if (t->term->which == Z_Term_general)
        {
            term = (const char *) t->term->u.general->buf;
            *len = t->term->u.general->len;
        }
        *occ = t->globalOccurrences ? *t->globalOccurrences : 0;
    }
    return term;
}

const char *ZOOM_scanset_option_get (ZOOM_scanset scan, const char *key)
{
    return ZOOM_options_get (scan->options, key);
}

void ZOOM_scanset_option_set (ZOOM_scanset scan, const char *key,
                              const char *val)
{
    ZOOM_options_set (scan->options, key, val);
}

static int ZOOM_connection_exec_task (ZOOM_connection c)
{
    ZOOM_task task = c->tasks;

    yaz_log (LOG_DEBUG, "ZOOM_connection_exec_task");
    if (!task)
	return 0;
    if (c->error != ZOOM_ERROR_NONE ||
        (!c->cs && task->which != ZOOM_TASK_CONNECT))
    {
	ZOOM_connection_remove_tasks (c);
	return 0;
    }
    yaz_log (LOG_DEBUG, "ZOOM_connection_exec_task type=%d", task->which);
    if (task->running)
	return 0;
    task->running = 1;
    switch (task->which)
    {
    case ZOOM_TASK_SEARCH:
	/* see if search hasn't been sent yet. */
	if (ZOOM_connection_send_search (c))
	    return 1;
	break;
    case ZOOM_TASK_RETRIEVE:
	if (send_present (c))
	    return 1;
	break;
    case ZOOM_TASK_CONNECT:
        if (do_connect(c))
            return 1;
        break;
    case ZOOM_TASK_SCAN:
        if (send_scan(c))
            return 1;
    }
    ZOOM_connection_remove_task (c);
    return 0;
}

static int send_sort_present (ZOOM_connection c)
{
    int r = send_sort (c);
    if (!r)
	r = send_present (c);
    return r;
}

static void handle_apdu (ZOOM_connection c, Z_APDU *apdu)
{
    Z_InitResponse *initrs;
    
    yaz_log (LOG_DEBUG, "hande_apdu type=%d", apdu->which);
    c->mask = 0;
    switch(apdu->which)
    {
    case Z_APDU_initResponse:
	initrs = apdu->u.initResponse;
	if (!*initrs->result)
	{
	    c->error = ZOOM_ERROR_INIT;
	}
	else
	{
	    char *cookie =
		yaz_oi_get_string_oidval (&apdu->u.initResponse->otherInfo,
					  VAL_COOKIE, 1, 0);
	    xfree (c->cookie_in);
	    c->cookie_in = 0;
	    if (cookie)
		c->cookie_in = xstrdup(cookie);
            if (ODR_MASK_GET(initrs->options, Z_Options_namedResultSets) &&
                ODR_MASK_GET(initrs->protocolVersion, Z_ProtocolVersion_3))
                c->support_named_resultsets = 1;
            if (c->tasks)
            {
                assert (c->tasks->which == ZOOM_TASK_CONNECT);
                ZOOM_connection_remove_task (c);
            }
	    ZOOM_connection_exec_task (c);
	}
	break;
    case Z_APDU_searchResponse:
	handle_search_response (c, apdu->u.searchResponse);
	if (!send_sort_present (c))
	    ZOOM_connection_remove_task (c);
	break;
    case Z_APDU_presentResponse:
	handle_present_response (c, apdu->u.presentResponse);
	if (!send_present (c))
	    ZOOM_connection_remove_task (c);
	break;
    case Z_APDU_sortResponse:
	sort_response (c, apdu->u.sortResponse);
	if (!send_present (c))
	    ZOOM_connection_remove_task (c);
        break;
    case Z_APDU_scanResponse:
        scan_response (c, apdu->u.scanResponse);
        ZOOM_connection_remove_task (c);
    }
}

static int do_read (ZOOM_connection c)
{
    int r;
    Z_APDU *apdu;
    ZOOM_Event event;
    
    event = ZOOM_Event_create (ZOOM_EVENT_RECV_DATA);
    ZOOM_connection_put_event (c, event);
    
    r = cs_get (c->cs, &c->buf_in, &c->len_in);
    if (r == 1)
	return 0;
    if (r <= 0)
    {
	c->error= ZOOM_ERROR_CONNECTION_LOST;
	do_close (c);
    }
    else
    {
        ZOOM_Event event;
	odr_reset (c->odr_in);
	odr_setbuf (c->odr_in, c->buf_in, r, 0);
        event = ZOOM_Event_create (ZOOM_EVENT_RECV_APDU);
        ZOOM_connection_put_event (c, event);
	if (!z_APDU (c->odr_in, &apdu, 0, 0))
	{
	    c->error = ZOOM_ERROR_DECODE;
	    do_close (c);
	}
	else
	{
	    handle_apdu (c, apdu);
	}
    }
    return 1;
}

static int do_write_ex (ZOOM_connection c, char *buf_out, int len_out)
{
    int r;
    ZOOM_Event event;
    
    event = ZOOM_Event_create(ZOOM_EVENT_SEND_DATA);
    ZOOM_connection_put_event (c, event);

    if ((r=cs_put (c->cs, buf_out, len_out)) < 0)
    {
	if (c->state == STATE_CONNECTING)
	    c->error = ZOOM_ERROR_CONNECT;
	else
	    c->error = ZOOM_ERROR_CONNECTION_LOST;
	do_close (c);
	return 1;
    }
    else if (r == 1)
    {    
        c->mask = ZOOM_SELECT_EXCEPT;
        if (c->cs->io_pending & CS_WANT_WRITE)
            c->mask += ZOOM_SELECT_WRITE;
        if (c->cs->io_pending & CS_WANT_READ)
            c->mask += ZOOM_SELECT_READ;
        yaz_log (LOG_DEBUG, "do_write_ex 1 mask=%d", c->mask);
    }
    else
    {
        c->mask = ZOOM_SELECT_READ|ZOOM_SELECT_EXCEPT;
        yaz_log (LOG_DEBUG, "do_write_ex 2 mask=%d", c->mask);
    }
    return 0;
}

static int do_write(ZOOM_connection c)
{
    return do_write_ex (c, c->buf_out, c->len_out);
}


const char *ZOOM_connection_option_get (ZOOM_connection c, const char *key)
{
    return ZOOM_options_get (c->options, key);
}

void ZOOM_connection_option_set (ZOOM_connection c, const char *key,
                                  const char *val)
{
    ZOOM_options_set (c->options, key, val);
}

const char *ZOOM_resultset_option_get (ZOOM_resultset r, const char *key)
{
    return ZOOM_options_get (r->options, key);
}

void ZOOM_resultset_option_set (ZOOM_resultset r, const char *key,
                                  const char *val)
{
    ZOOM_options_set (r->options, key, val);
}


int ZOOM_connection_errcode (ZOOM_connection c)
{
    return ZOOM_connection_error (c, 0, 0);
}

const char *ZOOM_connection_errmsg (ZOOM_connection c)
{
    const char *msg;
    ZOOM_connection_error (c, &msg, 0);
    return msg;
}

const char *ZOOM_connection_addinfo (ZOOM_connection c)
{
    const char *addinfo;
    ZOOM_connection_error (c, 0, &addinfo);
    return addinfo;
}

int ZOOM_connection_error (ZOOM_connection c, const char **cp,
			    const char **addinfo)
{
    int error = c->error;
    if (cp)
    {
	switch (error)
	{
	case ZOOM_ERROR_NONE:
	    *cp = "No error"; break;
	case ZOOM_ERROR_CONNECT:
	    *cp = "Connect failed"; break;
	case ZOOM_ERROR_MEMORY:
	    *cp = "Out of memory"; break;
	case ZOOM_ERROR_ENCODE:
	    *cp = "Encoding failed"; break;
	case ZOOM_ERROR_DECODE:
	    *cp = "Decoding failed"; break;
	case ZOOM_ERROR_CONNECTION_LOST:
	    *cp = "Connection lost"; break;
	case ZOOM_ERROR_INIT:
	    *cp = "Init rejected"; break;
	case ZOOM_ERROR_INTERNAL:
	    *cp = "Internal failure"; break;
	case ZOOM_ERROR_TIMEOUT:
	    *cp = "Timeout"; break;
	default:
	    *cp = diagbib1_str (error);
	}
    }
    if (addinfo)
    {
	if (c->addinfo)
	    *addinfo = c->addinfo;
	else
	    *addinfo = "";
    }
    return c->error;
}

int ZOOM_connection_do_io(ZOOM_connection c, int mask)
{
    ZOOM_Event event = 0;
    int r = cs_look(c->cs);
    yaz_log (LOG_DEBUG, "ZOOM_connection_do_io c=%p mask=%d cs_look=%d",
	     c, mask, r);
    
    if (r == CS_NONE)
    {
        event = ZOOM_Event_create (ZOOM_EVENT_CONNECT);
	c->error = ZOOM_ERROR_CONNECT;
	do_close (c);
        ZOOM_connection_put_event (c, event);
    }
    else if (r == CS_CONNECT)
    {
        int ret;
        event = ZOOM_Event_create (ZOOM_EVENT_CONNECT);

        ret = cs_rcvconnect (c->cs);
        yaz_log (LOG_DEBUG, "cs_rcvconnect returned %d", ret);
        if (ret == 1)
        {
            c->mask = ZOOM_SELECT_EXCEPT;
            if (c->cs->io_pending & CS_WANT_WRITE)
                c->mask += ZOOM_SELECT_WRITE;
            if (c->cs->io_pending & CS_WANT_READ)
                c->mask += ZOOM_SELECT_READ;
            ZOOM_connection_put_event (c, event);
        }
        else if (ret == 0)
        {
            ZOOM_connection_put_event (c, event);
            ZOOM_connection_send_init (c);
            c->state = STATE_ESTABLISHED;
        }
        else
        {
            c->error = ZOOM_ERROR_CONNECT;
            do_close (c);
            ZOOM_connection_put_event (c, event);
        }
    }
    else
    {
        if (mask & ZOOM_SELECT_READ)
            do_read (c);
        if (c->cs && (mask & ZOOM_SELECT_WRITE))
            do_write (c);
    }
    return 1;
}

int ZOOM_connection_last_event(ZOOM_connection cs)
{
    if (!cs)
        return ZOOM_EVENT_NONE;
    return cs->last_event;
}

int ZOOM_event (int no, ZOOM_connection *cs)
{
#if HAVE_SYS_POLL_H
    struct pollfd pollfds[1024];
    ZOOM_connection poll_cs[1024];
#else
    struct timeval tv;
    fd_set input, output, except;
#endif
    int i, r, nfds;
    int max_fd = 0;

    for (i = 0; i<no; i++)
    {
	ZOOM_connection c = cs[i];
        ZOOM_Event event;
	if (c && (event = ZOOM_connection_get_event(c)))
        {
            ZOOM_Event_destroy (event);
	    return i+1;
        }
    }
    for (i = 0; i<no; i++)
    {
        ZOOM_connection c = cs[i];
        ZOOM_Event event;
        if (c && ZOOM_connection_exec_task (c))
        {
            if ((event = ZOOM_connection_get_event(c)))
            {
                ZOOM_Event_destroy (event);
                return i+1;
            }
        }
    }
#if HAVE_SYS_POLL_H

#else
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    
    FD_ZERO (&input);
    FD_ZERO (&output);
    FD_ZERO (&except);
#endif
    nfds = 0;
    for (i = 0; i<no; i++)
    {
	ZOOM_connection c = cs[i];
	int fd, mask;
	
	if (!c)
	    continue;
	fd = z3950_connection_socket(c);
	mask = z3950_connection_mask(c);

	if (fd == -1)
	    continue;
	if (max_fd < fd)
	    max_fd = fd;

#if HAVE_SYS_POLL_H
        if (mask)
        {
            short poll_events = 0;

            if (mask & ZOOM_SELECT_READ)
                poll_events += POLLIN;
            if (mask & ZOOM_SELECT_WRITE)
                poll_events += POLLOUT;
            if (mask & ZOOM_SELECT_EXCEPT)
                poll_events += POLLERR;
            pollfds[nfds].fd = fd;
            pollfds[nfds].events = poll_events;
            pollfds[nfds].revents = 0;
            poll_cs[nfds] = c;
            nfds++;
        }
#else
	if (mask & ZOOM_SELECT_READ)
	{
	    FD_SET (fd, &input);
	    nfds++;
	}
	if (mask & ZOOM_SELECT_WRITE)
	{
	    FD_SET (fd, &output);
	    nfds++;
	}
	if (mask & ZOOM_SELECT_EXCEPT)
	{
	    FD_SET (fd, &except);
	    nfds++;
	}
#endif
    }
    if (!nfds)
        return 0;
#if HAVE_SYS_POLL_H
    r = poll (pollfds, nfds, 15000);
    for (i = 0; i<nfds; i++)
    {
        ZOOM_connection c = poll_cs[i];
        if (r && c->mask)
        {
            int mask = 0;
            if (pollfds[i].revents & POLLIN)
                mask += ZOOM_SELECT_READ;
            if (pollfds[i].revents & POLLOUT)
                mask += ZOOM_SELECT_WRITE;
            if (pollfds[i].revents & POLLERR)
                mask += ZOOM_SELECT_EXCEPT;
            if (mask)
                ZOOM_connection_do_io(c, mask);
        }
        else if (r == 0 && c->mask)
        {
            ZOOM_Event event = ZOOM_Event_create(ZOOM_EVENT_TIMEOUT);
	    /* timeout and this connection was waiting */
	    c->error = ZOOM_ERROR_TIMEOUT;
            do_close (c);
            ZOOM_connection_put_event(c, event);
        }
    }
#else
    yaz_log (LOG_DEBUG, "select start");
    r = select (max_fd+1, &input, &output, &except, &tv);
    yaz_log (LOG_DEBUG, "select stop, returned r=%d", r);
    for (i = 0; i<no; i++)
    {
	ZOOM_connection c = cs[i];
	int fd, mask;

	if (!c)
	    continue;
	fd = z3950_connection_socket(c);
	mask = 0;
	if (r && c->mask)
	{
	    /* no timeout and real socket */
	    if (FD_ISSET(fd, &input))
		mask += ZOOM_SELECT_READ;
	    if (FD_ISSET(fd, &output))
		mask += ZOOM_SELECT_WRITE;
	    if (FD_ISSET(fd, &except))
		mask += ZOOM_SELECT_EXCEPT;
	    if (mask)
		ZOOM_connection_do_io(c, mask);
	}
	if (r == 0 && c->mask)
	{
            ZOOM_Event event = ZOOM_Event_create(ZOOM_EVENT_TIMEOUT);
	    /* timeout and this connection was waiting */
	    c->error = ZOOM_ERROR_TIMEOUT;
            do_close (c);
            yaz_log (LOG_DEBUG, "timeout");
            ZOOM_connection_put_event(c, event);
	}
    }
#endif
    for (i = 0; i<no; i++)
    {
	ZOOM_connection c = cs[i];
        ZOOM_Event event;
	if (c && (event = ZOOM_connection_get_event(c)))
        {
            ZOOM_Event_destroy (event);
	    return i+1;
        }
    }
    return 0;
}
