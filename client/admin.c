/*
 * $Log: admin.c,v $
 * Revision 1.7  2000-04-05 07:39:54  adam
 * Added shared library support (libtool).
 *
 * Revision 1.6  2000/03/20 19:06:25  adam
 * Added Segment request for fronend server. Work on admin for client.
 *
 * Revision 1.5  2000/03/17 12:47:02  adam
 * Minor changes to admin client.
 *
 * Revision 1.4  2000/03/16 13:55:49  ian
 * Added commands for sending shutdown and startup admin requests via the admin ES.
 *
 * Revision 1.3  2000/03/14 15:23:17  ian
 * Removed unwanted ifdef and include of zes-admin.h
 *
 * Revision 1.2  2000/03/14 14:06:04  ian
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
#include <assert.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
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

/* Helper functions to get to various statics in the client */
ODR getODROutputStream();
void send_apdu(Z_APDU *a);

extern char *databaseNames[];
extern int num_databaseNames;

int sendAdminES(int type, char* param1)
{
    ODR out = getODROutputStream();
    char *dbname = odr_strdup (out, databaseNames[0]);

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

    toKeep->which=type;
    toKeep->databaseName = dbname;
    switch ( type )
    {
    case Z_ESAdminOriginPartToKeep_reIndex:
	toKeep->u.reIndex=odr_nullval();
	break;
	
    case Z_ESAdminOriginPartToKeep_truncate:
	toKeep->u.truncate=odr_nullval();
	break;
    case Z_ESAdminOriginPartToKeep_drop:
	toKeep->u.drop=odr_nullval();
	break;
    case Z_ESAdminOriginPartToKeep_create:
	toKeep->u.create=odr_nullval();
	break;
    case Z_ESAdminOriginPartToKeep_import:
	toKeep->u.import = (Z_ImportParameters*)odr_malloc(out, sizeof(*toKeep->u.import));
	toKeep->u.import->recordType=param1;
	/* Need to add additional setup of records here */
	break;
    case Z_ESAdminOriginPartToKeep_refresh:
	toKeep->u.refresh=odr_nullval();
	break;
    case Z_ESAdminOriginPartToKeep_commit:
	toKeep->u.commit=odr_nullval();
	break;
    case Z_ESAdminOriginPartToKeep_shutdown:
	toKeep->u.commit=odr_nullval();
	break;
    case Z_ESAdminOriginPartToKeep_start:
	toKeep->u.commit=odr_nullval();
	break;
    default:
	/* Unknown admin service */
	break;
    }

    notToKeep = r->u.adminService->u.esRequest->notToKeep =
	(Z_ESAdminOriginPartNotToKeep *)
	odr_malloc(out, sizeof(*r->u.adminService->u.esRequest->notToKeep));
    notToKeep->which=Z_ESAdminOriginPartNotToKeep_recordsWillFollow;
    notToKeep->u.recordsWillFollow=odr_nullval();
    
    send_apdu(apdu);
    
    return 0;
}

/* cmd_adm_reindex
   Ask the specified database to fully reindex itself */
int cmd_adm_reindex(char* arg)
{
    sendAdminES(Z_ESAdminOriginPartToKeep_reIndex, NULL);
    return 2;
}

/* cmd_adm_truncate
   Truncate the specified database, removing all records and index entries, but leaving 
   the database & it's explain information intact ready for new records */
int cmd_adm_truncate(char* arg)
{
    if ( arg )
    {
        sendAdminES(Z_ESAdminOriginPartToKeep_truncate, NULL);
	return 2;
    }
    return 0;
}

/* cmd_adm_create
   Create a new database */
int cmd_adm_create(char* arg)
{
    if ( arg )
    {
        sendAdminES(Z_ESAdminOriginPartToKeep_create, NULL);
	return 2;
    }
    return 0;
}

/* cmd_adm_drop
   Drop (Delete) a database */
int cmd_adm_drop(char* arg)
{
    if ( arg )
    {
        sendAdminES(Z_ESAdminOriginPartToKeep_drop, NULL);
	return 2;
    }
    return 0;
}

/* cmd_adm_import <dbname> <rectype> <sourcefile>
   Import the specified updated into the database
   N.B. That in this case, the import may contain instructions to delete records as well as new or updates
   to existing records */

int cmd_adm_import(char *arg)
{
    char type_str[20], dir_str[1024], pattern_str[1024];
    char *cp;
    char *sep = "/";
    DIR *dir;
    struct dirent *ent;
    int chunk = 10;
    Z_APDU *apdu = 0;
    ODR out = getODROutputStream();

    if (arg && sscanf (arg, "%19s %1023s %1023s", type_str,
		       dir_str, pattern_str) != 3)
	return 0;
    if (num_databaseNames != 1)
	return 0;
    dir = opendir(dir_str);
    if (!dir)
	return 0;
    
    sendAdminES(Z_ESAdminOriginPartToKeep_import, type_str);

    printf ("sent es request\n");
    if ((cp=strrchr(dir_str, '/')) && cp[1] == 0)
	sep="";
	
    while ((ent = readdir(dir)))
    {
	if (fnmatch (pattern_str, ent->d_name, 0) == 0)
	{
	    char fname[1024];
	    struct stat status;
	    FILE *inf;
		
	    sprintf (fname, "%s%s%s", dir_str, sep, ent->d_name);
	    stat (fname, &status);

	    if (S_ISREG(status.st_mode) && (inf = fopen(fname, "r")))
	    {
		Z_Segment *segment;
		Z_NamePlusRecord *rec;
		Odr_oct *oct = odr_malloc (out, sizeof(*oct));

		if (!apdu)
		{
		    apdu = zget_APDU(out, Z_APDU_segmentRequest);
		    segment = apdu->u.segmentRequest;
		    segment->segmentRecords = (Z_NamePlusRecord **)
			odr_malloc (out, chunk * sizeof(*segment->segmentRecords));
		}
		rec = (Z_NamePlusRecord *) odr_malloc (out, sizeof(*rec));
		rec->databaseName = 0;
		rec->which = Z_NamePlusRecord_intermediateFragment;
		rec->u.intermediateFragment = (Z_FragmentSyntax *)
		    odr_malloc (out, sizeof(*rec->u.intermediateFragment));
		rec->u.intermediateFragment->which =
		    Z_FragmentSyntax_notExternallyTagged;
		rec->u.intermediateFragment->u.notExternallyTagged = oct;
		
		oct->len = oct->size = status.st_size;
		oct->buf = odr_malloc (out, oct->size);
		fread (oct->buf, 1, oct->size, inf);
		fclose (inf);
		
		segment->segmentRecords[segment->num_segmentRecords++] = rec;

		if (segment->num_segmentRecords == chunk)
		{
		    send_apdu (apdu);
		    apdu = 0;
		}
	    }	
	}
    }
    if (apdu)
	send_apdu(apdu);
    apdu = zget_APDU(out, Z_APDU_segmentRequest);
    send_apdu (apdu);
    closedir(dir);
    return 2;
}

int cmd_adm_import2(char* arg)
{
    /* Size of chunks we wish to read from import file */
    size_t chunk_size = 8192;

    /* Buffer for reading chunks of data from import file */
    char chunk_buffer[chunk_size];
    
    if ( arg )
    {
        char rectype_buff[32];
        char filename_buff[32];
	FILE* pImportFile = NULL;

        if (sscanf (arg, "%s %s", rectype_buff, filename_buff) != 3)
	{
	    printf("Must specify database-name, record-type and filename for import\n");
	    return 0;
	}

        /* Attempt to open the file */

	pImportFile = fopen(filename_buff,"r");

        /* This chunk of code should move into client.c sometime soon for sending files via the update es */
	/* This function will then refer to the standard client.c one for uploading a file using es update */
	if ( pImportFile )
	{
	    int iTotalWritten = 0;

            /* We opened the import file without problems... So no we send the es request, ready to 
	       start sending fragments of the import file as segment messages */
            sendAdminES(Z_ESAdminOriginPartToKeep_import, rectype_buff);

	    while ( ! feof(pImportFile ) )
	    {
	        /* Read buffer_size bytes from the file */
	        size_t num_items = fread((void*)chunk_buffer, 1, sizeof(chunk_buffer),  pImportFile);

		/* Write num_bytes of data to */

		if ( feof(pImportFile ) )
		{
		    /* This is the last chunk... Write it as the last fragment */
		    printf("Last segment of %d bytes\n", num_items);
		}
		else if ( iTotalWritten == 0 )
		{
		    printf("First segment of %d bytes\n",num_items);
		}
		else
		{
		    printf("Writing %d bytes\n", num_items);
		}

		iTotalWritten += num_items;
	    }
	}
	return 2;
    }
    return 0;
}

/* "Freshen" the specified database, by checking metadata records against the sources from which they were 
   generated, and creating a new record if the source has been touched since the last extraction */
int cmd_adm_refresh(char* arg)
{
    if ( arg )
    {
        sendAdminES(Z_ESAdminOriginPartToKeep_refresh, NULL);
	return 2;
    }
    return 0;
}

/* cmd_adm_commit 
   Make imported records a permenant & visible to the live system */
int cmd_adm_commit(char* arg)
{
    sendAdminES(Z_ESAdminOriginPartToKeep_commit, NULL);
    return 2;
}

int cmd_adm_shutdown(char* arg)
{
    sendAdminES(Z_ESAdminOriginPartToKeep_shutdown, NULL);
    return 2;
}

int cmd_adm_startup(char* arg)
{
    sendAdminES(Z_ESAdminOriginPartToKeep_start, NULL);
    return 2;
}
#endif
