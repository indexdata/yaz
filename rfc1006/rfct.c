/*
 * 1995, Index Data I/S 
 * 
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: rfct.c,v $
 * Revision 1.10  2005-06-25 15:46:03  adam
 * Expanded tabs in all source files. Added vim/emacs local variables
 * trailer.
 *
 * Revision 1.9  1996/02/23 10:01:00  quinn
 * Smallish
 *
 * Revision 1.8  1995/11/01  13:54:52  quinn
 * Minor adjustments
 *
 * Revision 1.7  1995/06/16  10:46:48  quinn
 * *** empty log message ***
 *
 * Revision 1.6  1995/06/15  07:45:11  quinn
 * Moving to v3.
 *
 * Revision 1.5  1995/05/31  08:29:35  quinn
 * Nothing significant.
 *
 * Revision 1.4  1995/05/18  13:02:07  quinn
 * Smallish.
 *
 * Revision 1.3  1995/05/16  09:37:18  quinn
 * Fixed bug
 *
 * Revision 1.2  1995/05/02  08:53:09  quinn
 * Trying in vain to fix comm with ISODE
 *
 * Revision 1.1  1995/03/30  14:03:17  quinn
 * Added RFC1006 as separate library
 *
 * Revision 1.15  1995/03/30  10:54:43  quinn
 * Fiddling with packet sizes, trying to help ISODE.. :(
 *
 * Revision 1.14  1995/03/27  08:36:07  quinn
 * Some work on nonblocking operation in xmosi.c and rfct.c.
 * Added protocol parameter to cs_create()
 *
 * Revision 1.13  1995/03/20  11:27:16  quinn
 * Fixed bug in the _t_rcv stuff
 *
 * Revision 1.12  1995/03/20  09:54:07  quinn
 * Debugging session
 *
 * Revision 1.11  1995/03/20  09:47:16  quinn
 * Added server-side support to xmosi.c
 * Fixed possible problems in rfct
 * Other little mods
 *
 * Revision 1.10  1995/03/17  19:37:26  quinn
 * *** empty log message ***
 *
 * Revision 1.9  1995/03/17  19:28:32  quinn
 * Working on fixing our mystery-bug.
 *
 * Revision 1.8  1995/03/14  10:28:38  quinn
 * Adding server-side support to tcpip.c and fixing bugs in nonblocking I/O
 *
 * Revision 1.7  1995/03/09  18:42:32  quinn
 * Fixed another bug in t_rcv
 *
 * Revision 1.6  1995/03/09  15:22:42  quinn
 * Fixed two bugs in get/rcv
 *
 * Revision 1.5  1995/03/07  16:29:46  quinn
 * Various fixes.
 *
 * Revision 1.4  1995/03/06  16:48:03  quinn
 * Smallish changes.
 *
 * Revision 1.3  1995/03/06  10:54:32  quinn
 * Server-side functions (t_bind/t_listen/t_accept) seem to work ok, now.
 * Nonblocking mode needs work (and testing!)
 * Added makensap to replace function in mosiutil.c.
 *
 * Revision 1.2  1995/03/03  09:40:36  quinn
 * Added most of the remaining functionality.
 * Still need exstensive testing of server-side functions and nonblocking
 * I/O.
 *
 * Revision 1.1  1995/03/01  08:40:33  quinn
 * First working version of rfct. Addressing needs work.
 *
 */

/*
 * Simple implementation of XTI/TP0/RFC1006/Sockets.
 * Note: There is still some work to do in here, but basically it works.
 *
 * TODO: Asynchronous mode needs a lot of little adjustments to various
 * return values and error codes, etc. Formally, we should probably hold
 * this up against the protocol state tables, and see if it's correct.
 *
 * Check if addressing info is returned correctly by all calls.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>

#include <xti.h>
#include <fcntl.h>
#ifdef __linux__
#include <linux/limits.h>
#endif

#ifdef TRACE_TRANSPORT
#define TRC(x) x
#else
#define TRC(X)
#endif

#define TSEL_MAXLEN 20   /* is there a standard for this? */
#define RFC_DEFAULT_PORT 4500
#define MAX_QLEN 5        /* outstanding connect indications */

static int t_look_wait(int fd, int wait);
int t_rcvconnect(int fd, struct t_call *call);

int t_errno = 0;
int xti_trace = 0;

static struct rfct_control
{
    int state;          /* state of transport endpoint */
    int event;          /* current event */
    int tsize;          /* max TPDU size */
    int curcode;        /* code of current  TPDU */
    int pending;        /* # bytes of user data waiting on line */
    int eot_flag;       /* current incoming TPDU had EOT set */
    int hlen;           /* TPDU header suffix remaining on line */
    int eot;            /* current incoming DATA TPDU is last in TSDU */
    int togo;           /* # bytes waiting to go on current outgoing DATA */
    int qlen;           /* set by t_bind */
    int listen;         /* we are listening */
    int tmpfd;          /* holding space for client between listen and accept */
    int flags;          /* set by t_open */
    char rref[2];       /* remote reference */
    char ltsel[TSEL_MAXLEN]; /* local transport selector */
    int ltsel_len;
    int oci[MAX_QLEN];   /* outstanding connect indications */
} *control[NOFILE];

/*
 * In the best tradition of ThinOSI, we combine the RFC1006 and the common
 * part of the TPDU header into one. Given the complexity of the transport
 * layer, maybe it would have been nice to have it in a separate module
 * for reuse - but such is hindsight.
 * Using a bit-field like this may be dangerous. I would expect it to work
 * fine on any 32-bit or 64-bit machine, with any decent compiler. A DOS
 * compiler might make a mess of it, I suppose, but I wouldn't even expect
 * that.
 */
struct rfc_header
{
    /* RFC1006 */
    unsigned version:8;
#define RFC_VERSION 3
    unsigned reserved:8;
    unsigned len:16;         /* length of entire package, including header */
    /* TPDU common fields */
    unsigned char hlen;     /* length of TPDU-header minus length-octet */
    unsigned char code;     /* TPDU code (and an unused 4-bit field) */
    char suffix[100];       /* unstructured TPDU elements, for convenience */
};

/*
 * The TPDU-codes used by this package.
 */
#define TPDU_CODE_CREQ 0xe0
#define TPDU_CODE_CCON 0xd0
#define TPDU_CODE_DATA 0xf0
#define TPDU_CODE_DREQ 0x80

/*
 * Parameters that we care about.
 */
#define TPDU_PARM_TSIZE 0xc0  /* max TPDU size */
#define TPDU_PARM_CLGID 0xc1  /* Calling TSAP-ID */
#define TPDU_PARM_CLDID 0xc2  /* Called TSAP-ID */

struct tpdu_connect_header   /* length of fixed suffix: 5 octets */
{
    char dst_ref[2];     /* 3-4 - not used by TP0 */
    char src_ref[2];     /* 5-6 - not used by TP0 */
    char class;        /* 7 - always 0 */
};

struct tpdu_disconnect_header /* length of fixed suffix: 5 octets */
{
    char dst_ref[2];
    char src_ref[2];
    char reason;
};

struct tpdu_data_header      /* length of fixed suffix: 1 octet */
{
    unsigned char nr;       /* we only use bit 7 (1 << 7), to mark EOT */
#define DATA_EOT (1<<7)
};

static int rfc_running = 0; /* Have we been initialized? */

static void init_rfc(void)
{
    int i;

    for (i = 0; i < NOFILE; i++)
        control[i] = 0;
    rfc_running = 1;
}

int t_open(char *name, int oflag, struct t_info *info)
{
    struct rfct_control *cnt;
    struct protoent *proto;
    int s, i;

    TRC(fprintf(stderr, "T_OPEN\n"));

    if (!rfc_running)
        init_rfc();

    if (!(proto = getprotobyname("tcp")))
        return 0;
    if ((s = socket(AF_INET, SOCK_STREAM, proto->p_proto)) < 0)
        return 0;
#ifdef NONBLOCKING_OSI
    if ((oflag & O_NONBLOCK) && fcntl(s, F_SETFL, O_NONBLOCK) < 0)
    {
        t_errno = TSYSERR;
        return -1;
    }
#endif
    if (!(cnt = control[s] = malloc(sizeof(struct rfct_control))))
    {
        TRC(perror("malloc()"));
        t_errno = TSYSERR;
        return -1;
    }

    cnt->togo = 0;
    cnt->pending = 0;
    cnt->state = T_UNBND;
    cnt->event = 0;
    cnt->listen = 0;
    cnt->qlen = 0;
    cnt->flags = oflag;
    cnt->tmpfd = -1;
    cnt->ltsel_len = 0;
    for (i = 0; i < MAX_QLEN; i++)
        cnt->oci[i] = -1;

    /*
     * RFC1006 sets a higher than standard (TP) default max TPDU size, but the
     * Isode seems to like to negotiate it down. We'll keep it here to be
     * safe. Note that there's no harm in jumping it up. If it's higher
     * than 2048, t_connect won't try to negotiate.
     */
    cnt->tsize = 2048;

    if (info)
    {
        info->addr = TSEL_MAXLEN + sizeof(struct sockaddr_in) + 1;
        info->options = 1024;
        info->tsdu = -1; /* is this right? */
        info->etsdu = 0;
        info->connect = -2;
        info->discon = -2;
        info->servtype = T_COTS_ORD;  /* lets hope our user doesn't
                                        try something funny. */
    }
    return s;
}

int t_connect(int fd, struct t_call *sndcall, struct t_call *rcvcall)
{
    struct rfct_control *cnt = control[fd];
    struct sockaddr_in addr;
    char *p;
    struct iovec vec[3]; /* RFC1006 header + T-header + parms */
    struct rfc_header rfc;
    struct tpdu_connect_header tpdu;
    unsigned char pbuf[2 + TSEL_MAXLEN + 3]; /* CR parameters */
    int plen = 0;

    TRC(fprintf(stderr, "T_CONNECT\n"));
    if (!cnt || cnt->state != T_IDLE)
    {
        TRC(fprintf(stderr, "TOUTSTATE\n"));
        t_errno = TOUTSTATE;
        return -1;
    }
    /* take the address apart */
    p = sndcall->addr.buf;
    if (*p) /* transport selector */
    {
        TRC(fprintf(stderr, "Tsel length is %d.\n", *p));
        pbuf[0] = TPDU_PARM_CLDID;
        pbuf[1] = *p;
        memcpy(pbuf + 2, p + 1, *p);
        plen = *p + 2;
    }
    p += *p + 1; /* skip tsel */
    if (*p != sizeof(addr))
    {
        TRC(fprintf(stderr, "Expected  sockaddr here.\n"));
        t_errno = TBADADDR;
        return -1;
    }
    p++;
    memcpy(&addr, p, sizeof(addr));
    if (connect(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        t_errno = TSYSERR;
        return -1;
    }

    /*
     * If the default set by t_open() is higher than 2048, we don't try
     * to negotiate, hoping that the opponent goes by the high default
     * set by RFC1006. Otherwise, we just go for 2048 (max according to
     * transport standard). The rest of this module doesn't really care
     * about the size (although it's respected, of course), since we do
     * no internal buffering.
     */
    if (cnt->tsize <= 2048)
    {
        pbuf[plen++] = TPDU_PARM_TSIZE;
        pbuf[plen++] = 1;
        pbuf[plen++] = 0x0b; /* request max PDU size (2048 octets) */
    }

    rfc.version = RFC_VERSION;
    rfc.reserved = 0;
    rfc.len =  htons(4 + 7 + plen);
    rfc.hlen = 6 + plen;
    rfc.code = TPDU_CODE_CREQ;

    memset(tpdu.dst_ref, 0, 2);
    memset(tpdu.src_ref, 0, 2);
    tpdu.class = 0;

    vec[0].iov_base = (caddr_t) &rfc;
    vec[0].iov_len = 6;
    vec[1].iov_base = (caddr_t) &tpdu;
    vec[1].iov_len = 5;
    vec[2].iov_base = (caddr_t) pbuf;
    vec[2].iov_len = plen;

    /*
     * we don't expect flow-control problems on the first outgoing packet,
     * though I suppose it'd be possible on some weird types of link.
     */
    if (writev(fd, vec, 3) < 4 + 7 + plen)
    {
        TRC(fprintf(stderr, "writev came up short. Aborting connect\n"));
        t_errno = TSYSERR;
        return -1;
    }
    cnt->state = T_OUTCON;
    cnt->event = 0;
    return t_rcvconnect(fd, rcvcall);
}

/*
 * This needs work for asynchronous mode.
 */
static int read_n(int fd, char *buf, int toget)
{
    struct rfct_control *cnt = control[fd];
    int res, got = 0;

    do
    {
        if ((res = read(fd, buf, toget - got)) < 0)
        {
            if (errno == EAGAIN)
                t_errno = TNODATA;
            else
            {
                TRC(fprintf(stderr, "Error on read.\n"));
                t_errno = TSYSERR;
            }
            return -1;
        }
        if (!res) /* peer closed network connection */
        {
            t_errno = TLOOK;
            cnt->event = T_DISCONNECT;  /* is this correct ? ## */
            cnt->hlen = cnt->pending = 0;
            cnt->state = T_IDLE; /* this can't be correct ## */
            return 0;
        }
        buf += res;
    }
    while ((got += res) < toget);
    return toget;
}

int t_rcvconnect(int fd, struct t_call *call)
{
    struct rfct_control *cnt = control[fd];
    struct tpdu_connect_header chead;
    char buf[100], *p, *addrp;
    struct sockaddr_in peer;
    int len = sizeof(struct sockaddr_in);

    TRC(fprintf(stderr, "T_RCVCONNECT\n"));
    if (!cnt || cnt->state != T_OUTCON)
    {
        TRC(fprintf(stderr, "TOUTSTATE\n"));
        t_errno = TOUTSTATE;
        return -1;
    }
    if (!cnt->event)
        if (t_look_wait(fd, 1) <= 0)
            return -1;
    if (cnt->event != T_CONNECT)
    {
        t_errno = TLOOK;
        return -1;
    }
    /* read the rest of the CC TPDU */
    if (read_n(fd, buf, cnt->hlen) <= 0)
        return -1;
    memcpy(&chead, buf, 5);
    if (chead.class != 0)
    {
        TRC(fprintf(stderr, "Expected TP0, got %d\n", (int)chead.class));
        t_errno = TSYSERR;
        return -1;
    }
    if (call)
        *(addrp = call->addr.buf) = 0;
    cnt->hlen -= 5;
    for (p = buf + 5; cnt->hlen > 0;)
    {
        switch ((unsigned char)*p)
        {
            case TPDU_PARM_TSIZE:
                cnt->tsize = 1 << *(p + 2);
                break;
            case TPDU_PARM_CLDID:
                if (call)
                {
                    if (*(p + 1) > TSEL_MAXLEN)
                    {
                        TRC(fprintf(stderr, "Called TSEL too long.\n"));
                        t_errno = TSYSERR; /* Wrong.. ## */
                        return -1;
                    }
                    *addrp = *(p + 1); /* length */
                    memcpy(addrp + 1, p + 2, *(p + 1)); /* remote TSEL */
                    addrp += *(p + 1); /* move past TSEL */
                }
                break;
            case TPDU_PARM_CLGID: break; /* ignoring this for now */
            default:
                TRC(fprintf(stderr, "Inoring CR parameter: %d\n",
                    (unsigned char)*p));
            /* we silently ignore anything else */
        }
        p++;
        cnt->hlen -= (unsigned char) *p + 2;
        p += (unsigned char) *p + 1;
    }
    addrp++; /* skip to end of addr + 1 */
    if (call)
    {
        if (getpeername(fd, (struct sockaddr*) &peer, &len) < 0)
        {
            TRC(perror("getpeername()"));
            t_errno = TSYSERR;
            return -1;
        }
        *(addrp++) = sizeof(struct sockaddr_in);
        memcpy(addrp, &peer, sizeof(struct sockaddr_in));
        addrp += sizeof(struct sockaddr_in);
        call->addr.len = addrp - call->addr.buf + 1;
    }
    cnt->state = T_DATAXFER;
    cnt->event = 0;
    return 0;
}

int t_snd(int fd, char *buf, unsigned nbytes, int flags)
{
    struct rfct_control *cnt = control[fd];
    struct rfc_header rfc;
    struct iovec vec[2]; /* RFC1006 header + T-header + (user data) */
    int writ = 0, res, towrite, eot;
    int head_offset;

    TRC(fprintf(stderr, "T_SND [%d bytes, flags %d]\n", nbytes, flags));
    if (!cnt || cnt->state != T_DATAXFER)
    {
        TRC(fprintf(stderr, "Trying to write in the wrong state on fd %d.\n",
            fd));
        t_errno = TOUTSTATE;
        return -1;
    }
    if (!nbytes)
    {
        t_errno = TBADDATA;
        return -1;
    }
    do /* write the TSDU (segment) in chunks depending on the TPDU max size */
    {
        if (cnt->togo > 0) /* we have a partial TPDU on the wire */
        {
            TRC(fprintf(stderr, "  writing continuation block (%d)\n",
                cnt->togo));
            if ((res = write(fd, buf, cnt->togo)) < 0)
            {
                if (errno == EAGAIN)
                {
                    t_errno = TFLOW;
                    return -1;
                }
                cnt->togo -= res;
                return res;
            }
            writ += res;
            cnt->togo = 0;
            TRC(fprintf(stderr, "  wrote %d, total %d\n", res, writ));
        }
        else /* prepare and send (possibly partial) header */
        {
            towrite = nbytes - writ;
            if (towrite + 3 + 4 > cnt->tsize)
                towrite = cnt->tsize - (3 + 4); /* space for DATA header */
            rfc.version = RFC_VERSION;
            rfc.reserved = 0;
            rfc.len = htons(towrite + 4 + 3); /* RFC1006 length */
            rfc.hlen = 2;
            rfc.code = TPDU_CODE_DATA;
            if (flags & T_MORE || towrite + writ < nbytes)
                eot = 0;
            else
                eot = 1;
            rfc.suffix[0] = eot << 7; /* DATA EOT marker */
            if (cnt->togo < 0)
                head_offset = 7 + cnt->togo;
            else
                head_offset = 0;
            vec[0].iov_base = (caddr_t) (char*)&rfc + head_offset;
            vec[0].iov_len = 7 - head_offset;
            vec[1].iov_base = (caddr_t) buf + writ;
            vec[1].iov_len = towrite;
            TRC(fprintf(stderr, "  sending beg of block (%d+%d)\n",
                7 - head_offset, towrite));
            if ((res = writev(fd, vec, 2)) < 0)
            {
                TRC(fprintf(stderr, "  write returned -1\n"));
                /* thwarted by flow control */
                if (errno == EAGAIN)
                {
                    if (writ)
                        return writ;
                    else
                    {
                        t_errno = TFLOW;
                        return -1;
                    }
                }
                else
                    t_errno = TSYSERR;
                return -1;
            }
            /* somewhat thwarted */
            else if (res < towrite + 7 - head_offset)
            {
                /*
                 * Write came up short. We assume that this is a flow-
                 * control thing, and return immediately. Maybe it'd
                 * be better to take another loop, and generate an
                 * actual EAGAIN from write?
                 */
                TRC(fprintf(stderr, "  write returned %d\n", res));
                if (res < 7 - head_offset) /* we didn't send a full header */
                {
                    cnt->togo = -(7 - head_offset - res);
                    t_errno = TFLOW;
                    return -1;
                }
                else if ((res -= 7 - head_offset) < towrite) /* not all data */
                {
                    cnt->togo = towrite - res;
                    return nbytes - (writ + res);
                }
            }
            else /* whew... nonblocking I/O is hard work */
            {
                cnt->togo = 0;
                writ += res - (7 - head_offset);
            }
        }
    }
    while (writ < nbytes);
    TRC(fprintf(stderr, "  finishing with %d written\n", nbytes));
    return nbytes;
}

int _t_rcv(int fd, char *buf, unsigned nbytes, int *flags)
{
    struct rfct_control *cnt = control[fd];
    struct tpdu_data_header dhead;
    int res, got = 0;
    struct iovec vec[2];
    int toget;

    TRC(fprintf(stderr, "T_RCV [nbytes=%d, flags=0x%x]\n", nbytes, *flags));
    if (!cnt || cnt->state != T_DATAXFER)
    {
        t_errno = TOUTSTATE;
        return -1;
    }
    *flags = 0;
    do /* loop until we have a full TSDU or an error */
    {
        if (!cnt->event)
            if (t_look_wait(fd, 0) <= 0)
                return -1;
        if (cnt->event != T_DATA)
        {
            t_errno = TLOOK;
            return -1;
        }
        if (cnt->hlen)  /* beginning of block */
        {
            TRC(fprintf(stderr, "  Beginning of TPDU\n"));
            if (cnt->hlen > 2)
            {
                TRC(fprintf(stderr, "We can't handle parameters to DATA\n"));
                t_errno = TSYSERR;
                return -1;
            }
            toget = cnt->pending;
            if (toget > nbytes - got)
                toget = nbytes - got;
            TRC(fprintf(stderr, "  toget=%d\n", toget));
            vec[0].iov_base = (caddr_t) &dhead;
            vec[0].iov_len = 1;
            vec[1].iov_base = (caddr_t) buf + got;
            vec[1].iov_len = toget;
            if ((res = readv(fd, vec, 2)) < 0)
            {
                TRC(perror("readv()"));
                if (errno == EAGAIN)
                {
                    if (got)  /* we got some data in previous cycle */
                        break;
                    t_errno = TNODATA; /* no data */
                    return -1;
                }
                t_errno = TSYSERR;
                return -1;
            }
            TRC(fprintf(stderr, "  readv() returned %d\n", res));
            if (res == 0)
            {
                t_errno = TLOOK;
                cnt->event = T_DISCONNECT;
                return -1;
            }
            got += res - 1;
            cnt->eot_flag = (dhead.nr & DATA_EOT) >> 7;
            cnt->hlen = 0;
            cnt->pending -= got;
            TRC(fprintf(stderr, "  Got total of %d octets, %d pending\n",
                got, cnt->pending));
        }
        else /* continuation */
        {
            TRC(fprintf(stderr, "  Reading middle of TPDU\n"));
            toget = cnt->pending;
            if (toget > nbytes - got)
                toget = nbytes - got;
            TRC(fprintf(stderr, "  toget=%d\n", toget));
            if ((res = read(fd, buf + got, toget)) < 0)
            {
                TRC(perror("read()"));
                if (errno == EAGAIN)
                {
                    if (got)  /* we got some data in previous cycle */
                        break;
                    t_errno = TNODATA; /* no data */
                    return -1;
                }
                t_errno = TSYSERR;
                return -1;
            }
            TRC(fprintf(stderr, "  read() returned %d\n", res));
            if (res == 0)
            {
                t_errno = TLOOK;
                cnt->event = T_DISCONNECT;
                return -1;
            }
            got += res;
            cnt->pending -= res;
            TRC(fprintf(stderr, "  Got total of %d octets, %d pending\n",
                got, cnt->pending));
        }
        TRC(fprintf(stderr, "  bottom of loop: pending=%d, got=%d\n",
            cnt->pending, got));
    }
    while (cnt->pending && got < nbytes);
    *flags = cnt->pending || !cnt->eot_flag ? T_MORE : 0;
    TRC(fprintf(stderr, "  flags=0x%x\n", *flags));
    if (!cnt->pending)
        cnt->event = 0;
    TRC(fprintf(stderr, "  Return value: %d\n", got));
    memset(buf + got, 0, 10);
    return got;
}

#if 1

int t_rcv(int fd, char *buf, unsigned nbytes, int *flags)
{
    int res;
    int total = 0;

    do
    {
        if ((res = _t_rcv(fd, buf, nbytes, flags)) <= 0)
            return res;
        buf += res;
        nbytes -= res;
        total += res;
    }
    while (*flags & T_MORE && nbytes > 0);
    return total;
}

#endif

int t_close(int fd)
{
    struct rfct_control *cnt = control[fd];

    TRC(fprintf(stderr, "T_CLOSE\n"));
    if (!cnt || cnt->state == T_UNINIT)
    {
        TRC(fprintf(stderr, "Trying to close a bad fd.\n"));
        t_errno = TBADF;
        return -1;
    }
    free(cnt);
    return close(fd);
}

/*
 * This isn't right, obviously.
 */
int t_error(char *errmsg)
{
    TRC(fprintf(stderr, "T_ERROR\n"));
    fprintf(stderr, "t_error(t_errno=%d):", t_errno);
    perror(errmsg);
    return 0;
}    

/*
 * Put a select in here!!
 */
static int t_look_wait(int fd, int wait)
{
    struct rfct_control *cnt = control[fd];
    struct rfc_header head;
    int res;

    TRC(fprintf(stderr, "T_LOOK\n"));
    if (!cnt || cnt->state == T_UNINIT)
    {
        t_errno = TBADF;
        return -1;
    }
    if (cnt->event)
        return cnt->event;
    if (cnt->state == T_IDLE && cnt->tmpfd < 0)
        return T_LISTEN; /* the only possible type of event */
    if ((res = read_n(fd, (char*) &head, 6)) < 0)
        return -1;
    if (res == 0)
    {
        TRC(fprintf(stderr, "Network disconnect\n"));
        return cnt->event = T_DISCONNECT;
    }
    TRC(fprintf(stderr, "t_look got %d bytes\n", res));
    if (head.version != RFC_VERSION)
    {
        TRC(fprintf(stderr, "Got bad RFC1006 version in t_look: %d.\n",
            head.version));
        t_errno = TSYSERR; /* should signal protocol error, somehow ## */
        return -1;
    }
    cnt->curcode = head.code;
    cnt->hlen = head.hlen - 1; /* length of header suffix */
    cnt->pending = ntohs(head.len) - 6 - cnt->hlen;
    TRC(fprintf(stderr, "t_look: len=%d, code=0x%2.2x, hlen=%d.\n",
        cnt->pending + 6, cnt->curcode, cnt->hlen));
    switch (cnt->curcode)
    {
        case TPDU_CODE_CREQ: cnt->event = T_LISTEN; break;
        case TPDU_CODE_CCON: cnt->event = T_CONNECT; break;
        case TPDU_CODE_DATA: cnt->event = T_DATA; break;
        case TPDU_CODE_DREQ: cnt->event = T_DISCONNECT; break; 
        default:
            TRC(fprintf(stderr, "t_look: Bad package: 0x%2.2x.\n", cnt->curcode));
            t_errno = TSYSERR;  /* protocol error */
            return -1;
    }
    return cnt->event;
}    

int t_look(int fd)
{
    return t_look_wait(fd, 0);
}

/*
 * If the user doesn't provide a NSAP, and qlen > 0, we'll do a default
 * server bind. If qlen==0, we won't bind at all - connect will do that
 * for us.
 */
int t_bind(int fd, struct t_bind *req, struct t_bind *ret)
{
    struct rfct_control *cnt = control[fd];
    struct sockaddr_in addr;
    char *p;
    int got_addr = 0;

    TRC(fprintf(stderr, "T_BIND\n"));
    if (!cnt || cnt->state != T_UNBND)
    {
        TRC(fprintf(stderr, "Bad state\n"));
        t_errno = TOUTSTATE;
        return -1;
    }
    cnt->ltsel_len = 0;
    if (req)
    {
        cnt->qlen = req->qlen < MAX_QLEN ? req->qlen : MAX_QLEN;
        if (req->addr.len)
        {
            p = req->addr.buf;
            if (*p > TSEL_MAXLEN)
            {
                TRC(fprintf(stderr, "Tsel too large.\n"));
                t_errno = TBADADDR;
                return -1;
            }
            cnt->ltsel_len = *p;
            if (cnt->ltsel_len)
                memcpy(cnt->ltsel, p + 1, cnt->ltsel_len);
            p += cnt->ltsel_len + 1;
            if (*p < sizeof(addr))
            {
                TRC(fprintf(stderr, "W: No NSAP provided for local bind\n"));
            }
            else
            {
                memcpy(&addr, p + 1, sizeof(addr));
                got_addr = 1;
            }
        }
    }
    else
        cnt->qlen = 0;
    if (!got_addr) /* user didn't give an address - local bind */
    {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        if (cnt->qlen)
            addr.sin_port = htons(RFC_DEFAULT_PORT);
        else /* we'll leave binding to connect - just set dummy */
            addr.sin_port = 0; /* dummy for ret */
    }
    if (cnt->qlen && bind(fd, (struct sockaddr *) &addr,
         sizeof(addr)) < 0 )
    {
        TRC(perror("bind()"));
        t_errno = TSYSERR; /* this should be refined */
        return -1;
    }
    if (cnt->qlen)
    {
        if (listen(fd, cnt->qlen) < 0)
        {
            t_errno = TSYSERR;
            return -1;
        }
        cnt->listen = 1;
        TRC(fprintf(stderr, "  listen OK\n"));
    }
    cnt->state = T_IDLE;
    /* All right! Now let's give something back, if our user wants it */
    if (ret)
    {
        ret->qlen = cnt->qlen;
        if (ret->addr.maxlen < (ret->addr.len = cnt->ltsel_len + 2 +
            sizeof(addr)))
        {
            /* No space - but we're still bound */
            t_errno = TBUFOVFLW;
            ret->addr.len = 0;
            return -1;
        }
        p = ret->addr.buf;
        *(p++) = cnt->ltsel_len;
        if (cnt->ltsel_len)
            memcpy(p, cnt->ltsel, cnt->ltsel_len);
        p += cnt->ltsel_len;
        *(p++) = sizeof(addr);
        memcpy(p, &addr, sizeof(addr));
    }
    return 0;
}    

/*
 * need to consult RFC1006 on these. I think they just map to close()...
 */
int t_snddis(int fd, struct t_call *call)
{
    TRC(fprintf(stderr, "T_SNDDIS\n"));
    return 0;
}

int t_rcvdis(int fd, struct t_discon *discon)
{
    struct rfct_control *cnt = control[fd];
    struct tpdu_disconnect_header chead;
    char udata[64], buf[256], *p;

    TRC(fprintf(stderr, "T_RCVDIS\n"));
    if (!cnt) 
    {
        TRC(fprintf(stderr, "TOUTSTATE\n"));
        t_errno = TOUTSTATE;
        return -1;
    }
    if (!cnt->event)
        if (t_look_wait(fd, 1) <= 0)
            return -1;
    if (cnt->event != T_DISCONNECT)
    {
        t_errno = TLOOK;
        return -1;
    }
    /* read the rest of the DR TPDU */
    if (read_n(fd, buf, cnt->hlen) <= 0)
        return -1;
    memcpy(&chead, buf, 5);
    cnt->hlen -= 5;
    for (p = buf + 5; cnt->hlen > 0;)
    {
        switch ((unsigned char)*p)
        {
            default:
                TRC(fprintf(stderr, "Inoring RD parameter: %d\n",
                    (unsigned char)*p));
            /* we silently ignore anything else */
        }
        p++;
        cnt->hlen -= (unsigned char) *p + 2;
        p += (unsigned char) *p + 1;
    }
    if (cnt->pending)
    {
        if (read_n(fd, udata, cnt->pending) < cnt->pending)
        {
            TRC(fprintf(stderr, "Unable to read user data\n"));
            t_errno = TSYSERR;
            return -1;
        }
    }
    cnt->state = T_IDLE;  /* should we close transport? */
    cnt->event = 0;
    if (discon)
    {
        discon->sequence = -1; /* TOFIX */
        discon->reason = chead.reason;
        TRC(fprintf(stderr, "Diconnect reason %d\n", chead.reason));
        if (cnt->pending > discon->udata.maxlen)
        {
            t_errno = TBUFOVFLW;
            return -1;
        }
        if (cnt->pending)
        {
            memcpy(discon->udata.buf, udata, cnt->pending);
            udata[cnt->pending] = '\0';
            TRC(fprintf(stderr, "Discon udata: '%s'\n", udata));
        }
        discon->udata.len = cnt->pending;
    }
    return 0;
}    

/*
 * fix memory management, you bad Sebastian!
 */
int t_free(char *ptr, int struct_type)
{
    TRC(fprintf(stderr, "T_FREE\n"));
    free(ptr);
    return 0;
}    

char *t_alloc(int fd, int struct_type, int fields)
{
    char *r = malloc(1024);
    if (!r)
        return 0;
    TRC(fprintf(stderr, "T_ALLOC\n"));
    memset(r, 0, 1024);
    return r;
}

/*
 * this is required for t_listen... if a system doesn't support dup2(), we're
 * in trouble: We might not be able to do nonblocking listen. Time will tell.
 */
static int switch_fds(int fd1, int fd2)
{
    int tmp;
    struct rfct_control *tmpc;
    
    TRC(fprintf(stderr, "Switching fds %d <--> %d\n", fd1, fd2));
    if ((tmp = dup(fd1)) < 0 ||
        dup2(fd2, fd1) < 0 ||
        dup2(tmp, fd2) < 0 ||
        close(tmp) < 0)
            return -1;
    tmpc = control[fd1];
    control[fd1] = control[fd2];
    control[fd2] = tmpc;
    return 0;
}

static int rcvconreq(int fd, struct t_call *call)
{
    struct rfct_control *cnt = control[fd];
    struct rfct_control *new = control[cnt->tmpfd];
    struct tpdu_connect_header chead;
    char buf[100];
    char *p, *addrp = 0;
    struct sockaddr_in addr;
    int len = sizeof(struct sockaddr_in);
    int qslot;

    TRC(fprintf(stderr, "RCVCONRES\n"));
    if (!call)
    {
        t_errno = TSYSERR;
        return -1;
    }
    for (qslot = 0; qslot < cnt->qlen; qslot++)
        if (cnt->oci[qslot] < 0)
            break;
    if (qslot == cnt->qlen) /* no free slots - shouldn't happen here */
    {
        t_errno = TBADF;
        return -1;
    }
    /* read the rest of the CREQ TPDU */
    if (read_n(cnt->tmpfd, buf, new->hlen) <= 0)
        return -1;
    memcpy(&chead, buf, 5);
    if (chead.class != 0)
    {
        TRC(fprintf(stderr, "Expected TP0, got %d\n", (int)chead.class));
        t_errno = TSYSERR;
        return -1;
    }
    memcpy(new->rref, chead.src_ref, 2); /* we'll echo this back at her */
    new->hlen -= 5;
    if (call && call->addr.maxlen)
        *(addrp = call->addr.buf) = 0;
    for (p = buf + 5; new->hlen > 0;)
    {
        switch ((unsigned char)*p)
        {
            case TPDU_PARM_TSIZE:
                new->tsize = 1 << *(p + 2); /* we go with their max */
                break;
            case TPDU_PARM_CLDID: break; /* ignore */
            case TPDU_PARM_CLGID:
                if (addrp)
                {
                    if (*(p + 1) > TSEL_MAXLEN)
                    {
                        TRC(fprintf(stderr, "Called TSEL too long.\n"));
                        t_errno = TSYSERR; /* Wrong.. ## */
                        return -1;
                    }
                    *addrp = *(p + 1); /* length */
                    memcpy(addrp + 1, p + 2, *(p + 1)); /* remote TSEL */
                    addrp += *(p + 1); /* move past TSEL */
                }
                break;
            /* we silently ignore preferred TPDU size and others */
        }
        p++;
        new->hlen -= (unsigned char) *p + 2;
        p += (unsigned char) *p + 1;
    }
    if (addrp)
    {
        addrp++;
        if (getpeername(cnt->tmpfd, (struct sockaddr*) &addr, &len) < 0)
        {
            TRC(perror("getpeername()"));
            t_errno = TSYSERR;
            return -1;
        }
        *(addrp++) = sizeof(struct sockaddr_in);
        memcpy(addrp, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    }
    new->event = 0;
    cnt->event = 0;
    cnt->oci[qslot] = cnt->tmpfd; /* move the new semi-connection to oci */
    call->sequence = qslot;
    cnt->tmpfd = -1;
    cnt->state = T_INCON;
    return 0;
}

/*
 * This construction is tricky in async mode: listen calls accept, which
 * generates a new fd for the association. Call is supposed to return
 * immediately if there is no CR on the line (which is highly likely),
 * and t-user will then try to use select() on the wrong socket, waiting for
 * the transport level package. 
 *
 * Compared to the upper-level u_* routines, we have one major handicap
 * and one major benefit: We *have* to handle two different endpoints
 * after t_listen (which includes sockets accept); and we have access
 * to dup2().
 *
 * If the endpoint is configured for nonblocking I/O, and listen is not
 * able to immediately acquire the CR, the two fds  are switched, so that
 * subsequent selects take place on the data socket, rather than the
 * listener socket.
 *
 * At any rate, the data socket is saved, so that it can later be given
 * to the user in t_accept().
 */
int t_listen(int fd, struct t_call *call)
{
    struct rfct_control *cnt = control[fd], *new;
    struct sockaddr_in addr;
    int addrlen = sizeof(struct sockaddr_in);
    int tmpfd_is_new = 0; /* we've just accept()ed a connection */
    int event, i;

    TRC(fprintf(stderr, "T_LISTEN\n"));
    if (!cnt || cnt->state != T_IDLE)
    {
        TRC(fprintf(stderr, "T_listen expects state==T_IDLE (wrong?)\n"));
        t_errno = TOUTSTATE;
        return -1;
    }
    if (!cnt->qlen)
    {
        t_errno = TBADQLEN;
        return -1;
    }
    for (i = 0; i < cnt->qlen; i++)
        if (cnt->oci[i] < 0)
            break;
    if (i == cnt->qlen) /* no slots in queue */
    {
        TRC(fprintf(stderr, "No more space in queue\n"));
        t_errno = TBADF; /* what would be more correct? */
        return -1;
    }
    if (cnt->tmpfd < 0)
    {
        TRC(fprintf(stderr, "Accept..."));
        if ((cnt->tmpfd = accept(fd, (struct sockaddr*) &addr, &addrlen)) < 0)
        {
            if (errno == EWOULDBLOCK)
            {
                t_errno = TNODATA;
                TRC(fprintf(stderr, "Accept returned WOULDBLOCK\n"));
            }
            else
            {
                TRC(fprintf(stderr, "accept failed\n"));
                t_errno = TSYSERR;
            }
            return -1;
        }
#ifdef NONBLOCKING_OSI
        if ((cnt->flags & O_NONBLOCK) && fcntl(cnt->tmpfd, F_SETFL,
            O_NONBLOCK) < 0)
        {
            t_errno = TSYSERR;
            return -1;
        }
#endif
        tmpfd_is_new = 1;
        TRC(fprintf(stderr, "Accept OK\n"));
        if (!(new = control[cnt->tmpfd] = malloc(sizeof(*new))))
        {
            TRC(perror("malloc()"));
            t_errno = TSYSERR;
            return -1;
        }
        new->togo = 0;
        new->pending = 0;
        new->state = T_IDLE;
        new->event = 0;
        new->listen = 0;
        new->qlen = cnt->qlen;
        new->flags = cnt->flags;
        new->tmpfd = cnt->tmpfd;
        new->ltsel_len = 0;
        for (i = 0; i < MAX_QLEN; i++)
            new->oci[i] = -1;
    }
    /* we got a network connection. Now try to read transport CREQ TPDU */
    if ((event = t_look_wait(tmpfd_is_new ? cnt->tmpfd : fd, 1)) < 0)
    {
        if (t_errno == TNODATA)
        {
            if (tmpfd_is_new)
            {
                /*
                 * We give the user something to select on for the incoming
                 * CR. Switch the new association with the listener socket.
                 */
                TRC(fprintf(stderr, "Switching FDs\n"));
                if (switch_fds(cnt->tmpfd, fd) < 0)
                {
                    t_errno = TSYSERR;
                    return -1;
                }
            }
            return -1;
        }
        else
        {
            t_close(cnt->tmpfd);
            cnt->tmpfd = -1;
        }
        return -1; /* t_look & t_read hopefully set up the right errcodes */
    }
    else
    {
        /* We got something! */
        if (event != T_LISTEN)
        {
            TRC(fprintf(stderr, "Expected T_LISTEN\n"));
            t_errno = TLOOK;
            return -1;
        }
        /*
         * switch back the fds, if necessary */
        if (!tmpfd_is_new && switch_fds(fd, cnt->tmpfd) < 0)
        {
            t_errno = TSYSERR;
            return -1;
        }
        if (rcvconreq(fd, call) < 0)
        {
            t_close(cnt->tmpfd);
            cnt->tmpfd = -1;
        }
        return 0;
    }
}    

/*
 * There's no clean mapping of this onto the socket interface. If someone
 * wants it, we could fake it by first closing the socket, and then
 * opening a new one and doing a dup2. That should be functionally
 * equivalent(?).
 */
int t_unbind(int fd)
{
    TRC(fprintf(stderr,
        "T_UNBIND [not supported by transport implementation]\n"));
    t_errno = TNOTSUPPORT;
    return -1;
}    

int t_accept(int fd, int resfd, struct t_call *call)
{
    struct rfct_control *listener = control[fd]; /* listener handle */
    struct rfct_control *new; /* new, semi-complete association */
    struct rfct_control *res; /* resulting association handle */
    struct iovec vec[3]; /* RFC1006 header + T-header + parm */
    struct rfc_header rfc;
    struct tpdu_connect_header tpdu;
    unsigned char parm[3 + TSEL_MAXLEN + 2];
    int i, newfd;
    
    TRC(fprintf(stderr, "T_ACCEPT\n"));
    if (!listener || listener->state != T_INCON)
    {
        TRC(fprintf(stderr, "TOUTSTATE\n"));
        t_errno = TOUTSTATE;
        return -1;
    }
    /* Get the semi-connection */
    if (call->sequence >= listener->qlen || listener->oci[call->sequence] < 0)
    {
        TRC(fprintf(stderr, "TBADSEQ\n"));
        t_errno = TBADSEQ;
        return -1;
    }
    new = control[(newfd = listener->oci[call->sequence])];
    listener->oci[call->sequence] = -1;
    res = control[resfd];
    if (!res)
    {
        t_errno = TBADF;
        return -1;
    }
    if (res != listener) /* move the new connection */
    {
        TRC(fprintf(stderr, "  Moving to new fd (%d)\n", resfd));
        if (res->state != T_IDLE || res->qlen)
        {
            TRC(fprintf(stderr, "Trying to move new assc. to bad fd.\n"));
            t_errno = TBADF;
            return -1;
        }
        dup2(newfd, resfd); /* closes resfd */
        close(newfd);
        control[resfd] = new;
        /* transfer local bindings from res */
        if (res->ltsel_len)
            memcpy(control[resfd]->ltsel, res->ltsel, res->ltsel_len);
        control[resfd]->ltsel_len = res->ltsel_len;
        free(res);
        res = control[resfd];
        listener->event = 0;
        listener->state = T_IDLE;
    }
    else /* lose our listener */
    {
        TRC(fprintf(stderr, "  Moving to listener fd\n"));
        for (i = 0; i < listener->qlen; i++)
            if (listener->oci[i] >= 0)
            {
                TRC(fprintf(stderr, "Still conn indications on listener\n"));
                t_errno = TBADF;
                return -1;
            }
        dup2(newfd, fd);
        close(newfd);
        control[fd] = new;
        if (listener->ltsel_len)
            memcpy(control[resfd]->ltsel, listener->ltsel, listener->ltsel_len);
        control[resfd]->ltsel_len = listener->ltsel_len;
        free(listener);
        res = control[resfd];
    }
    rfc.version = RFC_VERSION;
    rfc.reserved = 0;
    rfc.code = TPDU_CODE_CCON;

    memcpy(tpdu.src_ref, "AA", 2);
    memcpy(tpdu.dst_ref, res->rref, 2); /* echo back at 'em */
    tpdu.class = 0;

    /* grant them their TPDU size */
    parm[0] = TPDU_PARM_TSIZE;
    parm[1] = 1;
    for (i = 7; i <= 11 && (1 << i) < res->tsize; i++) ; /* encode TPDU size */
    parm[2] = i;
    /* give our TSEL. ## Must we echo theirs, if given? check spec */
    /* I thought it was ok to give an empty TSEL. Does it have semantic sig? */
    parm[3] = TPDU_PARM_CLDID;
    parm[4] = res->ltsel_len; 
    if (res->ltsel_len)
        memcpy(parm + 5, res->ltsel, res->ltsel_len);

    rfc.len =  htons(4 + 7 + 3 + 2 + res->ltsel_len);
    rfc.hlen = 6 + 3 + 2 + res->ltsel_len;
    vec[0].iov_base = (caddr_t) &rfc;
    vec[0].iov_len = 6;
    vec[1].iov_base = (caddr_t) &tpdu;
    vec[1].iov_len = 5;
    vec[2].iov_base = (caddr_t) parm;
    vec[2].iov_len = 3 + 2 + res->ltsel_len;
    if (writev(resfd, vec, 3) < 4 + 7 + 3 + (2 + res->ltsel_len))
    {
        TRC(fprintf(stderr, "writev came up short. Aborting connect\n"));
        t_errno = TSYSERR;
        return -1;
    }
    res->state = T_DATAXFER;
    res->event = 0;
    return 0;
}    

int t_getstate(int fd)
{
    TRC(fprintf(stderr, "T_GETSTATE\n"));
    return control[fd] ? control[fd]->state : T_UNINIT;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

