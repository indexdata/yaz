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
 * \file zgdu.h
 * \brief Header for the Z_GDU (HTTP or Z39.50 package)
 */

#ifndef Z_GDU_H
#define Z_GDU_H

#include <yaz/z-core.h>

YAZ_BEGIN_CDECL

typedef struct Z_HTTP_Header Z_HTTP_Header;

struct Z_HTTP_Header {
    char *name;
    char *value;
    Z_HTTP_Header *next;
};

typedef struct {
    char *method;
    char *version;
    char *path;
    Z_HTTP_Header *headers;
    char *content_buf;
    int content_len;
} Z_HTTP_Request;

typedef struct {
    int code;
    char *version;
    Z_HTTP_Header *headers;
    char *content_buf;
    int content_len;
} Z_HTTP_Response;

#define Z_GDU_Z3950         1
#define Z_GDU_HTTP_Request  2
#define Z_GDU_HTTP_Response 3
typedef struct {
    int which;
    union {
        Z_APDU *z3950;
        Z_HTTP_Request *HTTP_Request;
        Z_HTTP_Response *HTTP_Response;
    } u;
} Z_GDU ;
/** \brief Encode, decode, or print an HTTP message or Z39.50 APDU.
    \param o ODR handle whose direction selects the operation
    \param p message to encode/print, or output pointer when decoding
    \param opt optional-value flag passed to the Z39.50 codec; ignored for HTTP
    \param name unused
    \returns non-zero on success, zero on failure.

    Decoding detects the message type from the input buffer and allocates
    the result in o. Encoding and printing dispatch on (*p)->which.
    Check the return value before using a decoded message or sending an
    encoded buffer. Failure does not clear previously buffered bytes or
    prevent transmission. Reset o before processing a new message.
*/
YAZ_EXPORT int z_GDU (ODR o, Z_GDU **p, int opt, const char *name);
/** \brief Append a header, preserving any existing headers of the same name.
    \param o ODR arena for the new node and copied strings
    \param hp pointer to the header-list head, which may initially be NULL
    \param n non-NULL header name
    \param v non-NULL header value

    Strings are copied without validation or escaping. Allocations remain
    valid until o is reset or destroyed.
*/
YAZ_EXPORT void z_HTTP_header_add(ODR o, Z_HTTP_Header **hp, const char *n,
                                  const char *v);
/** \brief Replace the first matching header value, or append a new header.
    \param o ODR arena for copied strings and any new node
    \param hp pointer to the header-list head, which may initially be NULL
    \param n non-NULL header name, matched case-insensitively
    \param v non-NULL replacement value

    Later duplicate headers are unchanged. Strings are copied without
    validation or escaping and are owned by o.
*/
YAZ_EXPORT void z_HTTP_header_set(ODR o, Z_HTTP_Header **hp, const char *n,
                                  const char *v);
/** \brief Append a Content-Type header with an optional charset parameter.
    \param o ODR arena for the header and its strings
    \param hp pointer to the header-list head
    \param content_type non-NULL media type
    \param charset charset appended as "; charset=...", or NULL to omit it

    Existing Content-Type headers are preserved. Values are not escaped.
*/
YAZ_EXPORT void z_HTTP_header_add_content_type(ODR o, Z_HTTP_Header **hp,
                                               const char *content_type,
                                               const char *charset);
/** \brief Set the first Authorization header to HTTP Basic credentials.
    \param o ODR arena for the header and encoded credentials
    \param hp pointer to the header-list head
    \param username username, or NULL to leave the headers unchanged
    \param password password, or NULL for an empty password

    Encodes username:password using Base64 and calls z_HTTP_header_set.
*/
YAZ_EXPORT void z_HTTP_header_add_basic_auth(ODR o, Z_HTTP_Header **hp,
                                             const char *username,
                                             const char *password);
/** \brief Find the first header with a case-insensitively matching name.
    \param hp header-list head, or NULL for an empty list
    \param n non-NULL name to find
    \returns the stored value, or NULL if no header matches.

    The returned string is borrowed from the header list; do not free it.
*/
YAZ_EXPORT const char *z_HTTP_header_lookup(const Z_HTTP_Header *hp,
                                            const char *n);
/** \brief Unlink the first header with a case-insensitively matching name.
    \param hp pointer to the header-list head
    \param n non-NULL name to remove
    \returns the removed header's value, or NULL if no header matches.

    Neither the node nor its strings are freed. The returned value remains
    owned by its original allocator (normally an ODR arena). Later headers
    with the same name remain in the list.
*/
YAZ_EXPORT const char *z_HTTP_header_remove(Z_HTTP_Header **hp,
                                            const char *n);

/** \brief Look up an HTTP status description.
    \param code HTTP status code
    \returns a static string, or "Unknown Error" for an unrecognized code.

    The returned string must not be modified or freed.
*/
YAZ_EXPORT const char *z_HTTP_errmsg(int code);

/** \brief Create an HTTP/1.1 response with the default YAZ server identity.
    \param o ODR arena owning the response
    \param code HTTP status code
    \returns a Z_GDU containing the new HTTP response.

    Adds a Server header. Code 200 has no body; other codes receive an HTML
    status page and a Content-Type header. Equivalent to
    z_get_HTTP_Response_details with NULL details.
*/
YAZ_EXPORT Z_GDU *z_get_HTTP_Response(ODR o, int code);
/** \brief Create an HTTP response with optional HTML status details.
    \param o ODR arena owning the response
    \param code HTTP status code
    \param details text inserted into the HTML body, or NULL to omit it
    \returns a Z_GDU containing the new HTTP/1.1 response.

    Uses the default YAZ server identity and URL. Details are used only
    when code is not 200 and are inserted without HTML escaping.
    See z_get_HTTP_Response_server.
*/
YAZ_EXPORT Z_GDU *z_get_HTTP_Response_details(ODR o, int code,
                                              const char *details);
/** \brief Create an HTTP response with a specified server identity.
    \param o ODR arena owning the response
    \param code HTTP status code
    \param details optional HTML status details, or NULL
    \param server non-NULL Server header value and HTML server name
    \param server_url non-NULL URL used for the HTML server link
    \returns a Z_GDU containing the new HTTP/1.1 response.

    Code 200 has an empty body. Other codes receive a bounded HTML status
    page and a text/html Content-Type header. Server name, URL, and details
    are inserted without HTML escaping; long output may be truncated.
*/
YAZ_EXPORT Z_GDU *z_get_HTTP_Response_server(ODR o,
                                             int code, const char *details,
                                             const char *server,
                                             const char *server_url);
/** \brief Create a default HTTP request.
    \param o ODR arena owning the request
    \returns a Z_GDU containing a POST request for / using HTTP/1.1.

    Sets the YAZ User-Agent header and an empty body. The caller may replace
    fields before encoding. Do not free the returned structure separately.
*/
YAZ_EXPORT Z_GDU *z_get_HTTP_Request(ODR o);
/** \brief Create a default HTTP request with a host and request target.
    \param odr ODR arena owning the request and copied strings
    \param host host with optional scheme/path, or NULL to omit Host
    \param path non-NULL request target, copied verbatim
    \returns a Z_GDU containing the request.

    Uses z_get_HTTP_Request defaults. The Host value excludes any prefix
    through :// and any following slash and path. This constructor does not
    validate or percent-encode its inputs; encoding performs validation.
*/
YAZ_EXPORT Z_GDU *z_get_HTTP_Request_host_path(ODR odr,
                                               const char *host,
                                               const char *path);
/** \brief Create a default HTTP request from a URI.
    \param odr ODR arena owning the request and copied strings
    \param uri non-NULL URI supplying the host and default path
    \param args replacement path/query without its leading slash, or NULL
    \param use_full_uri non-zero to include the URI prefix in the target
    \returns a Z_GDU containing the request.

    Uses z_get_HTTP_Request defaults and adds Host. If args is NULL, uses
    the portion after the first slash following the URI authority, or an
    empty string if absent. The target is / followed by args, optionally
    preceded by the URI prefix for proxy requests. This constructor does
    not validate or percent-encode inputs; encoding performs validation.
*/
YAZ_EXPORT Z_GDU *z_get_HTTP_Request_uri(ODR odr, const char *uri,
                                         const char *args,
                                         int use_full_uri);

/** \brief Parse the status line at the start of an HTTP response buffer.
    \param buf input bytes; NUL termination is not required
    \param sz number of available bytes
    \param code non-NULL output pointer for the status code
    \param version optional output pointer to the version after HTTP/
    \param version_len optional output pointer for the version length
    \param msg optional output pointer to the reason phrase
    \param msg_len optional output pointer for the reason-phrase length
    \returns the zero-based offset of the terminating LF on success,
             or zero for an invalid or incomplete status line.

    Returned strings are non-NUL-terminated slices of buf and remain valid
    only while buf remains valid. Output values may be changed on failure.
    No allocation is performed and no ODR error state is set.
*/
YAZ_EXPORT int yaz_decode_http_response_first(const char *buf, int sz, int *code,
                                              const char **version, int *version_len,
                                              const char **msg, int *msg_len);
/** \brief Decode an HTTP request from an ODR input buffer.
    \param o decoding handle with the complete message set by odr_setbuf
    \param hr_p non-NULL output pointer for the allocated request
    \returns non-zero on success, zero on failure.

    The request, headers, and body are owned by o and remain valid until
    it is reset or destroyed. Chunked bodies are decoded. On failure the
    output may point to a partially decoded request; check the return value
    before using it. Malformed input sets OHTTP.
*/
YAZ_EXPORT int yaz_decode_http_request(ODR o, Z_HTTP_Request **hr_p);
/** \brief Decode an HTTP response from an ODR input buffer.
    \param o decoding handle with the complete message set by odr_setbuf
    \param hr_p non-NULL output pointer for the allocated response
    \returns non-zero on success, zero on failure.

    The response, headers, and body are owned by o and remain valid until
    it is reset or destroyed. Chunked bodies are decoded. On failure the
    output may be NULL or partially decoded; check the return value before
    using it. Malformed input sets OHTTP.
*/
YAZ_EXPORT int yaz_decode_http_response(ODR o, Z_HTTP_Response **hr_p);
/** \brief Encode or print an HTTP response.
    \param o ODR encoding or printing handle
    \param hr response to encode
    \returns non-zero on success, zero on failure.

    Generates the status description with z_HTTP_errmsg and Content-Length
    from hr->content_len. Supplied Content-Length and Transfer-Encoding
    headers are omitted. The body is written without chunked encoding.
    Header strings are serialized without escaping or character validation.

    Check the return value before sending the buffer. Failure may leave
    partial output and does not itself prevent transmission. Reset o before
    encoding a new message.
*/
YAZ_EXPORT int yaz_encode_http_response(ODR o, Z_HTTP_Response *hr);
/** \brief Encode an HTTP request.
    \param o ODR encoding or printing handle
    \param hr HTTP request to encode
    \returns non-zero on success, zero on failure.

    Invalid request-line or header characters cause failure with OHTTP
    before any bytes from this request are written. URI spaces and control
    characters in the transmitted target must be percent-encoded by the
    caller. URI fragments are omitted from the request target.

    The caller must check the return value before sending the buffer.
    An ODR error does not prevent odr_getbuf from returning the buffer or
    the transport from sending it. Validation failure leaves any bytes
    already in the buffer unchanged; other encoding failures may leave
    a partial request. Reset the ODR handle before encoding a new message.
*/
YAZ_EXPORT int yaz_encode_http_request(ODR o, Z_HTTP_Request *hr);

/** \brief Combine a slash-prefixed location with a base address.
    \param odr ODR arena for a combined address
    \param uri non-NULL base connection address
    \param location non-NULL Location value
    \param host_change non-NULL output: 0 for slash-prefixed locations,
                       1 for all other locations
    \returns an ODR-owned combined address for a slash-prefixed location,
             otherwise the original location pointer.

    Replaces the base path when location starts with /. Other locations
    are returned unchanged; host_change does not compare actual hosts.
    This is not a general URI resolver and performs no scheme or origin
    validation. Use yaz_check_http_redirect for automatic HTTP redirects.
*/
YAZ_EXPORT const char *yaz_check_location(ODR odr, const char *uri,
                                    const char *location, int *host_change);

/** \brief Resolve an HTTP redirect, preserving a trusted CONNECT wrapper.
    \param odr memory for the resolved address
    \param uri original HTTP(S) address, optionally prefixed by connect:proxy,
    \param location untrusted Location header (HTTP(S) URI or absolute path)
    \param host_change non-NULL output: 0 for a slash-prefixed path,
                       1 otherwise, when resolution succeeds
    \param same_origin non-NULL output: 1 if the logical origins match,
                       0 otherwise
    \returns resolved connection address, or NULL for an invalid target.
    Redirects cannot supply COMSTACK transport wrappers. Any CONNECT prefix
    comes exclusively from uri and is retained for absolute redirects too.
    The result may borrow location or belong to odr; do not free it and
    keep both alive while using it. Success does not enforce same-origin
    policy: the caller must use same_origin to decide whether credentials
    may be forwarded. Request encoding separately validates characters.
*/
YAZ_EXPORT const char *yaz_check_http_redirect(ODR odr, const char *uri,
                                              const char *location,
                                              int *host_change,
                                              int *same_origin);

/** \brief Compare origins of absolute HTTP(S) URIs.
    \param uri first non-NULL absolute HTTP(S) URI
    \param other second non-NULL absolute HTTP(S) URI
    \returns 1 for equal scheme, hostname and effective port, otherwise 0.
    Invalid authorities and URIs containing user information return 0.
    A fragment immediately following the authority also returns 0.
    Authorities exceeding the transport parser's buffer are rejected.
    Scheme and hostname comparisons are case-insensitive; omitted ports
    default to 80 for HTTP and 443 for HTTPS. Paths and queries do not
    participate in the comparison. No DNS resolution is performed.
*/
YAZ_EXPORT int yaz_http_same_origin(const char *uri, const char *other);
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

