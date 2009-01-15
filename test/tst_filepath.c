/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <yaz/tpath.h>
#include <yaz/test.h>

void tst(void)
{
    char fullpath[1024];
    YAZ_CHECK(yaz_filepath_resolve("tst_filepath", ".", 0, fullpath));
    YAZ_CHECK(strcmp(fullpath, "./tst_filepath") == 0);
    YAZ_CHECK(!yaz_filepath_resolve("tst_filepath1", ".", 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve("tst_filepath", "bogus", 0, fullpath));
    YAZ_CHECK(yaz_filepath_resolve("tst_filepath", "bogus:.", 0, fullpath));
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
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

