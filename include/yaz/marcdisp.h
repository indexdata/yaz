/*
 * Copyright (C) 1995-2006, Index Data ApS
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * $Id: marcdisp.h,v 1.17 2006-04-20 20:35:02 adam Exp $
 */

/**
 * \file marcdisp.h
 * \brief MARC conversion
 */

#ifndef MARCDISP_H
#define MARCDISP_H

#include <yaz/yconfig.h>
#include <stdio.h>
#include <yaz/wrbuf.h>

YAZ_BEGIN_CDECL

/** \brief a yaz_marc_t handle (private content) */
typedef struct yaz_marc_t_ *yaz_marc_t;

/** \brief construct yaz_marc_t handle */
YAZ_EXPORT yaz_marc_t yaz_marc_create(void);

/** \brief destroy yaz_marc_t handle */
YAZ_EXPORT void yaz_marc_destroy(yaz_marc_t mt);

/** \brief set XML mode YAZ_MARC_LINE, YAZ_MARC_SIMPLEXML, ... */
YAZ_EXPORT void yaz_marc_xml(yaz_marc_t mt, int xmlmode);

/** \brief Output format: Line-format */
#define YAZ_MARC_LINE      0
/** \brief Output format: simplexml (no longer supported) */
#define YAZ_MARC_SIMPLEXML 1
/** \brief Output format: OAI-MARC (no longer supported) */
#define YAZ_MARC_OAIMARC   2
/** \brief Output format: MARCXML */
#define YAZ_MARC_MARCXML   3
/** \brief Output format: ISO2709 */
#define YAZ_MARC_ISO2709   4
/** \brief Output format: MarcXchange */
#define YAZ_MARC_XCHANGE   5

/** \brief supply iconv handle for character set conversion .. */
YAZ_EXPORT void yaz_marc_iconv(yaz_marc_t mt, yaz_iconv_t cd);

/** \brief set debug level 
    \param mt handle
    \param level level, where 0=lowest, 1 more debug, 2 even more 
*/
YAZ_EXPORT void yaz_marc_debug(yaz_marc_t mt, int level);

/** \brief decodes ISO2709 buffer using straight buffers
    \param mt marc handle
    \param buf input buffer
    \param bsize size of buffer or (-1 if "any size")
    \param result result to be stored here (allocate before use!)
    \param rsize size of result (set before calling)

    Decodes MARC in buf of size bsize.
    On success, result in *result with size *rsize. 
    Returns -1 on error, or size of input record (>0) if OK
*/
YAZ_EXPORT int yaz_marc_decode_buf(yaz_marc_t mt, const char *buf, int bsize,
                                   char **result, int *rsize);

/** \brief decodes ISO2709/MARC buffer and stores result in WRBUF
    \param mt handle
    \param buf input buffer
    \param bsize size of buffer (-1 if "any size")
    \param wrbuf WRBUF for output

    Decodes MARC in buf of size bsize.
    On success, result in wrbuf
    Returns -1 on error, or size of input record (>0) if OK
*/
YAZ_EXPORT int yaz_marc_decode_wrbuf(yaz_marc_t mt, const char *buf,
                                     int bsize, WRBUF wrbuf);

/** \brief depricated */
YAZ_EXPORT int marc_display(const char *buf, FILE *outf);
/** \brief depricated */
YAZ_EXPORT int marc_display_ex(const char *buf, FILE *outf, int debug);
/** \brief depricated */
YAZ_EXPORT int marc_display_exl(const char *buf, FILE *outf, int debug,
                                int length);
/** \brief depricated */
YAZ_EXPORT int marc_display_wrbuf(const char *buf, WRBUF wr, int debug,
                                  int bsize);
/** \brief depricated */
YAZ_EXPORT int yaz_marc_decode(const char *buf, WRBUF wr,
                               int debug, int bsize, int xml);

YAZ_EXPORT void yaz_marc_subfield_str(yaz_marc_t mt, const char *s);
YAZ_EXPORT void yaz_marc_endline_str(yaz_marc_t mt, const char *s);

/** \brief modifies part of the MARC leader */
YAZ_EXPORT void yaz_marc_modify_leader(yaz_marc_t mt, size_t off,
                                       const char *str);

/** \brief like atoi(3) except that it reads exactly len characters */
YAZ_EXPORT int atoi_n(const char *buf, int len);

/** \brief MARC control char: record separator (29 Dec, 1D Hex) */
#define ISO2709_RS 035
/** \brief MARC control char: field separator (30 Dec, 1E Hex) */
#define ISO2709_FS 036
/** \brief MARC control char: identifier-field separator (31 Dec, 1F Hex) */
#define ISO2709_IDFS 037

/** \brief read ISO2709/MARC record from buffer
    \param mt handle
    \param buf ISO2709 buffer of size bsize
    \param bsize size of buffer (-1 for unlimited size)

    Parses ISO2709 record from supplied buffer
    Returns > 0 for OK (same as length), -1=ERROR
*/
YAZ_EXPORT int yaz_marc_read_iso2709(yaz_marc_t mt,
                                     const char *buf, int bsize);

/** \brief parses MARCXML/MarcXchange record from xmlNode pointer 
    \param mt handle
    \param xmlnode is a pointer to root xmlNode pointer 

    Returns 0=OK, -1=ERROR
*/
YAZ_EXPORT int yaz_marc_read_xml(yaz_marc_t mt, const void *xmlnode);

/** \brief writes record in line format
    \param mt handle
    \param wrbuf WRBUF for output

    Returns 0=OK, -1=ERROR
*/
YAZ_EXPORT int yaz_marc_write_line(yaz_marc_t mt, WRBUF wrbuf);

/** \brief writes record in MARCXML format
    \param mt handle
    \param wrbuf WRBUF for output

    Sets leader[9]='a' . Returns 0=OK, -1=ERROR . 
*/

YAZ_EXPORT int yaz_marc_write_marcxml(yaz_marc_t mt, WRBUF wrbuf);

/** \brief writes record in MarcXchange XML
    \param mt handle
    \param wrbuf WRBUF for output
    \param format record format (e.g. "MARC21")
    \param type record type (e.g. Bibliographic)

    Returns 0=OK, -1=ERROR
*/
YAZ_EXPORT int yaz_marc_write_marcxchange(yaz_marc_t mt, WRBUF wrbuf,
                                          const char *format,
                                          const char *type);

/** \brief writes record in ISO2709 format
    \param mt handle
    \param wrbuf WRBUF for output
    Returns 0=OK, -1=ERROR
*/
YAZ_EXPORT int yaz_marc_write_iso2709(yaz_marc_t mt, WRBUF wrbuf);

/** \brief writes record in mode - given by yaz_marc_xml mode
    \param mt handle
    \param wrbuf WRBUF for output
    This function calls yaz_marc_write_iso2709, yaz_marc_write_marcxml,
    etc.. depending on mode given by yaz_marc_xml. 
    Returns 0=OK, -1=ERROR 
*/  
YAZ_EXPORT int yaz_marc_write_mode(yaz_marc_t mt, WRBUF wrbuf);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

