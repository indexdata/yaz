/*
 * Copyright (C) 1995-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: test.h,v 1.5 2006-04-20 20:50:51 adam Exp $
 */

/** \file test.h
    \brief Unit Test for YAZ
*/

#ifndef YAZ_TEST_H
#define YAZ_TEST_H

#include <yaz/yconfig.h>
#include <stdio.h>

/** \brief Test OK */
#define YAZ_TEST_TYPE_OK 1
/** \brief Test failed */
#define YAZ_TEST_TYPE_FAIL 2

/** \brief boolean test. as only evaluated once */
#define YAZ_CHECK(as) { \
  if (as) { \
    yaz_check_print1(YAZ_TEST_TYPE_OK, __FILE__, __LINE__, #as); \
  } else { \
    yaz_check_print1(YAZ_TEST_TYPE_FAIL, __FILE__, __LINE__, #as); \
  } \
}

/** \brief equality test. left, right only evaluated once */
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

/** \brief Macro to initialize the system (in start of main typically) */
#define YAZ_CHECK_INIT(argc, argv) yaz_check_init1(&argc, &argv)
/** \brief Macro to terminate the system (end of main, normally) */
#define YAZ_CHECK_TERM yaz_check_term1(); return 0

YAZ_BEGIN_CDECL
/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_init1(int *argc, char ***argv);
/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_term1(void);
/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_print1(int type, const char *file, int line,
                                 const char *expr);
/** \brief used by macro. Should not be called directly */
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

