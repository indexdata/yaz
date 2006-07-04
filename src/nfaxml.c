/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 * 
 *  $Id: nfaxml.c,v 1.1 2006-07-04 12:59:56 heikki Exp $ 
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




/** \brief Log XML errors in yaz_log 
 *
 */

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

/** \brief Parse the NFA from a XML document 
 */
yaz_nfa *yaz_nfa_parse_xml_doc(xmlDocPtr doc){
    if (!doc)
        return 0;

    return 0;
}


/** \brief Parse the NFA from a file 
 */
yaz_nfa *yaz_nfa_parse_xml_file(char *filepath) {
    int nSubst;
    xmlDocPtr doc = xmlParseFile(filepath);
    if (!doc) {
        log_xml_error(YLOG_FATAL,
                "Error in parsing charmap file");
        return 0;
    }
    nSubst=xmlXIncludeProcess(doc);
    if (nSubst==-1) {
        log_xml_error(YLOG_FATAL,
                "Error handling XIncludes in charmap file");
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
