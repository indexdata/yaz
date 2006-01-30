/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: test.h,v 1.4 2006-01-30 14:02:06 adam Exp $
 */

#ifndef YAZ_TEST_H
#define YAZ_TEST_H

#include <yaz/yconfig.h>
#include <stdio.h>

#define YAZ_TEST_TYPE_OK 1
#define YAZ_TEST_TYPE_FAIL 2

#define YAZ_CHECK(as) { \
  if (as) { \
    yaz_check_print1(YAZ_TEST_TYPE_OK, __FILE__, __LINE__, #as); \
  } else { \
    yaz_check_print1(YAZ_TEST_TYPE_FAIL, __FILE__, __LINE__, #as); \
  } \
}

#define YAZ_CHECK_EQ(left, right) { \
  int lval = left; \
  int rval = right; \
  if (lval == rval) { \
    yaz_check_eq1(YAZ_TEST_TYPE_OK, __FILE__, __LINE__, \
     #left, #right, lval, rval); \
  } else { \
    yaz_check_eq1(YAZ_TEST_TYPE_FAIL, __FILE__, __LINE__, \
     #left, #right, lval, rval); \
  } \
}

#define YAZ_CHECK_INIT(argc, argv) yaz_check_init1(&argc, &argv)
#define YAZ_CHECK_TERM yaz_check_term1(); return 0

YAZ_BEGIN_CDECL
YAZ_EXPORT void yaz_check_init1(int *argc, char ***argv);
YAZ_EXPORT void yaz_check_term1(void);
YAZ_EXPORT void yaz_check_print1(int type, const char *file, int line,
                                 const char *expr);
YAZ_EXPORT void yaz_check_eq1(int type, const char *file, int line,
                              const char *left, const char *right,
                              int lval, int rval);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

