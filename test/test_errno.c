/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <errno.h>
#include <string.h>
#include <yaz/errno.h>
#include <yaz/test.h>
#ifdef WIN32
#include <windows.h>
#endif

static void check_error(int error)
{
    char full[256];
    unsigned char storage[512];
    size_t size, i;
    yaz_set_errno(error);
#ifdef WIN32
    SetLastError(ERROR_ACCESS_DENIED);
#endif
    yaz_strerror(full, sizeof(full));
    YAZ_CHECK(full[0] != '\0');
    for (size = 0; size <= sizeof(full); size++)
    {
        char *buf = (char *) storage + 1;
        memset(storage, 0xa5, sizeof(storage));
        yaz_set_errno(error);
#ifdef WIN32
        SetLastError(ERROR_ACCESS_DENIED);
#endif
        yaz_strerror(buf, size);
        YAZ_CHECK_EQ(storage[0], 0xa5);
        for (i = size + 1; i < sizeof(storage); i++)
            if (storage[i] != 0xa5)
                break;
        YAZ_CHECK_EQ(i, sizeof(storage));
        if (size)
            YAZ_CHECK(memchr(buf, '\0', size) != 0);
#ifdef WIN32
        /* FormatMessage needs room for CR/LF before yaz_strerror trims it. */
        if (size == sizeof(full))
#else
        if (size == strlen(full) + 1)
#endif
            YAZ_CHECK(!strcmp(buf, full));
    }
    yaz_strerror(0, 0);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_error(EINVAL);
    check_error(ENOENT);
    check_error(-12345);
    YAZ_CHECK_TERM;
}
