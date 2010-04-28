/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

struct yaz_mutex {
#ifdef WIN32
    CRITICAL_SECTION handle;
#elif YAZ_POSIX_THREADS
    pthread_mutex_t handle;
#endif
    char *name;
    int log_level;
};

