/*
 * $Id: zoom-c.c,v 1.5 2001-11-13 22:57:03 adam Exp $
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

static Z3950_Event Z3950_Event_create (int kind)
{
    Z3950_Event event = xmalloc (sizeof(*event));
    event->kind = kind;
    event->next = 0;
    event->prev = 0;
    return event;
}

static void Z3950_Event_destroy (Z3950_Event event)
{
    xfree (event);
}

static void Z3950_connection_put_event (Z3950_connection c, Z3950_Event event)
{
    // put in back of queue
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

static Z3950_Event Z3950_connection_get_event(Z3950_connection c)
{
    // get from front of queue
    Z3950_Event event = c->m_queue_front;
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
    return event;
}

static void clear_error (Z3950_connection c)
{
    c->error = Z3950_ERROR_NONE;
    xfree (c->addinfo);
    c->addinfo = 0;
}

Z3950_task Z3950_connection_add_task (Z3950_connection c, int which)
{
    Z3950_task *taskp = &c->tasks;
    while (*taskp)
	taskp = &(*taskp)->next;
    *taskp = xmalloc (sizeof(**taskp));
    (*taskp)->running = 0;
    (*taskp)->which = which;
    (*taskp)->u.resultset = 0;  /* one null pointer there at least */
    (*taskp)->next = 0;
    clear_error (c);
    return *taskp;
}

void Z3950_connection_remove_task (Z3950_connection c)
{
    Z3950_task task = c->tasks;

    if (task)
    {
	c->tasks = task->next;
	switch (task->which)
	{
	case Z3950_TASK_SEARCH:
	    Z3950_resultset_destroy (task->u.resultset);
	    break;
	case Z3950_TASK_RETRIEVE:
	    Z3950_resultset_destroy (task->u.resultset);
	    break;
        case Z3950_TASK_CONNECT:
            break;
	default:
	    assert (0);
	}
	xfree (task);
    }
}

void Z3950_connection_remove_tasks (Z3950_connection c)
{
    while (c->tasks)
	Z3950_connection_remove_task(c);
}

static Z3950_record record_cache_lookup (Z3950_resultset r,
					 int pos,
					 const char *elementSetName);

Z3950_connection Z3950_connection_create (Z3950_options options)
{
    Z3950_connection c = xmalloc (sizeof(*c));

    c->cs = 0;
    c->mask = 0;
    c->state = STATE_IDLE;
    c->error = Z3950_ERROR_NONE;
    c->addinfo = 0;
    c->buf_in = 0;
    c->len_in = 0;
    c->buf_out = 0;
    c->len_out = 0;
    c->resultsets = 0;

    c->options = Z3950_options_create_with_parent(options);

    c->host_port = 0;
    c->proxy = 0;

    c->cookie_out = 0;
    c->cookie_in = 0;
    c->tasks = 0;

    c->odr_in = odr_createmem (ODR_DECODE);
    c->odr_out = odr_createmem (ODR_ENCODE);

    c->async = 0;

    c->m_queue_front = 0;
    c->m_queue_back = 0;
    return c;
}

/* set database names. Take local databases (if set); otherwise
   take databases given in ZURL (if set); otherwise use Default */
static char **set_DatabaseNames (Z3950_connection con, int *num)
{
    char **databaseNames;
    const char *c;
    int no = 2;
    const char *cp = Z3950_options_get (con->options, "databaseName");
    
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
    databaseNames = odr_malloc (con->odr_out, no * sizeof(*databaseNames));
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
	databaseNames[no] = odr_malloc (con->odr_out, 1+c-cp);
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

Z3950_connection Z3950_connection_new (const char *host, int portnum)
{
    Z3950_connection c = Z3950_connection_create (0);

    Z3950_connection_connect (c, host, portnum);
    return c;
}

void Z3950_connection_connect(Z3950_connection c,
			      const char *host, int portnum)
{
    const char *val;
    Z3950_task task;

    val = Z3950_options_get (c->options, "proxy");
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

    c->async = Z3950_options_get_bool (c->options, "async", 0);
    
    task = Z3950_connection_add_task (c, Z3950_TASK_CONNECT);

    if (!c->async)
    {
	while (Z3950_event (1, &c))
	    ;
    }
}

Z3950_query Z3950_query_create(void)
{
    Z3950_query s = xmalloc (sizeof(*s));

    s->refcount = 1;
    s->query = 0;
    s->sort_spec = 0;
    s->odr = odr_createmem (ODR_ENCODE);

    return s;
}

const char *Z3950_connection_host (Z3950_connection c)
{
    return c->host_port;
}

void Z3950_query_destroy(Z3950_query s)
{
    if (!s)
	return;

    (s->refcount)--;
    yaz_log (LOG_DEBUG, "Z3950_query_destroy count=%d", s->refcount);
    if (s->refcount == 0)
    {
	odr_destroy (s->odr);
	xfree (s);
    }
}

int Z3950_query_prefix(Z3950_query s, const char *str)
{
    s->query = odr_malloc (s->odr, sizeof(*s->query));
    s->query->which = Z_Query_type_1;
    s->query->u.type_1 =  p_query_rpn(s->odr, PROTO_Z3950, str);
    if (!s->query->u.type_1)
	return -1;
    return 0;
}

int Z3950_query_sortby(Z3950_query s, const char *criteria)
{
    s->sort_spec = yaz_sort_spec (s->odr, criteria);
    if (!s->sort_spec)
	return -1;
    return 0;
}

static int do_write(Z3950_connection c);

void Z3950_connection_destroy(Z3950_connection c)
{
    Z3950_resultset r;
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
    Z3950_options_destroy (c->options);
    Z3950_connection_remove_tasks (c);
    xfree (c->host_port);
    xfree (c);
}

void Z3950_resultset_addref (Z3950_resultset r)
{
    if (r)
	(r->refcount)++;
}
Z3950_resultset Z3950_resultset_create ()
{
    Z3950_resultset r = xmalloc (sizeof(*r));

    r->refcount = 1;
    r->size = 0;
    r->odr = odr_createmem (ODR_ENCODE);
    r->start = 0;
    r->piggyback = 1;
    r->count = 0;
    r->record_cache = 0;
    r->r_sort_spec = 0;
    r->r_query = 0;
    r->search = 0;
    r->connection = 0;
    r->next = 0;
    return r;
}

Z3950_resultset Z3950_connection_search_pqf(Z3950_connection c, const char *q)
{
    Z3950_resultset r;
    Z3950_query s = Z3950_query_create();

    Z3950_query_prefix (s, q);

    r = Z3950_connection_search (c, s);
    Z3950_query_destroy (s);
    return r;
}

Z3950_resultset Z3950_connection_search(Z3950_connection c, Z3950_query q)
{
    Z3950_resultset r = Z3950_resultset_create ();
    Z3950_task task;

    r->r_sort_spec = q->sort_spec;
    r->r_query = q->query;
    r->search = q;

    r->options = Z3950_options_create_with_parent(c->options);

    r->start = Z3950_options_get_int(r->options, "start", 0);
    r->count = Z3950_options_get_int(r->options, "count", 0);
    r->piggyback = Z3950_options_get_bool (r->options, "piggyback", 1);
    r->connection = c;

    r->next = c->resultsets;
    c->resultsets = r;

    task = Z3950_connection_add_task (c, Z3950_TASK_SEARCH);
    task->u.resultset = r;
    Z3950_resultset_addref (r);  

    (q->refcount)++;

    if (!c->async)
    {
	while (Z3950_event (1, &c))
	    ;
    }
    return r;
}

void Z3950_resultset_destroy(Z3950_resultset r)
{
    if (!r)
        return;
    (r->refcount)--;
    yaz_log (LOG_DEBUG, "destroy r = %p count=%d", r, r->refcount);
    if (r->refcount == 0)
    {
	if (r->connection)
	{
	    /* remove ourselves from the resultsets in connection */
	    Z3950_resultset *rp = &r->connection->resultsets;
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
	Z3950_query_destroy (r->search);
	Z3950_options_destroy (r->options);
	odr_destroy (r->odr);
	xfree (r);
    }
}

int Z3950_resultset_size (Z3950_resultset r)
{
    return r->size;
}

static void do_close (Z3950_connection c)
{
    if (c->cs)
	cs_close(c->cs);
    c->cs = 0;
    c->mask = 0;
    c->state = STATE_IDLE;
}

static void Z3950_resultset_retrieve (Z3950_resultset r,
				      int force_sync, int start, int count)
{
    Z3950_task task;
    Z3950_connection c;

    if (!r)
	return;
    c = r->connection;
    if (!c)
	return;
    task = Z3950_connection_add_task (c, Z3950_TASK_RETRIEVE);
    task->u.resultset = r;
    Z3950_resultset_addref (r);

    r->start = start;
    r->count = count;

    if (!r->connection->async || force_sync)
	while (r->connection && Z3950_event (1, &r->connection))
	    ;
}

void Z3950_resultset_records (Z3950_resultset r, Z3950_record *recs,
			      size_t start, size_t count)
{
    int force_present = 0;

    if (!r)
	return ;
    if (count && recs)
        force_present = 1;
    Z3950_resultset_retrieve (r, force_present, start, count);
    if (force_present)
    {
        size_t i;
        for (i = 0; i< count; i++)
            recs[i] = Z3950_resultset_record_immediate (r, i+start);
    }
}

static int do_connect (Z3950_connection c)
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
	    c->mask = Z3950_SELECT_READ | Z3950_SELECT_WRITE | 
                Z3950_SELECT_EXCEPT;
	    return 1;
	}
    }
    c->state = STATE_IDLE;
    c->error = Z3950_ERROR_CONNECT;
    return 0;
}

int z3950_connection_socket(Z3950_connection c)
{
    if (c->cs)
	return cs_fileno(c->cs);
    return -1;
}

int z3950_connection_mask(Z3950_connection c)
{
    if (c->cs)
	return c->mask;
    return 0;
}

static int encode_APDU(Z3950_connection c, Z_APDU *a, ODR out)
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
	c->error = Z3950_ERROR_ENCODE;
	do_close (c);
	return -1;
    }
    return 0;
}

static int send_APDU (Z3950_connection c, Z_APDU *a)
{
    assert (a);
    if (encode_APDU(c, a, c->odr_out))
	return -1;
    c->buf_out = odr_getbuf(c->odr_out, &c->len_out, 0);
    odr_reset(c->odr_out);
    do_write (c);
    return 0;	
}

static int Z3950_connection_send_init (Z3950_connection c)
{
    const char *impname;
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_initRequest);
    Z_InitRequest *ireq = apdu->u.initRequest;
    Z_IdAuthentication *auth = odr_malloc(c->odr_out, sizeof(*auth));
    const char *auth_groupId = Z3950_options_get (c->options, "group");
    const char *auth_userId = Z3950_options_get (c->options, "user");
    const char *auth_password = Z3950_options_get (c->options, "pass");
    
    ODR_MASK_SET(ireq->options, Z_Options_search);
    ODR_MASK_SET(ireq->options, Z_Options_present);
    ODR_MASK_SET(ireq->options, Z_Options_scan);
    ODR_MASK_SET(ireq->options, Z_Options_sort);
#if 0
    ODR_MASK_SET(ireq->options, Z_Options_extendedServices);
    ODR_MASK_SET(ireq->options, Z_Options_namedResultSets);
#endif
    
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_2);
    ODR_MASK_SET(ireq->protocolVersion, Z_ProtocolVersion_3);
    
    impname = Z3950_options_get (c->options, "implementationName");
    ireq->implementationName =
	odr_malloc (c->odr_out, 15 + (impname ? strlen(impname) : 0));
    strcpy (ireq->implementationName, "");
    if (impname)
    {
	strcat (ireq->implementationName, impname);
	strcat (ireq->implementationName, "/");
    }					       
    strcat (ireq->implementationName, "ZOOM-C/YAZ");
    
    *ireq->maximumRecordSize =
	Z3950_options_get_int (c->options, "maximumRecordSize", 1024*1024);
    *ireq->preferredMessageSize =
	Z3950_options_get_int (c->options, "preferredMessageSize", 1024*1024);
    
    if (auth_groupId || auth_password)
    {
	Z_IdPass *pass = odr_malloc(c->odr_out, sizeof(*pass));
	int i = 0;
	pass->groupId = 0;
	if (auth_groupId && *auth_groupId)
	{
	    pass->groupId = odr_malloc(c->odr_out, strlen(auth_groupId)+1);
	    strcpy(pass->groupId, auth_groupId);
	    i++;
	}
	pass->userId = 0;
	if (auth_userId && *auth_userId)
	{
	    pass->userId = odr_malloc(c->odr_out, strlen(auth_userId)+1);
	    strcpy(pass->userId, auth_userId);
	    i++;
	}
	pass->password = 0;
	if (auth_password && *auth_password)
	{
	    pass->password = odr_malloc(c->odr_out, strlen(auth_password)+1);
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
	auth->u.open = odr_malloc(c->odr_out, strlen(auth_userId)+1);
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

static int Z3950_connection_send_search (Z3950_connection c)
{
    Z3950_resultset r;
    int lslb, ssub, mspn;
    const char *syntax;
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_searchRequest);
    Z_SearchRequest *search_req = apdu->u.searchRequest;
    const char *elementSetName;
    const char *smallSetElementSetName;
    const char *mediumSetElementSetName;

    assert (c->tasks);
    assert (c->tasks->which == Z3950_TASK_SEARCH);

    r = c->tasks->u.resultset;

    elementSetName =
	Z3950_options_get (r->options, "elementSetName");
    smallSetElementSetName  =
	Z3950_options_get (r->options, "smallSetElementSetName");
    mediumSetElementSetName =
	Z3950_options_get (r->options, "mediumSetElementSetName");

    if (!smallSetElementSetName)
	smallSetElementSetName = elementSetName;

    if (!mediumSetElementSetName)
	mediumSetElementSetName = elementSetName;

    assert (r);
    assert (r->r_query);

    /* prepare query for the search request */
    search_req->query = r->r_query;

    search_req->databaseNames =
	set_DatabaseNames (c, &search_req->num_databaseNames);

    /* get syntax (no need to provide unless piggyback is in effect) */
    syntax = Z3950_options_get (r->options, "preferredRecordSyntax");

    lslb = Z3950_options_get_int (r->options, "largeSetLowerBound", -1);
    ssub = Z3950_options_get_int (r->options, "smallSetUpperBound", -1);
    mspn = Z3950_options_get_int (r->options, "mediumSetPresentNumber", -1);
    if (lslb != -1 && ssub != -1 && mspn != -1)
    {
	/* So're a Z39.50 expert? Let's hope you don't do sort */
	*search_req->largeSetLowerBound = lslb;
	*search_req->smallSetUpperBound = ssub;
	*search_req->mediumSetPresentNumber = mspn;
    }
    else if (r->start == 0 && r->count > 0
	     && r->piggyback && !r->r_sort_spec)
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
	Z_ElementSetNames *esn = odr_malloc (c->odr_out, sizeof(*esn));
	
	esn->which = Z_ElementSetNames_generic;
	esn->u.generic = odr_strdup (c->odr_out, smallSetElementSetName);
	search_req->smallSetElementSetNames = esn;
    }
    if (mediumSetElementSetName && *mediumSetElementSetName)
    {
	Z_ElementSetNames *esn = odr_malloc (c->odr_out, sizeof(*esn));
	
	esn->which = Z_ElementSetNames_generic;
	esn->u.generic = odr_strdup (c->odr_out, mediumSetElementSetName);
	search_req->mediumSetElementSetNames = esn;
    }
    if (syntax)
	search_req->preferredRecordSyntax =
	    yaz_str_to_z3950oid (c->odr_out, CLASS_RECSYN, syntax);

    /* send search request */
    send_APDU (c, apdu);
    r->r_query = 0;
    return 1;
}

static void response_diag (Z3950_connection c, Z_DiagRec *p)
{
    Z_DefaultDiagFormat *r;
    char *addinfo = 0;
    
    xfree (c->addinfo);
    c->addinfo = 0;
    if (p->which != Z_DiagRec_defaultFormat)
    {
	c->error = Z3950_ERROR_DECODE;
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

Z3950_record Z3950_record_dup (Z3950_record srec)
{
    char *buf;
    int size;
    ODR odr_enc;
    Z3950_record nrec;

    odr_enc = odr_createmem(ODR_ENCODE);
    if (!z_NamePlusRecord (odr_enc, &srec->npr, 0, 0))
	return 0;
    buf = odr_getbuf (odr_enc, &size, 0);
    
    nrec = xmalloc (sizeof(*nrec));
    nrec->odr = odr_createmem(ODR_DECODE);
    nrec->wrbuf_marc = 0;
    odr_setbuf (nrec->odr, buf, size, 0);
    z_NamePlusRecord (nrec->odr, &nrec->npr, 0, 0);
    
    odr_destroy (odr_enc);
    return nrec;
}

Z3950_record Z3950_resultset_record_immediate (Z3950_resultset s,size_t pos)
{
    Z3950_record rec = record_cache_lookup (s, pos, 0);
    if (!rec)
	return 0;
    return Z3950_record_dup (rec);
}

Z3950_record Z3950_resultset_record (Z3950_resultset r, size_t pos)
{
    Z3950_resultset_retrieve (r, 1, pos, 1);
    return Z3950_resultset_record_immediate (r, pos);
}

void Z3950_record_destroy (Z3950_record rec)
{
    if (!rec)
	return;
    if (rec->wrbuf_marc)
	wrbuf_free (rec->wrbuf_marc, 1);
    odr_destroy (rec->odr);
    xfree (rec);
}

void *Z3950_record_get (Z3950_record rec, const char *type, size_t *len)
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
    else if (!strcmp (type, "render"))
    {
	if (npr->which == Z_NamePlusRecord_databaseRecord)
	{
	    Z_External *r = (Z_External *) npr->u.databaseRecord;
	    oident *ent = oid_getentbyoid(r->direct_reference);
	    
	    if (r->which == Z_External_sutrs)
	    {
		*len = r->u.sutrs->len;
		return r->u.sutrs->buf;
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
		    if (marc_display_wrbuf (r->u.octet_aligned->buf,
					rec->wrbuf_marc, 0,
					    r->u.octet_aligned->len) > 0)
		    {
			*len = wrbuf_len(rec->wrbuf_marc);
			return wrbuf_buf(rec->wrbuf_marc);
		    }
		}
		*len = r->u.octet_aligned->len;
		return r->u.octet_aligned->buf;
	    }
	    else if (r->which == Z_External_grs1)
	    {
		*len = 5;
		return "GRS-1";
	    }
	}
	return 0;
    }
    else if (!strcmp (type, "raw"))
    {
	if (npr->which == Z_NamePlusRecord_databaseRecord)
	{
            *len = -1;
	    return (Z_External *) npr->u.databaseRecord;
	}
	return 0;
    }
    return 0;
}

void *Z3950_resultset_get (Z3950_resultset s, size_t pos, const char *type,
			   size_t *len)
{
    Z3950_record rec = record_cache_lookup (s, pos, 0);
    return Z3950_record_get (rec, type, len);
}

static void record_cache_add (Z3950_resultset r,
			      Z_NamePlusRecord *npr,
			      int pos,
			      const char *elementSetName)
{
    Z3950_record_cache rc;

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
    rc = odr_malloc (r->odr, sizeof(*rc));
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

static Z3950_record record_cache_lookup (Z3950_resultset r,
					 int pos,
					 const char *elementSetName)
{
    Z3950_record_cache rc;

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
					     
static void handle_records (Z3950_connection c, Z_Records *sr,
			    int present_phase)
{
    Z3950_resultset resultset;

    if (!c->tasks)
	return ;
    if (c->tasks->which != Z3950_TASK_SEARCH &&
	c->tasks->which != Z3950_TASK_RETRIEVE)
	return ;
    
    resultset = c->tasks->u.resultset;

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
	    c->error = Z3950_ERROR_DECODE;
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
		c->error = Z3950_ERROR_DECODE;
	    }
	}
	else if (present_phase)
	{
	    /* present response and we didn't get any records! */
	    c->error = Z3950_ERROR_DECODE;
	}
    }
}

static void handle_present_response (Z3950_connection c, Z_PresentResponse *pr)
{
    handle_records (c, pr->records, 1);
}

static void handle_search_response (Z3950_connection c, Z_SearchResponse *sr)
{
    Z3950_resultset resultset;

    yaz_log (LOG_DEBUG, "got search response");

    if (!c->tasks || c->tasks->which != Z3950_TASK_SEARCH)
	return ;

    resultset = c->tasks->u.resultset;

    resultset->size = *sr->resultCount;
    handle_records (c, sr->records, 0);
}

static void sort_response (Z3950_connection c, Z_SortResponse *res)
{
    if (res->diagnostics && res->num_diagnostics > 0)
	response_diag (c, res->diagnostics[0]);
}

static int send_sort (Z3950_connection c)
{
    Z3950_resultset  resultset;

    if (!c->tasks || c->tasks->which != Z3950_TASK_SEARCH)
	return 0;

    resultset = c->tasks->u.resultset;

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
	req->inputResultSetNames[0] = odr_strdup (c->odr_out, "default");
	req->sortedResultSetName = odr_strdup (c->odr_out, "default");
	req->sortSequence = resultset->r_sort_spec;
	resultset->r_sort_spec = 0;
	send_APDU (c, apdu);
	return 1;
    }
    return 0;
}

static int send_present (Z3950_connection c)
{
    Z_APDU *apdu = zget_APDU(c->odr_out, Z_APDU_presentRequest);
    Z_PresentRequest *req = apdu->u.presentRequest;
    int i = 0;
    const char *syntax = 
	Z3950_options_get (c->options, "preferredRecordSyntax");
    const char *element =
	Z3950_options_get (c->options, "elementSetName");
    Z3950_resultset  resultset;

    if (!c->tasks)
	return 0;
    if (c->tasks->which != Z3950_TASK_SEARCH && 
	c->tasks->which != Z3950_TASK_RETRIEVE)
	return 0;

    resultset = c->tasks->u.resultset;
    
    if (c->error)                  /* don't continue on error */
	return 0;
    if (resultset->start < 0)
	return 0;
    for (i = 0; i<resultset->count; i++)
    {
	Z3950_record rec =
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

    if (element && *element)
    {
	Z_ElementSetNames *esn = odr_malloc (c->odr_out, sizeof(*esn));
	Z_RecordComposition *compo = odr_malloc (c->odr_out, sizeof(*compo));
	
	esn->which = Z_ElementSetNames_generic;
	esn->u.generic = odr_strdup (c->odr_out, element);
	compo->which = Z_RecordComp_simple;
	compo->u.simple = esn;
	req->recordComposition = compo;
    }
    send_APDU (c, apdu);
    return 1;
}

static int Z3950_connection_exec_task (Z3950_connection c)
{
    Z3950_task task = c->tasks;

    yaz_log (LOG_LOG, "Z3950_connection_exec_task");
    if (!task)
	return 0;
    if (c->error != Z3950_ERROR_NONE ||
        (!c->cs && task->which != Z3950_TASK_CONNECT))
    {
	Z3950_connection_remove_tasks (c);
	return 0;
    }
    yaz_log (LOG_DEBUG, "Z3950_connection_exec_task type=%d", task->which);
    if (task->running)
	return 0;
    task->running = 1;
    switch (task->which)
    {
    case Z3950_TASK_SEARCH:
	/* see if search hasn't been sent yet. */
	if (Z3950_connection_send_search (c))
	    return 1;
	break;
    case Z3950_TASK_RETRIEVE:
	if (send_present (c))
	    return 1;
	break;
    case Z3950_TASK_CONNECT:
        if (do_connect(c))
            return 1;
    }
    Z3950_connection_remove_task (c);
    return 0;
}

static int send_sort_present (Z3950_connection c)
{
    int r = send_sort (c);
    if (!r)
	r = send_present (c);
    return r;
}

static void handle_apdu (Z3950_connection c, Z_APDU *apdu)
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
	    c->error = Z3950_ERROR_INIT;
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
            if (c->tasks)
            {
                assert (c->tasks->which == Z3950_TASK_CONNECT);
                Z3950_connection_remove_task (c);
            }
	    Z3950_connection_exec_task (c);
	}
	break;
    case Z_APDU_searchResponse:
	handle_search_response (c, apdu->u.searchResponse);
	if (!send_sort_present (c))
	    Z3950_connection_remove_task (c);
	break;
    case Z_APDU_presentResponse:
	handle_present_response (c, apdu->u.presentResponse);
	if (!send_present (c))
	    Z3950_connection_remove_task (c);
	break;
    case Z_APDU_sortResponse:
	sort_response (c, apdu->u.sortResponse);
	if (!send_present (c))
	    Z3950_connection_remove_task (c);
    }
}

static int do_read (Z3950_connection c)
{
    int r;
    Z_APDU *apdu;
    
    r = cs_get (c->cs, &c->buf_in, &c->len_in);
    if (r == 1)
	return 0;
    if (r <= 0)
    {
	c->error= Z3950_ERROR_CONNECTION_LOST;
	do_close (c);
    }
    else
    {
	odr_reset (c->odr_in);
	odr_setbuf (c->odr_in, c->buf_in, r, 0);
	if (!z_APDU (c->odr_in, &apdu, 0, 0))
	{
	    c->error = Z3950_ERROR_DECODE;
	    do_close (c);
	}
	else
	{
	    handle_apdu (c, apdu);
	}
    }
    return 1;
}

static int do_write_ex (Z3950_connection c, char *buf_out, int len_out)
{
    int r;
    
    if ((r=cs_put (c->cs, buf_out, len_out)) < 0)
    {
	if (c->state == STATE_CONNECTING)
	    c->error = Z3950_ERROR_CONNECT;
	else
	    c->error = Z3950_ERROR_CONNECTION_LOST;
	do_close (c);
	return 1;
    }
    else if (r == 1)
    {
	c->state = STATE_ESTABLISHED;
	c->mask = Z3950_SELECT_READ|Z3950_SELECT_WRITE|Z3950_SELECT_EXCEPT;
    }
    else
    {
	c->state = STATE_ESTABLISHED;
	c->mask = Z3950_SELECT_READ|Z3950_SELECT_EXCEPT;
    }
    return 0;
}

static int do_write(Z3950_connection c)
{
    return do_write_ex (c, c->buf_out, c->len_out);
}

const char *Z3950_connection_option (Z3950_connection c, const char *key,
				     const char *val)
{
    if (val)
    {
	Z3950_options_set (c->options, key, val);
        return val;
    }
    return Z3950_options_get (c->options, key);
}

const char *Z3950_resultset_option (Z3950_resultset r, const char *key,
				    const char *val)
{
    if (val)
    {
	Z3950_options_set (r->options, key, val);
        return val;
    }
    return Z3950_options_get (r->options, key);
}


int Z3950_connection_errcode (Z3950_connection c)
{
    return Z3950_connection_error (c, 0, 0);
}

const char *Z3950_connection_errmsg (Z3950_connection c)
{
    const char *msg;
    Z3950_connection_error (c, &msg, 0);
    return msg;
}

const char *Z3950_connection_addinfo (Z3950_connection c)
{
    const char *addinfo;
    Z3950_connection_error (c, 0, &addinfo);
    return addinfo;
}

int Z3950_connection_error (Z3950_connection c, const char **cp,
			    const char **addinfo)
{
    int error = c->error;
    if (cp)
    {
	switch (error)
	{
	case Z3950_ERROR_NONE:
	    *cp = "No error"; break;
	case Z3950_ERROR_CONNECT:
	    *cp = "Connect failed"; break;
	case Z3950_ERROR_MEMORY:
	    *cp = "Out of memory"; break;
	case Z3950_ERROR_ENCODE:
	    *cp = "Encoding failed"; break;
	case Z3950_ERROR_DECODE:
	    *cp = "Decoding failed"; break;
	case Z3950_ERROR_CONNECTION_LOST:
	    *cp = "Connection lost"; break;
	case Z3950_ERROR_INIT:
	    *cp = "Init rejected"; break;
	case Z3950_ERROR_INTERNAL:
	    *cp = "Internal failure"; break;
	case Z3950_ERROR_TIMEOUT:
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

int Z3950_connection_do_io(Z3950_connection c, int mask)
{
    Z3950_Event event;
#if 0
    int r = cs_look(c->cs);
    yaz_log (LOG_LOG, "Z3950_connection_do_io c=%p mask=%d cs_look=%d",
	     c, mask, r);
    
    if (r == CS_NONE)
    {
	c->error = Z3950_ERROR_CONNECT;
	do_close (c);
    }
    else if (r == CS_CONNECT)
    {
	yaz_log (LOG_LOG, "calling rcvconnect");
	if (cs_rcvconnect (c->cs) < 0)
	{
	    c->error = Z3950_ERROR_CONNECT;
	    do_close (c);
	}
	else
	    Z3950_connection_send_init (c);
    }
    else
    {
	if (mask & Z3950_SELECT_READ)
	    do_read (c);
	if (c->cs && (mask & Z3950_SELECT_WRITE))
	    do_write (c);
    }	
#else
    yaz_log (LOG_DEBUG, "Z3950_connection_do_io c=%p mask=%d", c, mask);
    if (c->state == STATE_CONNECTING)
    {
	if (mask & Z3950_SELECT_WRITE)
	    Z3950_connection_send_init (c);
	else
	{
	    c->error = Z3950_ERROR_CONNECT;
	    do_close (c);
	}
    }
    else if (c->state == STATE_ESTABLISHED)
    {
	if (mask & Z3950_SELECT_READ)
	    do_read (c);
	if (c->cs && (mask & Z3950_SELECT_WRITE))
	    do_write (c);
    }
    else
    {
	c->error = Z3950_ERROR_INTERNAL;
	do_close (c);
    }
#endif
    event = Z3950_Event_create (1);
    Z3950_connection_put_event (c, event);
    return 1;
}


int Z3950_event (int no, Z3950_connection *cs)
{
#if HAVE_SYS_POLL_H
    struct pollfd pollfds[1024];
    Z3950_connection poll_cs[1024];
#else
    struct timeval tv;
    fd_set input, output, except;
#endif
    int i, r, nfds;
    int max_fd = 0;

    for (i = 0; i<no; i++)
    {
	Z3950_connection c = cs[i];
        Z3950_Event event;
	if (c && (event = Z3950_connection_get_event(c)))
        {
            Z3950_Event_destroy (event);
	    return i+1;
        }
    }
    for (i = 0; i<no; i++)
    {
        Z3950_connection c = cs[i];
        if (c && Z3950_connection_exec_task (c))
            return i+1;
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
	Z3950_connection c = cs[i];
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

            if (mask & Z3950_SELECT_READ)
                poll_events += POLLIN;
            if (mask & Z3950_SELECT_WRITE)
                poll_events += POLLOUT;
            if (mask & Z3950_SELECT_EXCEPT)
                poll_events += POLLERR;
            pollfds[nfds].fd = fd;
            pollfds[nfds].events = poll_events;
            pollfds[nfds].revents = 0;
            poll_cs[nfds] = c;
            nfds++;
        }
#else
	if (mask & Z3950_SELECT_READ)
	{
	    FD_SET (fd, &input);
	    nfds++;
	}
	if (mask & Z3950_SELECT_WRITE)
	{
	    FD_SET (fd, &output);
	    nfds++;
	}
	if (mask & Z3950_SELECT_EXCEPT)
	{
	    FD_SET (fd, &except);
	    nfds++;
	}
#endif
    }
    if (!nfds)
        return 0;
#if HAVE_SYS_POLL_H
    yaz_log (LOG_LOG, "poll start");
    r = poll (pollfds, nfds, 15000);
    yaz_log (LOG_LOG, "poll stop, returned r=%d", r);
    for (i = 0; i<nfds; i++)
    {
        Z3950_connection c = poll_cs[i];
        if (r && c->mask)
        {
            int mask = 0;
            if (pollfds[i].revents & POLLIN)
                mask += Z3950_SELECT_READ;
            if (pollfds[i].revents & POLLOUT)
                mask += Z3950_SELECT_WRITE;
            if (pollfds[i].revents & POLLERR)
                mask += Z3950_SELECT_EXCEPT;
            if (mask)
                Z3950_connection_do_io(c, mask);
        }
        else if (r == 0 && c->mask)
        {
            Z3950_Event event = Z3950_Event_create(0);
	    /* timeout and this connection was waiting */
	    c->error = Z3950_ERROR_TIMEOUT;
            do_close (c);
            Z3950_connection_put_event(c, event);
        }
    }
#else
    yaz_log (LOG_DEBUG, "select start");
    r = select (max_fd+1, &input, &output, &except, &tv);
    yaz_log (LOG_DEBUG, "select stop, returned r=%d", r);
    for (i = 0; i<no; i++)
    {
	Z3950_connection c = cs[i];
	int fd, mask;

	if (!c)
	    continue;
	fd = z3950_connection_socket(c);
	mask = 0;
	if (r && c->mask)
	{
	    /* no timeout and real socket */
	    if (FD_ISSET(fd, &input))
		mask += Z3950_SELECT_READ;
	    if (FD_ISSET(fd, &output))
		mask += Z3950_SELECT_WRITE;
	    if (FD_ISSET(fd, &except))
		mask += Z3950_SELECT_EXCEPT;
	    if (mask)
		Z3950_connection_do_io(c, mask);
	}
	if (r == 0 && c->mask)
	{
            Z3950_Event event = Z3950_Event_create(0);
	    /* timeout and this connection was waiting */
	    c->error = Z3950_ERROR_TIMEOUT;
            do_close (c);
            yaz_log (LOG_LOG, "timeout");
            Z3950_connection_put_event(c, event);
	}
    }
#endif
    for (i = 0; i<no; i++)
    {
	Z3950_connection c = cs[i];
        Z3950_Event event;
	if (c && (event = Z3950_connection_get_event(c)))
        {
            Z3950_Event_destroy (event);
	    return i+1;
        }
    }
    return 0;
}
