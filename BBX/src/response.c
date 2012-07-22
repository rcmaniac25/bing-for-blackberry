/*
 * response.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#define RES_ID "id"
#define RES_UPDTAED "updated"

//Creation/update functions

BOOL response_def_create_standard_responses(bing_response_t response, data_dictionary_t dictionary)
{
	BOOL ret = TRUE;
	int size;
	void* data;
	char tmpChar;
	char* tmpString;
	char* tmpString2;
	long long ll;
	bing_response* res = (bing_response*)response;
	if(dictionary)
	{
		//Process ID to get total, max, and query
		size = hashtable_get_string((hashtable_t*)dictionary, RES_ID, NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, RES_ID, (char*)data);

				//Offset
				tmpString = strstr((char*)data, "$skip=");
				if(tmpString)
				{
					//Move to the numeric location
					tmpString += 6;

					//Get the number
					for(size = 0; *tmpString && (*tmpString >= '0' && *tmpString <= '9'); size++, tmpString++);

					//End the string
					tmpChar = *tmpString;
					*tmpString = '\0';

					//Parse the number
					ll = atoll(tmpString - size);

					//Reset string
					*tmpString = tmpChar;

					//Save the value
					hashtable_set_data(res->data, RESPONSE_OFFSET_STR, &ll, sizeof(long long));
				}

				//Max total
				tmpString = strstr((char*)data, "$top=");
				if(tmpString)
				{
					//Move to the numeric location
					tmpString += 5;

					//Get the number
					for(size = 0; *tmpString && (*tmpString >= '0' && *tmpString <= '9'); size++, tmpString++);

					//End the string
					tmpChar = *tmpString;
					*tmpString = '\0';

					//Parse the number
					ll = atoll(tmpString - size);

					//Reset string
					*tmpString = tmpChar;

					//Save the value
					hashtable_set_data(res->data, RESPONSE_MAX_TOTAL_STR, &ll, sizeof(long long));
				}

				//Query
				tmpString = strstr((char*)data, "Query='");
				if(tmpString)
				{
					//Move to the query location
					tmpString += 7;

					//Get the query
					for(size = 0; *tmpString; size++, tmpString++)
					{
						if((*tmpString == '\'') && (*(tmpString + 1) == '\''))
						{
							size++;
							tmpString++;
						}
						else if(*tmpString == '\'')
						{
							break;
						}
					}

					//End the string
					tmpChar = *tmpString;
					*tmpString = '\0';

					//Duplicate the string
					tmpString2 = bing_mem_strdup(tmpString - size);

					//Reset string
					*tmpString = tmpChar;

					//Save the value
					if(tmpString2)
					{
						//We need to do some extra processing first (we need to get rid of the double single quotes)
						for(tmpString = tmpString2, size = 0; *tmpString; size++, tmpString++)
						{
							*tmpString = tmpString2[size];
							if(tmpString2[size] == '\'')
							{
								size++;
							}
							if(*tmpString == '\0')
							{
								break;
							}
						}

						hashtable_set_data(res->data, RESPONSE_QUERY_STR, tmpString2, strlen(tmpString2) + 1);

						bing_mem_free((void*)tmpString2);
					}
				}

				bing_mem_free(data);
			}
		}

		//Process updated to get datetime
		size = (int)hashtable_get_item((hashtable_t*)dictionary, RES_UPDTAED, NULL);
		if(size == sizeof(long long))
		{
			//Get the datetime
			hashtable_get_item((hashtable_t*)dictionary, RES_UPDTAED, &ll);

			//Save the datetime
			hashtable_set_data(res->data, RES_UPDTAED, &ll, sizeof(long long));
		}

		//Process to get URL for next "page" of results
		size = hashtable_get_string((hashtable_t*)dictionary, PARSE_NEXT_LINK, NULL);
		if(size > 0)
		{
			data = bing_mem_malloc(size);
			if(data)
			{
				hashtable_get_string((hashtable_t*)dictionary, PARSE_NEXT_LINK, (char*)data);

				res->nextUrl = (char*)data;
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

//Search structure

typedef struct BING_RESPONSE_CREATOR_SEARCH_S
{
	bing_response_creator creator;
	enum BING_SOURCE_TYPE type;
	int tableCount;
	struct BING_RESPONSE_CREATOR_SEARCH_S* next;
} bing_response_creator_search;

static bing_response_creator_search response_def_creator[]=
{
		{{"Bing Web Search",	"Web",					response_def_create},		BING_SOURCETYPE_WEB,				0,	&response_def_creator[1]},
		{{"Bing Video Search",	"Video",				response_def_create},		BING_SOURCETYPE_VIDEO,				0,	&response_def_creator[2]},
		{{"Bing Image Search",	"Image",				response_def_create},		BING_SOURCETYPE_IMAGE,				0,	&response_def_creator[3]},
		{{"Bing News Search",	"News",					response_def_create},		BING_SOURCETYPE_NEWS,				0,	&response_def_creator[4]},
		{{"Bing Related Search","RelatedSearch",		response_def_create},		BING_SOURCETYPE_RELATED_SEARCH,		0,	&response_def_creator[5]},
		{{"Bing Spell Search",	"SpellingSuggestions",	response_def_create},		BING_SOURCETYPE_SPELL,				0,	&response_def_creator[6]},
		{{RESPONSE_COMPOSITE,	NULL,					response_def_create},		BING_SOURCETYPE_COMPOSITE,			0,	NULL},
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

long long bing_response_get_max_total(bing_response_t response)
{
	long long ret = -1;
	response_get(response, RESPONSE_MAX_TOTAL_STR, &ret);
	return ret;
}

long long bing_response_get_offset(bing_response_t response)
{
	long long ret = -1;
	response_get(response, RESPONSE_OFFSET_STR, &ret);
	return ret;
}

long long bing_response_get_updated(bing_response_t response)
{
	long long ret = -1;
	response_get(response, RES_UPDTAED, &ret);
	return ret;
}

int bing_response_get_query(bing_response_t response, char* buffer)
{
	return response_get(response, RESPONSE_QUERY_STR, buffer);
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

BOOL response_remove_from_bing(bing_response* res, BOOL composite_free)
{
	BOOL ret = FALSE;
	bing* bing;
	bing_response** t;
	unsigned int pos;

	//Don't run for composited responses (they are not included within Bing)
	if(!composite_free)
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

BOOL response_add_to_composite(bing_response* response, bing_response* responseParent)
{
	BOOL ret = FALSE;
	list* list_v = NULL;
	bing_response_t* responseList = NULL;

	//Add response to composite
	if(hashtable_get_item(responseParent->data, RESPONSE_COMPOSITE_SUBRES_STR, &list_v) > 0)
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
				if(hashtable_put_item(responseParent->data, RESPONSE_COMPOSITE_SUBRES_STR, list_v, sizeof(list*)) == -1)
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

BOOL response_remove_from_composite(bing_response* response, bing_response* responseParent)
{
	BOOL ret = FALSE;
	list* list_v = NULL;
	bing_response_t* responseList = NULL;
	unsigned int i;

	//Get response from composite
	if(hashtable_get_item(responseParent->data, RESPONSE_COMPOSITE_SUBRES_STR, &list_v) > 0)
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
					hashtable_remove_item(responseParent->data, RESPONSE_COMPOSITE_SUBRES_STR);
				}
				ret = TRUE;
				break;
			}
		}
	}
	return ret;
}

BOOL response_create(enum BING_SOURCE_TYPE type, bing_response_t* response, unsigned int bing, bing_response* responseParent, response_creation_func creation, int tableSize)
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

			res->nextUrl = NULL;

			res->creation = creation;

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
					//Add response to composite
					ret = response_add_to_composite(res, responseParent);
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
	unsigned int i;
	if(type && response &&
			(!(responseParent && responseParent->bing == 0) || bing > 0))
	{
		//Check default options
		for(cr = response_def_creator; cr != NULL; cr = cr->next)
		{
			if(strcmp(type, cr->creator.dedicatedName) == 0 || strcmp(type, cr->creator.compositeName) == 0)
			{
				break;
			}
		}
		if(cr)
		{
			//Create default options
			ret = response_create(cr->type, response, bing, responseParent, cr->creator.creation, cr->tableCount);
		}
		else
		{
			creationFunc = NULL;

			//Search custom creators
			pthread_mutex_lock(&bingSystem.mutex);

			for(i = 0; i < bingSystem.bingResponseCreatorCount; i++)
			{
				if(strcmp(type, bingSystem.bingResponseCreators[i].dedicatedName) == 0 || strcmp(type, bingSystem.bingResponseCreators[i].compositeName) == 0)
				{
					creationFunc = bingSystem.bingResponseCreators[i].creation;
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
				ret = response_create(BING_SOURCETYPE_CUSTOM, response, bing, responseParent, creationFunc, -1);
			}
		}
	}
	return ret;
}

void free_response_in(bing_response_t response, BOOL composite_free)
{
	bing_response* res;
	list* list;
	unsigned int i;
	if(response)
	{
		res = (bing_response*)response;

		//Remove response from Bing
		response_remove_from_bing(res, composite_free);

		//Free "next" URL if one exists
		bing_mem_free((void*)res->nextUrl);

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
			if(hashtable_get_item(res->data, RESPONSE_COMPOSITE_SUBRES_STR, &list) > 0)
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
		//Remove from composite
		if(response_remove_from_composite(response, responseParent))
		{
			//Add to Bing
			ret = response_add_to_bing(response, responseParent->bing);
			if(!ret)
			{
				//Error adding to Bing, revert to composite
				response_add_to_composite(response, responseParent);
			}
		}
		else if(response_remove_from_bing(response, FALSE)) //Remove from Bing
		{
			//Add to composite
			ret = response_add_to_composite(response, responseParent);
			if(!ret)
			{
				//Error adding to composite, revert to Bing
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

int bing_response_has_next_results(bing_response_t response)
{
	BOOL ret = FALSE;

	if(response)
	{
		ret = ((bing_response*)response)->nextUrl != NULL;
	}

	return ret;
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

int bing_response_get_composite_responses(bing_response_t response, bing_response_t* responses)
{
	int ret = -1;
	bing_response* res;
	bing_response_t* responseList;
	list* list_v = NULL;
	if(response)
	{
		res = (bing_response*)response;
		if(res->type == BING_SOURCETYPE_COMPOSITE && //Make sure of the proper type
				hashtable_get_item(res->data, RESPONSE_COMPOSITE_SUBRES_STR, &list_v) > 0)
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

int bing_response_custom_get_32bit_int(bing_response_t response, const char* field, int* value)
{
	return hashtable_get_data_key(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(int));
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
#if __SIZEOF_DOUBLE__ != __SIZEOF_LONG_LONG__
#error Double size is different than Long Long size
#endif
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

int bing_response_custom_set_32bit_int(bing_response_t response, const char* field, const int* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(int));
}

int bing_response_custom_set_64bit_int(bing_response_t response, const char* field, const long long* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, sizeof(long long));
}

int bing_response_custom_set_string(bing_response_t response, const char* field, const char* value)
{
	return hashtable_set_data(response ? ((bing_response*)response)->data : NULL, field, value, value ? (strlen(value) + 1) : 0);
}

int bing_response_custom_set_double(bing_response_t response, const char* field, const double* value)
{
#if __SIZEOF_DOUBLE__ != __SIZEOF_LONG_LONG__
#error Double size is different than Long Long size
#endif
	return bing_response_custom_set_64bit_int(response, field, (long long*)value);
}

int bing_response_custom_set_boolean(bing_response_t response, const char* field, const int* value)
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

int bing_response_register_response_creator(const char* dedicated_name, const char* composite_name, response_creation_func creation_func)
{
	BOOL ret = FALSE;
	BOOL cont = TRUE;
	unsigned int i;
	bing_response_creator* c;
	bing_response_creator_search* cr;
	char* nDName, *nCName;
	size_t size;
	if(dedicated_name && composite_name)
	{
		//Check if the name is a standard supported name, if so return
		for(cr = response_def_creator; cr != NULL; cr = cr->next)
		{
			if(strcmp(dedicated_name, cr->creator.dedicatedName) == 0 || strcmp(composite_name, cr->creator.compositeName) == 0)
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
				if(strcmp(bingSystem.bingResponseCreators[i++].dedicatedName, dedicated_name) == 0 || strcmp(bingSystem.bingResponseCreators[i++].compositeName, composite_name) == 0)
				{
					cont = FALSE;
				}
			}

			if(cont)
			{
				//Reproduce the dedicatedName
				size = strlen(dedicated_name) + 1;
				nDName = bing_mem_malloc(size);

				if(nDName)
				{
					strlcpy(nDName, dedicated_name, size);
					nDName[size - 1] = '\0';

					//Reproduce the composite_name
					size = strlen(composite_name) + 1;
					nCName = bing_mem_malloc(size);

					if(nCName)
					{
						strlcpy(nCName, composite_name, size);
						nCName[size - 1] = '\0';

						//Create the new version of the name
						c = (bing_response_creator*)bing_mem_realloc(bingSystem.bingResponseCreators, sizeof(bing_response_creator) * (bingSystem.bingResponseCreatorCount + 1));

						if(c)
						{
							bingSystem.bingResponseCreators = c;

							c[bingSystem.bingResponseCreatorCount].dedicatedName = nDName;
							c[bingSystem.bingResponseCreatorCount].compositeName = nCName;
							c[bingSystem.bingResponseCreatorCount].creation = creation_func;

							ret = TRUE;
						}
						else
						{
							bing_mem_free(nCName);
							bing_mem_free(nDName);
						}
					}
					else
					{
						bing_mem_free(nDName);
					}
				}
			}

			pthread_mutex_unlock(&bingSystem.mutex);
		}
	}
	return ret;
}

int bing_response_unregister_response_creator(const char* dedicated_name, const char* composite_name)
{
	BOOL ret = FALSE;

	const char* dn, *cn;
	unsigned int i;
	bing_response_creator* c;
	if(dedicated_name && composite_name && bingSystem.bingResponseCreatorCount > 0) //We don't want to run if there is nothing to run on
	{
		pthread_mutex_lock(&bingSystem.mutex);

		//Find the response
		i = 0;
		while(i < bingSystem.bingResponseCreatorCount)
		{
			if(strcmp(bingSystem.bingResponseCreators[i].dedicatedName, dedicated_name) == 0 && strcmp(bingSystem.bingResponseCreators[i].compositeName, composite_name) == 0)
			{
				break;
			}
			i++;
		}

		if(i < bingSystem.bingResponseCreatorCount)
		{
			//Get the strings
			dn = bingSystem.bingResponseCreators[i].dedicatedName;
			cn = bingSystem.bingResponseCreators[i].compositeName;

			//If there is only one creator, simply free everything
			if(bingSystem.bingResponseCreatorCount == 1)
			{
				bing_mem_free(bingSystem.bingResponseCreators);
				bingSystem.bingResponseCreators = NULL;
				bingSystem.bingResponseCreatorCount = 0;

				ret = TRUE;
			}
			else
			{
				//We don't want to reallocate because if we fail and the creator was not the last element, then we overwrote it
				c = (bing_response_creator*)bing_mem_malloc(sizeof(bing_response_creator) * (bingSystem.bingResponseCreatorCount - 1));

				if(c)
				{
					//If this is the last response then it's easy, we just free the data. Otherwise we need to move it.
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

			//Free memory if everything else worked
			if(ret)
			{
				bing_mem_free((void*)dn);
				bing_mem_free((void*)cn);
			}
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
	return ret;
}
