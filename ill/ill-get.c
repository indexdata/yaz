/*
 * Copyright (c) 1999-2000, Index Data.
 * See the file LICENSE for details.
 *
 * $Log: ill-get.c,v $
 * Revision 1.2  2000-01-15 09:38:51  adam
 * Implemented ill_get_ILLRequest. Added some type mappings for ILL protocol.
 *
 * Revision 1.1  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 */

#include <yaz/ill.h>

ILL_String *ill_get_ILL_String (ODR o, const char *str)
{
    ILL_String *r = (ILL_String *) odr_malloc (o, sizeof(*r));

    r->which = ILL_String_GeneralString;
    r->u.GeneralString = odr_strdup (o, str);
    return r;
}

ILL_Transaction_Id *ill_get_Transaction_Id (ODR o)
{
    ILL_Transaction_Id *r = (ILL_Transaction_Id *) odr_malloc (o, sizeof(*r));
    
    r->initial_requester_id = 0;
    r->transaction_group_qualifier = ill_get_ILL_String (o, "group");
    r->transaction_qualifier = ill_get_ILL_String (o, "qual");
    r->sub_transaction_qualifier = 0;
    return r;
}


ILL_Service_Date_this *ill_get_Service_Date_this (ODR o)
{
    ILL_Service_Date_this *r =
	(ILL_Service_Date_this *) odr_malloc (o, sizeof(*r));
    r->date = odr_strdup (o, "14012000");
    r->time = 0;
    return r;
}

ILL_Service_Date_Time *ill_get_Service_Date_Time (ODR o)
{
    ILL_Service_Date_Time *r =
	(ILL_Service_Date_Time *) odr_malloc (o, sizeof(*r));
    r->date_time_of_this_service = ill_get_Service_Date_this (o);
    r->date_time_of_original_service = 0;
    return r;
}

ILL_Transaction_Type *ill_get_Transaction_Type (ODR o)
{
    ILL_Transaction_Type *r =
	(ILL_Transaction_Type *) odr_malloc (o, sizeof(*r));
    *r = 1;
    return r;
}


bool_t *ill_get_bool (ODR o, int val)
{
    bool_t *r = odr_malloc (o, sizeof(*r));
    *r = val;
    return r;
}

int *ill_get_enumerated (ODR o, int val)
{
    int *r = odr_malloc (o, sizeof(*r));
    *r = val;
    return r;
}

int *ill_get_int (ODR o, int val)
{
    return ill_get_enumerated (o, val);
}


ILL_Requester_Optional_Messages_Type *ill_get_Requester_Optional_Messages_Type (ODR o)
{
    ILL_Requester_Optional_Messages_Type *r =
	(ILL_Requester_Optional_Messages_Type *) odr_malloc (o, sizeof(*r));
    r->can_send_RECEIVED = ill_get_bool (o, 0);
    r->can_send_RETURNED = ill_get_bool (o, 0);
    r->requester_SHIPPED =
	ill_get_enumerated (o, ILL_Requester_Optional_Messages_Type_requires);
    r->requester_CHECKED_IN =
	ill_get_enumerated (o, ILL_Requester_Optional_Messages_Type_requires);
    return r;
}

ILL_Item_Id *ill_get_Item_Id (ODR o)
{
    ILL_Item_Id *r = (ILL_Item_Id *) odr_malloc (o, sizeof(*r));

    r->item_type = ill_get_enumerated (o, ILL_Item_Id_monograph);
    r->held_medium_type = 0;
    r->call_number = 0;
    r->author = 0;
    r->title = 0;
    r->sub_title = 0;
    r->sponsoring_body = 0;
    r->place_of_publication = 0;
    r->publisher = 0;
    r->series_title_number = 0;
    r->volume_issue = 0;
    r->edition = 0;
    r->publication_date = 0;
    r->publication_date_of_component = 0;
    r->author_of_article = 0;
    r->title_of_article = 0;
    r->pagination = 0;
    r->national_bibliography_no = 0;
    r->iSBN = 0;
    r->iSSN = 0;
    r->system_no = 0;
    r->additional_no_letters = 0;
    r->verification_reference_source = 0;
    return r;
}

ILL_ItemRequest *ill_get_ItemRequest (ODR o)
{
    ILL_ItemRequest *r = (ILL_ItemRequest *)odr_malloc(o, sizeof(*r));
    
    r->protocol_version_num = ill_get_enumerated (o, ILL_Request_version_2);

    r->transaction_id = 0;
    r->service_date_time = 0;
    r->requester_id = 0;
    r->responder_id = 0;
    r->transaction_type = ill_get_Transaction_Type (o);
    r->delivery_address = 0;
    r->delivery_service = 0;
    r->billing_address = 0;

    r->num_iLL_service_type = 1;
    r->iLL_service_type = (ILL_Service_Type **)
	odr_malloc (o, sizeof(*r->iLL_service_type));
    *r->iLL_service_type =
	ill_get_enumerated (o, ILL_Service_Type_copy_non_returnable);

    r->responder_specific_service = 0;
    r->requester_optional_messages = 0;
    r->search_type = 0;
    r->num_supply_medium_info_type = 0;
    r->supply_medium_info_type = 0;

    r->place_on_hold = (int*) odr_malloc(o, sizeof(int));
    *r->place_on_hold = ILL_Place_On_Hold_Type_according_to_responder_policy;

    r->client_id = 0;
    r->item_id = 0;
    r->supplemental_item_description = 0;
    r->cost_info_type = 0;
    r->copyright_compliance = 0;
    r->third_party_info_type = 0;
    r->retry_flag = ill_get_bool (o, 0);
    r->forward_flag = ill_get_bool (o, 0);
    r->requester_note = 0;
    r->forward_note = 0;
    r->num_iLL_request_extensions = 0;
    r->iLL_request_extensions = 0;
    return r;
}

ILL_Request *ill_get_ILLRequest (ODR o)
{
    ILL_Request *r = (ILL_Request *) odr_malloc(o, sizeof(*r));

    r->protocol_version_num = ill_get_enumerated (o, ILL_Request_version_2);

    r->transaction_id = ill_get_Transaction_Id (o);

    r->service_date_time = ill_get_Service_Date_Time (o);
    r->requester_id = 0;
    r->responder_id = 0;
    r->transaction_type = ill_get_Transaction_Type(o);
    r->delivery_address = 0;
    r->delivery_service = 0;
    r->billing_address = 0;

    r->num_iLL_service_type = 1;
    r->iLL_service_type = (ILL_Service_Type **)
	odr_malloc (o, sizeof(*r->iLL_service_type));
    *r->iLL_service_type =
	ill_get_enumerated (o, ILL_Service_Type_copy_non_returnable);

    r->responder_specific_service = 0;
    r->requester_optional_messages =
	ill_get_Requester_Optional_Messages_Type (o);;
    r->search_type = 0;
    r->num_supply_medium_info_type = 0;
    r->supply_medium_info_type = 0;

    r->place_on_hold = (int*) odr_malloc(o, sizeof(int));
    *r->place_on_hold = ILL_Place_On_Hold_Type_according_to_responder_policy;

    r->client_id = 0;
    r->item_id = ill_get_Item_Id (o);
    r->supplemental_item_description = 0;
    r->cost_info_type = 0;
    r->copyright_compliance = 0;
    r->third_party_info_type = 0;
    r->retry_flag = ill_get_bool (o, 0);
    r->forward_flag = ill_get_bool (o, 0);
    r->requester_note = 0;
    r->forward_note = 0;
    r->num_iLL_request_extensions = 0;
    r->iLL_request_extensions = 0;
    return r;
}
