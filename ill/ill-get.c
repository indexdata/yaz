/*
 * Copyright (c) 1999-2000, Index Data.
 * See the file LICENSE for details.
 *
 * $Log: ill-get.c,v $
 * Revision 1.3  2000-01-31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.2  2000/01/15 09:38:51  adam
 * Implemented ill_get_ILLRequest. Added some type mappings for ILL protocol.
 *
 * Revision 1.1  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 */

#include <yaz/ill.h>

bool_t *ill_get_bool (struct ill_get_ctl *gc, const char *name,
		      const char *sub, int val)
{
    ODR o = gc->odr;
    char element[128];
    const char *v;
    bool_t *r = odr_malloc (o, sizeof(*r));
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }    

    v = (gc->f)(gc->clientData, element);
    if (v)
	val = atoi(v);
    else if (val < 0)
	return 0;
    *r = val;
    return r;
}

int *ill_get_int (struct ill_get_ctl *gc, const char *name,
		  const char *sub, int val)
{
    ODR o = gc->odr;
    char element[128];
    const char *v;
    int *r = odr_malloc (o, sizeof(*r));
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }    
    v = (gc->f)(gc->clientData, element);
    if (v)
	val = atoi(v);
    *r = val;
    return r;
}

int *ill_get_enumerated (struct ill_get_ctl *gc, const char *name,
			 const char *sub, int val)
{
    return ill_get_int(gc, name, sub, val);
}

ILL_String *ill_get_ILL_String (struct ill_get_ctl *gc, const char *name,
				const char *sub)
{
    ILL_String *r = (ILL_String *) odr_malloc (gc->odr, sizeof(*r));
    char element[128];
    const char *v;

    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    v = (gc->f)(gc->clientData, element);
    if (!v)
	return 0;
    r->which = ILL_String_GeneralString;
    r->u.GeneralString = odr_strdup (gc->odr, v);
    return r;
}

ILL_Person_Or_Institution_Symbol *ill_get_Person_Or_Insitution_Symbol (
    struct ill_get_ctl *gc, const char *name, const char *sub)
{
    char element[128];
    ODR o = gc->odr;
    ILL_Person_Or_Institution_Symbol *p = odr_malloc (o, sizeof(*p));
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    p->which = ILL_Person_Or_Institution_Symbol_person_symbol;
    if ((p->u.person_symbol = ill_get_ILL_String (gc, element, "person")))
	return p;

    p->which = ILL_Person_Or_Institution_Symbol_institution_symbol;
    if ((p->u.institution_symbol =
	 ill_get_ILL_String (gc, element, "institution")))
	return p;
    return 0;
}

static ILL_Name_Of_Person_Or_Institution *ill_get_Name_Of_Person_Or_Institution(
    struct ill_get_ctl *gc, const char *name, const char *sub)
{
    char element[128];
    ODR o = gc->odr;
    ILL_Name_Of_Person_Or_Institution *p = odr_malloc (o, sizeof(*p));
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    p->which = ILL_Name_Of_Person_Or_Institution_name_of_person;
    if ((p->u.name_of_person =
	 ill_get_ILL_String (gc, element, "name-of-person")))
	return p;

    p->which = ILL_Name_Of_Person_Or_Institution_name_of_institution;
    if ((p->u.name_of_institution =
	 ill_get_ILL_String (gc, element, "name-of-institution")))
	return p;
    return 0;
}
    
ILL_System_Id *ill_get_System_Id(struct ill_get_ctl *gc,
				 const char *name, const char *sub)
{
    ODR o = gc->odr;
    char element[128];
    ILL_System_Id *p;
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    p = (ILL_System_Id *) odr_malloc (o, sizeof(*p));
    p->person_or_institution_symbol =
	ill_get_Person_Or_Insitution_Symbol (gc, element,
					     "person-or-institution-symbol");
    p->name_of_person_or_institution =
	ill_get_Name_Of_Person_Or_Institution (gc, element,
					       "name-of-person-or-institution");
    return p;
}

ILL_Transaction_Id *ill_get_Transaction_Id (struct ill_get_ctl *gc,
					    const char *name, const char *sub)
{
    ODR o = gc->odr;
    ILL_Transaction_Id *r = (ILL_Transaction_Id *) odr_malloc (o, sizeof(*r));
    char element[128];
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }    
    r->initial_requester_id =
	ill_get_System_Id (gc, element, "initial-requester-id");
    r->transaction_group_qualifier =
	ill_get_ILL_String (gc, element, "transaction-group-qualifier");
    r->transaction_qualifier =
	ill_get_ILL_String (gc, element, "transaction-qualifier");
    r->sub_transaction_qualifier =
	ill_get_ILL_String (gc, element, "sub-transaction-qualifier");
    return r;
}


ILL_Service_Date_this *ill_get_Service_Date_this (
    struct ill_get_ctl *gc, const char *name, const char *sub)
{
    ODR o = gc->odr;
    ILL_Service_Date_this *r =
	(ILL_Service_Date_this *) odr_malloc (o, sizeof(*r));
    char element[128];
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    r->date = odr_strdup (o, "14012000");
    r->time = 0;
    return r;
}

ILL_Service_Date_Time *ill_get_Service_Date_Time (
    struct ill_get_ctl *gc, const char *name, const char *sub)
{
    ODR o = gc->odr;
    ILL_Service_Date_Time *r =
	(ILL_Service_Date_Time *) odr_malloc (o, sizeof(*r));
    char element[128];
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }    
    r->date_time_of_this_service = ill_get_Service_Date_this (gc, element, 
							      "this");
    r->date_time_of_original_service = 0;
    return r;
}

ILL_Requester_Optional_Messages_Type *ill_get_Requester_Optional_Messages_Type (
    struct ill_get_ctl *gc, const char *name, const char *sub)
{
    ODR o = gc->odr;
    ILL_Requester_Optional_Messages_Type *r =
	(ILL_Requester_Optional_Messages_Type *) odr_malloc (o, sizeof(*r));
    char element[128];
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    r->can_send_RECEIVED = ill_get_bool (gc, element, "can-send-RECEIVED", 0);
    r->can_send_RETURNED = ill_get_bool (gc, element, "can-send-RETURNED", 0);
    r->requester_SHIPPED =
	ill_get_enumerated (gc, element, "requester-SHIPPED", 1);
    r->requester_CHECKED_IN =
	ill_get_enumerated (gc, element, "requester-CHECKED-IN", 1);
    return r;
}

ILL_Item_Id *ill_get_Item_Id (
    struct ill_get_ctl *gc, const char *name, const char *sub)   
{
    ODR o = gc->odr;
    ILL_Item_Id *r = (ILL_Item_Id *) odr_malloc (o, sizeof(*r));
    char element[128];
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    r->item_type = ill_get_enumerated (gc, element, "item-type",
				       ILL_Item_Id_monograph);
    r->held_medium_type = 0;
    r->call_number = ill_get_ILL_String(gc, element, "call-number");
    r->author = ill_get_ILL_String(gc, element, "author");
    r->title = ill_get_ILL_String(gc, element, "title");
    r->sub_title = ill_get_ILL_String(gc, element, "sub-title");
    r->sponsoring_body = ill_get_ILL_String(gc, element, "sponsoring-body");
    r->place_of_publication =
	ill_get_ILL_String(gc, element, "place-of-publication");
    r->publisher = ill_get_ILL_String(gc, element, "publisher");
    r->series_title_number =
	ill_get_ILL_String(gc, element, "series-title-number");
    r->volume_issue = ill_get_ILL_String(gc, element, "volume-issue");
    r->edition = ill_get_ILL_String(gc, element, "edition");
    r->publication_date = ill_get_ILL_String(gc, element, "publication-date");
    r->publication_date_of_component =
	ill_get_ILL_String(gc, element, "publication-date-of-component");
    r->author_of_article = ill_get_ILL_String(gc, element,
					      "author-of-article");
    r->title_of_article = ill_get_ILL_String(gc, element, "title-or-article");
    r->pagination = ill_get_ILL_String(gc, element, "pagination");
    r->national_bibliography_no = 0;
    r->iSBN = ill_get_ILL_String(gc, element, "ISBN");
    r->iSSN = ill_get_ILL_String(gc, element, "ISSN");
    r->system_no = 0;
    r->additional_no_letters =
	ill_get_ILL_String(gc, element, "additional-no-letters");
    r->verification_reference_source = 
	ill_get_ILL_String(gc, element, "verification-reference-source");
    return r;
}

ILL_ItemRequest *ill_get_ItemRequest (
    struct ill_get_ctl *gc, const char *name, const char *sub)
{
    ODR o = gc->odr;
    ILL_ItemRequest *r = (ILL_ItemRequest *)odr_malloc(o, sizeof(*r));
    return 0;
}

ILL_Request *ill_get_ILLRequest (
    struct ill_get_ctl *gc, const char *name, const char *sub)
{
    ODR o = gc->odr;
    ILL_Request *r = (ILL_Request *) odr_malloc(o, sizeof(*r));
    char element[128];
    
    strcpy(element, name);
    if (sub)
    {
	strcat (element, ",");
	strcat (element, sub);
    }
    r->protocol_version_num =
	ill_get_enumerated (gc, element, "protocol-version-num", 
			    ILL_Request_version_2);
    
    r->transaction_id = ill_get_Transaction_Id (gc, element, "transaction-id");
    r->service_date_time =
	ill_get_Service_Date_Time (gc, element, "service-date-time");
    r->requester_id = ill_get_System_Id (gc, element, "requester-id");
    r->responder_id = ill_get_System_Id (gc, element, "responder-id");
    r->transaction_type =
	ill_get_enumerated(gc, element, "transaction-type", 1);
    r->delivery_address = 0;     /* TODO */
    r->delivery_service = 0;     /* TODO */
    r->billing_address = 0;      /* TODO */

    r->num_iLL_service_type = 1;
    r->iLL_service_type = (ILL_Service_Type **)
	odr_malloc (o, sizeof(*r->iLL_service_type));
    *r->iLL_service_type =
	ill_get_enumerated (gc, element, "ill-service-type",
			    ILL_Service_Type_copy_non_returnable);

    r->responder_specific_service = 0;
    r->requester_optional_messages =
	ill_get_Requester_Optional_Messages_Type (
	    gc, element,"requester-optional-messages");
    r->search_type = 0;            /* TODO */
    r->num_supply_medium_info_type = 0;
    r->supply_medium_info_type = 0;

    r->place_on_hold =
	ill_get_enumerated (gc, element, "place-on-hold", 
			    ILL_Place_On_Hold_Type_according_to_responder_policy);
    r->client_id = 0;             /* TODO */
    r->item_id = ill_get_Item_Id (gc, element, "item-id");
    r->supplemental_item_description = 0;
    r->cost_info_type = 0;
    r->copyright_compliance =
	ill_get_ILL_String(gc, element, "copyright-complicance");
    r->third_party_info_type = 0;
    r->retry_flag = ill_get_bool (gc, element, "retry-flag", 0);
    r->forward_flag = ill_get_bool (gc, element, "forward-flag", 0);
    r->requester_note = ill_get_ILL_String(gc, element, "requester-note");
    r->forward_note = ill_get_ILL_String(gc, element, "forward-note");
    r->num_iLL_request_extensions = 0;
    r->iLL_request_extensions = 0;
    return r;
}
