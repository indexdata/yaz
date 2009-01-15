/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
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
    _vsnprintf(buf, size-1, fmt, ap);
    buf[size-1] = '\0';
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
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

