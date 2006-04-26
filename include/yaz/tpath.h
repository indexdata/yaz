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
 * 2. The names of Index Data or the individual authors may not be used to
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
 * $Id: tpath.h,v 1.10 2006-04-26 10:01:31 adam Exp $
 *
 */
/**
 * \file tpath.h
 * \brief File Path utilities
 */

#ifndef TPATH_H
#define TPATH_H

#include <yaz/yconfig.h>
#include <stdio.h>

YAZ_BEGIN_CDECL

/** \brief checks whether path is absolute
    \param path path to checked

    Returns 1 if path is absolute, 0 if relative
*/
YAZ_EXPORT int yaz_is_abspath (const char *path);

/** \brief resolve file on path 
    \param fname "short" filename (without path)
    \param path the path (dir1:dir2,..) - ala Unix
    \param base can be added to relative paths (NULL for no append)
    \param fullpath the full path to filename (if succesful)

    Returns 0/NULL if no fname could be found in path; 
    pointer to fullpath if fname could be found.
    We assume fullpath is 1024 bytes in length!
*/
YAZ_EXPORT char *yaz_filepath_resolve(const char *fname, const char *path,
                                      const char *base, char *fullpath);


/** \brief opens first file in path in path 
    \param fname "short" filename (without path)
    \param path the path (dir1:dir2,..) - ala Unix
    \param base can be added to relative paths (NULL for no append)
    \param fullpath the full path to filename (if succesful)

    Returns 0/NULL if no fname could be found in path; 
    FILE * pointer if fname could be found.
*/
YAZ_EXPORT FILE *yaz_fopen(const char *path, const char *name,
                           const char *mode, const char *base);

/** \brief opens first file in path in path 
    \param fname "short" filename (without path)
    \param path the path (dir1:dir2,..) - ala Unix
    \param fullpath the full path to filename (if succesful)

    Returns 0/NULL if no fname could be found in path; 
    FILE * pointer if fname could be found.
*/
YAZ_EXPORT FILE *yaz_path_fopen(const char *path, const char *name,
                           const char *mode);

/** \brief closes file
    \param f FILE handle

    Returns -1 on failure; 0 on success
*/
YAZ_EXPORT int yaz_fclose(FILE *f);


YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

