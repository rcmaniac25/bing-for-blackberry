/*
 * request.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#define REQ_VERSION "version"
#define REQ_MARKET "market"
#define REQ_ADULT "adult"
#define REQ_OPTIONS "options"
#define REQ_LATITUDE "latitude"
#define REQ_LONGITUDE "longitude"
#define REQ_LANGUAGE "language"
#define REQ_RADIUS "radius"

#define REQ_AD_PAGENUMBER "pageNumber"
#define REQ_AD_ADUNITID "adUnitId"
#define REQ_AD_PROPID "propertyId"
#define REQ_AD_CHANID "channelId"
#define REQ_AD_MLCOUNT "mlAdcount"
#define REQ_AD_SBCOUNT "sbAdCount"

#define REQ_MW_OPTIONS "mobileWebOptions"

#define REQ_NEWS_CAT "category"
#define REQ_NEWS_LOCOVER "locationOverride"

#define REQ_PHONE_LOCID "locId"

#define REQ_TRANS_SOURCE "sourceLanguage"
#define REQ_TRANS_TARGET "targetLanguage"

#define REQ_WEB_OPTIONS "webOptions"

#define REQ_MULTI_COUNT "count"
#define REQ_MULTI_OFFSET "offset"
#define REQ_MULTI_FILTERS "filters"
#define REQ_MULTI_SORTBY "sortby"
#define REQ_MULTI_FILETYPE "filetype"

static bing_field_search request_fields[] =
{
		//Universal
		{{REQUEST_FIELD_VERSION,			FIELD_TYPE_STRING,	REQ_VERSION,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[1]},
		{{REQUEST_FIELD_MARKET,				FIELD_TYPE_STRING,	REQ_MARKET,			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[2]},
		{{REQUEST_FIELD_ADULT,				FIELD_TYPE_STRING,	REQ_ADULT,			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[3]},
		{{REQUEST_FIELD_OPTIONS,			FIELD_TYPE_STRING,	REQ_OPTIONS,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[4]},
		{{REQUEST_FIELD_LATITUDE,			FIELD_TYPE_DOUBLE,	REQ_LATITUDE,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[5]},
		{{REQUEST_FIELD_LONGITUDE,			FIELD_TYPE_DOUBLE,	REQ_LONGITUDE,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[6]},
		{{REQUEST_FIELD_LANGUAGE,			FIELD_TYPE_STRING,	REQ_LANGUAGE,		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[7]},
		{{REQUEST_FIELD_RADIUS,				FIELD_TYPE_DOUBLE,	REQ_RADIUS,			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[8]},

		//Ad
		{{REQUEST_FIELD_PAGE_NUMBER,		FIELD_TYPE_LONG,	REQ_AD_PAGENUMBER,	1,	{BING_SOURCETYPE_AD}},								&request_fields[9]},
		{{REQUEST_FIELD_AD_UNIT_ID,			FIELD_TYPE_LONG,	REQ_AD_ADUNITID,	1,	{BING_SOURCETYPE_AD}},								&request_fields[10]},
		{{REQUEST_FIELD_PROPERTY_ID,		FIELD_TYPE_LONG,	REQ_AD_PROPID,		1,	{BING_SOURCETYPE_AD}},								&request_fields[11]},
		{{REQUEST_FIELD_CHANNEL_ID,			FIELD_TYPE_LONG,	REQ_AD_CHANID,		1,	{BING_SOURCETYPE_AD}},								&request_fields[12]},
		{{REQUEST_FIELD_MAINLINE_AD_COUNT,	FIELD_TYPE_LONG,	REQ_AD_MLCOUNT,		1,	{BING_SOURCETYPE_AD}},								&request_fields[13]},
		{{REQUEST_FIELD_SIDEBAR_AD_COUNT,	FIELD_TYPE_LONG,	REQ_AD_SBCOUNT,		1,	{BING_SOURCETYPE_AD}},								&request_fields[14]},

		//MobileWeb
		{{REQUEST_FIELD_MOBILE_WEB_OPTIONS,	FIELD_TYPE_STRING,	REQ_MW_OPTIONS,		1,	{BING_SOURCETYPE_MOBILE_WEB}},						&request_fields[15]},

		//News
		{{REQUEST_FIELD_CATEGORY,			FIELD_TYPE_STRING,	REQ_NEWS_CAT,		1,	{BING_SOURCETYPE_NEWS}},							&request_fields[16]},
		{{REQUEST_FIELD_LOCATION_OVERRIDE,	FIELD_TYPE_STRING,	REQ_NEWS_LOCOVER,	1,	{BING_SOURCETYPE_NEWS}},							&request_fields[17]},

		//Phonebook
		{{REQUEST_FIELD_LOC_ID,				FIELD_TYPE_STRING,	REQ_PHONE_LOCID,	1,	{BING_SOURCETYPE_PHONEBOOK}},						&request_fields[18]},

		//Translation
		{{REQUEST_FIELD_SOURCE_LANGUAGE,	FIELD_TYPE_STRING,	REQ_TRANS_SOURCE,	1,	{BING_SOURCETYPE_TRANSLATION}},						&request_fields[19]},
		{{REQUEST_FIELD_TARGET_LANGUAGE,	FIELD_TYPE_STRING,	REQ_TRANS_TARGET,	1,	{BING_SOURCETYPE_TRANSLATION}},						&request_fields[20]},

		//Web
		{{REQUEST_FIELD_WEB_OPTIONS,		FIELD_TYPE_STRING,	REQ_WEB_OPTIONS,	1,	{BING_SOURCETYPE_WEB}},								&request_fields[21]},

		//Multi (less obvious breaks in original source type)
		{{REQUEST_FIELD_COUNT,				FIELD_TYPE_LONG,	REQ_MULTI_COUNT,	6,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},								&request_fields[22]},
		{{REQUEST_FIELD_OFFSET,				FIELD_TYPE_LONG,	REQ_MULTI_OFFSET,	6,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},								&request_fields[23]},
		{{REQUEST_FIELD_FILTERS,			FIELD_TYPE_STRING,	REQ_MULTI_FILTERS,	2,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_VIDEO}},	&request_fields[24]},
		{{REQUEST_FIELD_SORT_BY,			FIELD_TYPE_STRING,	REQ_MULTI_SORTBY,	3,	{BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_VIDEO}},																									&request_fields[25]},
		{{REQUEST_FIELD_FILE_TYPE,			FIELD_TYPE_STRING,	REQ_MULTI_FILETYPE,	2,	{BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_WEB}},	NULL}
};

#define DEFAULT_ELEMENT_COUNT 8

#define APPEND(fmt, key) append_data(req->data, (fmt), (key), &data, &cursize, &ret, &retSize);

typedef struct request_source_type_s
{
	enum SOURCE_TYPE type;
	const char* source_type;
	int maxElements;
	request_get_options_func getOptions;

	struct request_source_type_s* next;
} request_source_type;

typedef struct bundle_list_s
{
	int count;
	int cap;
	bing_request_t* requests;
} bundle_list;

const char* request_def_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = calloc(1, sizeof(char));
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&Version=%s",		REQ_VERSION)
		APPEND("&Market=%s",		REQ_MARKET)
		APPEND("&Adult=%s",			REQ_ADULT)
		APPEND("&Options=%s",		REQ_OPTIONS)
		APPEND("&Latitude=%f",		REQ_LATITUDE)
		APPEND("&Longitude=%f",		REQ_LONGITUDE)
		APPEND("&UILanguage=%s",	REQ_LANGUAGE)
		APPEND("&Radius=%f",		REQ_RADIUS)

		free(data);
	}
	return ret;
}

const char* request_bundle_get_options(bing_request_t request)
{
	bing_request* req;
	bing_request* inReq;
	char* ret = NULL;
	char* rett;
	const char* options;
	bundle_list* list = NULL;
	size_t len = 0;
	int i = 0;
	if(request)
	{
		req = (bing_request*)request;

		//Get the default options
		ret = (char*)request_def_get_options(request);
		if(ret)
		{
			len = strlen(ret) + 1;

			//Go through all the bundled elements
			hashtable_get_item(req->data, REQUEST_BUNDLE_SUBBUNDLES_STR, &list);
			if(list && list->count > 0)
			{
				for(i = 0; i < list->count; i++)
				{
					inReq = (bing_request*)list->requests[i];

					//Don't allow bundle's within bundles
					if(!inReq->sourceType)
					{
						continue;
					}

					//Get the options
					options = inReq->getOptions(list->requests[i]);
					len += strlen(options);

					//Resize the return data and append the results
					rett = (char*)realloc(ret, len);
					if(rett)
					{
						strlcat(rett, options, len);
						ret = rett;
					}

					//Free the options
					free((void*)options);
				}
			}
		}
	}
	if(!ret)
	{
		ret = calloc(1, sizeof(char));
	}
	return ret;
}

const char* request_ad_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&Ad.PageNumber=%llu",	REQ_AD_PAGENUMBER)
		APPEND("&Ad.AdUnitId=%llu",		REQ_AD_ADUNITID)
		APPEND("&Ad.PropertyId=%llu",	REQ_AD_PROPID)
		APPEND("&Ad.ChannelId=%s",		REQ_AD_CHANID)
		APPEND("&Ad.MlAdcount=%llu",	REQ_AD_MLCOUNT)
		APPEND("&Ad.SbAdCount=%llu",	REQ_AD_SBCOUNT)

		free(data);
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

		APPEND("&Image.Count=%llu",		REQ_MULTI_COUNT)
		APPEND("&Image.Offset=%llu",	REQ_MULTI_OFFSET)
		APPEND("&Image.Filters=%s",		REQ_MULTI_FILTERS)

		free(data);
	}
	return ret;
}

const char* request_mw_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&MobileWeb.Count=%llu",		REQ_MULTI_COUNT)
		APPEND("&MobileWeb.Offset=%llu",	REQ_MULTI_OFFSET)
		APPEND("&MobileWeb.Options=%s",		REQ_MW_OPTIONS)

		free(data);
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

		APPEND("&News.Count=%llu",			REQ_MULTI_COUNT)
		APPEND("&News.Offset=%llu",			REQ_MULTI_OFFSET)
		APPEND("&News.Category=%s",			REQ_NEWS_CAT)
		APPEND("&News.LocationOverride=%s",	REQ_NEWS_LOCOVER)
		APPEND("&News.SortBy=%s",			REQ_MULTI_SORTBY)

		free(data);
	}
	return ret;
}

const char* request_phone_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&Phonebook.Count=%llu",		REQ_MULTI_COUNT)
		APPEND("&Phonebook.Offset=%llu",	REQ_MULTI_OFFSET)
		APPEND("&Phonebook.FileType=%s",	REQ_MULTI_FILETYPE)
		APPEND("&Phonebook.LocId=%s",		REQ_PHONE_LOCID)
		APPEND("&Phonebook.SortBy=%s",		REQ_MULTI_SORTBY)

		free(data);
	}
	return ret;
}

const char* request_transl_get_options(bing_request_t request)
{
	bing_request* req;
	char* ret = (char*)request_def_get_options(request);
	void* data = NULL;
	size_t cursize = 0;
	size_t retSize = strlen(ret) + 1;
	if(request && ret)
	{
		req = (bing_request*)request;

		APPEND("&Translation.SourceLanguage=%s",	REQ_TRANS_SOURCE)
		APPEND("&Translation.TargetLanguage=%s",	REQ_TRANS_TARGET)

		free(data);
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

		APPEND("&Video.Count=%llu",		REQ_MULTI_COUNT)
		APPEND("&Video.Offset=%llu",	REQ_MULTI_OFFSET)
		APPEND("&Video.Filters=%s",		REQ_MULTI_FILTERS)
		APPEND("&Video.FileType=%s",	REQ_MULTI_SORTBY)

		free(data);
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

		APPEND("&Web.Count=%llu",	REQ_MULTI_COUNT)
		APPEND("&Web.Offset=%llu",	REQ_MULTI_OFFSET)
		APPEND("&Web.FileType=%s",	REQ_MULTI_FILETYPE)
		APPEND("&Web.Options=%s",	REQ_WEB_OPTIONS)

		free(data);
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

		resultOptions = calloc(size = strlen(ret) + strlen(custOptions) + 2, sizeof(char));
		if(resultOptions)
		{
			//First copy in the default options
			memcpy(resultOptions, ret, strlen(ret));

			//Free the return string
			free((void*)ret);

			//Conc. the user options
			strlcat(resultOptions, custOptions, size);

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

static request_source_type request_source_types[] =
{
		{BING_SOURCETYPE_AD,				"ad",				DEFAULT_ELEMENT_COUNT + 6,	request_ad_get_options,		&request_source_types[1]},
		{BING_SOURCETYPE_IMAGE,				"image",			DEFAULT_ELEMENT_COUNT + 3,	request_image_get_options,	&request_source_types[2]},
		{BING_SOURCETYPE_INSTANT_ANWSER,	"instantAnswer",	DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[3]},
		{BING_SOURCETYPE_MOBILE_WEB,		"mobileWeb",		DEFAULT_ELEMENT_COUNT + 3,	request_mw_get_options,		&request_source_types[4]},
		{BING_SOURCETYPE_NEWS,				"news",				DEFAULT_ELEMENT_COUNT + 5,	request_news_get_options,	&request_source_types[5]},
		{BING_SOURCETYPE_PHONEBOOK,			"phonebook",		DEFAULT_ELEMENT_COUNT + 5,	request_phone_get_options,	&request_source_types[6]},
		{BING_SOURCETYPE_RELATED_SEARCH,	"relatedSearch",	DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[7]},
		{BING_SOURCETYPE_SPELL,				"spell",			DEFAULT_ELEMENT_COUNT,		request_def_get_options,	&request_source_types[8]},
		{BING_SOURCETYPE_TRANSLATION,		"translation",		DEFAULT_ELEMENT_COUNT + 2,	request_transl_get_options,	&request_source_types[9]},
		{BING_SOURCETYPE_VIDEO,				"video",			DEFAULT_ELEMENT_COUNT + 4,	request_video_get_options,	&request_source_types[10]},
		{BING_SOURCETYPE_WEB,				"web",				DEFAULT_ELEMENT_COUNT + 4,	request_web_get_options,	NULL}
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
	char buffer[256];
	char* result = calloc(1, sizeof(char));
	size_t len = 1;
	bundle_list* list = NULL;
	int i;
	char* src;
	if(bundle && result)
	{
		//Get the list
		hashtable_get_item(bundle->data, REQUEST_BUNDLE_SUBBUNDLES_STR, &list);
		if(list && list->count > 0)
		{
			//Go through elements and get data
			for(i = 0; i < list->count; i++)
			{
				src = (char*)((bing_request*)list->requests[i])->sourceType;
				if(!src)
				{
					//We don't want bundle types within bundle types
					continue;
				}
				//Get the source type in the proper format
				if(i == 0)
				{
					strlcpy(buffer, src, 256);
				}
				else
				{
					snprintf(buffer, 256, "+%s", src);
					buffer[255] = '\0';
				}
				//Get the length and resize the string
				len += strlen(buffer);
				src = (char*)realloc(result, len);
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
				//Add default values
				hashtable_put_item(req->data, REQ_VERSION, DEFAULT_API_VERSION, strlen(DEFAULT_API_VERSION) + 1);
				hashtable_put_item(req->data, REQ_MARKET, DEFAULT_SEARCH_MARKET, strlen(DEFAULT_SEARCH_MARKET) + 1);

				//Save request
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

int request_get_data_key(bing_request_t request, const char* key, void* value, size_t size)
{
	BOOL ret = FALSE;
	if(request && value)
	{
		//Now get the data (we want to check sizes first so we don't copy something bigger into something smaller)
		if(hashtable_get_item(((bing_request*)request)->data, key, NULL) == size)
		{
			hashtable_get_item(((bing_request*)request)->data, key, value);
			ret = TRUE;
		}
	}
	return ret;
}

int request_get_data(bing_request_t request, enum REQUEST_FIELD field, enum FIELD_TYPE type, void* value, size_t size)
{
	BOOL ret = FALSE;
	const char* key;
	if(request && value)
	{
		//Get the key
		key = find_field(request_fields, field, type, request_get_source_type(request), TRUE);

		//Now get the data
		ret = request_get_data_key(request, key, value, size);
	}
	return ret;
}

int request_get_str_data(bing_request_t request, enum REQUEST_FIELD field, enum FIELD_TYPE type, char* value)
{
	int ret = -1;
	const char* key;
	if(request && value)
	{
		//Get the key
		key = find_field(request_fields, field, type, request_get_source_type(request), TRUE);

		//Now get the data
		ret = request_custom_get_string(request, key, value);
	}
	return ret;
}

int request_get_64bit_int(bing_request_t request, enum REQUEST_FIELD field, long long* value)
{
	return request_get_data(request, field, FIELD_TYPE_LONG, value, sizeof(long long));
}

int request_get_string(bing_request_t request, enum REQUEST_FIELD field, char* value)
{
	return request_get_str_data(request, field, FIELD_TYPE_STRING, value);
}

int request_get_double(bing_request_t request, enum REQUEST_FIELD field, double* value)
{
	return request_get_data(request, field, FIELD_TYPE_DOUBLE, value, sizeof(double));
}

void request_remove_parent_options(bing_request* request)
{
	hashtable_remove_item(request->data, REQ_VERSION);
	hashtable_remove_item(request->data, REQ_MARKET);
	hashtable_remove_item(request->data, REQ_ADULT);
	hashtable_remove_item(request->data, REQ_OPTIONS);
	hashtable_remove_item(request->data, REQ_LATITUDE);
	hashtable_remove_item(request->data, REQ_LONGITUDE);
	hashtable_remove_item(request->data, REQ_LANGUAGE);
	hashtable_remove_item(request->data, REQ_RADIUS);
}

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
						request_remove_parent_options((bing_request*)request_to_add);
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
	if(request && field)
	{
		ret = hashtable_key_exists(((bing_request*)request)->data, field) != -1;
	}
	return ret;
}

int request_custom_get_64bit_int(bing_request_t request, const char* field, long long* value)
{
	return request_get_data_key(request, field, value, sizeof(long long));
}

int request_custom_get_string(bing_request_t request, const char* field, char* value)
{
	int ret = -1;
	if(request)
	{
		//Now get the data
		ret = hashtable_get_item(((bing_request*)request)->data, field, value);
	}
	return ret;
}

int request_custom_get_double(bing_request_t request, const char* field, double* value)
{
	return request_get_data_key(request, field, value, sizeof(double));
}

int request_custom_set_64bit_int(bing_request_t request, const char* field, long long* value)
{
	BOOL ret = FALSE;
	bing_request* req;
	if(request && field)
	{
		req = (bing_request*)request;
		if(!value && hashtable_get_item(req->data, field, NULL) != -1)
		{
			hashtable_remove_item(req->data, field);
		}
		else if(value)
		{
			if(hashtable_put_item(req->data, field, value, sizeof(long long)) != -1)
			{
				ret =  TRUE;
			}
		}
	}
	return ret;
}

int request_custom_set_string(bing_request_t request, const char* field, const char* value)
{
	BOOL ret = FALSE;
	bing_request* req;
	if(request && field)
	{
		req = (bing_request*)request;
		if(!value && hashtable_get_item(req->data, field, NULL) != -1)
		{
			hashtable_remove_item(req->data, field);
		}
		else if(value)
		{
			if(hashtable_put_item(req->data, field, value, strlen(value) + 1) != -1)
			{
				ret =  TRUE;
			}
		}
	}
	return ret;
}

int request_custom_set_double(bing_request_t request, const char* field, double* value)
{
	//This needs to be true in order for this function to work
	assert(sizeof(double) == sizeof(long long));
	return request_custom_set_64bit_int(request, field, (long long*)value);
}

int request_create_custom_request(const char* source_type, bing_request_t* request, request_get_options_func get_options_func, request_finish_get_options_func get_options_done_func)
{
	bing_request* req;
	BOOL ret = FALSE;
	request_get_options_func uGet;
	request_finish_get_options_func uDone;
	request_source_type* type;

	//Only do the comparison if a source type exists, otherwise it will fail regardless when request creation occurs.
	if(source_type)
	{
		for(type = request_source_types; type; type = type->next)
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
