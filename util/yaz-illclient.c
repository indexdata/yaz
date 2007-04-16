/*
 * Copyright (C) 1995-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: yaz-illclient.c,v 1.1 2007-04-16 15:33:51 heikki Exp $
 */

/* NOTE - This is work in progress - not at all ready */

/** \file yaz-illclient.c
 *  \brief client for ILL requests (ISO 10161-1)
 *
 *  This is a test client for handling ISO 10161-1 ILL requests.
 *  Those are not directly Z39.50, but the protocol is quite similar
 *  and yaz already provides the APDUS for it.
 *
 *  This is not an interactive client like yaz-client, but driven by command-
 *  line arguments. Its output is a return code, and possibly some text on 
 *  stdout.
 *
 *  Exit codes  (note, the program exits as soon as it finds a good reason)
 *     0   ok
 *     1   errors in arguments
 *     2   Internal errors in creating objects (comstack, odr...)
 *         mostly programming errors.
 *     3   could not connect
 *     4   could not send request
 *     5   No reponse received
 *     6   Error decoding response packet
 *     7   Server returned an error (see log or stdout)
 *
 *
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif


#include <yaz/yaz-util.h>
#include <yaz/proto.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/unix.h>
#include <yaz/odr.h>
#include <yaz/log.h>
#include <yaz/ill.h>

/* A structure for storing all the arguments */
struct prog_args {
    char *host;
} ;


/* Call-back to be called for every field in the ill-request */
/* It can set values to any field, perhaps from the prog_args */
const char *get_ill_element(void *clientData, const char *element) {
    struct prog_args *args = clientData;
    char *ret=0;
    if (!strcmp(element,"foo")) {
        ret=args->host;
    } else if (!strcmp(element,"ill,protocol-version-num")) {
        ret="2";
    } else if (!strcmp(element,"ill,transaction-id,initial-requester-id,person-or-institution-symbol,institution")) {
        ret="IndexData";
    } 
    yaz_log(YLOG_DEBUG,"get_ill_element: '%s' -> '%s' ", element, ret );
    return ret;
}


/* * * * * * * * * * * * * * * * * */
/** \brief  Parse program arguments */
void parseargs( int argc, char * argv[],  struct prog_args *args) {
    int ret;
    char *arg;
    char *prog=*argv;
    char *version="$Id: yaz-illclient.c,v 1.1 2007-04-16 15:33:51 heikki Exp $"; /* from cvs */

    args->host=0; /* not known (yet) */

    while ((ret = options("k:c:q:a:b:m:v:p:u:t:Vxd:", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            if (!args->host)
            {
                args->host = xstrdup (arg);
            }
            else
            {
                fprintf(stderr, "%s: Specify most one server address\n",
                        prog);
                exit(1);
            }
            break;
        case 'v':
            yaz_log_init(yaz_log_mask_str(arg), "", 0);
            break;
        case 'V':
            printf("%s %s",prog, version );
            break;
        default:
            fprintf (stderr, "Usage: %s "
                     " [-v loglevel...]"
                     " [-V]"
                     " <server-addr>\n",
                     prog);
            exit (1);
        }
    }
} /* parseargs */

/* * * * * * * * * * * */
/** \brief  Validate the arguments make sense */
void validateargs( struct prog_args *args) {
    if (!args->host) {
        fprintf(stderr, "Specify a connection address, "
                        "as in 'bagel.indexdata.dk:210' \n");
        exit(1);
    }
} /* validateargs */


/* * * * * * * * * * * * * * * */
/** \brief  Connect to the target */
COMSTACK connect_to( char *hostaddr ){
    COMSTACK stack;
    void *server_address_ip;
    int status;

    yaz_log(YLOG_DEBUG,"Connecting to '%s'", hostaddr);
    stack = cs_create_host(hostaddr, 1, &server_address_ip );
    if (!stack) {
        yaz_log(YLOG_FATAL,"Error in creating the comstack '%s' ",
                 hostaddr );
        exit(2);
    }
    
    yaz_log(YLOG_DEBUG,"Created stack ok ");

    status = cs_connect(stack, server_address_ip);
    if (status != 0) {
        yaz_log(YLOG_FATAL|YLOG_ERRNO,"Can not connect '%s' ",
                 hostaddr );
        exit(3);
    }
    yaz_log(YLOG_DEBUG,"Connected OK to '%s'", hostaddr);
    return stack;
}

/* * * * * * * * * * * * * * * */
ILL_APDU *createrequest( struct prog_args *args, ODR out_odr) {
    struct ill_get_ctl ctl;
    ILL_APDU *apdu;
    ILL_Request *req;

    ctl.odr = out_odr;
    ctl.clientData = & args;
    ctl.f = get_ill_element;
    apdu = odr_malloc( out_odr, sizeof(*apdu) );
    apdu->which=ILL_APDU_ILL_Request;
    req = ill_get_ILLRequest(&ctl, "ill", 0);
    apdu->u.illRequest=req;
    if (!req) {
        yaz_log(YLOG_FATAL,"Could not create ill request");
        exit(2);
    }

    return apdu;
} /* createrequest */


/* * * * * * * * * * * * * * * */
/** \brief Send the request */
void sendrequest(ILL_APDU *apdu, ODR out_odr, COMSTACK stack ) {
    char *buf_out;
    int len_out;
    int res;
    if (!ill_APDU  (out_odr, &apdu, 0, 0)) { 
        yaz_log(YLOG_FATAL,"ill_Apdu failed");
        exit(2);
    }
    buf_out = odr_getbuf(out_odr, &len_out, 0);
    if (0) {
        yaz_log(YLOG_DEBUG,"Request PDU Dump");
        odr_dumpBER(yaz_log_file(), buf_out, len_out);
    }
    if (!buf_out) {
        yaz_log(YLOG_FATAL,"Encoding failed. Len=%d", len_out);
        odr_perror(out_odr, "encoding failed");
        exit(2);
    }
    yaz_log(YLOG_DEBUG,"About to send the request. Len=%d", len_out);
    res = cs_put(stack, buf_out, len_out);
    if ( res<0 ) {
        yaz_log(YLOG_FATAL,"Could not send packet. code %d",res );
        exit (4);
    }
} /* sendrequest */

/* * * * * * * * * * * * * * * */
/** \brief  Get a response */
ILL_APDU *getresponse( COMSTACK stack, ODR in_odr ){
    ILL_APDU *resp;
    int res;
    char *buf_in=0;
    int len_in=0;
    yaz_log(YLOG_DEBUG,"About to wait for a response");
    res = cs_get(stack, &buf_in, &len_in);
    yaz_log(YLOG_DEBUG,"Got a response of %d bytes at %x. res=%d", len_in,buf_in, res);
    if (res<0) {
        yaz_log(YLOG_FATAL,"Could not receive packet. code %d",res );
        yaz_log(YLOG_DEBUG,"%02x %02x %02x %02x %02x %02x %02x %02x ...", 
                buf_in[0], buf_in[1], buf_in[2], buf_in[3],
                buf_in[4], buf_in[5], buf_in[6], buf_in[7]  );
        yaz_log(YLOG_DEBUG,"PDU Dump:");
        odr_dumpBER(yaz_log_file(), buf_in, len_in);
        exit (5);
    }
    odr_setbuf(in_odr, buf_in, res, 0);
    if (!ill_APDU (in_odr, &resp, 0, 0))
    {
        int x;
        int err = odr_geterrorx(in_odr, &x);
        char msg[60];
        const char *element = odr_getelement(in_odr);
        sprintf(msg, "ODR code %d:%d element=%-20s",
                err, x, element ? element : "<unknown>");
        yaz_log(YLOG_FATAL,"Error decoding incoming packet: %s",msg);
        yaz_log(YLOG_DEBUG,"%02x %02x %02x %02x %02x %02x %02x %02x ...", 
                buf_in[0], buf_in[1], buf_in[2], buf_in[3],
                buf_in[4], buf_in[5], buf_in[6], buf_in[7]  );
        yaz_log(YLOG_DEBUG,"PDU Dump:");
        odr_dumpBER(yaz_log_file(), buf_in, len_in);
        yaz_log(YLOG_FATAL,"Error decoding incoming packet: %s",msg);
        exit(6);
    }
    return resp;
} /* getresponse */


/** \brief Dump a apdu */
void dumpapdu( ILL_APDU *apdu) {
    ODR print_odr = odr_createmem(ODR_PRINT);
    ill_APDU (print_odr, &apdu, 0, 0);
    odr_destroy(print_odr);
} /* dumpapdu */

/** \brief  Check apdu type and extract the status_or_error */
ILL_Status_Or_Error_Report *getstaterr( ILL_APDU *resp, ODR in_odr ) {
    if (resp->which != ILL_APDU_Status_Or_Error_Report ) {
        const char *element = odr_getelement(in_odr);
        if (!element) 
            element="unknown";
        printf("Server returned wrong packet type: %d\n", resp->which);
        yaz_log(YLOG_FATAL,"Server returned a (%d) and "
                 "not a 'Status_Or_Error_Report' (%d) ",
                 resp->which, ILL_APDU_Status_Or_Error_Report);
        exit(6);
    }
    return resp->u.Status_Or_Error_Report;
} /* getstaterr */

/** \brief  Return a printable string from an ILL_String */
char *getillstring( ILL_String *s) {
    if (s->which == ILL_String_GeneralString ) 
        return s->u.GeneralString;
    else if (s->which == ILL_String_EDIFACTString ) 
        return s->u.EDIFACTString;
    else {
        yaz_log(YLOG_FATAL,"Invalid ILL_String ");
        exit (6);
    }
} /* getillstring */

/** \brief Check if the status was an error packet */
/* The presence of an error_report indicates it was an error */
/* Then the problem is to find the right message. We dig around */
/* until we find the first message, print that, and exit the program */
void checkerr( ILL_Status_Or_Error_Report *staterr ) {
    yaz_log(YLOG_DEBUG, "err= %x ",staterr->error_report );
    if (staterr->error_report) {
        ILL_Error_Report *err= staterr->error_report;
        if ( err->user_error_report) {
            ILL_User_Error_Report *uerr= err->user_error_report;
            switch( uerr->which ) {
                case ILL_User_Error_Report_already_forwarded:
                    printf("Already forwarded: \n");
                    break;
                case ILL_User_Error_Report_intermediary_problem:
                    printf("Intermediary problem: %d\n", 
                        uerr->u.intermediary_problem);
                    break;
                case ILL_User_Error_Report_security_problem:
                    printf("Security problem: %s\n", 
                        getillstring(uerr->u.security_problem));
                    break;
                case ILL_User_Error_Report_unable_to_perform:
                    printf("Unable to perform: %d\n", 
                          uerr->u.unable_to_perform);
                    break;
                default:
                    printf("Unknown problem");
            }
            exit(7);
        }
        if ( err->provider_error_report) {
            ILL_Provider_Error_Report *perr= err->provider_error_report;
            switch( perr->which ) {
                case ILL_Provider_Error_Report_general_problem:
                    printf("General Problem: %d\n", 
                          perr->u.general_problem);
                    break;
                case ILL_Provider_Error_Report_transaction_id_problem:
                    printf("Transaction Id Problem: %d\n", 
                          perr->u.general_problem);
                    break;
                case ILL_Provider_Error_Report_state_transition_prohibited:
                    printf("State Transition prohibited \n");
                    break;
            }
            exit(7);
        } 
        /* fallbacks */
        if ( staterr->note ) 
            printf("%s", getillstring(staterr->note));
        else 
            printf("Unknown error type");
        exit(7);
    }
} /* checkerr */



/* * * * * * * * * * * * * * * */

/** \brief Main program 
 *
 * Parse arguments
 * Validate arguments
 * Establish connection
 * Build a request
 * Send a request
 * Get a reply
 * Parse reply
 * Produce output
 */

int main (int argc, char * argv[]) {
    struct prog_args args;
    COMSTACK stack;
    ODR out_odr = odr_createmem(ODR_ENCODE);
    ODR in_odr = odr_createmem(ODR_DECODE);
    ILL_APDU *apdu;
    ILL_APDU *resp;
    ILL_Status_Or_Error_Report *staterr;

    parseargs( argc, argv,  &args);
    validateargs(&args);
    stack = connect_to(args.host);
    apdu = createrequest(&args, out_odr);
    sendrequest(apdu, out_odr, stack ); 
    resp = getresponse(stack, in_odr );
    if (1) 
        dumpapdu(resp);
    staterr=getstaterr(resp, in_odr);
    checkerr(staterr);


    printf ("Ok\n"); /* while debugging */
    exit (0);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

