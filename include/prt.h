#ifndef PRT_H
#define PRT_H

#include <yconfig.h>

int ber_boolean(ODR o, int *val);
int ber_tag(ODR o, void *p, int class, int tag, int *constructed, int opt);
int ber_enctag(ODR o, int class, int tag, int constructed);
int ber_dectag(unsigned char *buf, int *class, int *tag, int *constructed);
int odr_bool(ODR o, int **p, int opt);
int odr_integer(ODR o, int **p, int opt);
int odr_implicit_settag(ODR o, int class, int tag);
#if 0
int odr_implicit(ODR o, int (*type)(ODR o, void *p, int opt), void *p,
    int class, int tag, int opt);
#endif
int ber_enclen(ODR o, int len, int lenlen, int exact);
int ber_declen(unsigned char *buf, int *len);
char *odr_indent(ODR o);
int ber_null(ODR o);
int odr_null(ODR o, Odr_null **p, int opt);
int ber_integer(ODR o, int *val);
int odr_constructed_begin(ODR o, void *p, int class, int tag);
int odr_constructed_end(ODR o);
int odr_sequence_begin(ODR o, void *p, int size);
int odr_sequence_end(ODR o);
int ber_octetstring(ODR o, Odr_oct *p, int cons);
int odr_octetstring(ODR o, Odr_oct **p, int opt);
int odp_more_chunks(ODR o, unsigned char *base, int len);
int odr_constructed_more(ODR o);
int odr_bitstring(ODR o, Odr_bitmask **p, int opt);
int ber_bitstring(ODR o, Odr_bitmask *p, int cons);
int odr_generalstring(ODR o, char **p, int opt);
int ber_oidc(ODR o, Odr_oid *p);
int odr_oid(ODR o, Odr_oid **p, int opt);
int odr_choice(ODR o, Odr_arm arm[], void *p, void *whichp);
int odr_cstring(ODR o, char **p, int opt);
int odr_sequence_of(ODR o, Odr_fun type, void *p, int *num);
int odr_any(ODR o, Odr_any **p, int opt);
int ber_any(ODR o, Odr_any **p);
int completeBER(unsigned char *buf, int len);
void odr_begin(ODR o);
void odr_end(ODR o);
void odr_release_mem(struct odr_memblock *p);
Odr_oid *odr_oiddup(ODR odr, Odr_oid *o);
int odr_grow_block(odr_ecblock *b, int min_bytes);
int odr_write(ODR o, unsigned char *buf, int bytes);
int odr_seek(ODR o, int whence, int offset);
int odr_dumpBER(FILE *f, char *buf, int len);
void odr_choice_bias(ODR o, int what);
int odr_total(ODR o);
char *odr_errmsg(int n);

#endif
