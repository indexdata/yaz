/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: yaz-util.h,v $
 * Revision 1.3  1997-05-14 06:53:54  adam
 * C++ support.
 *
 * Revision 1.2  1996/02/20 17:58:09  adam
 * Added const to yaz_matchstr.
 *
 * Revision 1.1  1996/02/20  16:32:49  quinn
 * Created util file.
 *
 *
 */

#ifndef YAZ_UTIL_H
#define YAZ_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

int yaz_matchstr(const char *s1, const char *s2);

#ifdef __cplusplus
}
#endif

#endif
