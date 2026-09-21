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
 * \brief Transport-independent communication endpoints for BER and HTTP
 *
 * Most operations dispatch through transport callbacks. Unless stated otherwise,
 * handles and output pointers must be non-NULL. Macro arguments must not have
 * side effects: a handle expression can be evaluated more than once.
 */

#ifndef COMSTACK_H
#define COMSTACK_H

#include <yaz/yconfig.h>
#include <yaz/oid_util.h>
#include <yaz/xmalloc.h>

YAZ_BEGIN_CDECL

struct comstack;
/** \brief Communication endpoint; release with cs_close. */
typedef struct comstack *COMSTACK;
/** \brief Transport constructor and transport identity
    \param s existing socket, or -1 to create a new endpoint
    \param flags combination of CS_FLAGS_* bits
    \param protocol application protocol (enum oid_proto)
    \param vp transport-specific initialization data, or NULL
    \returns new endpoint, or NULL on failure
 */
typedef COMSTACK (*CS_TYPE)(int s, int flags, int protocol, void *vp);

/** \brief Public endpoint state and transport dispatch table

    Transport implementations maintain these members. Applications may use user
    for their own data; use the API operations to change transport state.
    cprivate must only be interpreted after checking type.
 */
struct comstack
{
    CS_TYPE type; /**< Transport constructor, identifying the implementation (CS_TYPE). */
    int cerrno; /**< Last COMSTACK error code; see cs_get_error. */
    int iofile; /**< Socket descriptor, or -1 when no socket has been created. */
    void *cprivate; /**< Opaque transport-owned state; its layout depends on type. */
    int max_recv_bytes; /**< Maximum receive size in bytes; see cs_set_max_recv_bytes. */
    int state; /**< Current connection state (CS_ST_*). */
/** Unbound endpoint. */
#define CS_ST_UNBND      0
/** Bound endpoint ready to listen. */
#define CS_ST_IDLE       1
/** Incoming connection awaiting cs_accept. */
#define CS_ST_INCON      2
/** Outgoing connection state (legacy). */
#define CS_ST_OUTCON     3
/** Established connection ready for data transfer. */
#define CS_ST_DATAXFER   4
/** Accepted connection with a pending TLS handshake. */
#define CS_ST_ACCEPT     5
/** Outgoing connection establishment in progress. */
#define CS_ST_CONNECTING 6
    int newfd; /**< Accepted socket held between cs_listen and cs_accept. */
    int flags; /**< Endpoint options, a combination of CS_FLAGS_* bits. */
    unsigned io_pending; /**< Pending I/O directions, a combination of CS_WANT_* bits. */
    int event; /**< Last recorded event (CS_NONE, CS_CONNECT, etc.); see cs_look. */
/** No event recorded. */
#define CS_NONE       0
/** Connection establishment event. */
#define CS_CONNECT    1
/** Disconnection event. */
#define CS_DISCON     2
/** Listen event. */
#define CS_LISTEN     3
/** Data transfer event. */
#define CS_DATA       4
    enum oid_proto protocol; /**< Application protocol, such as PROTO_Z3950 or PROTO_HTTP. */
    /** Transport implementation of cs_put. */
    int (*f_put)(COMSTACK handle, char *buf, int size);
    /** Transport implementation of cs_get. */
    int (*f_get)(COMSTACK handle, char **buf, int *bufsize);
    /** Transport implementation of cs_more. */
    int (*f_more)(COMSTACK handle);
    /** Transport implementation of cs_connect. */
    int (*f_connect)(COMSTACK handle, void *address);
    /** Transport implementation of cs_rcvconnect. */
    int (*f_rcvconnect)(COMSTACK handle);
    /** Transport implementation of cs_bind. */
    int (*f_bind)(COMSTACK handle, void *address, int mode);
/** Bind a local address without listening. */
#define CS_CLIENT 0
/** Bind a local address and listen for connections. */
#define CS_SERVER 1
    /** Transport implementation of cs_listen. */
    int (*f_listen)(COMSTACK h, char *raddr, int *addrlen,
                   int (*check_ip)(void *cd, const char *a, int len, int type),
                   void *cd);
    /** Transport implementation of cs_accept. */
    COMSTACK (*f_accept)(COMSTACK handle);
    /** Transport implementation of cs_close. */
    void (*f_close)(COMSTACK handle);
    /** Transport implementation of cs_addrstr. */
    const char *(*f_addrstr)(COMSTACK handle);
    /** Transport implementation of cs_straddr. */
    void *(*f_straddr)(COMSTACK handle, const char *str);
    /** Transport implementation of cs_set_blocking. */
    int (*f_set_blocking)(COMSTACK handle, int blocking);
    void *user; /**< Application-owned data; cs_close does not free it. */
};

/** \brief Sends a complete message
    \param handle endpoint
    \param buf message bytes; keep valid until the write completes
    \param size number of bytes to send
    \returns 0 when complete, 1 when pending, -1 on error

    Retry pending writes with the same buffer and size after waiting for the
    directions indicated by cs_want_read and cs_want_write.
 */
#define cs_put(handle, buf, size) ((*(handle)->f_put)(handle, buf, size))
/** \brief Receives a BER or HTTP message
    \param handle endpoint
    \param buf address of a receive buffer pointer; initialize *buf to NULL
    \param size address of its allocated capacity; initialize *size to zero
    \returns message length (>1), 1 when incomplete, 0 on EOF, -1 on error

    The transport allocates, resizes, or replaces *buf and updates *size.
    Retain both values for subsequent calls and eventually free *buf with
    xfree. A pending read may need either readability or writability; consult
    cs_want_read and cs_want_write.
 */
#define cs_get(handle, buf, size) ((*(handle)->f_get)(handle, buf, size))
/** \brief Tests whether buffered input can be processed without another read
    \param handle endpoint
    \returns nonzero if a complete message or framing error is buffered

    Does not poll the socket. Call before waiting for socket readability.
 */
#define cs_more(handle) ((*(handle)->f_more)(handle))
/** \brief Starts an outgoing connection
    \param handle endpoint
    \param address transport address returned by cs_straddr or cs_create_host
    \returns 0 when connected, 1 when pending, -1 on error

    For a pending connection, wait as indicated by cs_want_read/cs_want_write
    and continue with cs_rcvconnect.
 */
#define cs_connect(handle, address) ((*(handle)->f_connect)(handle, address))
/** \brief Continues a pending outgoing connection, including TLS or CONNECT
    \param handle endpoint
    \returns 0 when connected, 1 when pending, -1 on error
 */
#define cs_rcvconnect(handle) ((*(handle)->f_rcvconnect)(handle))
/** \brief Binds an endpoint to a local address
    \param handle endpoint
    \param ad transport address returned by cs_straddr or cs_create_host
    \param mo CS_SERVER to bind and listen, CS_CLIENT to bind only
    \returns 0 on success, -1 on error
 */
#define cs_bind(handle, ad, mo) ((*(handle)->f_bind)(handle, ad, mo))
/** \brief Receives an incoming connection for subsequent cs_accept
    \param handle listening endpoint
    \param ap optional buffer for the transport's binary peer address
    \param al address buffer capacity on input, copied length on output;
           NULL to discard the address (ap may then be NULL)
    \returns 0 when a connection is ready, -1 on error (including CSNODATA
             when a nonblocking listener has no connection available)

    An insufficient address buffer produces an output length of zero.
 */
#define cs_listen(handle, ap, al) ((*(handle)->f_listen)(handle, ap, al, 0, 0))
/** \brief Receives an incoming connection with an optional access check
    \param handle listening endpoint
    \param ap optional peer address buffer, as for cs_listen
    \param al address buffer capacity/result length, as for cs_listen
    \param cf callback or NULL; nonzero rejects the connection with CSDENY
    \param cd application data passed to cf
    \returns 0 when a connection is ready, -1 on error

    The callback receives cd, a binary peer address, its byte length, and its
    address family. The check is supported by TCP/IP and TLS on non-Windows
    systems; the UNIX transport ignores it.
 */
#define cs_listen_check(handle, ap, al, cf, cd) ((*(handle)->f_listen)(handle, ap, al, cf, cd))
/** \brief Creates an endpoint for a connection received by cs_listen
    \param handle listener, or an accepted endpoint with a pending TLS handshake
    \returns connected or pending endpoint, or NULL on failure

    The original listener remains usable. If the returned endpoint has pending
    I/O, wait for the indicated directions and call cs_accept on that endpoint
    again to continue its TLS handshake. A failed TLS handshake releases the
    pending endpoint. Close a successfully accepted endpoint with cs_close.
 */
#define cs_accept(handle) ((*(handle)->f_accept)(handle))
/** \brief Closes the socket and frees the endpoint and its transport state
    \param handle endpoint, which must not be used after this call

    Does not free application data in user or the caller's cs_get buffer.
 */
#define cs_close(handle) ((*(handle)->f_close)(handle))
/** \brief Creates an endpoint using a transport constructor
    \param type CS_TYPE constructor, such as tcpip_type or unix_type
    \param flags combination of CS_FLAGS_* bits
    \param proto application protocol (enum oid_proto)
    \returns new endpoint, or NULL on failure; release with cs_close
 */
#define cs_create(type, flags, proto) ((*type)(-1, flags, proto, 0))
/** \brief Creates an endpoint around an existing socket
    \param sock socket descriptor; ownership transfers on success
    \param type CS_TYPE transport constructor
    \param flags combination of CS_FLAGS_* bits
    \param proto application protocol (enum oid_proto)
    \returns new endpoint, or NULL on failure; cs_close closes the socket
 */
#define cs_createbysocket(sock, type, flags, proto) \
        ((*type)(sock, flags, proto, 0))
/** \brief Returns the endpoint's CS_TYPE transport constructor
    \param handle endpoint
 */
#define cs_type(handle) ((handle)->type)
/** \brief Returns the socket descriptor, or -1 if none has been created
    \param handle endpoint
 */
#define cs_fileno(handle) ((handle)->iofile)
/** \brief Legacy accessor intended to return the connection state
    \param handle endpoint
    \warning This macro refers to a nonexistent getstate member. Read the
             public comstack::state member instead.
 */
#define cs_getstate(handle) ((handle)->getstate)
/** \brief Returns the last COMSTACK error code
    \param handle endpoint
    \see cs_get_error, cs_errmsg
 */
#define cs_errno(handle) ((handle)->cerrno)
/** \brief Returns the application protocol (enum oid_proto)
    \param handle endpoint
 */
#define cs_getproto(handle) ((handle)->protocol)
/** \brief Formats the peer address as text
    \param handle endpoint
    \returns borrowed transport-owned string; do not free it

    Subsequent calls may overwrite the result. Network transports may perform
    reverse DNS unless CS_FLAGS_NUMERICHOST is set.
 */
#define cs_addrstr(handle) ((*(handle)->f_addrstr)(handle))
/** \brief Parses a transport-specific address and prepares the endpoint
    \param handle endpoint
    \param str address without a transport prefix; see cs_create_host2 for
           the higher-level host/URI interface
    \returns transport-owned address, or NULL on failure

    May resolve names and create a socket. Do not free the result; another
    cs_straddr call may replace it, and cs_close releases it.
 */
#define cs_straddr(handle, str) ((*(handle)->f_straddr)(handle, str))
/** \brief Tests whether a pending operation needs socket readability
    \param handle endpoint
    \returns nonzero if readability is needed
 */
#define cs_want_read(handle) ((handle)->io_pending & CS_WANT_READ)
/** \brief Tests whether a pending operation needs socket writability
    \param handle endpoint
    \returns nonzero if writability is needed
 */
#define cs_want_write(handle) ((handle)->io_pending & CS_WANT_WRITE)
/** \brief Updates endpoint flags and socket blocking mode
    \param handle endpoint
    \param blocking complete set of CS_FLAGS_* bits, despite the parameter name
    \returns 1 on success, 0 on failure

    Preserve other desired flag bits when setting or clearing CS_FLAGS_BLOCKING;
    this operation replaces the stored flags.
 */
#define cs_set_blocking(handle,blocking) ((handle)->f_set_blocking(handle, blocking))

/** Pending operation needs socket readability. */
#define CS_WANT_READ 1
/** Pending operation needs socket writability. */
#define CS_WANT_WRITE 2

/** \brief Returns the last recorded event (CS_NONE, CS_CONNECT, etc.)

    The argument is the endpoint. This function does not poll for new events.
 */
YAZ_EXPORT int cs_look (COMSTACK);
/** \brief Returns a static message for the endpoint's last error
    \param h endpoint
    \returns borrowed error string; do not free it
 */
YAZ_EXPORT const char *cs_strerror(COMSTACK h);
/** \brief Returns a static message for a COMSTACK error code
    \param n error code (CSNONE through CSLASTERROR)
    \returns borrowed string; unknown codes use the CSNONE message
 */
YAZ_EXPORT const char *cs_errmsg(int n);
/** \brief Returns the last COMSTACK error and optional transport details
    \param cs endpoint
    \param details optional output pointer; set to borrowed text or NULL
    \returns error code (CSNONE through CSLASTERROR)

    Details are currently available for TCP/IP and TLS. Do not free the text;
    it can be overwritten by a later error and is released by cs_close.
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
/** \brief Finds the length of a BER message or HTTP headers
    \param buf input bytes
    \param len number of available bytes
    \returns message length, 0 if incomplete, -1 on protocol error

    Examines the first message only; does not modify the input.
 */
YAZ_EXPORT int cs_complete_auto_head(const char *buf, int len);
/** \brief Finds the length of a complete BER or HTTP message
    \param buf input bytes
    \param len number of available bytes
    \returns message length, 0 if incomplete, -1 on protocol error

    Examines the first message only; does not modify the input.
 */
YAZ_EXPORT int cs_complete_auto(const char *buf, int len);
/** \brief Legacy OpenSSL session accessor
    \param cs endpoint
    \returns NULL
    \deprecated OpenSSL support has been removed; this function is a no-op.
 */
YAZ_EXPORT void *cs_get_ssl(COMSTACK cs)
#ifdef __GNUC__
    __attribute__ ((deprecated))
#endif
    ;
/** \brief Legacy OpenSSL context setter
    \param cs endpoint, or NULL
    \param ctx ignored legacy context pointer
    \returns 1 for a TLS endpoint with GnuTLS support, otherwise 0
    \deprecated OpenSSL support has been removed; no context is installed.
 */
YAZ_EXPORT int cs_set_ssl_ctx(COMSTACK cs, void *ctx)
#ifdef __GNUC__
    __attribute__ ((deprecated))
#endif
    ;
/** \brief Selects certificate and private key files for a TLS endpoint
    \param cs endpoint, or NULL
    \param fname filename containing both certificate and key, or
           "certificate-file,key-file"; NULL or empty disables the default file
    \returns 1 if filenames were stored, 0 for an unsupported endpoint

    Call before binding or connecting. Filenames are copied; success does not
    mean the files have been loaded or validated. This overrides the default
    server certificate filename yaz.pem.
 */
YAZ_EXPORT int cs_set_ssl_certificate_file(COMSTACK cs, const char *fname);
/** \brief Gets a human-readable description of the first peer X.509 certificate
    \param cs endpoint with an established TLS session
    \param buf required output pointer to allocated, NUL-terminated text
    \param len required output pointer to text length, excluding the terminator
    \returns 1 on success, 0 if unavailable; outputs are unchanged on failure

    Requires GnuTLS certificate printing support. Free *buf with xfree after
    success. The result is descriptive text, not a PEM-encoded certificate.
 */
YAZ_EXPORT int cs_get_peer_certificate_x509(COMSTACK cs, char **buf, int *len);
/** \brief Sets the maximum incoming message size
    \param cs endpoint
    \param max_recv_bytes receive limit in bytes (default 16777216)

    Oversized input is reported as CSBUFSIZE by cs_get.
 */
YAZ_EXPORT void cs_set_max_recv_bytes(COMSTACK cs, int max_recv_bytes);
/** \brief Prints TLS peer certificate information to standard output
    \param cs endpoint

    Does nothing for other transports or when no TLS session is available.
 */
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

/** No error, or an unspecified error. */
#define CSNONE     0
/** System call failed; consult the platform error information. */
#define CSYSERR    1
/** Operation is invalid for this state or transport. */
#define CSOUTSTATE 2
/** No incoming connection is available yet. */
#define CSNODATA   3
/** A pending write was retried with a different buffer or size. */
#define CSWRONGBUF 4
/** Incoming connection was rejected by the access callback. */
#define CSDENY     5
/** TLS operation failed; cs_get_error may provide details. */
#define CSERRORSSL 6
/** Incoming data exceeds the configured receive limit. */
#define CSBUFSIZE  7
/** Malformed protocol data. */
#define CSPROTERR  8
/** Highest COMSTACK error code; keep equal to the last error. */
#define CSLASTERROR CSPROTERR

/** Use blocking I/O; without this bit operations may remain pending. */
#define CS_FLAGS_BLOCKING 1
/** Use numeric peer addresses instead of reverse DNS in cs_addrstr. */
#define CS_FLAGS_NUMERICHOST 2
/** Resolve asynchronously when resolver thread support is available. */
#define CS_FLAGS_DNS_NO_BLOCK 4
/** Verify the TLS server certificate and hostname when connecting. */
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
