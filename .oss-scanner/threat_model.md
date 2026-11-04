<!-- Template: copy to your project or place a copy at projects/<name>/threat_model.md. Free text; the scanner reads it before it starts. -->
# Threat model

## What this project does and where untrusted input enters
YAZ is a toolkit for implementing SRU and Z39.50 clients and servers. The
primary focus of this audit is validating remote input: requests received by
servers and responses received by clients. Treat remote peers as potentially
malicious, including servers contacted by a YAZ client.

All data received from a remote peer is untrusted. This includes protocol
headers and message bodies, search queries, record contents, and strings
passed to character conversion or ICU normalization routines. Local input,
including configuration files, command-line arguments, and locally supplied
queries and transformation rules, can be trusted. This does not make remote
data processed under that configuration trusted.

## Components that matter most / least
Prioritize memory corruption reachable through remote input anywhere along
the processing path: comstack network I/O and message framing; HTTP/SRU and
BER/Z39.50 encoding and decoding; record parsing and conversion, including
malformed MARC/ISO2709, JSON, and XML records; and character conversion and
ICU string handling. Both server-side request processing and client-side
response processing are in scope, including subsequent processing of records
and strings obtained from a remote peer.

Look especially for out-of-bounds reads and writes, use-after-free,
double-free, and length or integer handling errors that lead to unsafe memory
access. A useful finding should demonstrate how attacker-controlled remote
data reaches the affected code through a supported client or server path.

## How to exercise it
The ztest/yaz-ztest server is an example server, that exercises large portions of the API.
The zoom/zoomsh, client/yaz-client and util/yaz-url exercises the HTTP/BER/ISO23950 client behavior
The util/yaz-icu can be used problems with ICU normalization utilities.

## How you rate severity
Any remote input that can cause use-after-free, double-free, memory leaks are the biggest threads.

## Anything to leave alone
Issues that require malicious local input or configuration are out of scope
unless they can also be triggered by untrusted remote data with trusted local
input and configuration.
