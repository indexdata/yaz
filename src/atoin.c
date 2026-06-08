/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file atoin.c
 * \brief Implements atoi_n function.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <limits.h>
#include <yaz/marcdisp.h>
#include <yaz/yaz-iconv.h>

int atoi_n(const char *buf, int len)
{
    int val, ret;
    ret = yaz_atoi(buf, len, &val);
    return ret < 1 ? 0 : val;
}

int atoi_n_check(const char *buf, int size, int *val)
{
    int ret = yaz_atoi(buf, size, val);
    return ret != size ? 0 : 1;
}

int yaz_atoi(const char *ptr, int len, int *val)
{
    unsigned long uval;
    int ret = yaz_atoul(ptr, len, &uval);
    if (ret == -1)
        return -1;
    if (uval > (unsigned)INT_MAX)
        return -1;
    *val = (int) uval;
    return ret;
}

int yaz_atoul(const char *ptr, int len, unsigned long *val)
{
    int i;

    *val = 0;
    for (i = 0; i < len; i++)
    {
        if (!yaz_isdigit(ptr[i]))
            break;
        int digit = ptr[i] - '0';
        if (*val > (ULONG_MAX - digit) / 10) /* overflow */
            return -1;
        *val = *val * 10 + digit;
    }
    return i;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

