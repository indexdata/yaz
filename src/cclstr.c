/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/** 
 * \file cclstr.c
 * \brief Implements CCL string compare utilities
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <yaz/ccl.h>

static int ccli_toupper (int c)
{
    return toupper (c);
}

int (*ccl_toupper)(int c) = NULL;

int ccl_stricmp (const char *s1, const char *s2)
{
    if (!ccl_toupper)
        ccl_toupper = ccli_toupper;
    while (*s1 && *s2)
    {
        int c1, c2;
        c1 = (*ccl_toupper)(*s1);
        c2 = (*ccl_toupper)(*s2);
        if (c1 != c2)
            return c1 - c2;
        s1++;
        s2++;
    }
    return (*ccl_toupper)(*s1) - (*ccl_toupper)(*s2);
}

int ccl_memicmp (const char *s1, const char *s2, size_t n)
{
    if (!ccl_toupper)
        ccl_toupper = ccli_toupper;
    while (1)
    {
        int c1, c2;

        c1 = (*ccl_toupper)(*s1);
        c2 = (*ccl_toupper)(*s2);
        if (n <= 1 || c1 != c2)
            return c1 - c2;
        s1++;
        s2++;
        --n;
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

