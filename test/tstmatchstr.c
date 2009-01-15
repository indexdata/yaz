/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

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

