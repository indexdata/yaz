/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file proxunit.c
 * \brief ProximityUnit map to/from string
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/proxunit.h>
#include <yaz/z-core.h>

/* Hand-written, but in reality depends on z3950v3.asn */
static const char *units[] = {
    0,
    "character",
    "word",
    "sentence",
    "paragraph",
    "section",
    "chapter",
    "document",
    "element",
    "subelement",
    "elementType",
    "byte"
};

const char *z_ProxUnit_to_str(int u)
{
    if (u >= Z_ProxUnit_character && u <= Z_ProxUnit_byte)
        return units[u];
    return 0;
}

int z_str_to_ProxUnit(const char *str)
{
    int i;
    for (i = 1; i <= Z_ProxUnit_byte; i++)
        if (!strcmp(units[i], str))
            return i;
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
