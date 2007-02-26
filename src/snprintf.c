/*
 * Copyright (C) 2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: snprintf.c,v 1.2 2007-02-26 14:24:00 adam Exp $
 */
/**
 * \file snprintf.c
 * \brief snprintf wrapper
 */

#include <stdlib.h>
#include <stdarg.h>
#include <yaz/snprintf.h>

void yaz_vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
#if HAVE_VSNPRINTF
    vsnprintf(buf, size, fmt, ap);
#else
#ifdef WIN32
    _vsnprintf(buf, size, fmt, ap);
#else
    vsprintf(buf, fmt, ap);
#endif
#endif
}

void yaz_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    yaz_vsnprintf(buf, size, fmt, ap);
    va_end(ap);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

