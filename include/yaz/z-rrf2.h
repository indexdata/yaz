/* YC 0.2: Tue Feb 29 16:45:07 CET 2000 */
/* Module-H ResourceReport-Format-Resource-2 */

#ifndef z_rrf2_H
#define z_rrf2_H

#include <yaz/odr.h>
#include <yaz/z-core.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Z_ResourceReport2 Z_ResourceReport2;
YAZ_EXPORT int z_ResourceReport2 (ODR o, Z_ResourceReport2 **p, int opt, const char *name);

typedef struct Z_Estimate2 Z_Estimate2;
YAZ_EXPORT int z_Estimate2 (ODR o, Z_Estimate2 **p, int opt, const char *name);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
struct Z_ResourceReport2 {
	int num_estimates;
	Z_Estimate2 **estimates; /* OPT */
	Z_InternationalString *message; /* OPT */
};

struct Z_Estimate2 {
	Z_StringOrNumeric *type;
	Z_IntUnit *value;
};

#ifdef __cplusplus
}
#endif
#endif
