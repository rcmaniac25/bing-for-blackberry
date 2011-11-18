/*
 * response.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

void bing_event_get_response(bps_event_t* event, bing_response_t* response)
{
	if(response != NULL)
	{
		bps_event_payload_t* payload = bps_event_get_payload(event);
		response[0] = (bing_response_t)payload->data1;
	}
}

enum SOURCE_TYPE response_get_source_type(bing_response_t response)
{
	enum SOURCE_TYPE t = BING_SOURCETYPE_UNKNOWN;
	bing_response* res;
	if(response)
	{
		res = (bing_response*)response;
		t = res->type;
	}
	return t;
}

int response_get(bing_response_t response, const char* name, void* data)
{
	int ret = -1;
	bing_response* res;
	if(response)
	{
		res = (bing_response*)response;
		ret = hashtable_get_item(res->data, name, data);
	}
	return ret;
}

long long response_get_total(bing_response_t response)
{
	long long ret = -1;
	response_get(response, RESPONSE_TOTAL_STR, &ret);
	return ret;
}

long long response_get_offset(bing_response_t response)
{
	long long ret = -1;
	response_get(response, RESPONSE_OFFSET_STR, &ret);
	return ret;
}

int response_get_query(bing_response_t response, char* buffer)
{
	return response_get(response, RESPONSE_QUERY_STR, buffer);
}

int response_get_altered_query(bing_response_t response, char* buffer)
{
	return response_get(response, RESPONSE_ALTERED_QUERY_STR, buffer);
}

int response_get_alterations_override_query(bing_response_t response, char* buffer)
{
	return response_get(response, RESPONSE_ALTERATIONS_OVER_QUERY_STR, buffer);
}

int response_get_results(bing_response_t response, bing_result_t* results)
{
	int ret = -1;
	bing_response* res;
	if(response && results)
	{
		res = (bing_response*)response;
		ret = res->resultCount;
		memcpy(results, res->results, sizeof(bing_result_t) * ret);
	}
	return ret;
}

//TODO: free_response

//TODO: response_get_ad_api_version

//TODO: response_get_ad_page_number

//TODO: response_get_bundle_responses

//TODO: response_get_mobile_web_spell_total

//TODO: response_get_mobile_web_offset

//TODO: response_get_phonebook_title

//TODO: response_get_phonebook_local_serp_url

//TODO: response_get_news_related_searches

int response_custom_is_field_supported(bing_response_t response, const char* field)
{
	BOOL ret = FALSE;
	bing_response* res;
	if(response && field)
	{
		res = (bing_response*)response;
		ret = hashtable_key_exists(res->data, field) != -1;
	}
	return ret;
}

//TODO: response_custom_get_64bit_int
//TODO: response_custom_get_string
//TODO: response_custom_get_double
//TODO: response_custom_get_boolean
//TODO: response_custom_get_array

//TODO: response_custom_set_64bit_int
//TODO: response_custom_set_string
//TODO: response_custom_set_double
//TODO: response_custom_set_boolean
//TODO: response_custom_set_array

//TODO: response_register_response_creator

//TODO: response_unregister_response_creator
