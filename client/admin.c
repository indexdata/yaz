/*
 * $Log: admin.c,v $
 * Revision 1.2  2000-03-14 14:06:04  ian
 * Minor change to order of debugging output for send_apdu,
 * fixed encoding of admin request.
 *
 * Revision 1.1  2000/03/14 09:27:07  ian
 * Added code to enable sending of admin extended service requests
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <yaz/yaz-util.h>

#include <yaz/tcpip.h>
#ifdef USE_XTIMOSI
#include <yaz/xmosi.h>
#endif

#include <yaz/proto.h>
#include <yaz/marcdisp.h>
#include <yaz/diagbib1.h>

#include <yaz/pquery.h>

#ifdef ASN_COMPILED
/* #include <yaz/esadmin.h> */
#endif


/* Helper functions to get to various statics in the client */
ODR getODROutputStream();
void send_apdu(Z_APDU *a);



int sendAdminES(int type, char* dbname)
{
    ODR out = getODROutputStream();

    /* Type: 1=reindex, 2=truncate, 3=delete, 4=create, 5=import, 6=refresh, 7=commit */
    Z_APDU *apdu = zget_APDU(out, Z_APDU_extendedServicesRequest );
    Z_ExtendedServicesRequest *req = apdu->u.extendedServicesRequest;
    Z_External *r;
    int oid[OID_SIZE];
    Z_ESAdminOriginPartToKeep  *toKeep;
    Z_ESAdminOriginPartNotToKeep  *notToKeep;
    oident update_oid;
    printf ("Admin request\n");
    fflush(stdout);

    /* Set up the OID for the external */
    update_oid.proto = PROTO_Z3950;
    update_oid.oclass = CLASS_EXTSERV;
    update_oid.value = VAL_ADMINSERVICE;

    oid_ent_to_oid (&update_oid, oid);
    req->packageType = odr_oiddup(out,oid);
    req->packageName = "1.Extendedserveq";

    /* Allocate the external */
    r = req->taskSpecificParameters = (Z_External *) odr_malloc (out, sizeof(*r));
    r->direct_reference = odr_oiddup(out,oid);
    r->indirect_reference = 0;
    r->descriptor = 0;
    r->which = Z_External_ESAdmin;
    r->u.adminService = (Z_Admin *) odr_malloc(out, sizeof(*r->u.adminService));
    r->u.adminService->which = Z_Admin_esRequest;
    r->u.adminService->u.esRequest = (Z_AdminEsRequest *) odr_malloc(out, sizeof(*r->u.adminService->u.esRequest));

    toKeep = r->u.adminService->u.esRequest->toKeep = (Z_ESAdminOriginPartToKeep *) 
                     odr_malloc(out, sizeof(*r->u.adminService->u.esRequest->toKeep));

    switch ( type )
    {
        case 1:
            toKeep->which=Z_ESAdminOriginPartToKeep_reIndex;
	    toKeep->u.reIndex=odr_nullval();
	    break;
	case 2:
            toKeep->which=Z_ESAdminOriginPartToKeep_truncate;
	    toKeep->u.truncate=odr_nullval();
	    break;
	case 3:
            toKeep->which=Z_ESAdminOriginPartToKeep_delete;
	    toKeep->u.delete=odr_nullval();
	    break;
	case 4:
            toKeep->which=Z_ESAdminOriginPartToKeep_create;
	    toKeep->u.create=odr_nullval();
	    break;
        case 5:
            toKeep->which=Z_ESAdminOriginPartToKeep_import;
	    toKeep->u.import=odr_nullval();
	    break;
        case 6:
            toKeep->which=Z_ESAdminOriginPartToKeep_refresh;
	    toKeep->u.refresh=odr_nullval();
	    break;
        case 7:
            toKeep->which=Z_ESAdminOriginPartToKeep_commit;
	    toKeep->u.commit=odr_nullval();
	    break;
    }

    toKeep->databaseName = dbname;


    notToKeep = r->u.adminService->u.esRequest->notToKeep = (Z_ESAdminOriginPartNotToKeep *)
	odr_malloc(out, sizeof(*r->u.adminService->u.esRequest->notToKeep));
    notToKeep->which=Z_ESAdminOriginPartNotToKeep_recordsWillFollow;
    notToKeep->u.recordsWillFollow=odr_nullval();
    
    send_apdu(apdu);

    return 2;
}

/* cmd_adm_reindex <dbname>
   Ask the specified database to fully reindex itself */
int cmd_adm_reindex(char* arg)
{
    sendAdminES(1,arg);
}

/* cmd_adm_truncate <dbname>
   Truncate the specified database, removing all records and index entries, but leaving 
   the database & it's explain information intact ready for new records */
int cmd_adm_truncate(char* arg)
{
    sendAdminES(2,arg);
}

/* cmd_adm_create <dbname>
   Create a new database */
int cmd_adm_create(char* arg)
{
    sendAdminES(4,arg);
}

/* cmd_adm_delete <dbname>
   Delete a database */
int cmd_adm_delete(char* arg)
{
    sendAdminES(3,arg);
}

/* cmd_adm_import <dbname> <rectype> <sourcefile>
   Import the specified updated into the database
   N.B. That in this case, the import may contain instructions to delete records as well as new or updates
   to existing records */
int cmd_adm_import(char* arg)
{
    sendAdminES(5,arg);
}

/* "Freshen" the specified database, by checking metadata records against the sources from which they were 
   generated, and creating a new record if the source has been touched since the last extraction */
int cmd_adm_refresh(char* arg)
{
    sendAdminES(6,arg);
}

/* cmd_adm_commit 
   Make imported records a permenant & visible to the live system */
int cmd_adm_commit(char* arg)
{
    sendAdminES(7,NULL);
}

