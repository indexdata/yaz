/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/** \file
    \brief Log XML / XSLT Errors via yaz_log
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <yaz/log.h>
#include <yaz/snprintf.h>

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif
#if YAZ_HAVE_XSLT
#include <libxslt/xsltutils.h>
#endif

static int xml_error_log_level = YLOG_WARN;

#if YAZ_HAVE_XML2
static void xml_error_handler(void *ctx, const char *fmt, ...)
{
    char buf[1024];
    const char *prefix = (const char *) ctx;

    va_list ap;
    va_start(ap, fmt);

    yaz_vsnprintf(buf, sizeof(buf), fmt, ap);
    yaz_log(YLOG_WARN, "%s: %s", prefix, buf);

    va_end (ap);
}
#endif

void yaz_log_xml_errors(const char *prefix, int log_level)
{
    xml_error_log_level = log_level;
    
#if YAZ_HAVE_XML2
    xmlSetGenericErrorFunc((void *) (prefix ? prefix : "XML"),
                           xml_error_handler);
#if YAZ_HAVE_XSLT 
    xsltSetGenericErrorFunc((void *) (prefix ? prefix : "XSLT"),
                            xml_error_handler);
#endif
#endif
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

