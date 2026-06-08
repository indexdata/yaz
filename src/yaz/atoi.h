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
 * \file atoi.h
 * \brief ASCII to integer conversion
 */

#ifndef YAZ_ATOI_H
#define YAZ_ATOI_H

#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

/** \brief like atoi(3) except that it reads exactly len characters
    \param buf buffer to read
    \param len number of bytes to consider (being digits)
    \returns value
 */
YAZ_EXPORT int atoi_n(const char *buf, int len);

/** \brief like atoi_n but checks for proper formatting
    \param buf buffer to read values from
    \param size size of buffer
    \param val value of decimal number (if successful)
    \retval 0 no value found (non-digits found)
    \retval 1 value found and *val holds value
*/
YAZ_EXPORT
int atoi_n_check(const char *buf, int size, int *val);

/** \brief string to int conversion, with base
    \param base numerical base (2-36)
    \param ptr buffer to read
    \param len number of bytes to consider (being digits)
    \param val value of decimal number (if successful)
    \returns number of bytes read if successful, or -1 on error (overflow or invalid base)
    The function stops at first digit not valid for base.
    In particular, if no digits are found, 0 is returned and *val is set to 0.
 */
YAZ_EXPORT
int yaz_atoi(int base, const char *ptr, int len, int *val);

/** \brief string to unsigned long conversion, with base
    \param base numerical base (2-36)
    \param ptr buffer to read
    \param len maximum number of bytes to read
    \param val value of number (if successful)
    \returns number of bytes read if successful, or -1 on error (overflow or invalid base)
    The function stops at first digit not valid for base.
    In particular, if no digits are found, 0 is returned and *val is set to 0.
 */
YAZ_EXPORT
int yaz_atoul(int base, const char *ptr, int len, unsigned long *val);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

