#ifndef YCONFIG_H
#define YCONFIG_H

#ifdef WINDOWS
#define MDF pascal
#else
#ifndef MDF
#define MDF
#endif
#endif

#endif
