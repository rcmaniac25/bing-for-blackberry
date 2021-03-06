/*
 * request.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#define REQ_MAX_TOTAL "maxTotal"
#define REQ_OFFSET "offset"
#define REQ_MARKET "market"
#define REQ_ADULT "adult"
#define REQ_OPTIONS "options"
#define REQ_LATITUDE "latitude"
#define REQ_LONGITUDE "longitude"

#define REQ_IMAGE_FILTERS "filters"

#define REQ_NEWS_CAT "category"
#define REQ_NEWS_LOCOVER "locationOverride"
#define REQ_NEWS_SORTBY "sortby"

#define REQ_WEB_FILETYPE "filetype"
#define REQ_WEB_OPTIONS "weboptions"

//If a field is marked as BING_FIELD_SUPPORT_ALL_FIELDS, it will be removed when added to a composite request and/or blocked on that request afterwards
static bing_field_search request_fields[] =
{
		//Universal
		{{BING_REQUEST_FIELD_MAX_TOTAL,				FIELD_TYPE_LONG,	REQ_MAX_TOTAL,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[1]},
		{{BING_REQUEST_FIELD_OFFSET,				FIELD_TYPE_LONG,	REQ_OFFSET,			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[2]},
		{{BING_REQUEST_FIELD_MARKET,				FIELD_TYPE_STRING,	REQ_MARKET,			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[3]},
		{{BING_REQUEST_FIELD_ADULT,					FIELD_TYPE_STRING,	REQ_ADULT,			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[4]},
		{{BING_REQUEST_FIELD_OPTIONS,				FIELD_TYPE_STRING,	REQ_OPTIONS,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[5]},
		{{BING_REQUEST_FIELD_LATITUDE,				FIELD_TYPE_DOUBLE,	REQ_LATITUDE,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[6]},
		{{BING_REQUEST_FIELD_LONGITUDE,				FIELD_TYPE_DOUBLE,	REQ_LONGITUDE,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[7]},

		//Image
		{{BING_REQUEST_FIELD_FILTERS,				FIELD_TYPE_STRING,	REQ_IMAGE_FILTERS,	2,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_VIDEO}},	&request_fields[8]},

		//News
		{{BING_REQUEST_FIELD_CATEGORY,				FIELD_TYPE_STRING,	REQ_NEWS_CAT,		1,	{BING_SOURCETYPE_NEWS}},							&request_fields[9]},
		{{BING_REQUEST_FIELD_LOCATION_OVERRIDE,		FIELD_TYPE_STRING,	REQ_NEWS_LOCOVER,	1,	{BING_SOURCETYPE_NEWS}},							&request_fields[10]},
		{{BING_REQUEST_FIELD_SORT_BY,				FIELD_TYPE_STRING,	REQ_NEWS_SORTBY,	2,	{BING_SOURCETYPE_NEWS, BING_SOURCETYPE_VIDEO}},		&request_fields[11]},

		//Web
		{{BING_REQUEST_FIELD_FILE_TYPE,				FIELD_TYPE_STRING,	REQ_WEB_FILETYPE,	1,	{BING_SOURCETYPE_WEB}},								&request_fields[12]},
		{{BING_REQUEST_FIELD_WEB_OPTIONS,			FIELD_TYPE_STRING,	REQ_WEB_OPTIONS,	1,	{BING_SOURCETYPE_WEB}},								NULL}
};

#define DEFAULT_ELEMENT_COUNT 5

#define APPEND(fmt, key) append_data(req->data, (fmt), (key), &data, &cursize, &ret, &retSize);

typedef struct request_source_type_s
{
	enum BING_SOURCE_TYPE type;
	const char* source_type;
	const char* composite_source_type;
	int maxElements;
	request_get_options_func getOptions;

	struct request_source_type_s* next;
} request_source_type;

const char* request_def_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = bing_mem_calloc(1, sizeof(char));
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&$top=%lld",			REQ_MAX_TOTAL)
		APPEND("&$skip=%lld",			REQ_OFFSET)

		APPEND("&Market=%%27%s%%27",	REQ_MARKET)
		APPEND("&Adult=%%27%s%%27",		REQ_ADULT)
		APPEND("&Options=%%27%s%%27",	REQ_OPTIONS)
		APPEND("&Latitude=%f",			REQ_LATITUDE)
		APPEND("&Longitude=%f",			REQ_LONGITUDE)

		bing_mem_free(data);
	}
	return ret;
}

const char* request_composite_get_options(bing_request_t request)
{
	bing_request* req;
	bing_request* inReq;
	char* ret = NULL;
	char* rett;
	const char* options;
	list* list = NULL;
	size_t len = 0;
	unsigned int i = 0;
	if(request)
	{
		req = (bing_request*)request;

		//Get the default options
		ret = (char*)request_def_get_options(request);
		if(ret)
		{
			len = strlen(ret) + 1;

			//Go through all the composite elements
			hashtable_get_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list);
			if(list && list->count > 0)
			{
				for(i = 0; i < list->count; i++)
				{
					inReq = LIST_ELEMENT(list, i, bing_request*);

					//Don't allow composite's within composites
					if(!inReq->sourceType)
					{
						continue;
					}

					//Get the options
					options = inReq->getOptions(LIST_ELEMENT(list, i, bing_request_t));
					len += strlen(options);

					//Resize the return data and append the results
					rett = (char*)bing_mem_realloc(ret, len);
					if(rett)
					{
						strlcat(rett, options, len);
						ret = rett;
					}

					//Free the options
					bing_mem_free((void*)options);
				}
			}
		}
	}
	if(!ret)
	{
		ret = (char*)bing_mem_calloc(1, sizeof(char));
	}
	return ret;
}

const char* request_image_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&ImageFilters=%%27%s%%27",	REQ_IMAGE_FILTERS)

		bing_mem_free(data);
	}
	return ret;
}

const char* request_news_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&NewsCategory=%%27%s%%27",			REQ_NEWS_CAT)
		APPEND("&NewsLocationOverride=%%27%s%%27",	REQ_NEWS_LOCOVER)
		APPEND("&NewsSortBy=%%27%s%%27",			REQ_NEWS_SORTBY)

		bing_mem_free(data);
	}
	return ret;
}

const char* request_video_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&VideoFilters=%%27%s%%27",	REQ_IMAGE_FILTERS)
		APPEND("&VideoSortBy=%%27%s%%27",	REQ_NEWS_SORTBY)

		bing_mem_free(data);
	}
	return ret;
}

const char* request_web_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&WebFileType=%%27%s%%27",		REQ_WEB_FILETYPE)
		APPEND("&WebSearchOptions=%%27%s%%27",	REQ_WEB_OPTIONS)

		bing_mem_free(data);
	}
	return ret;
}

const char* request_custom_get_options(bing_request_t request)
{
	bing_request* req;
	const char* ret = NULL;
	const char* custOptions;
	char* resultOptions;
	size_t size;
	//Request exists, create the options string
	if(request)
	{
		req = (bing_request*)request;

		ret = request_def_get_options(request);
		custOptions = req->uGetOptions(request);

		resultOptions = bing_mem_calloc(size = strlen(ret) + strlen(custOptions) + 1, sizeof(char));
		if(resultOptions)
		{
			//First copy in the default options
			memcpy(resultOptions, ret, strlen(ret));

			//Free the return string
			bing_mem_free((void*)ret);

			//Conc. the user options
			strlcat(resultOptions, custOptions, size);

			//Set the return
			ret = resultOptions;
		}
		else
		{
			bing_mem_free((void*)ret);
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
		ret = bing_mem_calloc(1, sizeof(char));
	}
	return ret;
}

static request_source_type request_source_types[] =
{
		{BING_SOURCETYPE_IMAGE,				"Image",				"image",			DEFAULT_ELEMENT_COUNT + 1,	request_image_get_options,	&request_source_types[1]},
		{BING_SOURCETYPE_NEWS,				"News",					"news",				DEFAULT_ELEMENT_COUNT + 3,	request_news_get_options,	&request_source_types[2]},
		{BING_SOURCETYPE_RELATED_SEARCH,	"RelatedSearch",		"relatedsearch",	DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[3]},
		{BING_SOURCETYPE_SPELL,				"SpellingSuggestions",	"spell",			DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[4]},
		{BING_SOURCETYPE_VIDEO,				"Video",				"video",			DEFAULT_ELEMENT_COUNT + 2,	request_video_get_options,	&request_source_types[5]},
		{BING_SOURCETYPE_WEB,				"Web",					"web",				DEFAULT_ELEMENT_COUNT + 2,	request_web_get_options,	NULL}
};

enum BING_SOURCE_TYPE bing_request_get_source_type(bing_request_t request)
{
	enum BING_SOURCE_TYPE t = BING_SOURCETYPE_UNKNOWN;
	bing_request* req;
	request_source_type* type;
	if(request)
	{
		req = (bing_request*)request;
		if(req->custom)
		{
			t = BING_SOURCETYPE_CUSTOM;
		}
		else if(req->sourceType)
		{
			for(type = request_source_types; type != NULL; type = type->next)
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
			t = BING_SOURCETYPE_COMPOSITE;
		}
	}
	return t;
}

const char* request_get_composite_sourcetype(bing_request* composite)
{
#define BUFFER_SIZE 256

	char buffer[BUFFER_SIZE];
	char* result = bing_mem_calloc(1, sizeof(char));
	size_t len = 1;
	list* list = NULL;
	unsigned int i;
	char* src;
	if(composite && result)
	{
		//Get the list
		hashtable_get_item(composite->data, REQUEST_COMPOSITE_SUBREQ_STR, &list);
		if(list && list->count > 0)
		{
			//Go through elements and get data
			for(i = 0; i < list->count; i++)
			{
				src = (char*)LIST_ELEMENT(list, i, bing_request*)->compositeSourceType;
				if(!src)
				{
					//We don't want composite types within composite types
					continue;
				}
				//Get the source type in the proper format
				if(i == 0)
				{
					strlcpy(buffer, src, BUFFER_SIZE);
				}
				else
				{
					snprintf(buffer, BUFFER_SIZE, "%%2b%s", src);
					buffer[BUFFER_SIZE - 1] = '\0';
				}
				//Get the length and resize the string
				len += strlen(buffer);
				src = (char*)bing_mem_realloc(result, len);
				if(src)
				{
					//Append the source type
					strlcat(src, buffer, len);
					result = src;
				}
			}
		}
	}
	return result;

#undef BUFFER_SIZE
}

int request_create(const char* source_type, const char* composite_type, BOOL custom, bing_request_t* request, request_get_options_func get_options_func, int tableSize)
{
	BOOL ret = FALSE;
	bing_request* req;
	if(request)
	{
		req = (bing_request*)bing_mem_malloc(sizeof(bing_request));
		if(req)
		{
			if(custom)
			{
				if(source_type)
				{
					req->sourceType = bing_mem_strdup(source_type);
					if(!req->sourceType)
					{
						//Error, cleanup
						bing_mem_free((void*)req);
						req = NULL;
					}
				}
				if(req && composite_type)
				{
					req->compositeSourceType = bing_mem_strdup(composite_type);
					if(!req->compositeSourceType)
					{
						//Error, cleanup
						if(req->sourceType)
						{
							bing_mem_free((void*)req->sourceType);
						}
						bing_mem_free((void*)req);
						req = NULL;
					}
				}
			}
			else
			{
				req->sourceType = source_type;
				req->compositeSourceType = composite_type;
			}

			//Only continue if the request still exists
			if(req)
			{
				req->custom = custom;
				req->getOptions = get_options_func;
				req->data = NULL;
				req->compositeUse = 0;

				req->data = hashtable_create(tableSize);
				if(req->data)
				{
					//Save request
					*request = req;
					ret = TRUE;
				}
				else
				{
					bing_mem_free((void*)req);
				}
			}
		}
	}
	return ret;
}

int bing_request_create(enum BING_SOURCE_TYPE source_type, bing_request_t* request)
{
	BOOL ret = FALSE;
	int tableSize;
	request_source_type* type;
	const char* sourceT;
	const char* compositeT;
	request_get_options_func getOFun;
	if((source_type >= BING_SOURCETYPE_IMAGE && source_type <= BING_SOURCETYPE_WEB) || source_type == BING_SOURCETYPE_COMPOSITE) //This guarantees that the source_type will be a valid type
	{
		tableSize = -1;
		sourceT = NULL;
		compositeT = NULL;
		getOFun = NULL;

		if(source_type == BING_SOURCETYPE_COMPOSITE)
		{
			sourceT = NULL;
			compositeT = NULL;
			tableSize = DEFAULT_ELEMENT_COUNT + 1;
			getOFun = request_composite_get_options;
		}
		else
		{
			for(type = request_source_types; type != NULL; type = type->next)
			{
				if(type->type == source_type)
				{
					sourceT = type->source_type;
					compositeT = type->composite_source_type;
					tableSize = type->maxElements;
					getOFun = type->getOptions;
					break;
				}
			}
		}
		ret = request_create(sourceT, compositeT, FALSE, request, getOFun, tableSize);
	}
	return ret;
}

int bing_request_is_field_supported(bing_request_t request, enum BING_REQUEST_FIELD field)
{
	BOOL ret = FALSE;
	bing_request* req;
	const char* key;
	if(request)
	{
		//Get the key
		req = (bing_request*)request;
		key = find_field(request_fields, field, FIELD_TYPE_UNKNOWN, bing_request_get_source_type(request), FALSE);

		//Determine if the key is supported
		ret = bing_request_custom_is_field_supported(request, key);
	}
	return ret;
}

int request_get_data(bing_request_t request, enum BING_REQUEST_FIELD field, enum FIELD_TYPE type, void* value, size_t size)
{
	BOOL ret = FALSE;
	const char* key;
	if(request && value)
	{
		//Get the key
		key = find_field(request_fields, field, type, bing_request_get_source_type(request), TRUE);

		//Now get the data
		ret = hashtable_get_data_key(((bing_request*)request)->data, key, value, size);
	}
	return ret;
}

int request_set_data(bing_request_t request, enum BING_REQUEST_FIELD field, enum FIELD_TYPE type, const void* value, size_t size)
{
	BOOL ret = FALSE;
	const char* key;
	if(request && value)
	{
		//Get the key
		key = find_field(request_fields, field, type, bing_request_get_source_type(request), TRUE);

		//Now set the data
		ret = hashtable_set_data(((bing_request*)request)->data, key, value, size);
	}
	return ret;
}

int request_get_str_data(bing_request_t request, enum BING_REQUEST_FIELD field, enum FIELD_TYPE type, char* value)
{
	int ret = -1;
	const char* key;
	if(request)
	{
		//Get the key
		key = find_field(request_fields, field, type, bing_request_get_source_type(request), TRUE);

		//Now get the data
		ret = bing_request_custom_get_string(request, key, value);
	}
	return ret;
}

int request_set_str_data(bing_request_t request, enum BING_REQUEST_FIELD field, enum FIELD_TYPE type, const char* value)
{
	int ret = -1;
	const char* key;
	if(request)
	{
		//Get the key
		key = find_field(request_fields, field, type, bing_request_get_source_type(request), TRUE);

		//Now set the data
		ret = bing_request_custom_set_string(request, key, value);
	}
	return ret;
}

int bing_request_get_32bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, int* value)
{
	return request_get_data(request, field, FIELD_TYPE_INT, value, sizeof(int));
}

int bing_request_get_64bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, long long* value)
{
	return request_get_data(request, field, FIELD_TYPE_LONG, value, sizeof(long long));
}

int bing_request_get_string(bing_request_t request, enum BING_REQUEST_FIELD field, char* value)
{
	return request_get_str_data(request, field, FIELD_TYPE_STRING, value);
}

int bing_request_get_double(bing_request_t request, enum BING_REQUEST_FIELD field, double* value)
{
	return request_get_data(request, field, FIELD_TYPE_DOUBLE, value, sizeof(double));
}

int bing_request_set_32bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, int value)
{
	return bing_request_set_p_32bit_int(request, field, &value);
}

int bing_request_set_64bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, long long value)
{
	return bing_request_set_p_64bit_int(request, field, &value);
}

int bing_request_set_double(bing_request_t request, enum BING_REQUEST_FIELD field, double value)
{
	return bing_request_set_p_double(request, field, &value);
}

int bing_request_set_p_32bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, const int* value)
{
	return request_set_data(request, field, FIELD_TYPE_INT, value, sizeof(int));
}

int bing_request_set_p_64bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, const long long* value)
{
	return request_set_data(request, field, FIELD_TYPE_LONG, value, sizeof(long long));
}

int bing_request_set_string(bing_request_t request, enum BING_REQUEST_FIELD field, const char* value)
{
	return request_set_str_data(request, field, FIELD_TYPE_STRING, value);
}

int bing_request_set_p_double(bing_request_t request, enum BING_REQUEST_FIELD field, const double* value)
{
	return request_set_data(request, field, FIELD_TYPE_DOUBLE, value, sizeof(double));
}

void request_remove_parent_options(bing_request* request)
{
	bing_field_search* searchField;

	//Simply go through the fields and remove the "global" ones. We can do this here since we are using the generic search field
	for(searchField = request_fields; searchField != NULL; searchField = searchField->next)
	{
		if(searchField->field.sourceTypeCount == BING_FIELD_SUPPORT_ALL_FIELDS)
		{
			hashtable_remove_item(request->data, searchField->field.name);
		}
	}
}

int bing_request_composite_add_request(bing_request_t request, bing_request_t request_to_add)
{
	BOOL ret = FALSE;
	bing_request* req;
	list* list_v = NULL;
	bing_request_t* requestList = NULL;
	unsigned int i;

	if(request && request_to_add && //Make sure requests exist
			request != request_to_add && //Make sure we aren't trying to add a request to itself
			((bing_request*)request_to_add)->sourceType) //Make sure we aren't trying to add a composite to another composite
	{
		req = (bing_request*)request;

		if(!req->sourceType) //Composite source type is null
		{
			if(hashtable_get_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list_v) > 0)
			{
				//Get the list
				requestList = LIST_ELEMENTS(list_v, bing_request_t);
			}
			else
			{
				//Create the list
				list_v = (list*)bing_mem_malloc(sizeof(list));
				if(list_v)
				{
					list_v->count = 0;
					requestList = list_v->listElements = (bing_request_t*)bing_mem_calloc(BING_SOURCETYPE_COMPOSITE_COUNT, sizeof(bing_request_t));
					if(list_v->listElements)
					{
						//Save the list
						list_v->cap = BING_SOURCETYPE_COMPOSITE_COUNT;
						if(!hashtable_put_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list_v, sizeof(list*)))
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
				//See if the request already exists in the list
				for(i = 0; i < list_v->count; i++)
				{
					if(requestList[i] == request_to_add) //XXX Should thus check source type too?
					{
						break;
					}
				}
				//If i != list->count then the item has been found
				if(i == list_v->count)
				{
					if(list_v->count >= list_v->cap)
					{
						//Resize list
						requestList = (bing_request_t*)bing_mem_realloc(requestList, sizeof(bing_request_t) * (list_v->cap * 2));
						if(requestList)
						{
							list_v->cap *= 2;
							list_v->listElements = requestList;
						}
						else
						{
							requestList = NULL;
						}
					}

					if(requestList)
					{
						//Add to list
						request_remove_parent_options((bing_request*)request_to_add);
						((bing_request*)request_to_add)->compositeUse++;
						requestList[list_v->count++] = request_to_add;
						ret = TRUE;
					}
				}
			}
		}
	}
	return ret;
}

int bing_request_composite_remove_request(bing_request_t request, bing_request_t request_to_remove)
{
	BOOL ret = FALSE;
	bing_request* req;
	list* list_v = NULL;
	int i;
	if(request && request_to_remove && ((bing_request*)request_to_remove)->compositeUse > 0)
	{
		req = (bing_request*)request;
		if(!req->sourceType && hashtable_get_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list_v) > 0)
		{
			//Find the request...
			for(i = 0; i < list_v->count; i++)
			{
				if(LIST_ELEMENT(list_v, i, bing_request_t) == request_to_remove)
				{
					//...then use an already existing function to remove it. It duplicates some code, but makes up for the rest of it
					ret = bing_request_composite_remove_request_at_index(request, i) != NULL;
					break;
				}
			}
		}
	}
	return ret;
}

bing_request_t bing_request_composite_remove_request_at_index(bing_request_t request, int index)
{
	bing_request_t ret = NULL;
	bing_request* req;
	bing_request_t* requestList;
	list* list_v = NULL;
	if(request)
	{
		req = (bing_request*)request;
		if(!req->sourceType &&
				hashtable_get_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list_v) > 0 &&
				(index >= 0 && index < list_v->count))
		{
			//Get the list
			requestList = LIST_ELEMENTS(list_v, bing_request_t);

			//Get the request
			LIST_ELEMENT(list_v, index, bing_request*)->compositeUse--;
			ret = LIST_ELEMENT(list_v, index, bing_request_t);

			//Move the previous results
			memmove(requestList + index, requestList + (index + 1), (list_v->count - index - 1) * sizeof(bing_request_t));

			//Reset moved area of list
			LIST_ELEMENT(list_v, --list_v->count, bing_request_t) = NULL;
		}
	}
	return ret;
}

int bing_request_get_composite_requests(bing_request_t request, bing_request_t* requests)
{
	int ret = -1;
	bing_request* req;
	bing_request_t* requestList;
	list* list_v = NULL;
	if(request)
	{
		req = (bing_request*)request;
		if(!req->sourceType)
		{
			if(hashtable_get_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list_v) > 0)
			{
				ret = list_v->count;
				if(requests)
				{
					//Get the list
					requestList = LIST_ELEMENTS(list_v, bing_request_t);

					//Copy the data
					memcpy(requestList, requests, ret * sizeof(bing_request_t));
				}
			}
		}
	}
	return ret;
}

int bing_request_composite_count(bing_request_t request)
{
	bing_request* req;
	list* list_v;

	if(request)
	{
		req = (bing_request*)request;
		if(!req->sourceType) //Composite source type is null
		{
			//If we have a request list, return the count. Otherwise return 0
			if(hashtable_get_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list_v) > 0)
			{
				return list_v->count;
			}
			return 0;
		}
	}
	return -1;
}

int bing_request_is_part_of_composite(bing_request_t request)
{
	if(request)
	{
		return ((bing_request*)request)->compositeUse != 0;
	}
	return FALSE;
}

int bing_request_free(bing_request_t request)
{
	BOOL ret = FALSE;
	bing_request* req;
	list* list;
	unsigned int i;
	if(request)
	{
		req = (bing_request*)request;
		if(req->compositeUse == 0)
		{
			if(req->custom)
			{
				bing_mem_free((void*)req->sourceType);
				bing_mem_free((void*)req->compositeSourceType);
			}

			if(req->data)
			{
				//Composite, make sure data is freed
				if(!req->sourceType && hashtable_get_item(req->data, REQUEST_COMPOSITE_SUBREQ_STR, &list) > 0)
				{
					for(i = 0; i < list->count; i++)
					{
						LIST_ELEMENT(list, i, bing_request*)->compositeUse--; //Deincrement composite use so it will be freed if this is the last composite request to use it
						bing_request_free(LIST_ELEMENT(list, i, bing_request_t));
					}
					bing_mem_free((void*)list->listElements);
					bing_mem_free((void*)list);
				}
				hashtable_free(req->data);
			}
			req->data = NULL;

			bing_mem_free(req);

			ret = TRUE;
		}
	}
	return ret;
}

int bing_request_custom_is_field_supported(bing_request_t request, const char* field)
{
	bing_field_search* searchField;
	request_source_type* reqSourceType;
	bing_request* req;
	enum BING_SOURCE_TYPE stype = BING_SOURCETYPE_UNKNOWN;
	int i;

	if(request && field)
	{
		req = (bing_request*)request;
		if(req->custom)
		{
			//Anything is possible with custom
			return TRUE;
		}

		//Predetermined type, check field
		for(searchField = request_fields; searchField != NULL; searchField = searchField->next)
		{
			//First check to see if this is a global field
			if(searchField->field.sourceTypeCount == BING_FIELD_SUPPORT_ALL_FIELDS && strcmp(searchField->field.name, field) == 0)
			{
				return TRUE;
			}
			else
			{
				//First we need to find the source type
				if(req->sourceType == NULL)
				{
					stype = BING_SOURCETYPE_COMPOSITE;
				}
				else
				{
					for(reqSourceType = request_source_types; reqSourceType != NULL; reqSourceType = reqSourceType->next)
					{
						//For requests that are not custom, the source type is simply referenced. So we can just do a comparison instead of needing to strcmp the two types
						if(reqSourceType->source_type == req->sourceType)
						{
							stype = reqSourceType->type;
							break;
						}
					}
				}

				if(stype != BING_SOURCETYPE_UNKNOWN)
				{
					//Now we need to check types
					for(i = 0; i < searchField->field.sourceTypeCount; i++)
					{
						if(searchField->field.supportedTypes[i] == stype)
						{
							//Type matches, now check name
							if(strcmp(searchField->field.name, field) == 0)
							{
								return TRUE;
							}
							break;
						}
					}
				}
			}
		}
	}
	return FALSE;
}

int bing_request_custom_does_field_exist(bing_request_t request, const char* field)
{
	BOOL ret = FALSE;
	if(request && field)
	{
		ret = hashtable_key_exists(((bing_request*)request)->data, field);
	}
	return ret;
}

BOOL canSetField(bing_request_t request, const char* field)
{
	bing_request* req;
	bing_field_search* searchField;

	if(request && field)
	{
		req = (bing_request*)request;
		if(req->compositeUse != 0) //Only if the request is not set to any composite can any field be set
		{
			for(searchField = request_fields; searchField != NULL; searchField = searchField->next)
			{
				if(searchField->field.sourceTypeCount == BING_FIELD_SUPPORT_ALL_FIELDS && //If it's a global field...
						strcmp(searchField->field.name, field) == 0) //...and has the same name, don't allow it to continue.
				{
					return FALSE;
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

int bing_request_custom_get_32bit_int(bing_request_t request, const char* field, int* value)
{
	return hashtable_get_data_key(request ? ((bing_request*)request)->data : NULL, field, value, sizeof(int));
}

int bing_request_custom_get_64bit_int(bing_request_t request, const char* field, long long* value)
{
	return hashtable_get_data_key(request ? ((bing_request*)request)->data : NULL, field, value, sizeof(long long));
}

int bing_request_custom_get_string(bing_request_t request, const char* field, char* value)
{
	return hashtable_get_string(request ? ((bing_request*)request)->data : NULL, field, value);
}

int bing_request_custom_get_double(bing_request_t request, const char* field, double* value)
{
	return hashtable_get_data_key(request ? ((bing_request*)request)->data : NULL, field, value, sizeof(double));
}

int bing_request_custom_set_32bit_int(bing_request_t request, const char* field, int value)
{
	return bing_request_custom_set_p_32bit_int(request, field, &value);
}

int bing_request_custom_set_64bit_int(bing_request_t request, const char* field, long long value)
{
	return bing_request_custom_set_p_64bit_int(request, field, &value);
}

int bing_request_custom_set_double(bing_request_t request, const char* field, double value)
{
	return bing_request_custom_set_p_double(request, field, &value);
}

int bing_request_custom_set_p_32bit_int(bing_request_t request, const char* field, const int* value)
{
	if(canSetField(request, field))
	{
		return hashtable_set_data(((bing_request*)request)->data, field, value, sizeof(int));
	}
	return FALSE;
}

int bing_request_custom_set_p_64bit_int(bing_request_t request, const char* field, const long long* value)
{
	if(canSetField(request, field))
	{
		return hashtable_set_data(((bing_request*)request)->data, field, value, sizeof(long long));
	}
	return FALSE;
}

int bing_request_custom_set_string(bing_request_t request, const char* field, const char* value)
{
	if(canSetField(request, field))
	{
		return hashtable_set_data(((bing_request*)request)->data, field, value, value ? (strlen(value) + 1) : 0);
	}
	return FALSE;
}

int bing_request_custom_set_p_double(bing_request_t request, const char* field, const double* value)
{
	return bing_request_custom_set_p_64bit_int(request, field, (long long*)value);
}

//TODO: Add composite request field
int bing_request_create_custom_request(const char* source_type, bing_request_t* request, request_get_options_func get_options_func, request_finish_get_options_func get_options_done_func)
{
	bing_request* req;
	BOOL ret = FALSE;
	request_get_options_func uGet;
	request_finish_get_options_func uDone;
	request_source_type* type;

	uGet = NULL;

	//Only do the comparison if a source type exists, otherwise it will fail regardless when request creation occurs.
	if(source_type)
	{
		for(type = request_source_types; type != NULL; type = type->next)
		{
			if(stricmp(source_type, type->source_type) == 0)
			{
				//Source types can't match built in types, fail the function
				source_type = NULL;
				break;
			}
		}
	}

	if(source_type)
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
		ret = request_create(source_type, NULL, TRUE, request, get_options_func, -1);
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
