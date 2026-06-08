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
#include <yaz/yaz-iconv.h>
#include <yaz/atoi.h>

int atoi_n(const char *buf, int len)
{
    int val, ret;
    ret = yaz_atoi(10, buf, len, &val);
    return ret < 1 ? 0 : val;
}

int atoi_n_check(const char *buf, int size, int *val)
{
    int ret = yaz_atoi(10, buf, size, val);
    return ret != size ? 0 : 1;
}

int yaz_atoi(int base, const char *ptr, int len, int *val)
{
    unsigned long uval;
    int ret = yaz_atoul(base, ptr, len, &uval);
    if (ret == -1)
        return -1;
    if (uval > INT_MAX)
        return -1;
    *val = (int) uval;
    return ret;
}

int yaz_atoul(int base, const char *ptr, int len, unsigned long *val)
{
    int i;

    if (base < 2 || base > 36)
        return -1;
    *val = 0;
    for (i = 0; i < len; i++)
    {
        int digit;
        if (yaz_isdigit(ptr[i]))
            digit = ptr[i] - '0';
        else if (yaz_isupper(ptr[i]))
            digit = ptr[i] - ('A' - 10);
        else if (yaz_islower(ptr[i]))
            digit = ptr[i] - ('a' - 10);
        else
            break;
        if (digit >= base)
            break;
        if (*val > (ULONG_MAX - digit) / base) /* overflow */
            return -1;
        *val = *val * base + digit;
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

