#ifndef STATSERVER_H
#define STATSERVER_H

#include <oid.h>

typedef struct statserv_options_block
{
    int dynamic;                  /* fork on incoming requests */
    int loglevel;                 /* desired logging-level */
    char apdufile[ODR_MAXNAME+1];      /* file for pretty-printed PDUs */
    char logfile[ODR_MAXNAME+1];       /* file for diagnostic output */
    char default_listen[1024];    /* 0 == no default listen */
    enum oid_proto default_proto; /* PROTO_SR or PROTO_Z3950 */
    int idle_timeout;             /* how many minutes to wait before closing */
    int maxrecordsize;            /* maximum value for negotiation */
    char configname[ODR_MAXNAME+1];    /* given to the backend in bend_init */
} statserv_options_block;

int statserv_main(int argc, char **argv);
statserv_options_block *statserv_getcontrol(void);
void statserv_setcontrol(statserv_options_block *block);

#endif
