/*
 * Copyright (c) 1995-1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt.h,v $
 * Revision 1.25  1999-01-08 11:23:18  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.24  1998/03/20 17:29:20  adam
 * Include of odr_use.h in odr.h. Added prototype for odr_enum.
 *
 * Revision 1.23  1998/03/20 14:45:27  adam
 * Implemented odr_set_of and odr_enum.
 *
 * Revision 1.22  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.21  1997/10/31 12:20:08  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 */

#ifndef PRT_H
#define PRT_H

#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT int ber_boolean(ODR o, int *val);
YAZ_EXPORT int ber_tag(ODR o, void *p, int zclass, int tag, int *constructed, int opt);
YAZ_EXPORT int ber_enctag(ODR o, int zclass, int tag, int constructed);
YAZ_EXPORT int ber_dectag(const unsigned char *buf, int *zclass, int *tag, int *constructed);
YAZ_EXPORT int odr_bool(ODR o, int **p, int opt);
YAZ_EXPORT int odr_integer(ODR o, int **p, int opt);
YAZ_EXPORT int odr_enum(ODR o, int **p, int opt);
YAZ_EXPORT int odr_implicit_settag(ODR o, int zclass, int tag);
YAZ_EXPORT int ber_enclen(ODR o, int len, int lenlen, int exact);
YAZ_EXPORT int ber_declen(const unsigned char *buf, int *len);
YAZ_EXPORT char *odr_indent(ODR o);
YAZ_EXPORT int ber_null(ODR o);
YAZ_EXPORT int odr_null(ODR o, Odr_null **p, int opt);
YAZ_EXPORT int ber_integer(ODR o, int *val);
YAZ_EXPORT int odr_constructed_begin(ODR o, void *p, int zclass, int tag);
YAZ_EXPORT int odr_constructed_end(ODR o);
YAZ_EXPORT int odr_sequence_begin(ODR o, void *p, int size);
YAZ_EXPORT int odr_set_begin(ODR o, void *p, int size);
YAZ_EXPORT int odr_sequence_end(ODR o);
YAZ_EXPORT int odr_set_end(ODR o);
YAZ_EXPORT int ber_octetstring(ODR o, Odr_oct *p, int cons);
YAZ_EXPORT int odr_octetstring(ODR o, Odr_oct **p, int opt);
YAZ_EXPORT int odp_more_chunks(ODR o, const unsigned char *base, int len);
YAZ_EXPORT int odr_constructed_more(ODR o);
YAZ_EXPORT int odr_bitstring(ODR o, Odr_bitmask **p, int opt);
YAZ_EXPORT int ber_bitstring(ODR o, Odr_bitmask *p, int cons);
YAZ_EXPORT int odr_generalstring(ODR o, char **p, int opt);
YAZ_EXPORT int ber_oidc(ODR o, Odr_oid *p);
YAZ_EXPORT int odr_oid(ODR o, Odr_oid **p, int opt);
YAZ_EXPORT int odr_choice(ODR o, Odr_arm arm[], void *p, void *whichp);
YAZ_EXPORT int odr_cstring(ODR o, char **p, int opt);
YAZ_EXPORT int odr_sequence_of(ODR o, Odr_fun type, void *p, int *num);
YAZ_EXPORT int odr_set_of(ODR o, Odr_fun type, void *p, int *num);
YAZ_EXPORT int odr_any(ODR o, Odr_any **p, int opt);
YAZ_EXPORT int ber_any(ODR o, Odr_any **p);
YAZ_EXPORT int completeBER(const unsigned char *buf, int len);
YAZ_EXPORT void odr_begin(ODR o);
YAZ_EXPORT void odr_end(ODR o);
YAZ_EXPORT Odr_oid *odr_oiddup(ODR odr, Odr_oid *o);
YAZ_EXPORT Odr_oid *odr_oiddup_nmem(NMEM nmem, Odr_oid *o);
YAZ_EXPORT int odr_grow_block(odr_ecblock *b, int min_bytes);
YAZ_EXPORT int odr_write(ODR o, unsigned char *buf, int bytes);
YAZ_EXPORT int odr_seek(ODR o, int whence, int offset);
YAZ_EXPORT int odr_dumpBER(FILE *f, const char *buf, int len);
YAZ_EXPORT void odr_choice_bias(ODR o, int what);
YAZ_EXPORT void odr_choice_enable_bias(ODR o, int mode);
YAZ_EXPORT int odr_total(ODR o);
YAZ_EXPORT char *odr_errmsg(int n);
YAZ_EXPORT Odr_oid *odr_getoidbystr(ODR o, char *str);
YAZ_EXPORT Odr_oid *odr_getoidbystr_nmem(NMEM o, char *str);
YAZ_EXPORT int odr_initmember(ODR o, void *p, int size);
YAZ_EXPORT int odr_peektag(ODR o, int *zclass, int *tag, int *cons);
YAZ_EXPORT void odr_setlenlen(ODR o, int len);
#ifdef __cplusplus
}
#endif

#endif
