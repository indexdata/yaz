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

#include <yaz/nmem.h>
#include <yaz/test.h>

void tst_nmem_malloc(void)
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
        if ((int) sizeof(long) >= j)
            *(long*) cp = 123L;
#if HAVE_LONG_LONG
        if ((int) sizeof(long long) >= j)
            *(long long*) cp = 123L;
#endif
        if ((int) sizeof(double) >= j)
            *(double*) cp = 12.2;
    }

    for (j = 2000; j<20000; j+= 2000)
    {
        cp = (char *) nmem_malloc(n, j);
        YAZ_CHECK(cp);
    }
    nmem_destroy(n);
}

void tst_nmem_strsplit(void)
{
    NMEM nmem = nmem_create();
    int num = 0;
    char **array = 0;

    nmem_strsplit(nmem, ",", "", &array, &num);
    YAZ_CHECK(num == 0);

    nmem_strsplitx(nmem, ",", "", &array, &num, 0);
    YAZ_CHECK(num == 1);
    YAZ_CHECK(num > 0 && !strcmp(array[0], ""));

    nmem_strsplit(nmem, ",", ",,", &array, &num);
    YAZ_CHECK(num == 0);

    nmem_strsplitx(nmem, ",", ",,", &array, &num, 0);
    YAZ_CHECK(num == 3);
    YAZ_CHECK(num > 0 && !strcmp(array[0], ""));
    YAZ_CHECK(num > 1 && !strcmp(array[1], ""));
    YAZ_CHECK(num > 2 && !strcmp(array[2], ""));

    nmem_strsplit(nmem, ",", ",a,b,,cd", &array, &num);
    YAZ_CHECK(num == 3);
    YAZ_CHECK(num > 0 && !strcmp(array[0], "a"));
    YAZ_CHECK(num > 1 && !strcmp(array[1], "b"));
    YAZ_CHECK(num > 2 && !strcmp(array[2], "cd"));

    nmem_strsplitx(nmem, ",", ",a,b,,cd", &array, &num, 0);
    YAZ_CHECK(num == 5);
    YAZ_CHECK(num > 0 && !strcmp(array[0], ""));
    YAZ_CHECK(num > 1 && !strcmp(array[1], "a"));
    YAZ_CHECK(num > 2 && !strcmp(array[2], "b"));
    YAZ_CHECK(num > 3 && !strcmp(array[3], ""));
    YAZ_CHECK(num > 4 && !strcmp(array[4], "cd"));

    nmem_strsplit_escape(nmem, ",", ",a,b,,cd", &array, &num, 0, '\\');
    YAZ_CHECK(num == 5);
    YAZ_CHECK(num > 0 && !strcmp(array[0], ""));
    YAZ_CHECK(num > 1 && !strcmp(array[1], "a"));
    YAZ_CHECK(num > 2 && !strcmp(array[2], "b"));
    YAZ_CHECK(num > 3 && !strcmp(array[3], ""));
    YAZ_CHECK(num > 4 && !strcmp(array[4], "cd"));

    nmem_strsplit_escape(nmem, ",", ",a,b\\,,cd", &array, &num, 0, '\\');
    YAZ_CHECK(num == 4);
    YAZ_CHECK(num > 0 && !strcmp(array[0], ""));
    YAZ_CHECK(num > 1 && !strcmp(array[1], "a"));
    YAZ_CHECK(num > 2 && !strcmp(array[2], "b,"));
    YAZ_CHECK(num > 3 && !strcmp(array[3], "cd"));

    nmem_strsplit_escape(nmem, ",", "\\,a,b\\,,c\\d", &array, &num, 0, '\\');
    YAZ_CHECK(num == 3);
    YAZ_CHECK(num > 0 && !strcmp(array[0], ",a"));
    YAZ_CHECK(num > 1 && !strcmp(array[1], "b,"));
    YAZ_CHECK(num > 2 && !strcmp(array[2], "cd"));

    nmem_strsplit_escape2(nmem, ",", "\\,a,b\\,\\,c\\\\\\|d",
                          &array, &num, 0, '\\', 1);
    YAZ_CHECK(num == 2);
    YAZ_CHECK(num > 0 && !strcmp(array[0], ",a"));
    YAZ_CHECK(num > 1 && !strcmp(array[1], "b,,c\\|d"));


    nmem_strsplit_escape2(nmem, ",", "\\,a,b\\,\\,c\\\\\\|d",
                          &array, &num, 0, '\\', 0);
    YAZ_CHECK(num == 2);
    YAZ_CHECK(num > 0 && !strcmp(array[0], "\\,a"));
    YAZ_CHECK(num > 1 && !strcmp(array[1], "b\\,\\,c\\\\\\|d"));

    nmem_destroy(nmem);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_nmem_malloc();
    tst_nmem_strsplit();
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

