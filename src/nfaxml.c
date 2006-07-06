/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 * 
 *  $Id: nfaxml.c,v 1.7 2006-07-06 13:10:31 heikki Exp $ 
 */

/**
 * \file nfaxml.c
 * \brief Routines for reading a NFA spec from an XML file
 *
 */

#if YAZ_HAVE_XML2

#include <string.h>

/* #include <libxml/parser.h> */
#include <libxml/tree.h>
#include <libxml/xinclude.h>

#include <yaz/nfa.h>
#include <yaz/nmem.h> 
#include <yaz/yconfig.h>
#include <yaz/nfa.h>
#include <yaz/nfaxml.h>
#include <yaz/libxml2_error.h>

/** \brief Parse the NFA from a XML document 
 */
yaz_nfa *yaz_nfa_parse_xml_doc(xmlDocPtr doc)
{
    libxml2_error_to_yazlog(YLOG_FATAL, "yaz_nfa_parse_xml_file");

    if (!doc)
        return 0;

    return 0;
}


/** \brief Parse the NFA from a file 
 */
yaz_nfa *yaz_nfa_parse_xml_file(const char *filepath) {
    int nSubst;

    libxml2_error_to_yazlog(YLOG_FATAL, "yaz_nfa_parse_xml_file");

    xmlDocPtr doc = xmlParseFile(filepath);
    if (!doc) {
        return 0;
    }
    nSubst=xmlXIncludeProcess(doc);
    if (nSubst==-1) {
        return 0;
    }
    return yaz_nfa_parse_xml_doc(doc);
}

/** \brief Parse the NFA from a memory buffer
 */
yaz_nfa *yaz_nfa_parse_xml_memory(const char *xmlbuff) {
    int nSubst;

    libxml2_error_to_yazlog(YLOG_FATAL, "yaz_nfa_parse_xml_memory");
    xmlDocPtr doc = xmlParseMemory(xmlbuff, strlen(xmlbuff));
    return yaz_nfa_parse_xml_doc(doc);
}



#endif /* YAZ_HAVE_XML2 */


/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
