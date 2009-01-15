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

#include <yaz/nmem.h>
#include <yaz/test.h>

void tst(void)
{
    NMEM n;
    int j;
    char *cp;

    n = nmem_create();
    YAZ_CHECK(n);

    for (j = 1; j<500; j++)
    {
        cp = (char *) nmem_malloc(n, j);
        YAZ_CHECK(cp);
        if (sizeof(long) >= j)
            *(long*) cp = 123L;
#if HAVE_LONG_LONG
        if (sizeof(long long) >= j)
            *(long long*) cp = 123L;
#endif
        if (sizeof(double) >= j)
            *(double*) cp = 12.2;
    }
    
    for (j = 2000; j<20000; j+= 2000)
    {
        cp = (char *) nmem_malloc(n, j);
        YAZ_CHECK(cp);
    }
    nmem_destroy(n);
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

