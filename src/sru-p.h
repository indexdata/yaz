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
 * \file sru-p.h
 * \brief SRU private header
 */

void yaz_add_name_value_str(ODR o, char **name, char **value,  int *i,
                            char *a_name, char *val);

void yaz_add_name_value_int(ODR o, char **name, char **value, int *i,
                            char *a_name, Odr_int *val);

char *yaz_negotiate_sru_version(char *input_ver);

void yaz_sru_facet_request(ODR, Z_FacetList **facetList,
                           const char **limit, const char **start,
                           const char **sort);

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>

int yaz_match_xsd_string(xmlNodePtr ptr, const char *elem, ODR o,
                         char **val);
int yaz_match_xsd_integer(xmlNodePtr ptr, const char *elem, ODR o,
                          Odr_int **val);
int yaz_match_xsd_string_n(xmlNodePtr ptr, const char *elem, ODR o,
                       char **val, int *len);
int yaz_match_xsd_string_n_nmem(xmlNodePtr ptr, const char *elem, NMEM nmem,
                                char **val, int *len);
int yaz_match_xsd_element(xmlNodePtr ptr, const char *elem);

int yaz_match_xsd_XML_n2(xmlNodePtr ptr, const char *elem, ODR o,
                         char **val, int *len, int fixup_root);

int yaz_match_xsd_XML_n(xmlNodePtr ptr, const char *elem, ODR o,
                        char **val, int *len);

xmlNodePtr add_xsd_string(xmlNodePtr ptr, const char *elem, const char *val);

void add_xsd_integer(xmlNodePtr ptr, const char *elem, const Odr_int *val);

xmlNodePtr add_xsd_string_n(xmlNodePtr ptr, const char *elem, const char *val,
                            int len);

void add_XML_n(xmlNodePtr ptr, const char *elem, char *val, int len,
               xmlNsPtr ns_ptr);

xmlNodePtr add_xsd_string_ns(xmlNodePtr ptr, const char *elem, const char *val,
                             xmlNsPtr ns_ptr);

void yaz_sru_facet_response(ODR o, Z_FacetList **facetList, xmlNodePtr n);

const char *yaz_element_attribute_value_get(xmlNodePtr ptr,
                                            const char *node_name, const char *attribute_name);
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

