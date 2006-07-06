/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 * 
 *  $Id: nfaxml.c,v 1.2 2006-07-06 06:09:12 adam Exp $ 
 */

/**
 * \file nfaxml.c
 * \brief Routines for reading a NFA spec from an XML file
 *
 */

#if HAVE_XML2

#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>

#include <yaz/nfa.h>
#include <yaz/nmem.h> 
#include <yaz/yconfig.h>
#include <yaz/nfa.h>
#include <yaz/nfaxml.h>


/** \brief Parse the NFA from a XML document 
 */
yaz_nfa *yaz_nfa_parse_xml_doc(xmlDocPtr doc){
    if (!doc)
        return 0;

    return 0;
}

/** \brief Log XML errors in yaz_log
 *
 * Disabled because xmlErrorPtr does not exist for older Libxml2's
 */
#if 0
static void log_xml_error(int errlevel, char *msg) {
    xmlErrorPtr e=xmlGetLastError();
    if (!e)  /* no error happened */
        return;
    if (!errlevel)
        errlevel=YLOG_FATAL;
    yaz_log(errlevel,"%s %d/%d: %s:%d: '%s' ",
            msg, e->domain, e->code, e->file, e->line, e->message);
    if (e->str1 || e->str2 || e->str3 )
        yaz_log(errlevel,"extra info: '%s' '%s' '%s' %d %d",
            e->str1, e->str2, e->str3, e->int1, e->int2 );
}
#endif

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



#endif /* HAVE_XML2 */


/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
