/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <yaz/xmalloc.h>
#include <yaz/test.h>

void tst(void)
{
    char *p = 0;

    p = (char *) xmalloc(10);
    YAZ_CHECK(p);
    p = (char *) xrealloc(p, 20);
    YAZ_CHECK(p);
    xfree(p);

    p = xstrdup("hello");
    YAZ_CHECK(p);
    if (!p)
        return;
    YAZ_CHECK(!strcmp(p, "hello"));
    xfree(p);

    p = xstrndup("hello", 2);
    YAZ_CHECK(p);
    if (!p)
        return;
    YAZ_CHECK(!strcmp(p, "he"));
    xfree(p);

    p = xstrndup("hello", 6);
    YAZ_CHECK(p);
    if (!p)
        return;
    YAZ_CHECK(!strcmp(p, "hello"));
    xfree(p);
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

