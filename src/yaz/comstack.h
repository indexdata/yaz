/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file comstack.h
 * \brief Header for COMSTACK
 */

#ifndef COMSTACK_H
#define COMSTACK_H

#include <yaz/yconfig.h>
#include <yaz/oid_util.h>
#include <yaz/xmalloc.h>

YAZ_BEGIN_CDECL

struct comstack;
typedef struct comstack *COMSTACK;
typedef COMSTACK (*CS_TYPE)(int s, int flags, int protocol, void *vp);

struct comstack
{
    CS_TYPE type;
    int cerrno;     /* current error code of this stack */
    int iofile;    /* UNIX file descriptor for iochannel */
    void *cprivate;/* state info for lower stack */
    int max_recv_bytes;      /* max size of incoming package */
    int state;     /* current state */
#define CS_ST_UNBND      0
#define CS_ST_IDLE       1
#define CS_ST_INCON      2
#define CS_ST_OUTCON     3
#define CS_ST_DATAXFER   4
#define CS_ST_ACCEPT     5
#define CS_ST_CONNECTING 6
    int newfd;     /* storing new descriptor between listen and accept */
    int flags;     /* flags, blocking etc.. CS_FLAGS_..  */
    unsigned io_pending; /* flag to signal read / write op is incomplete */
    int event;     /* current event */
#define CS_NONE       0
#define CS_CONNECT    1
#define CS_DISCON     2
#define CS_LISTEN     3
#define CS_DATA       4
    enum oid_proto protocol;  /* what application protocol are we talking? */
    int (*f_put)(COMSTACK handle, char *buf, int size);
    int (*f_get)(COMSTACK handle, char **buf, int *bufsize);
    int (*f_more)(COMSTACK handle);
    int (*f_connect)(COMSTACK handle, void *address);
    int (*f_rcvconnect)(COMSTACK handle);
    int (*f_bind)(COMSTACK handle, void *address, int mode);
#define CS_CLIENT 0
#define CS_SERVER 1
    int (*f_listen)(COMSTACK h, char *raddr, int *addrlen,
                   int (*check_ip)(void *cd, const char *a, int len, int type),
                   void *cd);
    COMSTACK (*f_accept)(COMSTACK handle);
    void (*f_close)(COMSTACK handle);
    const char *(*f_addrstr)(COMSTACK handle);
    void *(*f_straddr)(COMSTACK handle, const char *str);
    int (*f_set_blocking)(COMSTACK handle, int blocking);
    void *user;       /* user defined data associated with COMSTACK */
};

#define cs_put(handle, buf, size) ((*(handle)->f_put)(handle, buf, size))
#define cs_get(handle, buf, size) ((*(handle)->f_get)(handle, buf, size))
#define cs_more(handle) ((*(handle)->f_more)(handle))
#define cs_connect(handle, address) ((*(handle)->f_connect)(handle, address))
#define cs_rcvconnect(handle) ((*(handle)->f_rcvconnect)(handle))
#define cs_bind(handle, ad, mo) ((*(handle)->f_bind)(handle, ad, mo))
#define cs_listen(handle, ap, al) ((*(handle)->f_listen)(handle, ap, al, 0, 0))
#define cs_listen_check(handle, ap, al, cf, cd) ((*(handle)->f_listen)(handle, ap, al, cf, cd))
#define cs_accept(handle) ((*(handle)->f_accept)(handle))
#define cs_close(handle) ((*(handle)->f_close)(handle))
#define cs_create(type, flags, proto) ((*type)(-1, flags, proto, 0))
#define cs_createbysocket(sock, type, flags, proto) \
        ((*type)(sock, flags, proto, 0))
#define cs_type(handle) ((handle)->type)
#define cs_fileno(handle) ((handle)->iofile)
#define cs_getstate(handle) ((handle)->getstate)
#define cs_errno(handle) ((handle)->cerrno)
#define cs_getproto(handle) ((handle)->protocol)
#define cs_addrstr(handle) ((*(handle)->f_addrstr)(handle))
#define cs_straddr(handle, str) ((*(handle)->f_straddr)(handle, str))
#define cs_want_read(handle) ((handle)->io_pending & CS_WANT_READ)
#define cs_want_write(handle) ((handle)->io_pending & CS_WANT_WRITE)
#define cs_set_blocking(handle,blocking) ((handle)->f_set_blocking(handle, blocking))

#define CS_WANT_READ 1
#define CS_WANT_WRITE 2

YAZ_EXPORT int cs_look (COMSTACK);
YAZ_EXPORT const char *cs_strerror(COMSTACK h);
YAZ_EXPORT const char *cs_errmsg(int n);
/** \brief returns COMSTACK error and additional information
    \param cs COMSTACK handle
    \param details additional error information (result), or NULL
    \returns error code
 */
YAZ_EXPORT int cs_get_error(COMSTACK cs, const char **details);
/** \brief Creates an endpoint from a host or URI specification
    \param type_and_host host or URI; see cs_create_host2 for syntax
    \param flags combination of CS_FLAGS_* bits; zero for nonblocking I/O
    \param vp required address output pointer; owned by the returned COMSTACK
    \returns new COMSTACK, or NULL on failure

    Equivalent to cs_create_host2 with no proxy_host and with the proxy mode
    output discarded. This creates an endpoint but does not connect it.
    Pass the address of a void * variable as vp, then use *vp with cs_connect
    or cs_bind. Do not free *vp; cs_straddr may replace it and cs_close
    releases it. Do not use *vp on failure. Close the endpoint with cs_close.
    \see cs_create_host2
 */
YAZ_EXPORT COMSTACK cs_create_host(const char *type_and_host,
                                   int flags, void **vp);

/** \brief Creates an endpoint with an optional proxy
    \param vhost target host or URI; see cs_create_host2 for syntax
    \param flags combination of CS_FLAGS_* bits; zero for nonblocking I/O
    \param vp required address output pointer; owned by the returned COMSTACK
    \param proxy_host proxy host or URI, or NULL for no separate proxy
    \returns new COMSTACK, or NULL on failure

    Equivalent to cs_create_host2 with the proxy mode output discarded.
    Address ownership and connection handling are as for cs_create_host.
    \see cs_create_host2
 */
YAZ_EXPORT COMSTACK cs_create_host_proxy(const char *vhost,
                                         int flags, void **vp,
                                         const char *proxy_host);
/** \brief Creates an endpoint and selects direct or proxy connection handling
    \param vhost target host or URI specification (required)
    \param flags combination of CS_FLAGS_* bits; zero for nonblocking I/O
    \param vp required address output pointer; owned by the returned COMSTACK
    \param proxy_host proxy host or URI, or NULL for no separate proxy
    \param proxy_mode required output pointer: 0 for direct/CONNECT, 1 for
           application-level proxying; only use this value on success
    \returns new COMSTACK, or NULL on failure

    Supported case-sensitive target forms are:
    - host[:port] or tcp:host[:port] for Z39.50 over TCP/IP (default port 210).
    - ssl:host[:port] for Z39.50 over TLS (default port 210).
    - http://host[:port][/path] for HTTP (default port 80).
    - https://host[:port][/path] for HTTPS (default port 443).
    - unix:path for Z39.50 over a UNIX socket.
    - unix:path:target to use a UNIX socket with the protocol and arguments
      from target, for example unix:/tmp/yaz.sock:http://localhost/sru.
      Use an unencrypted target to retain the UNIX transport.
    - connect:[user:password@]proxy-host:proxy-port,target for an explicit
      HTTP CONNECT tunnel to a TCP/IP or TLS target.

    Bracket IPv6 addresses, for example tcp:[::1]:210. Only HTTP/HTTPS
    prefixes strip leading slashes; tcp://host is not the TCP syntax.
    Network address resolution ignores suffixes starting with / or ?;
    the application interprets paths, queries, and database names.
    These specifications are not percent-decoded. Direct URI user information
    and fragments are not interpreted. TLS requires GnuTLS support, and UNIX
    sockets are unavailable on Windows. UNIX addresses also accept
    file=path,user=name-or-id,group=name-or-id,umask=mode, with file required
    and the other keys optional. The octal umask value sets socket permissions
    directly when binding; it is not a mask to subtract.

    A local source address may follow the first space in vhost for TCP/IP or
    TLS. Put a path slash before the space, for example
    "tcp:server.example:210/ 192.0.2.10", so resolution excludes the suffix.
    The source port is forced to zero; for an IPv6 source include a port
    placeholder, for example "http://[::1]/ [::1]:0".

    A separate proxy_host must select TCP/IP, not TLS or UNIX sockets, and
    must not contain another proxy specification. TLS targets always use
    CONNECT. A Z39.50 target with an HTTP proxy also uses CONNECT. Other
    TCP/IP target/proxy combinations use application-level proxying.
    In that case *proxy_mode is 1 and the caller must supply the proxy
    protocol messages (for example absolute HTTP request URIs). With CONNECT,
    *proxy_mode is 0 and cs_connect/cs_rcvconnect perform the tunnel handshake.
    Explicit connect: and unix:path:target forms override proxy_host.

    Specify both proxy and target ports explicitly for CONNECT: the target
    authority is sent without adding a default port, and an omitted proxy
    port defaults according to the target transport/protocol. Credentials
    before @ generate a Basic Proxy-Authorization header for CONNECT only;
    they may also appear in proxy_host when CONNECT is selected.

    This function does not establish a connection. On success, pass *vp to
    cs_connect or cs_bind and eventually release the endpoint with cs_close.
    Do not free *vp; cs_straddr may replace it. Do not use *vp on failure.
    With CS_FLAGS_DNS_NO_BLOCK and resolver thread support, resolution errors
    may be reported later during connection establishment.
    \see cs_create_host, cs_create_host_proxy, cs_parse_host, cs_get_host_args
 */
YAZ_EXPORT COMSTACK cs_create_host2(const char *vhost, int flags, void **vp,
                                    const char *proxy_host, int *proxy_mode);
/** \brief Extracts application arguments from a host or URI specification
    \param type_and_host host or URI specification (required)
    \param args required output pointer to borrowed arguments, or an empty string

    Returns the portion after the first path slash, skipping a :// separator.
    For unix:path:args, returns args directly if that part contains no colon;
    otherwise extracts the target path. For example tcp:localhost:210/books
    yields books, and http://localhost/sru?version=1.2 yields sru?version=1.2.
    No decoding is performed. Do not free *args; a nonempty result points
    into type_and_host and is valid only while that input remains valid.
 */
YAZ_EXPORT void cs_get_host_args(const char *type_and_host, const char **args);
/** Returns number of bytes for complete PDU, 0 if incomplete, -1 on protocol error */
YAZ_EXPORT int cs_complete_auto_head(const char *buf, int len);
/** Returns number of bytes for complete PDU, 0 if incomplete, -1 on protocol error */
YAZ_EXPORT int cs_complete_auto(const char *buf, int len);
YAZ_EXPORT void *cs_get_ssl(COMSTACK cs)
#ifdef __GNUC__
    __attribute__ ((deprecated))
#endif
    ;
YAZ_EXPORT int cs_set_ssl_ctx(COMSTACK cs, void *ctx)
#ifdef __GNUC__
    __attribute__ ((deprecated))
#endif
    ;
YAZ_EXPORT int cs_set_ssl_certificate_file(COMSTACK cs, const char *fname);
YAZ_EXPORT int cs_get_peer_certificate_x509(COMSTACK cs, char **buf, int *len);
YAZ_EXPORT void cs_set_max_recv_bytes(COMSTACK cs, int max_recv_bytes);
YAZ_EXPORT void cs_print_session_info(COMSTACK cs);

/** \brief Parses transport and protocol prefixes without resolving an address
    \param uri host or URI specification (required); see cs_create_host2
    \param host required output pointer into uri after recognized prefixes
    \param t required transport output pointer
    \param proto required protocol output pointer: PROTO_Z3950 or PROTO_HTTP
    \param connect_host required output pointer to an allocated proxy/socket
           address, or NULL when no such address is specified
    \returns 1 on success, 0 for an unavailable transport

    Recognizes connect: and unix: wrappers followed by tcp:, ssl:, http:,
    or https:. With no recognized protocol prefix, selects Z39.50.
    The default transport is TCP/IP. This function only parses prefixes;
    success does not imply that the address is valid or resolvable.
    On success, free a non-NULL *connect_host with xfree. Do not free *host;
    it is valid only while uri remains valid. On failure, *connect_host is
    NULL and the other outputs must not be used.
 */
YAZ_EXPORT int cs_parse_host(const char *uri, const char **host,
                             CS_TYPE *t, enum oid_proto *proto,
                             char **connect_host);
/** \brief Selects HTTP header-only framing for TCP/IP, SSL or UNIX transports
    \param cs COMSTACK handle
    \param head_only nonzero to stop at the end of HTTP headers; zero to
           restore normal message framing
    \returns 0 on success, -1 with CSOUTSTATE for unsupported transports
 */
YAZ_EXPORT int cs_set_head_only(COMSTACK cs, int head_only);

/*
 * error management.
 */

#define CSNONE     0
#define CSYSERR    1
#define CSOUTSTATE 2
#define CSNODATA   3
#define CSWRONGBUF 4
#define CSDENY     5
#define CSERRORSSL 6
#define CSBUFSIZE  7
#define CSPROTERR  8
#define CSLASTERROR CSPROTERR  /* must be the value of last CS error */

#define CS_FLAGS_BLOCKING 1
#define CS_FLAGS_NUMERICHOST 2
#define CS_FLAGS_DNS_NO_BLOCK 4
#define CS_FLAGS_CHECK_CERT 8

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
