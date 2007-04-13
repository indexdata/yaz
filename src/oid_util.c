/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: oid_util.c,v 1.3 2007-04-13 07:04:42 adam Exp $
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

void oid_oidcpy(int *t, const int *s)
{
    while ((*(t++) = *(s++)) > -1);
}

void oid_oidcat(int *t, const int *s)
{
    while (*t > -1)
        t++;
    while ((*(t++) = *(s++)) > -1);
}

int oid_oidcmp(const int *o1, const int *o2)
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

int oid_oidlen(const int *o)
{
    int len = 0;

    while (*(o++) >= 0)
        len++;
    return len;
}


char *oid_oid_to_dotstring(const int *oid, char *oidbuf)
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

int oid_dotstring_to_oid(const char *name, int *oid)
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
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

