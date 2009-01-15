/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file waislen.c
 * \brief Implements WAIS package handling
 */

#include <stdio.h>
#include <yaz/comstack.h>
/*
 * Return length of WAIS package or 0
 */
int completeWAIS(const char *buf, int len)
{
    int i, lval = 0;

    if (len < 25)
        return 0;
    if (*buf != '0')
        return 0;
    /* calculate length */
    for (i = 0; i < 10; i++)
        lval = lval * 10 + (buf[i] - '0');
    lval += 25;
    if (len >= lval)
        return lval;
    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

