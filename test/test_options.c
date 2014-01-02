/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/options.h>
#include <yaz/log.h>
#include <yaz/test.h>

static void tst(void)
{
    char *argv[16] = {
        "program",
        "-a",
        "-b",
        "-",
        "-cd",
        "--longa",
        "-n11",
        "-n", "12",
        "--marmelade", "13",
        "file",
        "--",
        "-a",
        "--b",
        "other"};
    int argc = sizeof(argv) / sizeof(*argv);
    char *arg = 0;
    const char *desc = "a{longa}b{longb}cdn:m{marmelade}:";

    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'a');
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'b');
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 0);
    YAZ_CHECK(arg && !strcmp(arg, "-"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'c');
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'd');
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'a');
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'n');
    YAZ_CHECK(arg && !strcmp(arg, "11"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'n');
    YAZ_CHECK(arg && !strcmp(arg, "12"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 'm');
    YAZ_CHECK(arg && !strcmp(arg, "13"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 0);
    YAZ_CHECK(arg && !strcmp(arg, "file"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 0);
    YAZ_CHECK(arg && !strcmp(arg, "-a"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 0);
    YAZ_CHECK(arg && !strcmp(arg, "--b"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), 0);
    YAZ_CHECK(arg && !strcmp(arg, "other"));
    YAZ_CHECK_EQ(options(desc, argv, argc, &arg), YAZ_OPTIONS_EOF);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
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

