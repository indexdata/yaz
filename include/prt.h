#ifndef PRT_H
#define PRT_H

#include <yconfig.h>

int MDF ber_boolean(ODR o, int *val);
int MDF ber_tag(ODR o, void *p, int class, int tag, int *constructed, int opt);
int MDF ber_enctag(ODR o, int class, int tag, int constructed);
int MDF ber_dectag(unsigned char *buf, int *class, int *tag, int *constructed);
int MDF odr_bool(ODR o, int **p, int opt);
int MDF odr_integer(ODR o, int **p, int opt);
int MDF odr_implicit_settag(ODR o, int class, int tag);
#if 0
int MDF odr_implicit(ODR o, int (*type)(ODR o, void *p, int opt), void *p,
    int class, int tag, int opt);
#endif
int MDF ber_enclen(ODR o, int len, int lenlen, int exact);
int MDF ber_declen(unsigned char *buf, int *len);
char MDF *odr_indent(ODR o);
int MDF ber_null(ODR o);
int MDF odr_null(ODR o, Odr_null **p, int opt);
int MDF ber_integer(ODR o, int *val);
int MDF odr_constructed_begin(ODR o, void *p, int class, int tag);
int MDF odr_constructed_end(ODR o);
int MDF odr_sequence_begin(ODR o, void *p, int size);
int MDF odr_sequence_end(ODR o);
int MDF ber_octetstring(ODR o, Odr_oct *p, int cons);
int MDF odr_octetstring(ODR o, Odr_oct **p, int opt);
int MDF odp_more_chunks(ODR o, unsigned char *base, int len);
int MDF odr_constructed_more(ODR o);
int MDF odr_bitstring(ODR o, Odr_bitmask **p, int opt);
int MDF ber_bitstring(ODR o, Odr_bitmask *p, int cons);
int MDF odr_generalstring(ODR o, char **p, int opt);
int MDF ber_oidc(ODR o, Odr_oid *p);
int MDF odr_oid(ODR o, Odr_oid **p, int opt);
int MDF odr_choice(ODR o, Odr_arm arm[], void *p, void *whichp);
int MDF odr_cstring(ODR o, char **p, int opt);
int MDF odr_sequence_of(ODR o, Odr_fun type, void *p, int *num);
int MDF odr_any(ODR o, Odr_any **p, int opt);
int MDF ber_any(ODR o, Odr_any **p);
int MDF completeBER(unsigned char *buf, int len);
void MDF odr_begin(ODR o);
void MDF odr_end(ODR o);
void MDF odr_release_mem(struct odr_memblock *p);
Odr_oid MDF *odr_oiddup(ODR odr, Odr_oid *o);
int MDF odr_grow_block(odr_ecblock *b, int min_bytes);
int MDF odr_write(ODR o, unsigned char *buf, int bytes);
int MDF odr_seek(ODR o, int whence, int offset);
int MDF odr_dumpBER(FILE *f, char *buf, int len);
void MDF odr_choice_bias(ODR o, int what);
int MDF odr_total(ODR o);
char *odr_errmsg(int n);

#endif
