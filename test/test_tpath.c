/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/tpath.h>
#include <yaz/test.h>
#include <string.h>
#include <yaz/log.h>

static void tst_tpath(void)
{
    char fullpath[FILENAME_MAX];
    const char *filename = "test_tpath.tmp";
    FILE *f = fopen(filename, "w");
    YAZ_CHECK(f != 0);
    if (!f)
        return;
    fclose(f);

    YAZ_CHECK(!yaz_filepath_resolve("etc", 0, 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve("etc", "", 0, fullpath)); /* bug #606 */
    YAZ_CHECK(!yaz_filepath_resolve("etc", ".", 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve("does_not_exist", "", 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve("does_not_exist", ".", 0, fullpath));
    YAZ_CHECK(yaz_filepath_resolve(filename, 0, 0, fullpath));

    YAZ_CHECK(yaz_filepath_resolve(filename, "", 0, fullpath));
    YAZ_CHECK(yaz_filepath_resolve(filename, ".", 0, fullpath));

    YAZ_CHECK(!yaz_filepath_resolve(filename, "unknown_dir", 0, fullpath));
    YAZ_CHECK(yaz_filepath_resolve(filename, "unknown_dir:.", 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve(filename, "unknown_dir:", 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve(filename, "unknown_dir:c:", 0, fullpath));
    YAZ_CHECK(!yaz_filepath_resolve(filename, "unknown_dir:c:\\other", 0, fullpath));
    YAZ_CHECK_EQ(remove(filename), 0);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst_tpath();
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

