#ifndef YCONFIG_H
#define YCONFIG_H

#ifndef YAZ_EXPORT
# ifdef WIN32
#  define YAZ_EXPORT __declspec(dllexport)
# else
#  define YAZ_EXPORT
# endif
#endif

#ifdef __cplusplus
#define YAZ_BEGIN_CDECL extern "C" {
#define YAZ_END_CDECL }
#else
#define YAZ_BEGIN_CDECL 
#define YAZ_END_CDECL 
#endif

#endif
