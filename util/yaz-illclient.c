/*
 * Copyright (C) 1995-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: yaz-illclient.c,v 1.4 2007-04-18 13:21:46 heikki Exp $
 */

/* WARNING - This is work in progress - not at all ready */

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
#include <yaz/oclc-ill-req-ext.h>


/* A structure for storing all the arguments */
struct prog_args {
    char *host;
    char *auth_userid;
    char *auth_passwd;
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
 /*
    } else if (!strcmp(element,"ill,transaction-id,initial-requester-id,person-or-institution-symbol,institution")) {
        ret="IndexData";
 */
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
    char *version="$Id: yaz-illclient.c,v 1.4 2007-04-18 13:21:46 heikki Exp $"; /* from cvs */

    /* default values */
    args->host = 0; /* not known (yet) */
#if 0    
    args->auth_userid = "100-228-301" ; /* FIXME - get from cmd line */
    args->auth_passwd = "dxg5magxc" ;   /* FIXME - get from cmd line */
#else
    args->auth_userid = "100070049" ; /* FIXME - get from cmd line */
    args->auth_passwd = "cowgirl" ;   /* FIXME - get from cmd line */
#endif


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
/* Makes a Z39.50-like prompt package with username and password */
Z_PromptObject1 *makeprompt(struct prog_args *args, ODR odr) {
    Z_PromptObject1 *p = odr_malloc(odr, sizeof(*p) );
    Z_ResponseUnit1 *ru = odr_malloc(odr, sizeof(*ru) );
    
    p->which=Z_PromptObject1_response;
    p->u.response = odr_malloc(odr, sizeof(*(p->u.response)) );
    p->u.response->num=2;
    p->u.response->elements=odr_malloc(odr, 
             p->u.response->num*sizeof(*(p->u.response->elements)) );
    /* user id, aka "oclc authorization number" */
    p->u.response->elements[0] = ru;
    ru->promptId = odr_malloc(odr, sizeof(*(ru->promptId) ));
    ru->promptId->which = Z_PromptId_enumeratedPrompt;
    ru->promptId->u.enumeratedPrompt =  
        odr_malloc(odr, sizeof(*(ru->promptId->u.enumeratedPrompt) ));
    ru->promptId->u.enumeratedPrompt->type = 
         odr_intdup(odr,Z_PromptIdEnumeratedPrompt_userId);
    ru->promptId->u.enumeratedPrompt->suggestedString = 0 ;
    ru->which = Z_ResponseUnit1_string ;
    ru->u.string = odr_strdup(odr, args->auth_userid);
    /* password */
    ru = odr_malloc(odr, sizeof(*ru) );
    p->u.response->elements[1] = ru;
    ru->promptId = odr_malloc(odr, sizeof(*(ru->promptId) ));
    ru->promptId->which = Z_PromptId_enumeratedPrompt;
    ru->promptId->u.enumeratedPrompt =  
        odr_malloc(odr, sizeof(*(ru->promptId->u.enumeratedPrompt) ));
    ru->promptId->u.enumeratedPrompt->type = 
         odr_intdup(odr,Z_PromptIdEnumeratedPrompt_password);
    ru->promptId->u.enumeratedPrompt->suggestedString = 0 ;
    ru->which = Z_ResponseUnit1_string ;
    ru->u.string = odr_strdup(odr, args->auth_passwd);
    return p;
} /* makeprompt */

ILL_Extension *makepromptextension(struct prog_args *args, ODR odr) {
    ODR odr_ext = odr_createmem(ODR_ENCODE);
    ODR odr_prt = odr_createmem(ODR_PRINT);
    ILL_Extension *e = odr_malloc(odr, sizeof(*e));
    Z_PromptObject1 *p = makeprompt(args,odr_ext);
    char * buf;
    int siz;
    Z_External *ext = odr_malloc(odr, sizeof(*ext));
    ext->direct_reference = odr_getoidbystr(odr,"1.2.840.10003.8.1");
    ext->indirect_reference=0;
    ext->descriptor=0;
    ext->which=Z_External_single;
    if ( ! z_PromptObject1(odr_ext, &p, 0,0 ) ) {
        yaz_log(YLOG_FATAL,"Encoding of z_PromptObject1 failed ");
        exit (6);
    }
    
    printf ("Prompt: \n"); /*!*/
    z_PromptObject1(odr_prt, &p, 0,0 ); /*!*/

    buf= odr_getbuf(odr_ext,&siz,0);
    ext->u.single_ASN1_type=odr_malloc(odr,sizeof(*ext->u.single_ASN1_type));
    ext->u.single_ASN1_type->buf= odr_malloc(odr, siz);
    memcpy(ext->u.single_ASN1_type->buf,buf, siz );
    ext->u.single_ASN1_type->len = ext->u.single_ASN1_type->size = siz;
    odr_reset(odr_ext);
    odr_reset(odr_prt); /*!*/

    e->identifier = odr_intdup(odr,1);
    e->critical = odr_intdup(odr,0);
    e->item=odr_malloc(odr,sizeof(*e->item));
    if ( ! z_External(odr_ext, &ext,0,0) ) {
        yaz_log(YLOG_FATAL,"Encoding of z_External failed ");
        exit (6);
    }
    printf("External: \n");
    z_External(odr_prt, &ext,0,0);  /*!*/
    buf= odr_getbuf(odr_ext,&siz,0); 
    e->item->buf= odr_malloc(odr, siz);
    memcpy(e->item->buf,buf, siz );
    e->item->len = e->item->size = siz;

    odr_destroy(odr_prt);
    odr_destroy(odr_ext);
    return e;
} /* makepromptextension */

ILL_Extension *makeoclcextension(struct prog_args *args, ODR odr) {
    /* The oclc extension is required, but only contains optional */
    /* fields. Here we just null them all out */
    ODR odr_ext = odr_createmem(ODR_ENCODE);
    ODR odr_prt = odr_createmem(ODR_PRINT);
    ILL_Extension *e = odr_malloc(odr, sizeof(*e));
    ILL_OCLCILLRequestExtension *oc = odr_malloc(odr_ext, sizeof(*oc));
    char * buf;
    int siz;
    Z_External *ext = odr_malloc(odr, sizeof(*ext));
    oc->clientDepartment = 0;
    oc->paymentMethod = 0;
    oc->uniformTitle = 0;
    oc->dissertation = 0;
    oc->issueNumber = 0;
    oc->volume = 0;
    oc->affiliations = 0;
    oc->source = 0;
    ext->direct_reference = odr_getoidbystr(odr,"1.0.10161.13.2");
    ext->indirect_reference=0;
    ext->descriptor=0;
    ext->which=Z_External_single;
    if ( ! ill_OCLCILLRequestExtension(odr_ext, &oc, 0,0 ) ) {
        yaz_log(YLOG_FATAL,"Encoding of ill_OCLCILLRequestExtension failed ");
        exit (6);
    }
    
    printf ("OCLC: \n"); /*!*/
    ill_OCLCILLRequestExtension(odr_prt, &oc, 0,0 ); /*!*/

    buf= odr_getbuf(odr_ext,&siz,0);
    ext->u.single_ASN1_type=odr_malloc(odr,sizeof(*ext->u.single_ASN1_type));
    ext->u.single_ASN1_type->buf= odr_malloc(odr, siz);
    memcpy(ext->u.single_ASN1_type->buf,buf, siz );
    ext->u.single_ASN1_type->len = ext->u.single_ASN1_type->size = siz;
    odr_reset(odr_ext);
    odr_reset(odr_prt); /*!*/

    e->identifier = odr_intdup(odr,1);
    e->critical = odr_intdup(odr,0);
    e->item=odr_malloc(odr,sizeof(*e->item));
    if ( ! z_External(odr_ext, &ext,0,0) ) {
        yaz_log(YLOG_FATAL,"Encoding of z_External failed ");
        exit (6);
    }
    printf("External: \n");
    z_External(odr_prt, &ext,0,0);  /*!*/
    buf= odr_getbuf(odr_ext,&siz,0); 
    e->item->buf= odr_malloc(odr, siz);
    memcpy(e->item->buf,buf, siz );
    e->item->len = e->item->size = siz;

    odr_destroy(odr_prt);
    odr_destroy(odr_ext);
    return e;

} /* makeoclcextension */

ILL_APDU *createrequest( struct prog_args *args, ODR odr) {
    struct ill_get_ctl ctl;
    ILL_APDU *apdu;
    ILL_Request *req;

    ctl.odr = odr;
    ctl.clientData = & args;
    ctl.f = get_ill_element;
    apdu = odr_malloc( odr, sizeof(*apdu) );
    apdu->which=ILL_APDU_ILL_Request;
    req = ill_get_ILLRequest(&ctl, "ill", 0);
    apdu->u.illRequest=req;
    req->num_iLL_request_extensions=2;
    req->iLL_request_extensions=
        odr_malloc(odr, req->num_iLL_request_extensions*sizeof(*req->iLL_request_extensions));
    req->iLL_request_extensions[0]=makepromptextension(args,odr);
    req->iLL_request_extensions[1]=makeoclcextension(args,odr);
    if (!req) {
        yaz_log(YLOG_FATAL,"Could not create ill request");
        exit(2);
    }
    return apdu;
} /* createrequest */


/* * * * * * * * * * * * * * * */
/** \brief Send the request */
void sendrequest(ILL_APDU *apdu, ODR odr, COMSTACK stack ) {
    char *buf_out;
    int len_out;
    int res;
    if (!ill_APDU  (odr, &apdu, 0, 0)) { 
        yaz_log(YLOG_FATAL,"ill_Apdu failed");
        exit(2);
    }
    buf_out = odr_getbuf(odr, &len_out, 0);
    if (0) {
        yaz_log(YLOG_DEBUG,"Request PDU Dump");
        odr_dumpBER(yaz_log_file(), buf_out, len_out);
    }
    if (!buf_out) {
        yaz_log(YLOG_FATAL,"Encoding failed. Len=%d", len_out);
        odr_perror(odr, "encoding failed");
        exit(2);
    }
    yaz_log(YLOG_DEBUG,"About to send the request. Len=%d", len_out);
    res = cs_put(stack, buf_out, len_out);
    if ( res<0 ) {
        yaz_log(YLOG_FATAL,"Could not send packet. code %d",res );
        exit (4);
    }
    if (1) {
        FILE *F = fopen("req.apdu","w");
        fwrite ( buf_out, 1, len_out, F);
        fclose(F);
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
    yaz_log(YLOG_DEBUG,"Got a response of %d bytes at %p. res=%d", len_in,buf_in, res);
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
    yaz_log(YLOG_DEBUG, "err= %p ",staterr->error_report );
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
                        *uerr->u.intermediary_problem);
                    break;
                case ILL_User_Error_Report_security_problem:
                    printf("Security problem: %s\n", 
                        getillstring(uerr->u.security_problem));
                    break;
                case ILL_User_Error_Report_unable_to_perform:
                    printf("Unable to perform: %d\n", 
                          *uerr->u.unable_to_perform);
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
                          *perr->u.general_problem);
                    break;
                case ILL_Provider_Error_Report_transaction_id_problem:
                    printf("Transaction Id Problem: %d\n", 
                          *perr->u.general_problem);
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
    if (1) 
        dumpapdu(apdu);
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

