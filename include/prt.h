#ifndef PRT_H
#define PRT_H

#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT int ber_boolean(ODR o, int *val);
YAZ_EXPORT int ber_tag(ODR o, void *p, int zclass, int tag, int *constructed, int opt);
YAZ_EXPORT int ber_enctag(ODR o, int zclass, int tag, int constructed);
YAZ_EXPORT int ber_dectag(unsigned char *buf, int *zclass, int *tag, int *constructed);
YAZ_EXPORT int odr_bool(ODR o, int **p, int opt);
YAZ_EXPORT int odr_integer(ODR o, int **p, int opt);
YAZ_EXPORT int odr_implicit_settag(ODR o, int zclass, int tag);
#if 0
YAZ_EXPORT int odr_implicit(ODR o, int (*type)(ODR o, void *p, int opt), void *p,
    int zclass, int tag, int opt);
#endif
YAZ_EXPORT int ber_enclen(ODR o, int len, int lenlen, int exact);
YAZ_EXPORT int ber_declen(unsigned char *buf, int *len);
YAZ_EXPORT char *odr_indent(ODR o);
YAZ_EXPORT int ber_null(ODR o);
YAZ_EXPORT int odr_null(ODR o, Odr_null **p, int opt);
YAZ_EXPORT int ber_integer(ODR o, int *val);
YAZ_EXPORT int odr_constructed_begin(ODR o, void *p, int zclass, int tag);
YAZ_EXPORT int odr_constructed_end(ODR o);
YAZ_EXPORT int odr_sequence_begin(ODR o, void *p, int size);
YAZ_EXPORT int odr_sequence_end(ODR o);
YAZ_EXPORT int ber_octetstring(ODR o, Odr_oct *p, int cons);
YAZ_EXPORT int odr_octetstring(ODR o, Odr_oct **p, int opt);
YAZ_EXPORT int odp_more_chunks(ODR o, unsigned char *base, int len);
YAZ_EXPORT int odr_constructed_more(ODR o);
YAZ_EXPORT int odr_bitstring(ODR o, Odr_bitmask **p, int opt);
YAZ_EXPORT int ber_bitstring(ODR o, Odr_bitmask *p, int cons);
YAZ_EXPORT int odr_generalstring(ODR o, char **p, int opt);
YAZ_EXPORT int ber_oidc(ODR o, Odr_oid *p);
YAZ_EXPORT int odr_oid(ODR o, Odr_oid **p, int opt);
YAZ_EXPORT int odr_choice(ODR o, Odr_arm arm[], void *p, void *whichp);
YAZ_EXPORT int odr_cstring(ODR o, char **p, int opt);
YAZ_EXPORT int odr_sequence_of(ODR o, Odr_fun type, void *p, int *num);
YAZ_EXPORT int odr_any(ODR o, Odr_any **p, int opt);
YAZ_EXPORT int ber_any(ODR o, Odr_any **p);
YAZ_EXPORT int completeBER(unsigned char *buf, int len);
YAZ_EXPORT void odr_begin(ODR o);
YAZ_EXPORT void odr_end(ODR o);
YAZ_EXPORT Odr_oid *odr_oiddup(ODR odr, Odr_oid *o);
YAZ_EXPORT int odr_grow_block(odr_ecblock *b, int min_bytes);
YAZ_EXPORT int odr_write(ODR o, unsigned char *buf, int bytes);
YAZ_EXPORT int odr_seek(ODR o, int whence, int offset);
YAZ_EXPORT int odr_dumpBER(FILE *f, char *buf, int len);
YAZ_EXPORT void odr_choice_bias(ODR o, int what);
YAZ_EXPORT void odr_choice_enable_bias(ODR o, int mode);
YAZ_EXPORT int odr_total(ODR o);
YAZ_EXPORT char *odr_errmsg(int n);
YAZ_EXPORT Odr_oid *odr_getoidbystr(ODR o, char *str);
YAZ_EXPORT int odr_initmember(ODR o, void *p, int size);
YAZ_EXPORT int odr_peektag(ODR o, int *zclass, int *tag, int *cons);
YAZ_EXPORT void odr_setlenlen(ODR o, int len);

#ifdef __cplusplus
}
#endif

#endif
