/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

#include <stdlib.h>
#include <stdio.h>

#include <yaz/mutex.h>
#include <yaz/test.h>
#include <yaz/log.h>

static void tst(void)
{
    YAZ_MUTEX p = 0;

    yaz_mutex_create(&p);
    YAZ_CHECK(p);
    yaz_mutex_enter(p);
    yaz_mutex_leave(p);
    yaz_mutex_destroy(&p);
    YAZ_CHECK(p == 0);

    yaz_mutex_create(&p);
    YAZ_CHECK(p);
    yaz_mutex_set_name(p, YLOG_LOG, "mymutex");
    yaz_mutex_enter(p);
    yaz_mutex_leave(p);
    yaz_mutex_destroy(&p);
    YAZ_CHECK(p == 0);

    yaz_mutex_destroy(&p); /* OK to "destroy" NULL handle */
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
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

