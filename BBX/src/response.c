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
	if(response)
	{
		res = (bing_response*)response;
		ret = res->resultCount;
		if(results)
		{
			memcpy(results, res->results, sizeof(bing_result_t) * ret);
		}
	}
	return ret;
}

//TODO: Create response (don't forget to add to bing.responses)

//TODO: Add result

void free_response(bing_response_t response)
{
	bing_response* res;
	if(response)
	{
		res = (bing_response*)response;

		//TODO: Remove from Bing object

		//TODO: Free allocated memory (allocated by response and result)

		//Free data
		if(res->data)
		{
			//TODO: Free bundled responses

			hashtable_free(res->data);
		}

		//Free results
		while(res->resultCount > 0)
		{
			free_result(&res->results[--res->resultCount], FALSE);
		}
		free(res->results);

		free(res);
	}
}

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

int response_register_response_creator(const char* name, response_creation_func creation_func, response_additional_data_func additional_func)
{
	BOOL ret = FALSE;
	BOOL cont = TRUE;
	unsigned int i;
	bing_response_creator* c;
	char* nName;
	size_t size;
	if(name && creation_func)
	{
		//TODO: check if the name is a standard supported name, if so return

		if(cont)
		{
			pthread_mutex_lock(&bingSystem.mutex);

			//Go through all registered creators and make sure it doesn't already exist
			i = 0;
			while(i < bingSystem.bingResponseCreatorCount && cont)
			{
				if(strcmp(bingSystem.bingResponseCreators[i++].name, name) == 0)
				{
					cont = FALSE;
				}
			}

			if(cont)
			{
				//Reproduce the name
				size = strlen(name) + 1;
				nName = malloc(size);

				if(nName)
				{
					strlcpy(nName, name, size);

					//Create the new version of the name
					c = (bing_response_creator*)realloc(bingSystem.bingResponseCreators, sizeof(bing_response_creator) * (bingSystem.bingResponseCreatorCount + 1));

					if(c)
					{
						bingSystem.bingResponseCreators = c;

						c[bingSystem.bingResponseCreatorCount].name = nName;
						c[bingSystem.bingResponseCreatorCount].creation = creation_func;
						c[bingSystem.bingResponseCreatorCount++].additionalData = additional_func;

						ret = TRUE;
					}
					else
					{
						free(nName);
					}
				}
			}

			pthread_mutex_unlock(&bingSystem.mutex);
		}
	}
	return ret;
}

int response_unregister_response_creator(const char* name)
{
	BOOL ret = FALSE;
	unsigned int i;
	bing_response_creator* c;
	if(name && bingSystem.bingResponseCreatorCount > 0) //We don't want to run if there is nothing to run on
	{
		pthread_mutex_lock(&bingSystem.mutex);

		//Find the response
		i = 0;
		while(i < bingSystem.bingResponseCreatorCount)
		{
			if(strcmp(bingSystem.bingResponseCreators[i].name, name) == 0)
			{
				break;
			}
			i++;
		}

		if(i < bingSystem.bingResponseCreatorCount)
		{
			//We don't want to reallocate because if we fail and the creator was not the last element, then we overwrote it
			c = (bing_response_creator*)malloc(sizeof(bing_response_creator) * (bingSystem.bingResponseCreatorCount - 1));

			if(c)
			{
				//If this is the last response then it's easy, we just free the data
				if(i != bingSystem.bingResponseCreatorCount - 1)
				{
					memmove(bingSystem.bingResponseCreators + i, bingSystem.bingResponseCreators + (i + 1), (bingSystem.bingResponseCreatorCount - i - 1) * sizeof(bing_response_creator));
				}
				memcpy(c, bingSystem.bingResponseCreators, (--bingSystem.bingResponseCreatorCount) * sizeof(bing_response_creator));
				free(bingSystem.bingResponseCreators);
				bingSystem.bingResponseCreators = c;

				ret = TRUE;
			}
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
	return ret;
}
