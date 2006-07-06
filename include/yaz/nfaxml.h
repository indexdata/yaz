/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *  $Id: nfaxml.h,v 1.4 2006-07-06 13:10:29 heikki Exp $
 */

/**
 * \file nfaxml.h
 * \brief Routines for reading NFA specs from an XML file
 *
 * The xml file is something like this (using round brakcets 
 * on tags, not to confuse our documentation tools)
 *   (?xml ...)
 *   (charmap)
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
yaz_nfa *yaz_nfa_parse_xml_doc(xmlDocPtr doc);


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
yaz_nfa *yaz_nfa_parse_xml_memory(const char *xmlbuff);


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

