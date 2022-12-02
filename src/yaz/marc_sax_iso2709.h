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
 * \file marc_sax.h
 * \brief Parsing MARCXML collection using Libxml2's SAX parser.
 */

#ifndef MARC_SAX_ISO2709_H
#define MARC_SAX_ISO2709_H

#include <yaz/yconfig.h>
#include <yaz/marcdisp.h>

YAZ_BEGIN_CDECL

typedef struct sax *yaz_marc_sax_iso2709_t;

YAZ_EXPORT yaz_marc_sax_iso2709_t yaz_marc_sax_iso2709_new(void);

YAZ_EXPORT void yaz_marc_sax_iso2709_destroy(yaz_marc_sax_iso2709_t p);

YAZ_EXPORT void yaz_marc_sax_iso2709_end(yaz_marc_sax_iso2709_t p);

YAZ_EXPORT void yaz_marc_sax_iso2709_push(yaz_marc_sax_iso2709_t p,
    const char *buf, size_t bufsz);

/**
 * @brief get next MARC record.
 *
 * @param p MARC sax handler.
 * @param mt MARC data where record is stored if avaiable.
 * @return int 0: incomplete, -1: EOF, -2: ERROR, >0 record length.
 */
YAZ_EXPORT int yaz_marc_sax_iso2709_next(yaz_marc_sax_iso2709_t p, yaz_marc_t mt);


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

