/*
 * Copyright (c) 1999, Index Data.
 * See the file LICENSE for details.
 *
 * $Log: ill-get.c,v $
 * Revision 1.1  1999-12-16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 */

#include <yaz/ill.h>

ILL_ItemRequest *ill_get_ItemRequest (ODR o)
{
    ILL_ItemRequest *r = (ILL_ItemRequest *)odr_malloc(o, sizeof(*r));
    
    r->protocol_version_num = (int*) odr_malloc(o, sizeof(int));
    *r->protocol_version_num = ILL_Request_version_2;

    r->service_date_time = 0;
    r->requester_id = 0;
    r->responder_id = 0;
    r->transaction_type = 0;
    r->delivery_address = 0;
    r->delivery_service = 0;
    r->billing_address = 0;

    r->num_iLL_service_type = 1;
    r->iLL_service_type = (ILL_Service_Type **)
	odr_malloc (o, sizeof(*r->iLL_service_type));
    *r->iLL_service_type = (ILL_Service_Type *)
	odr_malloc (o, sizeof(**r->iLL_service_type));
    **r->iLL_service_type = ILL_Service_Type_copy_non_returnable;

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
    r->retry_flag = (int *)odr_malloc(o, sizeof(bool_t));
    *r->retry_flag = 0;
    r->forward_flag = (int *)odr_malloc(o, sizeof(bool_t));
    *r->forward_flag = 0;
    r->requester_note = 0;
    r->forward_note = 0;
    r->num_iLL_request_extensions = 0;
    r->iLL_request_extensions = 0;
    return r;
}
