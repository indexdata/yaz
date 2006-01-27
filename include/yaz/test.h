/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: test.h,v 1.1 2006-01-27 17:30:44 adam Exp $
 */

#ifndef YAZ_TEST_H
#define YAZ_TEST_H

#include <yaz/yconfig.h>
#include <stdio.h>

#define YAZ_CHECK_INIT(thename) \
static char *yaz_unit_test_name = #thename;
static FILE *yaz_unit_file = 0; \
static int yaz_unit_test_number = 0; \

#define YAZ_CHECK(as) \
yaz_unit_test_number++; \
if (!yaz_unit_file) yaz_unit_file = stderr; \
if (!as) { \
   fprintf(yaz_unit_file, "%s:%d test failed: %s\n", __FILE__, __LINE__, #as); \
}

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

