/*
 * response.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#define RESPONSE_AD_API_VERSION "AdAPIVersion"
#define RESPONSE_AD_PAGE_NUMBER "PageNumber"

#define RESPONSE_PHONEBOOK_TITLE "Title"
#define RESPONSE_PHONEBOOK_LOCAL_SERP_URL "LocalSerpUrl"

#define RESPONSE_NEWS_RELATEDSEARCHES "RelatedSearches"

//Taken from result.c
#define RES_COM_RELSEARCH_ARRAY_NAME "RelatedSearchArray"
#define RES_COM_WEB_RELATEDSEARCH "RelatedSearch"

//Creation/update functions

BOOL response_def_create_standard_responses(bing_response_t response, data_dictionary_t dictionary)
{
	BOOL ret = TRUE;
	int size;
	void* data;
	long long ll;
	bing_response* res = (bing_response*)response;
	if(dictionary)
	{
		size = hashtable_get_string((hashtable_t*)dictionary, "Total", NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, "Total", (char*)data);

				ll = atoll((char*)data);

				hashtable_set_data(res->data, RESPONSE_TOTAL_STR, &ll, sizeof(long long));

				bing_mem_free(data);
			}
		}

		size = hashtable_get_string((hashtable_t*)dictionary, "Offset", NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, "Offset", (char*)data);

				ll = atoll((char*)data);

				hashtable_set_data(res->data, RESPONSE_OFFSET_STR, &ll, sizeof(long long));

				bing_mem_free(data);
			}
		}
	}
	return ret;
}

int response_def_create(const char* name, bing_response_t response, data_dictionary_t dictionary)
{
	//Nothing to do, no values are used by default
	return TRUE;
}

void response_def_additional_data(bing_response_t response, data_dictionary_t dictionary)
{
	//Don't do anything. The dictionary is automatically freed.
}

void response_phonebook_additional_data(bing_response_t response, data_dictionary_t dictionary)
{
	int size;
	void* data;
	bing_response* res = (bing_response*)response;
	if(dictionary)
	{
		//Get the data, then save the data

		size = hashtable_get_string((hashtable_t*)dictionary, RESPONSE_PHONEBOOK_TITLE, NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, RESPONSE_PHONEBOOK_TITLE, (char*)data);

				hashtable_set_data(res->data, RESPONSE_PHONEBOOK_TITLE, data, strlen((char*)data) + 1);

				bing_mem_free(data);
			}
		}

		size = hashtable_get_string((hashtable_t*)dictionary, RESPONSE_PHONEBOOK_LOCAL_SERP_URL, NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, RESPONSE_PHONEBOOK_LOCAL_SERP_URL, (char*)data);

				hashtable_set_data(res->data, RESPONSE_PHONEBOOK_LOCAL_SERP_URL, data, strlen((char*)data) + 1);

				bing_mem_free(data);
			}
		}
	}
}

void response_news_additional_data(bing_response_t response, data_dictionary_t dictionary)
{
	int size;
	char* str;
	bing_result* result;
	bing_response* res = (bing_response*)response;
	if(dictionary)
	{
		size = hashtable_get_string((hashtable_t*)dictionary, RESPONSE_NEWS_RELATEDSEARCHES, NULL);
		if(size > 0)
		{
			str = bing_mem_malloc(size);
			if(str)
			{
				//We need to get the result
				hashtable_get_string((hashtable_t*)dictionary, RESPONSE_NEWS_RELATEDSEARCHES, str);
				result = (bing_result*)str;

				bing_mem_free(str);

				//Make sure this is a valid result
				if(result->type == BING_RESULT_COMMON)
				{
					size = hashtable_get_string(result->data, BING_RESULT_COMMON_TYPE, NULL);
					if(size > 0) //Does it contain a common type?
					{
						str = bing_mem_malloc(size);
						if(str)
						{
							hashtable_get_string(result->data, BING_RESULT_COMMON_TYPE, str);
							if(strcmp(str, RES_COM_RELSEARCH_ARRAY_NAME) == 0) //Is it the correct common type?
							{
								bing_mem_free(str);

								//It's a valid result, time to get the array
								size = hashtable_get_string(result->data, RES_COM_WEB_RELATEDSEARCH, NULL);
								if(size > 0)
								{
									str = bing_mem_malloc(size);
									if(str)
									{
										hashtable_get_string(result->data, RES_COM_WEB_RELATEDSEARCH, str);

										//Save the array
										hashtable_set_data(res->data, RESPONSE_NEWS_RELATEDSEARCHES, str, size);

										bing_mem_free(str);
									}
								}
							}
							else
							{
								bing_mem_free(str);
							}
						}
					}
				}
			}
		}
	}
}

void response_ad_additional_data(bing_response_t response, data_dictionary_t dictionary)
{
	int size;
	void* data;
	long long ll;
	bing_response* res = (bing_response*)response;
	if(dictionary)
	{
		//Get the data, then save the data

		size = hashtable_get_string((hashtable_t*)dictionary, RESPONSE_AD_API_VERSION, NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, RESPONSE_AD_API_VERSION, (char*)data);

				hashtable_set_data(res->data, RESPONSE_AD_API_VERSION, data, strlen((char*)data) + 1);

				bing_mem_free(data);
			}
		}

		size = hashtable_get_string((hashtable_t*)dictionary, RESPONSE_AD_PAGE_NUMBER, NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, RESPONSE_AD_PAGE_NUMBER, (char*)data);

				ll = atoll((char*)data);

				hashtable_set_data(res->data, RESPONSE_AD_PAGE_NUMBER, &ll, sizeof(long long));

				bing_mem_free(data);
			}
		}
	}
}

//Search structure

typedef struct BING_RESPONSE_CREATOR_SEARCH_S
{
	bing_response_creator creator;
	enum BING_SOURCE_TYPE type;
	int tableCount;
	struct BING_RESPONSE_CREATOR_SEARCH_S* next;
} bing_response_creator_search;

#define RESPONSE_DEF_COUNT 5

static bing_response_creator_search response_def_creator[]=
{
		{{"web:Web",			response_def_create,	response_def_additional_data},			BING_SOURCETYPE_WEB,			RESPONSE_DEF_COUNT,		&response_def_creator[1]},
		{{"pho:Phonebook",		response_def_create,	response_phonebook_additional_data},	BING_SOURCETYPE_PHONEBOOK,		RESPONSE_DEF_COUNT + 2,	&response_def_creator[2]},
		{{"mms:Image",			response_def_create,	response_def_additional_data},			BING_SOURCETYPE_IMAGE, 			RESPONSE_DEF_COUNT,		&response_def_creator[3]},
		{{"mms:Video",			response_def_create,	response_def_additional_data},			BING_SOURCETYPE_VIDEO,			RESPONSE_DEF_COUNT,		&response_def_creator[4]},
		{{"news:News",			response_def_create,	response_news_additional_data},			BING_SOURCETYPE_NEWS,			RESPONSE_DEF_COUNT + 1,	&response_def_creator[5]},
		{{"ads:Ad",				response_def_create,	response_ad_additional_data},			BING_SOURCETYPE_AD, 			RESPONSE_DEF_COUNT + 2,	&response_def_creator[6]},
		{{"rs:RelatedSearch",	response_def_create,	response_def_additional_data},			BING_SOURCETYPE_RELATED_SEARCH,	RESPONSE_DEF_COUNT,		&response_def_creator[7]},
		{{"tra:Translation",	response_def_create,	response_def_additional_data},			BING_SOURCETYPE_TRANSLATION,	RESPONSE_DEF_COUNT,		&response_def_creator[8]},
		{{"spl:Spell",			response_def_create,	response_def_additional_data},			BING_SOURCETYPE_SPELL,			RESPONSE_DEF_COUNT,		&response_def_creator[9]},
		{{"mw:MobileWeb",		response_def_create,	response_def_additional_data},			BING_SOURCETYPE_MOBILE_WEB,		RESPONSE_DEF_COUNT,		&response_def_creator[10]},
		{{"ia:InstantAnswer",	response_def_create,	response_def_additional_data},			BING_SOURCETYPE_INSTANT_ANWSER,	RESPONSE_DEF_COUNT,		&response_def_creator[11]},
		{{"bundle",				response_def_create,	response_def_additional_data},			BING_SOURCETYPE_BUNDLE,			RESPONSE_DEF_COUNT + 1,	&response_def_creator[12]},
		{{"Errors",				response_def_create,	response_def_additional_data},			BING_RESULT_ERROR,				RESPONSE_DEF_COUNT,		NULL}
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

enum BING_SOURCE_TYPE bing_response_get_source_type(bing_response_t response)
{
	enum BING_SOURCE_TYPE t = BING_SOURCETYPE_UNKNOWN;
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
		ret = (int)hashtable_get_item(res->data, name, data);
		if(ret == 0)
		{
			//If there size == 0, there is nothing valid to get (even if there is, there is no substance to it. It simply exists)
			ret = -1;
		}
	}
	return ret;
}

long long bing_response_get_total(bing_response_t response)
{
	long long ret = -1;
	response_get(response, RESPONSE_TOTAL_STR, &ret);
	return ret;
}

long long bing_response_get_offset(bing_response_t response)
{
	long long ret = -1;
	response_get(response, RESPONSE_OFFSET_STR, &ret);
	return ret;
}

int bing_response_get_query(bing_response_t response, char* buffer)
{
	return response_get(response, RESPONSE_QUERY_STR, buffer);
}

int bing_response_get_altered_query(bing_response_t response, char* buffer)
{
	return response_get(response, RESPONSE_ALTERED_QUERY_STR, buffer);
}

int bing_response_get_alterations_override_query(bing_response_t response, char* buffer)
{
	return response_get(response, RESPONSE_ALTERATIONS_OVER_QUERY_STR, buffer);
}

int bing_response_get_results(bing_response_t response, bing_result_t* results)
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

BOOL response_add_result(bing_response* response, bing_result* result, BOOL internal)
{
	BOOL ret = FALSE;
	unsigned int i;
	unsigned int* c;
	bing_result_t* resultList;
	bing_result_t** resultListSrc;
	if(response && result)
	{
		//Get array data
		if(internal)
		{
			c = &response->internalResultCount;
			resultListSrc = &response->internalResults;
		}
		else
		{
			c = &response->resultCount;
			resultListSrc = &response->results;
		}
		resultList = *resultListSrc;

		//See if the result already exists
		for(i = 0; i < (*c); i++)
		{
			if(resultList[i] == result)
			{
				break;
			}
		}

		if(i == (*c)) //Result not found, add it
		{
			resultList = (bing_result_t*)bing_mem_realloc(resultList, ((*c) + 1) * sizeof(bing_result_t));
			if(resultList)
			{
				*resultListSrc = resultList;
				resultList[(*c)++] = result;
				ret = TRUE;
			}
		}
	}
	return ret;
}

BOOL response_remove_result(bing_response* response, bing_result* result, BOOL internal, BOOL freeResult)
{
	BOOL ret = FALSE;
	unsigned int i;
	unsigned int* c;
	bing_result_t* resultList;
	bing_result_t** resultListSrc;
	if(response && result)
	{
		//Get array data
		if(internal)
		{
			c = &response->internalResultCount;
			resultListSrc = &response->internalResults;
		}
		else
		{
			c = &response->resultCount;
			resultListSrc = &response->results;
		}
		resultList = *resultListSrc;

		//Find result
		for(i = 0; i < (*c); i++)
		{
			if(resultList[i] == result)
			{
				break;
			}
		}

		if(i < (*c)) //Result found, remove it
		{
			if((i + 1) == (*c)) //At end of array, we can simply remove it
			{
				if((*c) == 1) //If there is only one item, simply free the array
				{
					bing_mem_free(resultList);
					*resultListSrc = NULL;
					(*c) = 0;
					ret = TRUE;
				}
				else
				{
					resultList = (bing_result_t*)bing_mem_realloc(resultList, ((*c) - 1) * sizeof(bing_result_t));
					if(resultList)
					{
						*resultListSrc = resultList;
						(*c)--;
						ret = TRUE;
					}
				}
			}
			else
			{
				resultList = (bing_result_t*)bing_mem_calloc(((*c) - 1), sizeof(bing_result_t));
				if(resultList) //Need to recreate array in order to resize it
				{
					memcpy(resultList, *resultListSrc, i * sizeof(bing_result_t));
					memcpy(resultList + i, (*resultListSrc) + i + 1, ((*c) - i - 1) * sizeof(bing_result_t));
					bing_mem_free(*resultListSrc);
					*resultListSrc = resultList;
					(*c)--;
					ret = TRUE;
				}
			}
			if(ret && freeResult)
			{
				free_result(result);
			}
		}
	}
	return ret;
}

BOOL response_swap_result(bing_response* response, bing_result* result, BOOL internal)
{
	BOOL ret = response_remove_result(response, result, internal, FALSE);
	if(ret)
	{
		ret = response_add_result(response, result, !internal);
		if(!ret)
		{
			//Error, re-add to original array
			if(!response_add_result(response, result, internal))
			{
				//Error, free the result as we can't get it added to anything
				free_result(result);
			}
		}
	}
	return ret;
}

int response_add_to_bing(bing_response* res, unsigned int bingID)
{
	BOOL ret = FALSE;
	bing_response** t;
	unsigned int i;
	bing* bing = retrieveBing(res->bing == 0 ? bingID : res->bing);

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
			t = bing_mem_realloc(bing->responses, (bing->responseCount + 1) * sizeof(bing_response*));
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

BOOL response_remove_from_bing(bing_response* res, BOOL bundle_free)
{
	BOOL ret = FALSE;
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
						if(bing->responseCount == 1)
						{
							//Only response in the list
							bing_mem_free(bing->responses);
							bing->responses = NULL;
							bing->responseCount = 0;
							ret = TRUE;
						}
						else
						{
							//Simply remove the last item
							t = bing_mem_realloc(bing->responses, (bing->responseCount - 1) * sizeof(bing_response*));
							if(t)
							{
								bing->responses = t;
								bing->responseCount--;
								ret = TRUE;
							}
						}
					}
					else
					{
						//Create a new array and copy responses into it
						t = bing_mem_malloc((bing->responseCount - 1) * sizeof(bing_response*));
						if(t)
						{
							memcpy(t, bing->responses, pos * sizeof(bing_response*));
							memcpy(t + pos, bing->responses + pos + 1, (bing->responseCount - pos - 1) * sizeof(bing_response*));

							bing_mem_free(bing->responses);
							bing->responses = t;

							bing->responseCount--;
							ret = TRUE;
						}
					}

					if(ret)
					{
						//Reset bing value
						res->bing = 0;
					}

					//Stop execution
					break;
				}
			}

			pthread_mutex_unlock(&bing->mutex);
		}
	}
	return ret;
}

BOOL response_add_to_bundle(bing_response* response, bing_response* responseParent)
{
	BOOL ret = FALSE;
	list* list_v = NULL;
	bing_response_t* responseList = NULL;

	//Add response to bundle
	if(hashtable_get_item(responseParent->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, &list_v) > 0)
	{
		//Get the list
		responseList = LIST_ELEMENTS(list_v, bing_response_t);
	}
	else
	{
		//Create the list
		list_v = (list*)bing_mem_malloc(sizeof(list));
		if(list_v)
		{
			list_v->count = 0;
			responseList = list_v->listElements = (bing_response_t*)bing_mem_calloc(11, sizeof(bing_response_t));
			if(list_v->listElements)
			{
				//Save the list
				list_v->cap = 11;
				if(hashtable_put_item(responseParent->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, list_v, sizeof(list*)) == -1)
				{
					//List creation failed, cleanup
					bing_mem_free(list_v);
					list_v = NULL;
				}
			}
			else
			{
				//List creation failed, cleanup
				bing_mem_free(list_v);
				list_v = NULL;
			}
		}
	}
	if(list_v)
	{
		if(list_v->count >= list_v->cap)
		{
			//Resize list
			responseList = (bing_response_t*)bing_mem_realloc(responseList, sizeof(bing_response_t) * (list_v->cap * 2));
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
			responseList[list_v->count++] = response;
			response->bing = 0;
			ret = TRUE;
		}
	}
	return ret;
}

BOOL response_remove_from_bundle(bing_response* response, bing_response* responseParent)
{
	BOOL ret = FALSE;
	list* list_v = NULL;
	bing_response_t* responseList = NULL;
	unsigned int i;

	//Add response to bundle
	if(hashtable_get_item(responseParent->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, &list_v) > 0)
	{
		//Get the list
		responseList = LIST_ELEMENTS(list_v, bing_response_t);

		//Find the response
		for(i = 0; i < list_v->count; i++)
		{
			if(responseList[i] == response)
			{
				//Found, move the results to cover it up
				if(list_v->count > 1)
				{
					memmove(responseList + i, responseList + i + 1, (list_v->count - i - 1) * sizeof(bing_response_t));
					LIST_ELEMENT(list_v, --list_v->count, bing_response_t) = NULL;
				}
				else
				{
					//There are no more elements, we can remove the list
					bing_mem_free(list_v->listElements);
					bing_mem_free(list_v);
					hashtable_remove_item(responseParent->data, RESPONSE_BUNDLE_SUBBUNDLES_STR);
				}
				ret = TRUE;
				break;
			}
		}
	}
	return ret;
}

BOOL response_create(enum BING_SOURCE_TYPE type, bing_response_t* response, unsigned int bing, bing_response* responseParent, response_creation_func creation, response_additional_data_func additionalData, int tableSize)
{
	BOOL ret = FALSE;
	bing_response* res;
	list* list_v = NULL;
	bing_response_t* responseList = NULL;
	if(response &&
			(!(responseParent && responseParent->bing == 0) || bing > 0)) //We only want a response to be created if it is destined to be a either a parent or child response, not a child of a child response.
	{
		res = (bing_response*)bing_mem_malloc(sizeof(bing_response));
		if(res)
		{
			//Set variables
			res->type = type;
			res->bing = responseParent ? 0 : bing;

			res->creation = creation;
			res->additionalData = additionalData;

			res->resultCount = 0;
			res->results = NULL;

			res->internalResultCount = 0;
			res->internalResults = NULL;

			res->allocatedMemoryCount = 0;
			res->allocatedMemory = NULL;

			//Create hashtable
			res->data = hashtable_create(tableSize);
			if(res->data)
			{
				if(res->bing != 0)
				{
					//Add response to Bing object
					ret = response_add_to_bing(res, bing);
				}
				else
				{
					//Add response to bundle
					ret = response_add_to_bundle(res, responseParent);
				}
				//Check if we succeeded or failed
				if(ret)
				{
					//Save response
					response[0] = res;
				}
				else
				{
					hashtable_free(res->data);
					bing_mem_free(res);
				}
			}
			else
			{
				//Hashtable couldn't be created
				bing_mem_free(res);
			}
		}
	}
	return ret;
}

BOOL response_create_raw(const char* type, bing_response_t* response, unsigned int bing, bing_response* responseParent)
{
	BOOL ret = FALSE;
	bing_response_creator_search* cr;
	response_creation_func creationFunc;
	response_additional_data_func additionalDataFunc;
	unsigned int i;
	if(type && response &&
			(!(responseParent && responseParent->bing == 0) || bing > 0))
	{
		//Check default options
		for(cr = response_def_creator; cr != NULL; cr = cr->next)
		{
			if(strcmp(type, cr->creator.name) == 0)
			{
				break;
			}
		}
		if(cr)
		{
			//Create default options
			ret = response_create(cr->type, response, bing, responseParent, cr->creator.creation, cr->creator.additionalData, cr->tableCount);
		}
		else
		{
			creationFunc = NULL;
			additionalDataFunc = NULL;

			//Search custom creators
			pthread_mutex_lock(&bingSystem.mutex);

			for(i = 0; i < bingSystem.bingResponseCreatorCount; i++)
			{
				if(strcmp(type, bingSystem.bingResponseCreators[i].name) == 0)
				{
					creationFunc = bingSystem.bingResponseCreators[i].creation;
					additionalDataFunc = bingSystem.bingResponseCreators[i].additionalData;
					ret = TRUE; //We only want a response to be created if we find something
					break;
				}
			}

			pthread_mutex_unlock(&bingSystem.mutex);

			if(ret)
			{
				//Create the custom response
				if(!creationFunc)
				{
					//The "do nothing" function
					creationFunc = response_def_create;
				}
				if(!additionalDataFunc)
				{
					//The "do nothing" function
					additionalDataFunc = response_def_additional_data;
				}
				ret = response_create(BING_SOURCETYPE_CUSTOM, response, bing, responseParent, creationFunc, additionalDataFunc, -1);
			}
		}
	}
	return ret;
}

void free_response_in(bing_response_t response, BOOL bundle_free)
{
	bing_response* res;
	list* list;
	unsigned int i;
	if(response)
	{
		res = (bing_response*)response;

		//Remove response from Bing
		response_remove_from_bing(res, bundle_free);

		//Free allocated memory (allocated by response and result)
		for(i = 0; i < res->allocatedMemoryCount; i++)
		{
			bing_mem_free(res->allocatedMemory[i]);
		}
		bing_mem_free(res->allocatedMemory);
		res->allocatedMemory = NULL;
		res->allocatedMemoryCount = 0;

		//Free data
		if(res->data)
		{
			if(hashtable_get_item(res->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, &list) > 0)
			{
				for(i = 0; i < list->count; i++)
				{
					free_response_in(LIST_ELEMENT(list, i, bing_response_t), TRUE);
				}
				bing_mem_free((void*)list->listElements);
				bing_mem_free((void*)list);
			}

			hashtable_free(res->data);
		}
		res->data = NULL;

		//Free results
		while(res->resultCount > 0)
		{
			free_result((bing_result*)res->results[--res->resultCount]);
		}
		bing_mem_free(res->results);
		res->results = NULL;

		//Free internal results
		while(res->internalResultCount > 0)
		{
			free_result((bing_result*)res->internalResults[--res->internalResultCount]);
		}
		bing_mem_free(res->internalResults);
		res->internalResults = NULL;

		bing_mem_free(res);
	}
}

void bing_response_free(bing_response_t response)
{
	free_response_in(response, FALSE);
}

BOOL response_swap_response(bing_response* response, bing_response* responseParent)
{
	BOOL ret = FALSE;
	if(response && responseParent && response != responseParent)
	{
		//Remove from bundle
		if(response_remove_from_bundle(response, responseParent))
		{
			//Add to Bing
			ret = response_add_to_bing(response, responseParent->bing);
			if(!ret)
			{
				//Error adding to Bing, revert to bundle
				response_add_to_bundle(response, responseParent);
			}
		}
		else if(response_remove_from_bing(response, FALSE)) //Remove from Bing
		{
			//Add to bundle
			ret = response_add_to_bundle(response, responseParent);
			if(!ret)
			{
				//Error adding to bundle, revert to Bing
				response_add_to_bing(response, responseParent->bing);
			}
		}
	}
	return ret;
}

int response_get_string(bing_response_t response, char* buffer, const char* field, enum BING_SOURCE_TYPE type)
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

int bing_response_get_ad_api_version(bing_response_t response, char* buffer)
{
	return response_get_string(response, buffer, RESPONSE_AD_API_VERSION, BING_SOURCETYPE_AD);
}

long long response_get_int64(bing_response_t response, const char* field, enum BING_SOURCE_TYPE type)
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

long long bing_response_get_ad_page_number(bing_response_t response)
{
	return response_get_int64(response, RESPONSE_AD_PAGE_NUMBER, BING_SOURCETYPE_AD);
}

int bing_response_get_bundle_responses(bing_response_t response, bing_response_t* responses)
{
	int ret = -1;
	bing_response* res;
	bing_response_t* responseList;
	list* list_v = NULL;
	if(response)
	{
		res = (bing_response*)response;
		if(res->type == BING_SOURCETYPE_BUNDLE && //Make sure of the proper type
				hashtable_get_item(res->data, RESPONSE_BUNDLE_SUBBUNDLES_STR, &list_v) > 0)
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

int bing_response_get_phonebook_title(bing_response_t response, char* buffer)
{
	return response_get_string(response, buffer, RESPONSE_PHONEBOOK_TITLE, BING_SOURCETYPE_PHONEBOOK);
}

int bing_response_get_phonebook_local_serp_url(bing_response_t response, char* buffer)
{
	return response_get_string(response, buffer, RESPONSE_PHONEBOOK_LOCAL_SERP_URL, BING_SOURCETYPE_PHONEBOOK);
}

int bing_response_get_news_related_searches(bing_response_t response, bing_related_search_t searches)
{
	int ret = response_get_string(response, (char*)searches, RESPONSE_NEWS_RELATEDSEARCHES, BING_SOURCETYPE_NEWS);
	if(ret != -1)
	{
		//We need to convert byte count to item count
		ret /= sizeof(bing_related_search_s);
	}
	return ret;
}

int bing_response_custom_is_field_supported(bing_response_t response, const char* field)
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

int bing_response_custom_get_64bit_int(bing_response_t response, const char* field, long long* value)
{
	return hashtable_get_data_key(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(long long));
}

int bing_response_custom_get_string(bing_response_t response, const char* field, char* value)
{
	return hashtable_get_string(response ? ((bing_response*)response)->data : NULL, field, value);
}

int bing_response_custom_get_double(bing_response_t response, const char* field, double* value)
{
	return bing_response_custom_get_64bit_int(response, field, (long long*)value);
}

int bing_response_custom_get_boolean(bing_response_t response, const char* field, int* value)
{
	return hashtable_get_data_key(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(int));
}

int bing_response_custom_get_array(bing_response_t response, const char* field, void* value)
{
	return bing_response_custom_get_string(response, field, (char*)value);
}

int bing_response_custom_set_64bit_int(bing_response_t response, const char* field, long long* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(long long));
}

int bing_response_custom_set_string(bing_response_t response, const char* field, const char* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, value ? (strlen(value) + 1) : 0);
}

int bing_response_custom_set_double(bing_response_t response, const char* field, double* value)
{
	return bing_response_custom_set_64bit_int(response, field, (long long*)value);
}

int bing_response_custom_set_boolean(bing_response_t response, const char* field, int* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(int));
}

int bing_response_custom_set_array(bing_response_t response, const char* field, const void* value, size_t size)
{
	//This could be a safety hazard but we have no way of checking the size of the data passed in
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, size);
}

void* allocateMemory(size_t size, bing_response* response)
{
	void* ret = NULL;
	void** mem = bing_mem_malloc((response->allocatedMemoryCount + 1) * sizeof(void*));
	if(mem)
	{
		ret = bing_mem_malloc(size);
		if(ret)
		{
			//Copy the memory
			memcpy(mem, response->allocatedMemory, response->allocatedMemoryCount * sizeof(void*));

			bing_mem_free(response->allocatedMemory);
			response->allocatedMemory = mem;

			//Set the new memory allocation
			mem[response->allocatedMemoryCount++] = ret;
		}
		else
		{
			bing_mem_free(mem);
		}
	}
	return ret;
}

void* rallocateMemory(void* ptr, size_t size, bing_response* response)
{
	unsigned int i;
	void* nptr = NULL;
	//Find the memory
	for(i = 0; i < response->allocatedMemoryCount; i++)
	{
		if(response->allocatedMemory[i] == ptr)
		{
			//Reallocate the memory
			nptr = bing_mem_realloc(ptr, size);
			if(nptr)
			{
				response->allocatedMemory[i] = nptr;
			}
			break;
		}
	}
	return nptr;
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
			mem = bing_mem_malloc((response->allocatedMemoryCount - 1) * sizeof(void*));
			if(mem)
			{
				//Copy over pointers
				memcpy(mem, response->allocatedMemory, i * sizeof(void*));
				memcpy(mem + i, response->allocatedMemory + (i + 1), (response->allocatedMemoryCount - i - 1) * sizeof(void*));

				//Free old allocated memory pointer and ptr
				bing_mem_free(response->allocatedMemory);
				bing_mem_free(ptr);

				//Set the memory
				response->allocatedMemoryCount--;
				response->allocatedMemory = mem;
			}
		}
		else
		{
			//Free all memory
			bing_mem_free(response->allocatedMemory);
			bing_mem_free(ptr);

			//Reset allocated memory
			response->allocatedMemoryCount = 0;
			response->allocatedMemory = NULL;
		}
	}
}

void* bing_response_custom_allocation(bing_response_t response, size_t size)
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

int bing_response_register_response_creator(const char* name, response_creation_func creation_func, response_additional_data_func additional_func)
{
	BOOL ret = FALSE;
	BOOL cont = TRUE;
	unsigned int i;
	bing_response_creator* c;
	bing_response_creator_search* cr;
	char* nName;
	size_t size;
	if(name)
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
				nName = bing_mem_malloc(size);

				if(nName)
				{
					strlcpy(nName, name, size);

					//Create the new version of the name
					c = (bing_response_creator*)bing_mem_realloc(bingSystem.bingResponseCreators, sizeof(bing_response_creator) * (bingSystem.bingResponseCreatorCount + 1));

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
						bing_mem_free(nName);
					}
				}
			}

			pthread_mutex_unlock(&bingSystem.mutex);
		}
	}
	return ret;
}

int bing_response_unregister_response_creator(const char* name)
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
			c = (bing_response_creator*)bing_mem_malloc(sizeof(bing_response_creator) * (bingSystem.bingResponseCreatorCount - 1));

			if(c)
			{
				//If this is the last response then it's easy, we just free the data
				if(i != bingSystem.bingResponseCreatorCount - 1)
				{
					memmove(bingSystem.bingResponseCreators + i, bingSystem.bingResponseCreators + (i + 1), (bingSystem.bingResponseCreatorCount - i - 1) * sizeof(bing_response_creator));
				}
				memcpy(c, bingSystem.bingResponseCreators, (--bingSystem.bingResponseCreatorCount) * sizeof(bing_response_creator));
				bing_mem_free(bingSystem.bingResponseCreators);
				bingSystem.bingResponseCreators = c;

				ret = TRUE;
			}
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
	return ret;
}
