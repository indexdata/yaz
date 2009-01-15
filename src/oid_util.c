/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file oid_util.c
 * \brief Implements OID base utilities
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <yaz/snprintf.h>
#include <yaz/oid_util.h>

void oid_oidcpy(Odr_oid *t, const Odr_oid *s)
{
    while ((*(t++) = *(s++)) > -1);
}

void oid_oidcat(Odr_oid *t, const Odr_oid *s)
{
    while (*t > -1)
        t++;
    while ((*(t++) = *(s++)) > -1);
}

int oid_oidcmp(const Odr_oid *o1, const Odr_oid *o2)
{
    while (*o1 == *o2 && *o1 > -1)
    {
        o1++;
        o2++;
    }
    if (*o1 == *o2)
        return 0;
    else if (*o1 > *o2)
        return 1;
    else
        return -1;
}

int oid_oidlen(const Odr_oid *o)
{
    int len = 0;

    while (*(o++) >= 0)
        len++;
    return len;
}


char *oid_oid_to_dotstring(const Odr_oid *oid, char *oidbuf)
{
    char tmpbuf[20];
    int i;

    oidbuf[0] = '\0';
    for (i = 0; oid[i] != -1 && i < OID_SIZE; i++) 
    {
        yaz_snprintf(tmpbuf, sizeof(tmpbuf)-1, "%d", oid[i]);
        if (i > 0)
            strcat(oidbuf, ".");
        strcat(oidbuf, tmpbuf);
    }
    return oidbuf;
}

int oid_dotstring_to_oid(const char *name, Odr_oid *oid)
{
    int i = 0;
    int val = 0;
    while (isdigit (*(unsigned char *) name))
    {
        val = val*10 + (*name - '0');
        name++;
        if (*name == '.')
        {
            if (i < OID_SIZE-1)
                oid[i++] = val;
            val = 0;
            name++;
        }
    }
    if (i == 0)
        return -1;
    oid[i] = val;
    oid[i+1] = -1;
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

