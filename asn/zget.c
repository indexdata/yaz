/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: zget.c,v $
 * Revision 1.13  1998-01-29 13:13:39  adam
 * Function zget_presentRequest fills resultSetId with "default" instead
 * of "Default".
 *
 * Revision 1.12  1997/10/29 12:00:37  adam
 * Routine zget_SearchRequest fills resultSetName member with "default"
 * instead of "Default".
 *
 * Revision 1.11  1997/05/02 08:39:10  quinn
 * New PDUs added, thanks to Ronald van der Meer
 *
 * Revision 1.10  1996/01/02 08:57:23  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.9  1995/09/29  17:11:55  quinn
 * Smallish
 *
 * Revision 1.8  1995/09/27  15:02:43  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.7  1995/06/15  07:44:52  quinn
 * Moving to v3.
 *
 * Revision 1.6  1995/06/14  15:26:37  quinn
 * *** empty log message ***
 *
 * Revision 1.5  1995/06/07  14:42:30  quinn
 * Fixed CLOSE
 *
 * Revision 1.4  1995/06/07  14:36:25  quinn
 * Added CLOSE
 *
 * Revision 1.3  1995/06/05  10:52:06  quinn
 * Fixed some negligences.
 *
 * Revision 1.2  1995/05/30  10:15:49  quinn
 * Added our implementor's ID
 *
 * Revision 1.1  1995/05/22  11:30:20  quinn
 * Adding Z39.50-1992 stuff to proto.c. Adding zget.c
 *
 *
 */

#include <proto.h>

Z_InitRequest *zget_InitRequest(ODR o)
{
    Z_InitRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->options = odr_malloc(o, sizeof(*r->options));
    ODR_MASK_ZERO(r->options);
    r->protocolVersion = odr_malloc(o, sizeof(*r->protocolVersion));
    ODR_MASK_ZERO(r->protocolVersion);
    r->preferredMessageSize = odr_malloc(o, sizeof(int));
    *r->preferredMessageSize = 30*1024;
    r->maximumRecordSize = odr_malloc(o, sizeof(int));
    *r->maximumRecordSize = 30*1024;
    r->idAuthentication = 0;
    r->implementationId = "YAZ (id=81)";
    r->implementationName = "Index Data/YAZ";
    r->implementationVersion = YAZ_VERSION;
    r->userInformationField = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_InitResponse *zget_InitResponse(ODR o)
{
    Z_InitResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->options = odr_malloc(o, sizeof(*r->options));
    ODR_MASK_ZERO(r->options);
    r->protocolVersion = odr_malloc(o, sizeof(*r->protocolVersion));
    ODR_MASK_ZERO(r->protocolVersion);
    r->preferredMessageSize = odr_malloc(o, sizeof(int));
    *r->preferredMessageSize = 30*1024;
    r->maximumRecordSize = odr_malloc(o, sizeof(int));
    *r->maximumRecordSize = 30*1024;
    r->result = odr_malloc(o, sizeof(bool_t));
    *r->result = 1;
    r->implementationId = "YAZ (id=81)";
    r->implementationName = "Index Data/YAZ";
    r->implementationVersion = YAZ_VERSION;
    r->userInformationField = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_SearchRequest *zget_SearchRequest(ODR o)
{
    Z_SearchRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->smallSetUpperBound = odr_malloc(o, sizeof(int));
    *r->smallSetUpperBound = 0;
    r->largeSetLowerBound = odr_malloc(o, sizeof(int));
    *r->largeSetLowerBound = 1;
    r->mediumSetPresentNumber = odr_malloc(o, sizeof(int));
    *r->mediumSetPresentNumber = 0;
    r->replaceIndicator = odr_malloc(o, sizeof(bool_t));
    *r->replaceIndicator = 1;
    r->resultSetName = "default";
    r->num_databaseNames = 0;
    r->databaseNames = 0;
    r->smallSetElementSetNames = 0;
    r->mediumSetElementSetNames = 0;
    r->preferredRecordSyntax = 0;
    r->query = 0;
#ifdef Z_95
    r->additionalSearchInfo = 0;
    r->otherInfo = 0;
#endif
    return r;
}

Z_SearchResponse *zget_SearchResponse(ODR o)
{
    Z_SearchResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->resultCount = odr_malloc(o, sizeof(int));
    *r->resultCount = 0;
    r->numberOfRecordsReturned = odr_malloc(o, sizeof(int));
    *r->numberOfRecordsReturned = 0;
    r->nextResultSetPosition = odr_malloc(o, sizeof(int));
    *r->nextResultSetPosition = 0;
    r->searchStatus = odr_malloc(o, sizeof(bool_t));
    *r->searchStatus = 1;
    r->resultSetStatus = 0;
    r->presentStatus = 0;
    r->records = 0;
#ifdef Z_95
    r->additionalSearchInfo = 0;
    r->otherInfo = 0;
#endif
    return r;
}

Z_PresentRequest *zget_PresentRequest(ODR o)
{
    Z_PresentRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->resultSetId = "default";
    r->resultSetStartPoint = odr_malloc(o, sizeof(int));
    *r->resultSetStartPoint = 1;
    r->numberOfRecordsRequested = odr_malloc(o, sizeof(int));
    *r->numberOfRecordsRequested = 10;
#ifdef Z_95
    r->num_ranges = 0;
    r->additionalRanges = 0;
    r->recordComposition = 0;
#else
    r->elementSetNames = 0;
#endif
    r->preferredRecordSyntax = 0;
#ifdef Z_95
    r->maxSegmentCount = 0;
    r->maxRecordSize = 0;
    r->maxSegmentSize = 0;
    r->otherInfo = 0;
#endif
    return r;
}

Z_PresentResponse *zget_PresentResponse(ODR o)
{
    Z_PresentResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->numberOfRecordsReturned = odr_malloc(o, sizeof(int));
    *r->numberOfRecordsReturned = 0;
    r->nextResultSetPosition = odr_malloc(o, sizeof(int));
    *r->nextResultSetPosition = 0;
    r->presentStatus = odr_malloc(o, sizeof(int));
    *r->presentStatus = Z_PRES_SUCCESS;
    r->records = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_DeleteResultSetRequest *zget_DeleteResultSetRequest(ODR o)
{
    Z_DeleteResultSetRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->deleteFunction = odr_malloc(o, sizeof(int));
    *r->deleteFunction = Z_DeleteRequest_list;
    r->num_ids = 0;
    r->resultSetList = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_DeleteResultSetResponse *zget_DeleteResultSetResponse(ODR o)
{
    Z_DeleteResultSetResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->deleteOperationStatus = odr_malloc(o, sizeof(int));
    *r->deleteOperationStatus = Z_DeleteStatus_success;
    r->num_statuses = 0;
    r->deleteListStatuses = 0;
    r->numberNotDeleted = 0;
    r->num_bulkStatuses = 0;
    r->bulkStatuses = 0;
    r->deleteMessage = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_ScanRequest *zget_ScanRequest(ODR o)
{
    Z_ScanRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->num_databaseNames = 0;
    r->databaseNames = 0;
    r->attributeSet = 0;
    r->termListAndStartPoint = 0;
    r->stepSize = 0;
    r->numberOfTermsRequested = odr_malloc(o, sizeof(int));
    *r->numberOfTermsRequested = 20;
    r->preferredPositionInResponse = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_ScanResponse *zget_ScanResponse(ODR o)
{
    Z_ScanResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->stepSize = 0;
    r->scanStatus = odr_malloc(o, sizeof(int));
    *r->scanStatus = Z_Scan_success;
    r->numberOfEntriesReturned = odr_malloc(o, sizeof(int));
    *r->numberOfEntriesReturned = 0;
    r->positionOfTerm =0;
    r->entries = 0;
    r->attributeSet = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_TriggerResourceControlRequest *zget_TriggerResourceControlRequest(ODR o)
{
    Z_TriggerResourceControlRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->requestedAction = odr_malloc(o, sizeof(int));
    *r->requestedAction = Z_TriggerResourceCtrl_resourceReport;
    r->prefResourceReportFormat = 0;
    r->resultSetWanted = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_ResourceControlRequest *zget_ResourceControlRequest(ODR o)
{
    Z_ResourceControlRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->suspendedFlag = 0;
    r->resourceReport = 0;
    r->partialResultsAvailable = 0;
    r->responseRequired = odr_malloc(o, sizeof(bool_t));
    *r->responseRequired = 0;
    r->triggeredRequestFlag = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_ResourceControlResponse *zget_ResourceControlResponse(ODR o)
{
    Z_ResourceControlResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->continueFlag = odr_malloc(o, sizeof(bool_t));
    *r->continueFlag = 1;
    r->resultSetWanted = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_AccessControlRequest *zget_AccessControlRequest(ODR o)
{
    Z_AccessControlRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->which = Z_AccessRequest_simpleForm;
    r->u.simpleForm = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_AccessControlResponse *zget_AccessControlResponse(ODR o)
{
    Z_AccessControlResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->which = Z_AccessResponse_simpleForm;
    r->u.simpleForm = 0;
    r->diagnostic = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_Segment *zget_Segment(ODR o)
{
    Z_Segment *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->numberOfRecordsReturned = odr_malloc(o, sizeof(int));
    *r->numberOfRecordsReturned = 0;
    r->num_segmentRecords = 0;
    r->segmentRecords = 0;
    r->otherInfo = 0;
    return r;
}

Z_Close *zget_Close(ODR o)
{
    Z_Close *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->closeReason = odr_malloc(o, sizeof(int));
    *r->closeReason = Z_Close_finished;
    r->diagnosticInformation = 0;
    r->resourceReportFormat = 0;
    r->resourceReport = 0;
#ifdef Z_95
    r->otherInfo = 0;
#endif
    return r;
}

Z_ResourceReportRequest *zget_ResourceReportRequest(ODR o)
{
    Z_ResourceReportRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->opId = 0;
    r->prefResourceReportFormat = 0;
    r->otherInfo = 0;
    return r;
}

Z_ResourceReportResponse *zget_ResourceReportResponse(ODR o)
{
    Z_ResourceReportResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->resourceReportStatus = odr_malloc(o, sizeof(int));
    *r->resourceReportStatus = Z_ResourceReportStatus_success;
    r->resourceReport = 0;
    r->otherInfo = 0;
    return r;
}

Z_SortRequest *zget_SortRequest(ODR o)
{
    Z_SortRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->inputResultSetNames = 0;
    r->sortedResultSetName = 0;
    r->sortSequence = 0;
    r->otherInfo = 0;
    return r;
}

Z_SortResponse *zget_SortResponse(ODR o)
{
    Z_SortResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->sortStatus = odr_malloc(o, sizeof(int));
    *r->sortStatus = Z_SortStatus_success;
    r->resultSetStatus = odr_malloc(o, sizeof(int));
    *r->resultSetStatus = Z_SortResultSetStatus_empty;
    r->diagnostics = 0;
    r->otherInfo = 0;
    return r;
}

Z_ExtendedServicesRequest *zget_ExtendedServicesRequest(ODR o)
{
    Z_ExtendedServicesRequest *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->function = odr_malloc(o, sizeof(int));
    *r->function = Z_ExtendedServicesRequest_create;
    r->packageType = 0;
    r->packageName = 0;
    r->userId = 0;
    r->retentionTime = 0;
    r->permissions = 0;
    r->description = 0;
    r->taskSpecificParameters = 0;
    r->waitAction = odr_malloc(o, sizeof(int));
    *r->waitAction = Z_ExtendedServicesRequest_wait;
    r->elements = 0;
    r->otherInfo = 0;
    return r;
}

Z_ExtendedServicesResponse *zget_ExtendedServicesResponse(ODR o)
{
    Z_ExtendedServicesResponse *r = odr_malloc(o, sizeof(*r));

    r->referenceId = 0;
    r->operationStatus = odr_malloc(o, sizeof(int));
    *r->operationStatus = Z_ExtendedServicesResponse_done;
    r->num_diagnostics = 0;
    r->diagnostics = 0;
    r->taskPackage = 0;
    r->otherInfo = 0;
    return r;
}

Z_APDU *zget_APDU(ODR o, int which)
{
    Z_APDU *r = odr_malloc(o, sizeof(*r));

    switch (r->which = which)
    {
    	case Z_APDU_initRequest:
	    r->u.initRequest = zget_InitRequest(o);
            break;
	case Z_APDU_initResponse:
	    r->u.initResponse = zget_InitResponse(o);
            break;
	case Z_APDU_searchRequest:
	    r->u.searchRequest = zget_SearchRequest(o);
            break;
	case Z_APDU_searchResponse:
	    r->u.searchResponse = zget_SearchResponse(o);
            break;
	case Z_APDU_presentRequest:
	    r->u.presentRequest = zget_PresentRequest(o);
            break;
	case Z_APDU_presentResponse:
	    r->u.presentResponse = zget_PresentResponse(o);
            break;
	case Z_APDU_deleteResultSetRequest:
	    r->u.deleteResultSetRequest = zget_DeleteResultSetRequest(o);
            break;
	case Z_APDU_deleteResultSetResponse:
	    r->u.deleteResultSetResponse = zget_DeleteResultSetResponse(o);
	    break;
	case Z_APDU_scanRequest:
	    r->u.scanRequest = zget_ScanRequest(o);
            break;
	case Z_APDU_scanResponse:
	    r->u.scanResponse = zget_ScanResponse(o);
            break;
	case Z_APDU_triggerResourceControlRequest:
	    r->u.triggerResourceControlRequest =
                zget_TriggerResourceControlRequest(o);
            break;
	case Z_APDU_resourceControlRequest:
	    r->u.resourceControlRequest = zget_ResourceControlRequest(o);
	    break;
	case Z_APDU_resourceControlResponse:
	    r->u.resourceControlResponse = zget_ResourceControlResponse(o);
	    break;
	case Z_APDU_segmentRequest:
	    r->u.segmentRequest = zget_Segment(o);
	    break;
	case Z_APDU_close:
	    r->u.close = zget_Close(o);
	    break;
	case Z_APDU_accessControlRequest:
	    r->u.accessControlRequest = zget_AccessControlRequest(o);
	    break;
	case Z_APDU_accessControlResponse:
	    r->u.accessControlResponse = zget_AccessControlResponse(o);
	    break;
	case Z_APDU_resourceReportRequest:
	    r->u.resourceReportRequest = zget_ResourceReportRequest(o);
	    break;
	case Z_APDU_resourceReportResponse:
	    r->u.resourceReportResponse = zget_ResourceReportResponse(o);
	    break;
	case Z_APDU_sortRequest:
	    r->u.sortRequest = zget_SortRequest(o);
	    break;
	case Z_APDU_sortResponse:
	    r->u.sortResponse = zget_SortResponse(o);
	    break;
	case Z_APDU_extendedServicesRequest:
	    r->u.extendedServicesRequest = zget_ExtendedServicesRequest(o);
	    break;
	case Z_APDU_extendedServicesResponse:
	    r->u.extendedServicesResponse = zget_ExtendedServicesResponse(o);
	    break;
	default:
	    fprintf(stderr, "Bad APDU-type to zget_APDU");
	    exit(1);
    }
    return r;
}
