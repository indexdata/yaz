/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <yaz/matchstr.h>
#include <yaz/test.h>

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);

    YAZ_CHECK(yaz_matchstr("x", "x") == 0);
    YAZ_CHECK(yaz_matchstr("x", "X") == 0);
    YAZ_CHECK(yaz_matchstr("a", "b") > 0);
    YAZ_CHECK(yaz_matchstr("b", "a") > 0);
    YAZ_CHECK(yaz_matchstr("aa","a") > 0);
    YAZ_CHECK(yaz_matchstr("a-", "a") > 0);
    YAZ_CHECK(yaz_matchstr("A-b", "ab") == 0);
    YAZ_CHECK(yaz_matchstr("A--b", "ab") > 0);
    YAZ_CHECK(yaz_matchstr("A--b", "a-b") > 0);
    YAZ_CHECK(yaz_matchstr("A--b", "a--b") == 0);
    YAZ_CHECK(yaz_matchstr("a123",  "a?") == 0);
    YAZ_CHECK(yaz_matchstr("a123",   "a1.3") == 0);
    YAZ_CHECK(yaz_matchstr("a123",   "..?") == 0);
    YAZ_CHECK(yaz_matchstr("a123",   "a1.") > 0);
    YAZ_CHECK(yaz_matchstr("a123",   "a...") == 0);

    YAZ_CHECK_EQ(yaz_strncasecmp("a",  "b", 0), 0);
    YAZ_CHECK_EQ(yaz_strncasecmp("a",  "a", 1),  0);
    YAZ_CHECK_EQ(yaz_strncasecmp("a",  "a", 2), 0);
    YAZ_CHECK_EQ(yaz_strncasecmp("a",  "b", 1), -1);
    YAZ_CHECK_EQ(yaz_strncasecmp("a",  "b", 2), -1);
    YAZ_CHECK_EQ(yaz_strncasecmp("b",  "a", 1), 1);
    YAZ_CHECK_EQ(yaz_strncasecmp("b",  "a", 2), 1);

    YAZ_CHECK_EQ(yaz_strncasecmp("bb",  "ba", 1), 0);
    YAZ_CHECK_EQ(yaz_strncasecmp("bb",  "ba", 2), 1);
    YAZ_CHECK_EQ(yaz_strncasecmp("ba",  "bb", 2), -1);
    YAZ_CHECK_EQ(yaz_strncasecmp("ba",  "b", 2), 'a');
    YAZ_CHECK_EQ(yaz_strncasecmp("b",  "ba", 2), -'a');

    YAZ_CHECK_EQ(yaz_strcasecmp("",  ""), 0);
    YAZ_CHECK_EQ(yaz_strcasecmp("a",  "a"),  0);
    YAZ_CHECK_EQ(yaz_strcasecmp("a",  "b"), -1);
    YAZ_CHECK_EQ(yaz_strcasecmp("b",  "a"), 1);

    YAZ_CHECK_EQ(yaz_strcasecmp("bb",  "ba"), 1);
    YAZ_CHECK_EQ(yaz_strcasecmp("ba",  "bb"), -1);
    YAZ_CHECK_EQ(yaz_strcasecmp("ba",  "b"), 'a');
    YAZ_CHECK_EQ(yaz_strcasecmp("b",  "ba"), -'a');

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

