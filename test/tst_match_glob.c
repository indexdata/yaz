/* $Id: tst_match_glob.c,v 1.1 2007-10-24 13:50:03 adam Exp $
   Copyright (C) 1995-2007
   Index Data ApS

This file is part of the Zebra server.

Zebra is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

Zebra is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

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
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

