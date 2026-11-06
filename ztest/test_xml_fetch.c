/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include <yaz/marcdisp.h>
#include <yaz/log.h>
#include <yaz/snprintf.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "ztest.h"

/* read-marc.c uses these replacements so every filesystem lookup is
   observable, including traversal attempts that would otherwise miss a file. */
static const char *prefix;
static const char *expected_path;
static const char record[] = "<record>fixture</record>";
static int stat_calls, open_calls, fail_stat, fail_open;

static char *test_xml_getenv(const char *name)
{
    YAZ_CHECK(!strcmp(name, "YAZ_ZTEST_XML_FETCH"));
    return (char *) prefix;
}

static int test_xml_stat(const char *path, struct stat *st)
{
    stat_calls++;
    YAZ_CHECK(expected_path && !strcmp(path, expected_path));
    memset(st, 0, sizeof(*st));
    st->st_size = sizeof(record) - 1;
    return fail_stat ? -1 : 0;
}

static FILE *test_xml_fopen(const char *path, const char *mode)
{
    FILE *f;
    open_calls++;
    YAZ_CHECK(expected_path && !strcmp(path, expected_path));
    YAZ_CHECK(!strcmp(mode, "rb"));
    if (fail_open)
        return 0;
    f = tmpfile();
    YAZ_CHECK(f != 0);
    if (f)
    {
        YAZ_CHECK_EQ(fwrite(record, 1, sizeof(record) - 1, f),
                     sizeof(record) - 1);
        rewind(f);
    }
    return f;
}

/* Include system headers before replacing calls. With large-file support,
   renaming declarations can otherwise export the mocks as fopen64/stat64
   and intercept calls from the logger, causing recursive logging to hang.
   Function-like macros also leave the struct stat type unchanged. */
#undef HAVE_CONFIG_H
#define HAVE_CONFIG_H 0
#define getenv(name) test_xml_getenv(name)
#define stat(path, st) test_xml_stat(path, st)
#define fopen(path, mode) test_xml_fopen(path, mode)
#include "read-marc.c"
#undef getenv
#undef stat
#undef fopen

static void check_fetch(const char *base, const char *name, const char *path)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    char *result;
    prefix = base;
    expected_path = path;
    stat_calls = open_calls = 0;
    result = dummy_xml_record(1, odr, name);
    YAZ_CHECK_EQ(stat_calls, path ? 1 : 0);
    YAZ_CHECK_EQ(open_calls, path && !fail_stat ? 1 : 0);
    if (path && !fail_stat && !fail_open)
    {
        YAZ_CHECK(result && !strcmp(result, record));
    }
    else
    {
        YAZ_CHECK(result == 0);
    }
    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    static const char *invalid[] = {
        "", "../secret", "../../secret", "sub/../../secret",
        "/tmp/secret", "sub/record", "..\\secret", "sub\\record",
        "C:secret", "C:\\secret", "\\\\server\\share\\secret",
        "record:stream", "http://example.org/schema", "%2e%2e%2fsecret",
        "bad\nname", "bad\rname", "bad\tname", "bad name", "bad\x80name"
    };
    static const char *builtins[] = { 0, "marcxml", "OP", "pz2" };
    size_t i;
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_fetch("/records/", "mods", "/records/mods.1.xml");
    check_fetch("", "mods", "mods.1.xml");
    check_fetch("/records/prefix-", "MODS_v3.7", "/records/prefix-MODS_v3.7.1.xml");
    check_fetch("C:\\records\\", "mods", "C:\\records\\mods.1.xml");
    check_fetch("/records/", "a..b", "/records/a..b.1.xml");
    for (i = 0; i < sizeof(invalid) / sizeof(*invalid); i++)
    {
        check_fetch("/records/", invalid[i], 0);
        check_fetch("", invalid[i], 0);
    }
    check_fetch(0, "mods", 0);
    fail_stat = 1;
    check_fetch("/records/", "mods", "/records/mods.1.xml");
    fail_stat = 0;
    fail_open = 1;
    check_fetch("/records/", "mods", "/records/mods.1.xml");
    fail_open = 0;
    for (i = 0; i < sizeof(builtins) / sizeof(*builtins); i++)
    {
        ODR odr = odr_createmem(ODR_ENCODE);
        stat_calls = open_calls = 0;
        YAZ_CHECK(dummy_xml_record(1, odr, builtins[i]) != 0);
        YAZ_CHECK_EQ(stat_calls, 0);
        YAZ_CHECK_EQ(open_calls, 0);
        odr_destroy(odr);
    }
    YAZ_CHECK_TERM;
}
