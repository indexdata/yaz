/* YC 0.2: Wed Mar 01 10:28:12 CET 2000 */
/* Module-H RecordSyntax-SUTRS */

#ifndef z_sutrs_H
#define z_sutrs_H

#include <yaz/odr.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef Odr_oct Z_SUTRS;
YAZ_EXPORT int z_SUTRS (ODR o, Odr_oct **p, int opt);

#ifdef __cplusplus
}
#endif
#include <yaz/z-core.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef Z_InternationalString  Z_SutrsRecord;
YAZ_EXPORT int z_SutrsRecord (ODR o, Z_SutrsRecord **p, int opt, const char *name);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif
#endif
