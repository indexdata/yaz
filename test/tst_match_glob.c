/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#include <yaz/test.h>
#include <yaz/match_glob.h>
#include <stdlib.h>
#include <string.h>

void tst1(void)
{
    YAZ_CHECK_EQ(yaz_match_glob("a", "a"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("", ""), 1);
    YAZ_CHECK_EQ(yaz_match_glob("a", ""), 0);
    YAZ_CHECK_EQ(yaz_match_glob("", "a"), 0);
    YAZ_CHECK_EQ(yaz_match_glob("a", "b"), 0);
    YAZ_CHECK_EQ(yaz_match_glob("b", "a"), 0);

    YAZ_CHECK_EQ(yaz_match_glob("?", "a"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("a", "?"), 0);
    YAZ_CHECK_EQ(yaz_match_glob("?", "aa"), 0);
    YAZ_CHECK_EQ(yaz_match_glob("?a", "aa"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("a?", "aa"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("??", "aa"), 1);

    YAZ_CHECK_EQ(yaz_match_glob("*", ""), 1);
    YAZ_CHECK_EQ(yaz_match_glob("*", "a"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("**", "a"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("*a", "a"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("a*", "a"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("b*", "a"), 0);
    YAZ_CHECK_EQ(yaz_match_glob("*b", "a"), 0);
    YAZ_CHECK_EQ(yaz_match_glob("**b", "a"), 0);
    YAZ_CHECK_EQ(yaz_match_glob("*b*", "a"), 0);

    YAZ_CHECK_EQ(yaz_match_glob("*:w", "title:w"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("title:w", "title:w"), 1);
    YAZ_CHECK_EQ(yaz_match_glob("title:*", "title:w"), 1);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);

    tst1();

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

