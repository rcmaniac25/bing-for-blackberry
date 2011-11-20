/*
 * request.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

static bing_field_search request_fields[] =
{
		//Universal
		{{REQUEST_FIELD_VERSION,			FIELD_TYPE_STRING,	"version",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[1]},
		{{REQUEST_FIELD_MARKET,				FIELD_TYPE_STRING,	"market",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[2]},
		{{REQUEST_FIELD_ADULT,				FIELD_TYPE_STRING,	"adult",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[3]},
		{{REQUEST_FIELD_OPTIONS,			FIELD_TYPE_STRING,	"options",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[4]},
		{{REQUEST_FIELD_LATITUDE,			FIELD_TYPE_DOUBLE,	"latitude",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[5]},
		{{REQUEST_FIELD_LONGITUDE,			FIELD_TYPE_DOUBLE,	"longitude",		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[6]},
		{{REQUEST_FIELD_LANGUAGE,			FIELD_TYPE_STRING,	"language",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[7]},
		{{REQUEST_FIELD_RADIUS,				FIELD_TYPE_DOUBLE,	"radius",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[8]},

		//Ad
		{{REQUEST_FIELD_PAGE_NUMBER,		FIELD_TYPE_LONG,	"pageNumber",		1,	{BING_SOURCETYPE_AD}},								&request_fields[9]},
		{{REQUEST_FIELD_AD_UNIT_ID,			FIELD_TYPE_LONG,	"adUnitId",			1,	{BING_SOURCETYPE_AD}},								&request_fields[10]},
		{{REQUEST_FIELD_PROPERTY_ID,		FIELD_TYPE_LONG,	"propertyId",		1,	{BING_SOURCETYPE_AD}},								&request_fields[11]},
		{{REQUEST_FIELD_CHANNEL_ID,			FIELD_TYPE_LONG,	"channelId",		1,	{BING_SOURCETYPE_AD}},								&request_fields[12]},
		{{REQUEST_FIELD_MAINLINE_AD_COUNT,	FIELD_TYPE_LONG,	"mlAdcount",		1,	{BING_SOURCETYPE_AD}},								&request_fields[13]},
		{{REQUEST_FIELD_SIDEBAR_AD_COUNT,	FIELD_TYPE_LONG,	"sbAdCount",		1,	{BING_SOURCETYPE_AD}},								&request_fields[14]},

		//MobileWeb
		{{REQUEST_FIELD_MOBILE_WEB_OPTIONS,	FIELD_TYPE_STRING,	"mobileWebOptions",	1,	{BING_SOURCETYPE_MOBILE_WEB}},						&request_fields[15]},

		//News
		{{REQUEST_FIELD_CATEGORY,			FIELD_TYPE_STRING,	"category",			1,	{BING_SOURCETYPE_NEWS}},							&request_fields[16]},
		{{REQUEST_FIELD_LOCATION_OVERRIDE,	FIELD_TYPE_STRING,	"locationOverride",	1,	{BING_SOURCETYPE_NEWS}},							&request_fields[17]},

		//Phonebook
		{{REQUEST_FIELD_LOC_ID,				FIELD_TYPE_STRING,	"locId",			1,	{BING_SOURCETYPE_PHONEBOOK}},						&request_fields[18]},

		//Translation
		{{REQUEST_FIELD_SOURCE_LANGUAGE,	FIELD_TYPE_STRING,	"sourceLanguage",	1,	{BING_SOURCETYPE_TRANSLATION}},						&request_fields[19]},
		{{REQUEST_FIELD_TARGET_LANGUAGE,	FIELD_TYPE_STRING,	"targetLanguage",	1,	{BING_SOURCETYPE_TRANSLATION}},						&request_fields[20]},

		//Web
		{{REQUEST_FIELD_WEB_OPTIONS,		FIELD_TYPE_STRING,	"webOptions",		1,	{BING_SOURCETYPE_WEB}},								&request_fields[21]},

		//Multi (less obvious breaks in original source type)
		{{REQUEST_FIELD_COUNT,				FIELD_TYPE_LONG,	"count",			6,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},								&request_fields[22]},
		{{REQUEST_FIELD_OFFSET,				FIELD_TYPE_LONG,	"offset",			6,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},								&request_fields[23]},
		{{REQUEST_FIELD_FILTERS,			FIELD_TYPE_STRING,	"filters",			2,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_VIDEO}},	&request_fields[24]},
		{{REQUEST_FIELD_SORT_BY,			FIELD_TYPE_STRING,	"sortby",			3,	{BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_VIDEO}},																									&request_fields[25]},
		{{REQUEST_FIELD_FILE_TYPE,			FIELD_TYPE_STRING,	"filetype",			2,	{BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_WEB}},	NULL}
};

const char* request_def_get_options(bing_request_t request)
{
	//TODO
	return NULL;
}

const char* request_bundle_get_options(bing_request_t request)
{
	//TODO
	return NULL;
}

//TODO: Do other special options

const char* request_custom_get_options(bing_request_t request)
{
	bing_request* req;
	const char* ret = NULL;
	const char* custOptions;
	char* resultOptions;
	//Request exists, create the options string
	if(request)
	{
		req = (bing_request*)request;

		ret = request_def_get_options(request);
		custOptions = req->uGetOptions(request);

		resultOptions = calloc(strlen(ret) + strlen(custOptions) + 2, sizeof(char));
		if(resultOptions)
		{
			//First copy in the default options
			memcpy(resultOptions, ret, strlen(ret));

			//Free the return string
			free((void*)ret);

			//Conc. the user options
			strcat(resultOptions, custOptions);

			//Set the return
			ret = resultOptions;
		}
		else
		{
			free((void*)ret);
			ret = NULL;
		}
		if(req->uFinishGetOptions)
		{
			req->uFinishGetOptions(request, custOptions);
		}
	}
	//No return exists, create a empty string (that won't be invalid on "custom finish")
	if(ret == NULL)
	{
		ret = calloc(1, sizeof(char));
	}
	return ret;
}

#define DEFAULT_ELEMENT_COUNT 8

typedef struct request_source_type_s
{
	enum SOURCE_TYPE type;
	const char* source_type;
	int maxElements;
	request_get_options_func getOptions;

	struct request_source_type_s* next;
} request_source_type;

static request_source_type request_source_types[] =
{
		//Unsure if capitalization is required or not, leave it like this since it worked for the BBOS version
		//TODO: Replace null options
		{BING_SOURCETYPE_AD,				"Ad",				DEFAULT_ELEMENT_COUNT + 6,	NULL,						&request_source_types[1]},
		{BING_SOURCETYPE_IMAGE,				"image",			DEFAULT_ELEMENT_COUNT + 3,	NULL,						&request_source_types[2]},
		{BING_SOURCETYPE_INSTANT_ANWSER,	"InstantAnswer",	DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[3]},
		{BING_SOURCETYPE_MOBILE_WEB,		"MobileWeb",		DEFAULT_ELEMENT_COUNT + 3,	NULL,						&request_source_types[4]},
		{BING_SOURCETYPE_NEWS,				"news",				DEFAULT_ELEMENT_COUNT + 5,	NULL,						&request_source_types[5]},
		{BING_SOURCETYPE_PHONEBOOK,			"phonebook",		DEFAULT_ELEMENT_COUNT + 5,	NULL,						&request_source_types[6]},
		{BING_SOURCETYPE_RELATED_SEARCH,	"RelatedSearch",	DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[7]},
		{BING_SOURCETYPE_SPELL,				"Spell",			DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[8]},
		{BING_SOURCETYPE_TRANSLATION,		"Translation",		DEFAULT_ELEMENT_COUNT + 2,	NULL,						&request_source_types[9]},
		{BING_SOURCETYPE_VIDEO,				"video",			DEFAULT_ELEMENT_COUNT + 4,	NULL,						&request_source_types[10]},
		{BING_SOURCETYPE_WEB,				"web",				DEFAULT_ELEMENT_COUNT + 4,	NULL,						NULL}
};

enum SOURCE_TYPE request_get_source_type(bing_request_t request)
{
	enum SOURCE_TYPE t = BING_SOURCETYPE_UNKNOWN;
	bing_request* req;
	request_source_type* type;
	if(request)
	{
		req = (bing_request*)request;
		if(req->sourceType)
		{
			for(type = request_source_types; type; type = type->next)
			{
				if(strcmp(type->source_type, req->sourceType) == 0)
				{
					t = type->type;
					break;
				}
			}
		}
		else
		{
			t = BING_SOURCETYPE_BUNDLE;
		}
	}
	return t;
}

const char* request_get_bundle_sourcetype(bing_request* bundle)
{
	//TODO
	return NULL;
}

int request_create(const char* source_type, bing_request_t* request, request_get_options_func get_options_func, int tableSize)
{
	BOOL ret = FALSE;
	bing_request* req;
	if(request)
	{
		req = (bing_request*)malloc(sizeof(bing_request));
		if(req)
		{
			req->sourceType = source_type;
			req->getOptions = get_options_func;
			req->data = NULL;

			req->data = hashtable_create(tableSize);
			if(req->data)
			{
				request[0] = req;
				ret = TRUE;
			}
			else
			{
				free(req);
			}
		}
	}
	return ret;
}

int request_create_request(enum SOURCE_TYPE source_type, bing_request_t* request)
{
	BOOL ret = FALSE;
	int tableSize;
	request_source_type* type;
	const char* sourceT;
	request_get_options_func getOFun;
	if(source_type >= BING_SOURCETYPE_AD && source_type <= BING_SOURCETYPE_WEB && source_type == BING_SOURCETYPE_BUNDLE) //This guarantees that the source_type will be a valid type
	{
		tableSize = -1;

		if(source_type == BING_SOURCETYPE_BUNDLE)
		{
			for(type = request_source_types; type; type = type->next)
			{
				if(type->type == source_type)
				{
					sourceT = type->source_type;
					tableSize = type->maxElements;
					getOFun = type->getOptions;
					break;
				}
			}
		}
		else
		{
			sourceT = NULL;
			tableSize = DEFAULT_ELEMENT_COUNT + 1;
			getOFun = request_bundle_get_options;
		}
		ret = request_create(sourceT, request, getOFun, tableSize);
	}
	return ret;
}

int request_is_field_supported(bing_request_t request, enum REQUEST_FIELD field)
{
	BOOL ret = FALSE;
	bing_request* req;
	const char* key;
	if(request)
	{
		//Get the key
		req = (bing_request*)request;
		key = find_field(request_fields, field, FIELD_TYPE_UNKNOWN, request_get_source_type(request), FALSE);

		//Determine if the key is within the result
		ret = hashtable_key_exists(req->data, key) != -1;
	}
	return ret;
}

//TODO: request_get_64bit_int
//TODO: request_get_string
//TODO: request_get_double

typedef struct bundle_list_s
{
	int count;
	int cap;
	bing_request_t* requests;
} bundle_list;

int request_bundle_add_request(bing_request_t request, bing_request_t request_to_add)
{
	BOOL ret = FALSE;
	bing_request* req;
	bundle_list* list = NULL;
	bing_request_t* requestList = NULL;
	int i;

	if(request && request_to_add && request == request_to_add)
	{
		req = (bing_request*)request;

		if(!req->sourceType) //Bundle's source type is null
		{
			if(hashtable_get_item(req->data, REQUEST_BUNDLE_SUBBUNDLES_STR, &list) != -1)
			{
				//Get the list
				requestList = list->requests;
			}
			else
			{
				//Create the list
				list = (bundle_list*)malloc(sizeof(bundle_list));
				if(list)
				{
					list->count = 0;
					requestList = list->requests = (bing_request_t*)calloc(11, sizeof(bing_request_t));
					if(list->requests)
					{
						//Save the list
						list->cap = 11;
						if(hashtable_put_item(req->data, REQUEST_BUNDLE_SUBBUNDLES_STR, list, sizeof(bundle_list*)) == -1)
						{
							//List creation failed, cleanup
							free(list);
							list = NULL;
						}
					}
					else
					{
						//List creation failed, cleanup
						free(list);
						list = NULL;
					}
				}
			}
			if(list)
			{
				if(list->count >= list->cap)
				{
					//Resize list
					requestList = (bing_request_t*)realloc(requestList, sizeof(bing_request_t) * (list->cap * 2));
					if(requestList)
					{
						list->cap *= 2;
						list->requests = requestList;
					}
					else
					{
						requestList = NULL;
					}
				}
				if(requestList)
				{
					//See if the request already exists in the list
					for(i = 0; i < list->count; i++)
					{
						if(requestList[i] == request_to_add)
						{
							break;
						}
					}
					//If i != list->count then the item has been found
					if(i == list->count)
					{
						//TODO: Remove all "parent options"
						requestList[i] = request_to_add;
						ret = TRUE;
					}
				}
			}
		}
	}
	return ret;
}

void free_request(bing_request_t request)
{
	bing_request* req;
	bundle_list* list;
	int i;
	if(request)
	{
		req = (bing_request*)request;

		if(req->data)
		{
			//Bundle, make sure data is freed
			if(!req->sourceType && hashtable_get_item(req->data, REQUEST_BUNDLE_SUBBUNDLES_STR, &list) != -1)
			{
				for(i = 0; i < list->count; i++)
				{
					free_request(list->requests[i]);
				}
				free((void*)list->requests);
				free((void*)list);
			}
			hashtable_free(req->data);
		}

		free(req);
	}
}

int request_custom_is_field_supported(bing_request_t request, const char* field)
{
	BOOL ret = FALSE;
	bing_request* req;
	if(request && field)
	{
		req = (bing_request*)request;
		ret = hashtable_key_exists(req->data, field) != -1;
	}
	return ret;
}

//TODO: request_custom_get_64bit_int
//TODO: request_custom_get_string
//TODO: request_custom_get_double

//TODO: request_custom_set_64bit_int
//TODO: request_custom_set_string
//TODO: request_custom_set_double

int request_create_custom_request(const char* source_type, bing_request_t* request, request_get_options_func get_options_func, request_finish_get_options_func get_options_done_func)
{
	bing_request* req;
	BOOL ret = FALSE;
	request_get_options_func uGet;
	request_finish_get_options_func uDone;

	//TODO: Need to do non-case sensitive source_type comp

	if(source_type && get_options_func)
	{
		if(get_options_func)
		{
			//We need to save the user functions because we will be using a different function to actually create the options string
			uGet = get_options_func;
			uDone = get_options_done_func;
			get_options_func = request_custom_get_options;
		}
		else
		{
			get_options_func = request_def_get_options;
		}
		ret = request_create(source_type, request, get_options_func, -1);
		if(ret && uGet)
		{
			//The options exist, we need to save the user options
			req = (bing_request*)(*request);
			req->uGetOptions = uGet;
			req->uFinishGetOptions = uDone;
		}
	}
	return ret;
}
