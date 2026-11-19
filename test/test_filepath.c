/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <yaz/tpath.h>
#include <yaz/test.h>

void tst(void)
{
    char fullpath[1024];
    const char *filename = "test_filepath.tmp";
    FILE *f = fopen(filename, "w");
    YAZ_CHECK(f != 0);
    if (!f)
        return;
    fclose(f);
    YAZ_CHECK(yaz_filepath_resolve(filename, ".", 0, fullpath));
    YAZ_CHECK(strcmp(fullpath, "./test_filepath.tmp") == 0);
    YAZ_CHECK(!yaz_filepath_resolve("test_filepath1", ".", 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve(filename, "bogus", 0, fullpath));
    YAZ_CHECK(yaz_filepath_resolve(filename, "bogus:.", 0, fullpath));
    YAZ_CHECK_EQ(remove(filename), 0);
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

