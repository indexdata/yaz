/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: xmosi.c,v $
 * Revision 1.4  1995-09-27 15:02:45  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/06/16  10:30:38  quinn
 * Added REUSEADDR.
 *
 * Revision 1.2  1995/06/15  12:30:07  quinn
 * Added @ as hostname alias for INADDR ANY.
 *
 * Revision 1.1  1995/06/14  09:58:20  quinn
 * Renamed yazlib to comstack.
 *
 * Revision 1.15  1995/05/29  08:12:33  quinn
 * Updates to aynch. operations.
 *
 * Revision 1.14  1995/05/16  09:37:31  quinn
 * Fixed bug
 *
 * Revision 1.13  1995/05/16  08:51:19  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.12  1995/05/02  08:53:24  quinn
 * Trying in vain to fix comm with ISODE
 *
 * Revision 1.11  1995/04/21  16:32:08  quinn
 * *** empty log message ***
 *
 * Revision 1.10  1995/03/27  08:36:14  quinn
 * Some work on nonblocking operation in xmosi.c and rfct.c.
 * Added protocol parameter to cs_create()
 *
 * Revision 1.9  1995/03/20  09:47:23  quinn
 * Added server-side support to xmosi.c
 * Fixed possible problems in rfct
 * Other little mods
 *
 * Revision 1.8  1995/03/16  13:29:30  quinn
 * Beginning to add server-side functions
 *
 * Revision 1.7  1995/03/14  10:28:47  quinn
 * Adding server-side support to tcpip.c and fixing bugs in nonblocking I/O
 *
 * Revision 1.6  1995/03/09  15:22:43  quinn
 * Fixed two bugs in get/rcv
 *
 * Revision 1.5  1995/03/07  16:29:47  quinn
 * Various fixes.
 *
 * Revision 1.4  1995/03/07  10:26:56  quinn
 * Initialized type field in the comstacks.
 *
 * Revision 1.3  1995/03/06  16:48:03  quinn
 * Smallish changes.
 *
 * Revision 1.2  1995/03/06  10:54:41  quinn
 * Server-side functions (t_bind/t_listen/t_accept) seem to work ok, now.
 * Nonblocking mode needs work (and testing!)
 * Added makensap to replace function in mosiutil.c.
 *
 * Revision 1.1  1995/03/01  08:40:33  quinn
 * First working version of rfct. Addressing needs work.
 *
 */

/*
 * Glue layer for Peter Furniss' xtimosi package.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <comstack.h>
#include <xmosi.h>

#include <oid.h>

int mosi_connect(COMSTACK h, void *address);
int mosi_get(COMSTACK h, char **buf, int *bufsize);
int mosi_put(COMSTACK h, char *buf, int size);
int mosi_more(COMSTACK h) { return 0; } /* not correct */
int mosi_close(COMSTACK h);
int mosi_rcvconnect(COMSTACK h);
int mosi_bind(COMSTACK h, void *address, int mode);
int mosi_listen(COMSTACK h, char *addrp, int *addrlen);
COMSTACK mosi_accept(COMSTACK h);

typedef struct mosi_state
{
    struct t_info info;        /* data returned by t_open */
    struct t_call *call;
    int hasread;               /* how many bytes read of current PDU */
    int haswrit;               /* how many bytes have we written */
} mosi_state;

static char *oidtostr(int *o)
{
    static char buf[512];

    buf[0] = '\0';
    while (*o >= 0)
    {
    	sprintf(buf + strlen(buf), "%d", *o);
	if (*(++o) >= 0)
	    strcat(buf, " ");
    }
    return buf;
}

static int addopt(struct netbuf *optbuf, unsigned long level, unsigned long
    name, enum oid_proto proto, enum oid_class class, enum oid_value value)
{
    int *oid;
    oident ent;
    char *str;

    ent.proto = proto;
    ent.class = class;
    ent.value = value;
    if (!(oid = oid_getoidbyent(&ent)))
    	return -1;
    str = oidtostr(oid);
    if (addoidoption(optbuf, level, name, str) < 0)
    	return -1;
    return 0;
}

COMSTACK MDF mosi_type(int blocking, int protocol)
{
    COMSTACK r;
    mosi_state *state;
    int flags = O_RDWR;

    if (!(r = malloc(sizeof(*r))))
    	return 0;
    if (!(state = r->private = malloc(sizeof(*state))))
    	return 0;

    state->call = 0;
    state->hasread = 0;
    state->haswrit = 0;
    r->protocol = protocol;
    r->state = CS_UNBND;
    r->type = mosi_type;
    r->blocking = blocking;
    r->f_connect = mosi_connect;
    r->f_put = mosi_put;
    r->f_get = mosi_get;
    r->f_close = mosi_close;
    r->f_more = mosi_more;
    r->f_rcvconnect = mosi_rcvconnect;
    r->f_bind = mosi_bind;
    r->f_listen = mosi_listen;
    r->f_accept = mosi_accept;

    if (!blocking)
    	flags |= O_NONBLOCK;
    if ((r->iofile = u_open(CO_MOSI_NAME, flags, &state->info)) < 0)
    	return 0;

    r->timeout = COMSTACK_DEFAULT_TIMEOUT;

    return r;
}

int hex2oct(char *hex, char *oct)
{
    int len = 0;
    unsigned val;

    while (sscanf(hex, "%2x", &val) == 1)
    {
    	if (strlen(hex) < 2)
	    return -1;
	*((unsigned char*) oct++) = (unsigned char) val;
	len++;
	hex += 2;
    }
    return len;
}

/*
 * addressing specific to our hack of OSI transport. A sockaddr_in wrapped
 * up in a t_mosiaddr in a netbuf (on a stick).
 */
struct MDF netbuf MDF *mosi_strtoaddr(const char *str)
{
    struct netbuf *ret = malloc(sizeof(struct netbuf));
    struct sockaddr_in *add = malloc(sizeof(struct sockaddr_in));
    struct t_mosiaddr *mosiaddr = malloc(sizeof(struct t_mosiaddr));
    struct hostent *hp;
    char *p, *b, buf[512], *nsap;
    short int port = 102;
    unsigned long tmpadd;
    int ll = 0;

    assert(ret && add && mosiaddr);
#if 0
    mosiaddr->osi_ap_inv_id = NO_INVOKEID;
    mosiaddr->osi_ae_inv_id = NO_INVOKEID;
#endif
    mosiaddr->osi_apt_len = 0;
    mosiaddr->osi_aeq_len = 0;
    p = (char*)MOSI_PADDR(mosiaddr);
    *(p++) = 0; /* No presentation selector */
    ll++;
    *(p++) = 0; /* no session selector */
    ll++;
    /* do we have a transport selector? */
    strcpy(buf, str);
    if ((nsap = strchr(buf, '/')))
    {
    	*(nsap++) = '\0';
    	if ((*p = hex2oct(buf, p + 1)) < 0)
	    return 0;
	ll += *p + 1;
	p += *p + 1;
    }
    else
    {
    	nsap = buf;
    	*(p++) = 0;
    	ll++;
    }
    if (nsap && *nsap)
    {
	add->sin_family = AF_INET;
	strcpy(buf, nsap);
	if ((b = strchr(buf, ':')))
	{
	    *b = 0;
	    port = atoi(b + 1);
	}
	add->sin_port = htons(port);
	if (!strcmp("@", buf))
	    add->sin_addr.s_addr = INADDR_ANY;
	else if ((hp = gethostbyname(buf)))
	    memcpy(&add->sin_addr.s_addr, *hp->h_addr_list, sizeof(struct in_addr));
	else if ((tmpadd = inet_addr(buf)) != 0)
	    memcpy(&add->sin_addr.s_addr, &tmpadd, sizeof(struct in_addr));
	else
	    return 0;
	*(p++) = (char) sizeof(*add);
	ll++;
	memcpy(p, add, sizeof(*add));
	ll += sizeof(*add);
    }
    else
    {
    	*(p++) = 0;
	ll++;
    }
    mosiaddr->osi_paddr_len = ll;
    ret->buf = (char*)mosiaddr;
    ret->len = ret->maxlen = 100 /* sizeof(*mosiaddr) */ ;

    return ret;
}

int mosi_connect(COMSTACK h, void *address)
{
    struct netbuf *addr = address, *local;
    struct t_call *snd, *rcv;
    struct t_bind bnd;

    if (!(snd = (struct t_call *) u_alloc(h->iofile, T_CALL, T_ALL)))
    	return -1;
    if (!(rcv = (struct t_call *) u_alloc(h->iofile, T_CALL, T_ALL)))
    	return -1;

    snd->udata.len = 0;
    if (addopt(&snd->opt, ISO_APCO, AP_CNTX_NAME, h->protocol, CLASS_APPCTX,
    	VAL_BASIC_CTX) < 0)
    	return -1;
    if (addopt(&snd->opt, ISO_APCO, AP_ABS_SYN, h->protocol, CLASS_ABSYN,
    	VAL_APDU) < 0)
    	return -1;
    /*
     * We don't specify record formats yet.
     *
     * Xtimosi adds the oid for BER as transfer syntax automatically.
     */

    bnd.qlen = 0;

    if (h->state == CS_UNBND)
    {
	local = mosi_strtoaddr("");   /* not good in long run */
	memcpy(&bnd.addr, local, sizeof(*local));
	if (u_bind(h->iofile, &bnd, 0) < 0)
	    return -1;
    }

    memcpy(&snd->addr, addr, sizeof(*addr));
    if (u_connect(h->iofile, snd, rcv) < 0)
    {
    	if (t_errno == TNODATA)
	    return 1;
    	return -1; 
    }
    return 0;
}

int mosi_rcvconnect(COMSTACK h)
{
    if (u_rcvconnect(h->iofile, 0) < 0)
    {
    	if (t_errno == TNODATA)
	    return 1;
	return -1;
    }
    return 0;
}

int mosi_bind(COMSTACK h, void *address, int mode)
{
    int res;
    struct t_bind bnd;
    int one = 1;

    if (setsockopt(h->iofile, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
    {
    	h->errno = CSYSERR;
	return -1;
    }
    if (mode == CS_SERVER)
	bnd.qlen = 3;
    else
    	bnd.qlen = 0;
    memcpy(&bnd.addr, address, sizeof(struct netbuf));
    if ((res = u_bind(h->iofile, &bnd, 0)) < 0)
    	return -1;
    h->state = CS_IDLE;
    return 0;
}

int mosi_listen(COMSTACK h, char *addp, int *addrlen)
{
    int res;
    mosi_state *st = h->private;

    if (!(st->call = (struct t_call*) t_alloc(h->iofile, T_CALL_STR,
   	 T_ALL)))
    	return -1;
    if ((res = u_listen(h->iofile, st->call)) < 0)
    {
    	if (t_errno == TNODATA)
	    return 1;
	return -1;
    }
    h->state = CS_INCON;
    return 0;
}

COMSTACK mosi_accept(COMSTACK h)
{
    COMSTACK new;
    void *local;
    struct mosi_state *st = h->private, *ns;
    int flags = O_RDWR;

    if (h->state != CS_INCON)
    {
    	h->errno = CSOUTSTATE;
	return 0;
    }
    if (!(new = malloc(sizeof(*new))))
    	return 0;
    *new = *h;
    if (!(new->private = ns = malloc(sizeof(*ns))))
    	return 0;
    *ns = *st;
    if (!h->blocking)
    	flags |= O_NONBLOCK;
    if ((new->iofile = u_open_r(CO_MOSI_NAME, flags, &st->info, st->call)) < 0)
	return 0;
    if (!(local = mosi_strtoaddr("")))
    	return 0;
    if (mosi_bind(new, local, CS_CLIENT) < 0) /* CS_CLIENT: qlen == 0 */
    	return 0;
    memcpy(&st->call->addr, local, sizeof(st->call->addr));
    if (u_accept(h->iofile, new->iofile, st->call) < 0)
    {
    	mosi_close(new);
    	return 0;
    }
    return new;
}

#define CS_MOSI_BUFCHUNK 4096

int mosi_get(COMSTACK h, char **buf, int *bufsize)
{
    int flags = 0, res;
    mosi_state *ct = h->private;
    int got;

    do
    {
    	if (!*bufsize)
    	{
	    if (!(*buf = malloc(*bufsize = CS_MOSI_BUFCHUNK)))
	    	return -1;
	}
	else if (*bufsize - ct->hasread < CS_MOSI_BUFCHUNK)
	    if (!(*buf = realloc(*buf, *bufsize *= 2)))
	    	return -1;

    	if ((res = u_rcv(h->iofile, *buf + ct->hasread, CS_MOSI_BUFCHUNK,
	    &flags)) <= 0)
	{
	    if (t_errno == TNODATA)
	    	return 1;
	    return -1;
	}
	ct->hasread += res;
    }
    while (flags & T_MORE);

    /* all done. Reset hasread */
    got = ct->hasread;
    ct->hasread = 0;  
    return got;
}

int mosi_put(COMSTACK h, char *buf, int size)
{
    mosi_state *ct = h->private;
    int res = u_snd(h->iofile, buf + ct->haswrit, size - ct->haswrit, 0);

    if (res == size - ct->haswrit)
    {
	ct->haswrit = 0;
    	return 0;
    }
    else if (res < 0)
    {
    	if (t_errno == TFLOW)
	    return 1;
	return -1;
    }
    ct->haswrit += res;
    return 1;
}

int mosi_close(COMSTACK h)
{
    free(h->private);
    if (h->iofile >= 0)
	u_close(h->iofile);
    free(h);
    return 0;
}    
