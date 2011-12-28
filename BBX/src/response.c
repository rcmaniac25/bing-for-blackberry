/*
 * response.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

//Creation/update functions

//TODO

//XXX Are update functions really necessary? Seems like only results have the potential to be updated (prior to investigation).

//Search structure

typedef struct BING_RESPONSE_CREATOR_SEARCH_S
{
	bing_response_creator creator;
	enum SOURCE_TYPE type;
	struct BING_RESPONSE_CREATOR_SEARCH_S* next;
} bing_response_creator_search;

static bing_response_creator_search response_def_creator[]=
{
		//TODO: Replace NULL with dedicated options
		{{"web:Web",			NULL, NULL}, BING_SOURCETYPE_WEB,				&response_def_creator[1]},
		{{"pho:Phonebook",		NULL, NULL}, BING_SOURCETYPE_PHONEBOOK,			&response_def_creator[2]},
		{{"mms:Image",			NULL, NULL}, BING_SOURCETYPE_IMAGE, 			&response_def_creator[3]},
		{{"mms:Video",			NULL, NULL}, BING_SOURCETYPE_VIDEO,				&response_def_creator[4]},
		{{"news:News",			NULL, NULL}, BING_SOURCETYPE_NEWS,				&response_def_creator[5]},
		{{"ads:Ad",				NULL, NULL}, BING_SOURCETYPE_AD, 				&response_def_creator[6]},
		{{"rs:RelatedSearch",	NULL, NULL}, BING_SOURCETYPE_RELATED_SEARCH,	&response_def_creator[7]},
		{{"tra:Translation",	NULL, NULL}, BING_SOURCETYPE_TRANSLATION,		&response_def_creator[8]},
		{{"spl:Spell",			NULL, NULL}, BING_SOURCETYPE_SPELL,				&response_def_creator[9]},
		{{"mw:MobileWeb",		NULL, NULL}, BING_SOURCETYPE_MOBILE_WEB,		&response_def_creator[10]},
		{{"ia:InstantAnswer",	NULL, NULL}, BING_SOURCETYPE_INSTANT_ANWSER,	NULL}
};

//Functions

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

int response_add_result(bing_response* response, bing_result* result)
{
	BOOL ret = FALSE;
	int i;
	bing_result_t* resultList;
	if(response && result)
	{
		for(i = 0; i < response->resultCount; i++)
		{
			if(response->results[i] == result)
			{
				break;
			}
		}
		if(i == response->resultCount) //Result not found, add it
		{
			resultList = (bing_result_t*)realloc(response->results, (response->resultCount + 1) * sizeof(bing_result_t));
			if(resultList)
			{
				response->results = resultList;
				resultList[response->resultCount++] = result;
				ret = TRUE;
			}
		}
	}
	return ret;
}

int response_add_to_bing(bing_response* res)
{
	BOOL ret = FALSE;
	bing_response** t;
	int i;
	bing* bing = retrieveBing(res->bing);

	if(bing)
	{
		//Make sure this is locked and isn't changed on multiple threads
		pthread_mutex_lock(&bing->mutex);

		for(i = 0; i < bing->responseCount; i++)
		{
			if(bing->responses[i] == res)
			{
				break;
			}
		}
		if(i == bing->responseCount) //Response not found, add it
		{
			t = realloc(bing->responses, (bing->responseCount + 1) * sizeof(bing_response*));
			if(t)
			{
				bing->responses = t;
				t[bing->responseCount++] = res;
				ret = TRUE;
			}
		}

		pthread_mutex_unlock(&bing->mutex);
	}
	return ret;
}

void response_remove_from_bing(bing_response* res, BOOL bundle_free)
{
	bing* bing;
	bing_response** t;
	unsigned int pos;

	//Don't run for bundled responses (they are not included within Bing)
	if(!bundle_free)
	{
		//Get the bing response
		bing = retrieveBing(res->bing);
		if(bing)
		{
			//Make sure this is locked and isn't changed on multiple threads
			pthread_mutex_lock(&bing->mutex);

			//Find the component
			for(pos = 0; pos < bing->responseCount; pos++)
			{
				if(bing->responses[pos] == res)
				{
					//Found the response
					if(pos == bing->responseCount - 1)
					{
						//Simply remove the last item
						t = realloc(bing->responses, (bing->responseCount - 1) * sizeof(bing_response*));
						if(t)
						{
							bing->responses = t;
							bing->responseCount--;
						}
					}
					else
					{
						//Create a new array and copy responses into it
						t = malloc((bing->responseCount - 1) * sizeof(bing_response*));
						if(t)
						{
							memcpy(t, bing->responses, pos * sizeof(bing_response*));
							memcpy(t + pos, bing->responses + pos + 1, (bing->responseCount - pos - 1) * sizeof(bing_response*));

							free(bing->responses);
							bing->responses = t;

							bing->responseCount--;
						}
					}

					//Stop execution
					break;
				}
			}

			pthread_mutex_unlock(&bing->mutex);
		}
	}
}

int response_create(enum SOURCE_TYPE type, bing_response_t* response, unsigned int bing, bing_response* responseParent, response_creation_func creation, response_additional_data_func additionalData, int tableSize)
{
	BOOL ret = FALSE;
	bing_response* res;
	list* list_v = NULL;
	bing_response_t* responseList = NULL;
	if(response &&
			(!(responseParent && responseParent->bing == 0) || bing > 0)) //We only want a response to be created if it is destined to be a either a parent or child response, not a child of a child response.
	{
		res = (bing_response*)malloc(sizeof(bing_response));
		if(res)
		{
			//Set variables
			res->type = type;
			res->bing = responseParent ? 0 : bing;

			res->creation = creation;
			res->additionalData = additionalData;

			res->resultCount = 0;
			res->results = NULL;

			res->allocatedMemoryCount = 0;
			res->allocatedMemory = NULL;

			//Create hashtable
			res->data = hashtable_create(tableSize);
			if(res->data)
			{
				if(res->bing != 0)
				{
					//Add response to Bing object
					if(response_add_to_bing(res))
					{
						//Save response
						response[0] = res;
						ret = TRUE;
					}
					else
					{
						//Couldn't save response
						hashtable_free(res->data);
						free(res);
					}
				}
				else
				{
					//Add response to bundle
					if(hashtable_get_item(responseParent->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, &list_v) != -1)
					{
						//Get the list
						responseList = LIST_ELEMENTS(list_v, bing_response_t);
					}
					else
					{
						//Create the list
						list_v = (list*)malloc(sizeof(list));
						if(list_v)
						{
							list_v->count = 0;
							responseList = list_v->listElements = (bing_response_t*)calloc(11, sizeof(bing_response_t));
							if(list_v->listElements)
							{
								//Save the list
								list_v->cap = 11;
								if(hashtable_put_item(responseParent->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, list_v, sizeof(list*)) == -1)
								{
									//List creation failed, cleanup
									free(list_v);
									list_v = NULL;
								}
							}
							else
							{
								//List creation failed, cleanup
								free(list_v);
								list_v = NULL;
							}
						}
					}
					if(list_v)
					{
						if(list_v->count >= list_v->cap)
						{
							//Resize list
							responseList = (bing_response_t*)realloc(responseList, sizeof(bing_response_t) * (list_v->cap * 2));
							if(responseList)
							{
								list_v->cap *= 2;
								list_v->listElements = responseList;
							}
							else
							{
								responseList = NULL;
							}
						}
						if(responseList)
						{
							//Add the response to list
							responseList[list_v->count++] = res;

							//Save response
							response[0] = res;
							ret = TRUE;
						}
						else
						{
							//List couldn't be resized
							hashtable_free(res->data);
							free(res);
						}
					}
					else
					{
						//List couldn't be found/created
						hashtable_free(res->data);
						free(res);
					}
				}
			}
			else
			{
				//Hashtable couldn't be created
				free(res);
			}
		}
	}
	return ret;
}

//TODO: response_create_raw

void free_response_in(bing_response_t response, BOOL bundle_free)
{
	bing_response* res;
	list* list;
	int i;
	if(response)
	{
		res = (bing_response*)response;

		//Remove response from Bing
		response_remove_from_bing(res, bundle_free);

		//Free allocated memory (allocated by response and result)
		for(i = 0; i < res->allocatedMemoryCount; i++)
		{
			free(res->allocatedMemory[i]);
		}
		free(res->allocatedMemory);

		//Free data
		if(res->data)
		{
			if(hashtable_get_item(res->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, &list) != -1)
			{
				for(i = 0; i < list->count; i++)
				{
					free_response_in(LIST_ELEMENT(list, i, bing_response_t), TRUE);
				}
				free((void*)list->listElements);
				free((void*)list);
			}

			hashtable_free(res->data);
		}

		//Free results
		while(res->resultCount > 0)
		{
			free_result((bing_result*)res->results[--res->resultCount]);
		}
		free(res->results);

		free(res);
	}
}

void free_response(bing_response_t response)
{
	free_response_in(response, FALSE);
}

int response_get_string(bing_response_t response, char* buffer, const char* field, enum SOURCE_TYPE type)
{
	int ret = -1;
	bing_response* res;
	if(response)
	{
		res = (bing_response*)response;
		if(res->type == type) //Make sure of the proper type
		{
			ret = hashtable_get_string(res->data, field, buffer);
		}
	}
	return ret;
}

int response_get_ad_api_version(bing_response_t response, char* buffer)
{
	return response_get_string(response, buffer, "AdAPIVersion", BING_SOURCETYPE_AD);
}

long long response_get_int64(bing_response_t response, const char* field, enum SOURCE_TYPE type)
{
	long long ret = -1;
	bing_response* res;
	if(response)
	{
		res = (bing_response*)response;
		if(res->type == type) //Make sure of the proper type
		{
			if(hashtable_get_data_key(res->data, field, &ret, sizeof(long long)) == FALSE)
			{
				//If an error occurs, make sure it returns the error value
				ret = -1;
			}
		}
	}
	return ret;
}

long long response_get_ad_page_number(bing_response_t response)
{
	return response_get_int64(response, "PageNumber", BING_SOURCETYPE_AD);
}

int response_get_bundle_responses(bing_response_t response, bing_response_t* responses)
{
	int ret = -1;
	bing_response* res;
	bing_response_t* responseList;
	list* list_v = NULL;
	if(response)
	{
		res = (bing_response*)response;
		if(res->type == BING_SOURCETYPE_BUNDLE && //Make sure of the proper type
				hashtable_get_item(res->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, &list_v) != -1)
		{
			ret = list_v->count;
			if(responses)
			{
				//Get the list
				responseList = LIST_ELEMENTS(list_v, bing_response_t);

				//Copy the data
				memcpy(responseList, responses, ret * sizeof(bing_response_t));
			}
		}
	}
	return ret;
}

long long response_get_mobile_web_spell_total(bing_response_t response)
{
	return response_get_int64(response, "Total", BING_SOURCETYPE_MOBILE_WEB);
}

long long response_get_mobile_web_offset(bing_response_t response)
{
	return response_get_int64(response, "Offset", BING_SOURCETYPE_MOBILE_WEB);
}

int response_get_phonebook_title(bing_response_t response, char* buffer)
{
	return response_get_string(response, buffer, "Title", BING_SOURCETYPE_PHONEBOOK);
}

int response_get_phonebook_local_serp_url(bing_response_t response, char* buffer)
{
	return response_get_string(response, buffer, "LocalSerpUrl", BING_SOURCETYPE_PHONEBOOK);
}

int response_get_news_related_searches(bing_response_t response, bing_related_search_t searches)
{
	int ret = response_get_string(response, (char*)searches, "RelatedSearches", BING_SOURCETYPE_NEWS);
	if(ret != -1)
	{
		//We need to convert byte count to item count
		ret /= sizeof(bing_related_search_t);
	}
	return ret;
}

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

int response_custom_get_64bit_int(bing_response_t response, const char* field, long long* value)
{
	return hashtable_get_data_key(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(long long));
}

int response_custom_get_string(bing_response_t response, const char* field, char* value)
{
	return hashtable_get_string(response ? ((bing_response*)response)->data : NULL, field, value);
}

int response_custom_get_double(bing_response_t response, const char* field, double* value)
{
	return response_custom_get_64bit_int(response, field, (long long*)value);
}

int response_custom_get_boolean(bing_response_t response, const char* field, int* value)
{
	return hashtable_get_data_key(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(int));
}

int response_custom_get_array(bing_response_t response, const char* field, void* value)
{
	return response_custom_get_string(response, field, (char*)value);
}

int response_custom_set_64bit_int(bing_response_t response, const char* field, long long* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(long long));
}

int response_custom_set_string(bing_response_t response, const char* field, const char* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, value ? (strlen(value) + 1) : 0);
}

int response_custom_set_double(bing_response_t response, const char* field, double* value)
{
	return response_custom_set_64bit_int(response, field, (long long*)value);
}

int response_custom_set_boolean(bing_response_t response, const char* field, int* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(int));
}

int response_custom_set_array(bing_response_t response, const char* field, const void* value, size_t size)
{
	//This could be a safety hazard but we have no way of checking the size of the data passed in
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, size);
}

void* allocateMemory(size_t size, bing_response* response)
{
	void* ret = NULL;
	void** mem = malloc((response->allocatedMemoryCount + 1) * sizeof(void*));
	if(mem)
	{
		ret = malloc(size);
		if(ret)
		{
			//Copy the memory
			memcpy(mem, response->allocatedMemory, response->allocatedMemoryCount * sizeof(void*));

			free(response->allocatedMemory);
			response->allocatedMemory = mem;

			//Set the new memory allocation
			mem[response->allocatedMemoryCount++] = ret;
		}
		else
		{
			free(mem);
		}
	}
	return ret;
}

void freeMemory(void* ptr, bing_response* response)
{
	unsigned int i;
	void** mem;
	//Find the memory
	for(i = 0; i < response->allocatedMemoryCount; i++)
	{
		if(response->allocatedMemory[i] == ptr)
		{
			break;
		}
	}
	if(i != response->allocatedMemoryCount)
	{
		//New allocation
		if(response->allocatedMemoryCount > 1)
		{
			mem = malloc((response->allocatedMemoryCount - 1) * sizeof(void*));
			if(mem)
			{
				//Copy over pointers
				memcpy(mem, response->allocatedMemory, i * sizeof(void*));
				memcpy(mem + i, response->allocatedMemory + (i + 1), (response->allocatedMemoryCount - i - 1) * sizeof(void*));

				//Free old allocated memory pointer and ptr
				free(response->allocatedMemory);
				free(ptr);

				//Set the memory
				response->allocatedMemoryCount--;
				response->allocatedMemory = mem;
			}
		}
		else
		{
			//Free all memory
			free(response->allocatedMemory);
			free(ptr);

			//Reset allocated memory
			response->allocatedMemoryCount = 0;
			response->allocatedMemory = NULL;
		}
	}
}

void* response_custom_allocation(bing_response_t response, size_t size)
{
	void* ret = NULL;
	bing_response* res;
	if(response)
	{
		res = (bing_response*)response;
		if(res->type == BING_SOURCETYPE_CUSTOM && //We only want to allocate memory for custom types
				(size <= (10 * 1024) && size >= 1)) //We want it to be within a small range to prevent nieve misuse.
		{
			ret = allocateMemory(size, res);
		}
	}
	return ret;
}

int response_register_response_creator(const char* name, response_creation_func creation_func, response_additional_data_func additional_func)
{
	BOOL ret = FALSE;
	BOOL cont = TRUE;
	unsigned int i;
	bing_response_creator* c;
	bing_response_creator_search* cr;
	char* nName;
	size_t size;
	if(name && creation_func)
	{
		//Check if the name is a standard supported name, if so return
		for(cr = response_def_creator; cr != NULL; cr = cr->next)
		{
			if(strcmp(name, cr->creator.name) == 0)
			{
				cont = FALSE;
				break;
			}
		}

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
