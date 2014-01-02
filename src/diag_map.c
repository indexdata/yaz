/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file diag_map.c
 * \brief Implements SRU/Z39.50 diagnostic mappings
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <yaz/srw.h>

/* bib1:srw */
static int bib1_srw_map[] = {
    1, 1,
    2, 2,
    3, 48,
    4, 35,
    5, 12,
    6, 38,
    7, 30,
    8, 32,
    9, 29,
    10, 10,
    11, 12,
    11, 23,
    12, 60,
    13, 61,
    13, 62,
    14, 63,
    14, 64,
    14, 65,
    15, 68,
    15, 69,
    16, 70,
    17, 70,
    18, 50,
    19, 55,
    20, 56,
    21, 52,
    22, 50,
    23, 3,
    24, 66,
    25, 66,
    26, 66,
    27, 51,
    28, 52,
    29, 52,
    30, 51,
    31, 57,
    32, 58,
    33, 59,
    100, 1, /* bad map */
    101, 3,
    102, 3,
    103, 3,
    104, 3,
    105, 3,
    106, 66,
    107, 11,
    108, 10,
    108, 13,
    108, 14,
    108, 25,
    108, 26,
    108, 27,
    108, 45,

    109, 235,
    110, 37,
    111, 1,
    112, 58,
    113, 10,
    114, 16,
    115, 16,
    116, 16,
    117, 19,
    117, 20,
    118, 22,
    119, 32,
    119, 31,
    120, 28,
    121, 15,
    122, 32,
    123, 22,
    123, 17,
    123, 18,
    124, 24,
    125, 36,
    126, 36,
    127, 36,
    128, 51,
    129, 39,
    130, 43,
    131, 40,
    132, 42,
    201, 44,
    201, 33,
    201, 34,
    202, 41,
    203, 43,
    205, 1,  /* bad map */
    206, 1,  /* bad map */
    207, 89,
    208, 1,  /* bad map */
    209, 80,
    210, 80,
    210, 81,
    211, 84,
    212, 85,
    213, 92,
    214, 90,
    215, 91,
    216, 92,
    217, 63,
    218, 1,  /* bad map */
    219, 1,  /* bad map */
    220, 1,  /* bad map */
    221, 1,  /* bad map */
    222, 3,
    223, 1,  /* bad map */
    224, 1,  /* bad map */
    225, 1,  /* bad map */
    226, 1,  /* bad map */
    227, 66,
    228, 1,  /* bad map */
    229, 36,
    230, 83,
    231, 89,
    232, 1,
    233, 1, /* bad map */
    234, 1, /* bad map */
    235, 235,
    236, 236,
    237, 82,
    238, 67,
    239, 66,
    240, 1, /* bad map */
    241, 1, /* bad map */
    242, 70,
    243, 1, /* bad map */
    244, 66,
    245, 10,
    246, 10,
    247, 10,
    1001, 1, /* bad map */
    1002, 1, /* bad map */
    1003, 1, /* bad map */
    1004, 1, /* bad map */
    1005, 1, /* bad map */
    1006, 1, /* bad map */
    1007, 100,
    1008, 1,
    1009, 1,
    1010, 3,
    1011, 3,
    1012, 3,
    1013, 3,
    1014, 3,
    1015, 1015,
    1015, 3,
    1016, 3,
    1017, 3,
    1018, 2,
    1019, 2,
    1020, 2,
    1021, 3,
    1022, 3,
    1023, 3,
    1024, 16,
    1025, 3,
    1026, 64,
    1027, 1,
    1028, 65,
    1029, 1,
    1040, 1,
    /* 1041-1065 */
    1066, 66,
    1066, 67,
    1073, 16,
    1074, 1074,
    0
};

/*
 * This array contains overrides for when the first occurrence of a
 * particular SRW error in the array above does not correspond with
 * the best back-translation of that SRW error.
 */
static int srw_bib1_map[] = {
    10, 108,
    66, 238,
    235, 235,
    /* No doubt there are many more */
    0
};


int yaz_diag_bib1_to_srw (int code)
{
    const int *p = bib1_srw_map;
    while (*p)
    {
        if (code == p[0])
            return p[1];
        p += 2;
    }
    return 1;
}

int yaz_diag_srw_to_bib1(int code)
{
    /* Check explicit reverse-map first */
    const int *p = srw_bib1_map;
    while (*p)
    {
        if (code == p[0])
            return p[1];
        p += 2;
    }

    /* Fall back on reverse lookup in main map */
    p = bib1_srw_map;
    while (*p)
    {
        if (code == p[1])
            return p[0];
        p += 2;
    }
    return 1;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

