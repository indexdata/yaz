/*
 * Copyright (c) 2002-2004, Index Data
 * See the file LICENSE for details.
 *
 * $Id: tstmatchstr.c,v 1.2 2004-09-29 20:15:42 adam Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <yaz/yaz-iconv.h>

struct {
    char *s1;
    char *s2;
    int res;
} comp_strings[] = {
    { "x", "x", 0 },
    { "x", "X", 0 },
    { "a", "b", 1 },
    { "b", "a", 1 },
    { "aa","a", 1 },
    { "a-", "a", 1 },
    { "A-b", "ab", 0},
    { "A--b", "ab", 1},
    { "A--b", "a-b", 1},
    { "A--b", "a--b", 0},
    { "a123",  "a?", 0},
    {"a123",   "a1.3", 0},
    {"a123",   "..?", 0},
    {"a123",   "a1.", 1},
    {"a123",   "a...", 0},
    {0,  0, 0} };

int main (int argc, char **argv)
{
    int i;
    for (i = 0; comp_strings[i].s1; i++)
    {
        int got = yaz_matchstr(comp_strings[i].s1,comp_strings[i].s2);
        if (got > 0)
            got = 1;
        else if (got < 0)
            got = -1;
        if (got != comp_strings[i].res)
        {
            printf ("tststr %d got=%d res=%d\n", i,
                    got, comp_strings[i].res);
            exit(1);
        }
    }
    exit(0);
}

