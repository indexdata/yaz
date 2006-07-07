/*
 * Copyright (C) 1995-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: test.h,v 1.9 2006-07-07 13:39:02 heikki Exp $
 */

/** \file test.h
    \brief Unit Test for YAZ
*/

#ifndef YAZ_TEST_H
#define YAZ_TEST_H

#include <yaz/yconfig.h>
#include <stdio.h>

/** \brief Get the verbosity level */
int yaz_test_get_verbosity();

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

/** \brief a test we know will fail at this time. 
 *
 * Later, when the bug is fixed, this test will suddenly pass,
 * which will be reported as an error, to remind you to go and fix 
 * your tests.
 */

#define YAZ_CHECK_TODO(as) { \
  yaz_check_inc_todo(); \
  if (!as) { \
    yaz_check_print1(YAZ_TEST_TYPE_OK, __FILE__, __LINE__, "TODO: " #as); \
  } else { \
    yaz_check_print1(YAZ_TEST_TYPE_FAIL, __FILE__, __LINE__, "TODO: "#as); \
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

/** \brief Macro to enable and initialize the yaz_log(start of main) */
#define YAZ_CHECK_LOG() yaz_check_init_log(argv[0])

YAZ_BEGIN_CDECL

/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_init1(int *argc, char ***argv);

/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_term1(void);

/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_init_log(const char *argv0);

/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_print1(int type, const char *file, int line,
                                 const char *expr);
/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void yaz_check_eq1(int type, const char *file, int line,
                              const char *left, const char *right,
                              int lval, int rval);
/** \brief used by macro. Should not be called directly */
YAZ_EXPORT void  yaz_check_inc_todo(void);
YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

