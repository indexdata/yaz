#ifndef YCONFIG_H
#define YCONFIG_H

#include <xmalloc.h>

#ifdef WINDOWS
#define MDF
#else
#ifndef MDF
#define MDF
#endif
#endif

#endif
