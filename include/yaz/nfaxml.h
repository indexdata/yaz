/*
 * Copyright (c) 1995-2006, Index Data
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
/* $Id: nfaxml.h,v 1.6 2006-10-09 21:02:41 adam Exp $ */

/**
 * \file nfaxml.h
 * \brief Routines for reading NFA specs from an XML file
 *
 * The xml file is something like this (using round brakcets 
 * on tags, not to confuse our documentation tools)
 *   (?xml ...)
 *   (ruleset)
 *      (rule)
 *         (fromstring) FOO (/fromstring)
 *         (tostring)   BAR (/tostring)
 *      (/rule)
 *      (rule)
 *         (fromrange) a-z (/fromrange)
 *         (torange)   A-Z (/torange)
 *      (/rule)
 *      ...
 *
 *  The rules consist of two parts, 'from' and 'to'.
 *  From rules can be
 *     fromstring (which can be a single character)
 *     fromrange  (like a-z)
 *     (later, perhaps a fromregexp)
 *  To rules can be
 *     tostring (which can be a single character)
 *     torange  (only with a fromrange)
 *     (later, perhaps backrefs from regexps)
 */

#ifndef YAZ_NFA_XML_H
#define YAZ_NFA_XML_H

#if YAZ_HAVE_XML2

#include <libxml/parser.h>

#include <yaz/yconfig.h>
#include <yaz/log.h>
#include <yaz/nfa.h>

YAZ_BEGIN_CDECL

/** \brief Parse the NFA from a XML document 
 * 
 * \param doc the xml tree to parse
 * \param filename used for info in error messages
 * 
 * \returns either the NFA, or null in case of errors 
 *
 * It is up to the caller to destroy the nfa when done.
 *
 * Does not expand XIncludes.
 *
 * In case of errors, returns a null pointer.  You can then
 * call xmlGetLastError() to get the details of the error,
 * if you have a recent enough libxml2. Those are already 
 * logged in yazlog.
 *
 */
yaz_nfa *yaz_nfa_parse_xml_doc(xmlDocPtr doc, const char *filename);


/** \brief Parse the NFA from a file 
 *
 * \param filepath path to the xml file to parse
 * \param error_info will be filled in case of errors
 * 
 * \returns either the NFA, or null in case of errors 
 *
 * It is up to the caller to destroy the nfa when done.
 *
 * This routine also expands XIncludes.
 * 
 * In case of errors, returns a null pointer.  You can then
 * call xmlGetLastError() to get the details of the error,
 * if you have a recent enough libxml2. Those are already 
 * logged in yazlog.
 *
 */
yaz_nfa *yaz_nfa_parse_xml_file(const char *filepath);


/** \brief Parse the NFA from a memory buffer
 *
 * \param filepath path to the xml file to parse
 * \param error_info will be filled in case of errors
 * 
 * \returns either the NFA, or null in case of errors 
 *
 * It is up to the caller to destroy the nfa when done.
 *
 * Does not expand XIncludes.
 *
 * In case of errors, returns a null pointer.  You can then
 * call xmlGetLastError() to get the details of the error,
 * if you have a recent enough libxml2. Those are already 
 * logged in yazlog.
 *
 */
yaz_nfa *yaz_nfa_parse_xml_memory(const char *xmlbuff, const char *filename);


YAZ_END_CDECL
   
#endif /* YAZ_HAVE_XML2 */
#endif /* YAZ_NFA_XML_H */

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

