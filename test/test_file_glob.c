/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <yaz/file_glob.h>
#include <yaz/test.h>
#include <yaz/log.h>
#include <yaz/wrbuf.h>

void tst_with_path(const char *tpath)
{
    yaz_glob_res_t glob_res;
    int ret = yaz_file_glob(tpath, &glob_res);
    if (ret == 0)
    {
        size_t n = yaz_file_glob_get_num(glob_res);
        size_t i;
        for (i = 0; i < n; i++)
        {
            yaz_log(YLOG_LOG, "match %s", yaz_file_glob_get_file(glob_res, i));
        }
    }
    yaz_file_globfree(&glob_res);
}

void tst(void)
{
    yaz_glob_res_t glob_res;
    int ret;
    WRBUF tpath = wrbuf_alloc();
    const char *srcdir = getenv("srcdir");
    
    if (srcdir)
    {
        wrbuf_puts(tpath, srcdir);
        wrbuf_puts(tpath, "/");
    }
    wrbuf_puts(tpath, "Make*.am");
    ret = yaz_file_glob(wrbuf_cstr(tpath), &glob_res);
    YAZ_CHECK_EQ(ret, 0);

    YAZ_CHECK_EQ(1, yaz_file_glob_get_num(glob_res));
    if (yaz_file_glob_get_num(glob_res) == 1)
    {
        const char *f = yaz_file_glob_get_file(glob_res, 0);
        size_t l_match = strlen("Makefile.am");
        YAZ_CHECK(f && strlen(f) >= l_match);
        if (f && strlen(f) >= l_match)
        {
            YAZ_CHECK(!strcmp(f + strlen(f) - l_match, "Makefile.am"));
        }
    }
    wrbuf_destroy(tpath);
    yaz_file_globfree(&glob_res);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    if (argc >= 2)
        tst_with_path(argv[1]);
    else
        tst();
    YAZ_CHECK_TERM;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

