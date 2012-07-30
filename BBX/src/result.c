/*
 * result.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

//Result names
#define RES_IMAGE_HEIGHT "d:Height"
#define RES_IMAGE_WIDTH "d:Width"
#define RES_IMAGE_FILESIZE "d:FileSize"
#define RES_IMAGE_MEDIA_URL "d:MediaUrl"
#define RES_IMAGE_CONTENTTYPE "d:ContentType"

//Type names
#define RES_TYPE_THUMBNAIL "Bing.Thumbnail"

//Creation/update functions

//Creation
int result_def_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	//Default response is to duplicate the dictionary (we can remove the ID and title, but it doesn't cause any issues other then memory usage)
	if(!dictionary)
	{
		//If NULL, then everything is good, carry on.
		return TRUE;
	}
	return hashtable_copy(((bing_result*)result)->data, (hashtable_t*)dictionary);
}

int result_def_common_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	hashtable_t* table;
	const char* str = NULL;
	BOOL ret = (BOOL)result_def_create(name, result, dictionary);
	if(ret)
	{
		//We need to set expected names for the element
		table = ((bing_result*)result)->data;
		if(strcmp(name, RES_TYPE_THUMBNAIL) == 0)
		{
			str = RES_TYPE_THUMBNAIL;
		}
		if(str)
		{
			hashtable_set_data(table, BING_RESULT_TYPE_FIELD, str, strlen(str) + 1);
		}
	}
	return ret;
}

//Additional results
void result_def_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
}

void result_additional_result_helper(bing_result_t result, bing_result_t new_result, const char* commonType, void* data, void (*specificProcessing)(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data))
{
	int size;
	char* str;
	bing_result* res = (bing_result*)new_result;
	bing_response* pres = ((bing_result*)result)->parent;
	if(res->type == BING_RESULT_TYPE) //Is this a type?
	{
		size = hashtable_get_string(res->data, BING_RESULT_TYPE_FIELD, NULL);
		if(size > 0) //Does it contain a common type?
		{
			str = bing_mem_malloc(size);
			if(str)
			{
				hashtable_get_string(res->data, BING_RESULT_TYPE_FIELD, str);
				if(strcmp(str, commonType) == 0) //Is it the correct common type?
				{
					bing_mem_free(str);

					specificProcessing(((bing_result*)result)->data, res->data, pres, data);
				}
				else
				{
					bing_mem_free(str);
				}
			}
		}
	}
}

void copyArray(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	const char* name = (char*)data;
	int size = hashtable_get_string(new_resultData, name, NULL);
	if(size > 0)
	{
		data = bing_mem_malloc(size); //We want a new item, so we add one to an existing array or create a new one
		hashtable_get_string(new_resultData, name, (char*)data); //If the collection doesn't exist, then nothing happens

		//Save the array
		hashtable_set_data(resultData, name, data, size);

		bing_mem_free(data);
	}
}

void loadThumbnail(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	int size;
	char* str;

	//Convert to thumbnail
	bing_thumbnail_t thumbnail = (bing_thumbnail_t)bing_mem_malloc(sizeof(bing_thumbnail_s));
	if(thumbnail)
	{
		//Setup the thumbnail
		size = hashtable_get_string(new_resultData, RES_IMAGE_MEDIA_URL, NULL);
		if(size > 0)
		{
			thumbnail->media_url = str = allocateMemory(size, pres);
			if(str)
			{
				hashtable_get_string(new_resultData, RES_IMAGE_MEDIA_URL, str);
			}
		}
		else
		{
			thumbnail->media_url = NULL;
		}

		size = hashtable_get_string(new_resultData, RES_IMAGE_CONTENTTYPE, NULL);
		if(size > 0)
		{
			thumbnail->content_type = str = allocateMemory(size, pres);
			if(str)
			{
				hashtable_get_string(new_resultData, RES_IMAGE_CONTENTTYPE, str);
			}
		}
		else
		{
			thumbnail->content_type = NULL;
		}

		hashtable_get_data_key(new_resultData, RES_IMAGE_HEIGHT, &thumbnail->height, sizeof(int));

		hashtable_get_data_key(new_resultData, RES_IMAGE_WIDTH, &thumbnail->width, sizeof(int));

		hashtable_get_data_key(new_resultData, RES_IMAGE_FILESIZE, &thumbnail->file_size, sizeof(long long));

		//Save the thumbnail
		hashtable_set_data(resultData, (char*)data, thumbnail, sizeof(bing_thumbnail_s));

		//Free the thumbnail
		bing_mem_free(thumbnail);
	}
}

void result_image_video_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	//keepResult is set to FALSE by default. Let it be freed,
	result_additional_result_helper(result, new_result, RES_TYPE_THUMBNAIL, (void*)name, loadThumbnail);
}

//Search structure

typedef struct BING_RESULT_CREATOR_SEARCH_S
{
	bing_result_creator creator;
	enum BING_SOURCE_TYPE type;
	int tableCount;
	struct BING_RESULT_CREATOR_SEARCH_S* next;
} bing_result_creator_search;

static bing_result_creator_search result_def_creator[]=
{
		//Results
		{{"WebResult",				FALSE,	result_def_create,			result_def_additional_result},			BING_SOURCETYPE_WEB,				5,	&result_def_creator[1]},
		{{"VideoResult",			FALSE,	result_def_create,			result_image_video_additional_result},	BING_SOURCETYPE_VIDEO, 				6,	&result_def_creator[2]},
		{{"ImageResult",			FALSE,	result_def_create,			result_image_video_additional_result},	BING_SOURCETYPE_IMAGE,				10,	&result_def_creator[3]},
		{{"NewsResult",				FALSE,	result_def_create,			result_def_additional_result},			BING_SOURCETYPE_NEWS,				6,	&result_def_creator[4]},
		{{"RelatedSearchResult",	FALSE,	result_def_create,			result_def_additional_result},			BING_SOURCETYPE_RELATED_SEARCH,		3,	&result_def_creator[5]},
		{{"SpellResult",			FALSE,	result_def_create,			result_def_additional_result},			BING_SOURCETYPE_SPELL,				2,	&result_def_creator[6]},

		//Type
		{{RES_TYPE_THUMBNAIL,		TRUE,	result_def_common_create,	result_def_additional_result},			BING_RESULT_TYPE,					5,	NULL},
};

static bing_field_search result_fields[] =
{
		//Common
		{{BING_RESULT_FIELD_ID,					FIELD_TYPE_STRING,	"d:ID",						6,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_NEWS,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB, BING_SOURCETYPE_SPELL}},									&result_fields[1]},
		{{BING_RESULT_FIELD_TITLE,				FIELD_TYPE_STRING,	"d:Title",					5,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_NEWS,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},															&result_fields[2]},

		//Image
		{{BING_RESULT_FIELD_HEIGHT,				FIELD_TYPE_INT,		RES_IMAGE_HEIGHT,			1,	{BING_SOURCETYPE_IMAGE}},							&result_fields[3]},
		{{BING_RESULT_FIELD_WIDTH,				FIELD_TYPE_INT,		RES_IMAGE_WIDTH,			1,	{BING_SOURCETYPE_IMAGE}},							&result_fields[4]},
		{{BING_RESULT_FIELD_FILE_SIZE,			FIELD_TYPE_LONG,	RES_IMAGE_FILESIZE,			1,	{BING_SOURCETYPE_IMAGE}},							&result_fields[5]},
		{{BING_RESULT_FIELD_MEDIA_URL,			FIELD_TYPE_STRING,	RES_IMAGE_MEDIA_URL,		2,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_VIDEO}},	&result_fields[6]},
		{{BING_RESULT_FIELD_CONTENT_TYPE,		FIELD_TYPE_STRING,	RES_IMAGE_CONTENTTYPE,		1,	{BING_SOURCETYPE_IMAGE}},							&result_fields[7]},
		{{BING_RESULT_FIELD_THUMBNAIL,			FIELD_TYPE_ARRAY,	"d:Thumbnail",				2,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_VIDEO}},	&result_fields[8]},
		{{BING_RESULT_FIELD_SOURCE_URL,			FIELD_TYPE_STRING,	"d:SourceUrl",				1,	{BING_SOURCETYPE_IMAGE}},							&result_fields[9]},

		//News
		{{BING_RESULT_FIELD_DESCRIPTION,		FIELD_TYPE_STRING,	"d:Description",			2,	{BING_SOURCETYPE_NEWS, BING_SOURCETYPE_WEB}},		&result_fields[10]},
		{{BING_RESULT_FIELD_URL,				FIELD_TYPE_STRING,	"d:Url",					2,	{BING_SOURCETYPE_NEWS, BING_SOURCETYPE_WEB}},		&result_fields[11]},
		{{BING_RESULT_FIELD_DATE,				FIELD_TYPE_LONG,	"d:Date",					1,	{BING_SOURCETYPE_NEWS}},							&result_fields[12]},
		{{BING_RESULT_FIELD_SOURCE,				FIELD_TYPE_STRING,	"d:Source",					1,	{BING_SOURCETYPE_NEWS}},							&result_fields[13]},

		//RelatedSearch
		{{BING_RESULT_FIELD_BING_URL,			FIELD_TYPE_STRING,	"d:BingUrl",				1,	{BING_SOURCETYPE_RELATED_SEARCH}},					&result_fields[13]},

		//Spell
		{{BING_RESULT_FIELD_VALUE,				FIELD_TYPE_STRING,	"d:Value",					1,	{BING_SOURCETYPE_SPELL}},							&result_fields[14]},

		//Video
		{{BING_RESULT_FIELD_RUN_TIME_LENGTH,	FIELD_TYPE_INT,		"d:RunTime",				1,	{BING_SOURCETYPE_VIDEO}},							&result_fields[15]},

		//Web
		{{BING_RESULT_FIELD_DISPLAY_URL,		FIELD_TYPE_STRING,	"d:DisplayUrl",				3,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_VIDEO,
				BING_SOURCETYPE_WEB}},																													NULL},
};

//Functions

enum BING_SOURCE_TYPE bing_result_get_source_type(bing_result_t result)
{
	enum BING_SOURCE_TYPE t = BING_SOURCETYPE_UNKNOWN;
	bing_result* res;
	if(result)
	{
		res = (bing_result*)result;
		t = res->type;
	}
	return t;
}

int bing_result_is_field_supported(bing_result_t result, enum BING_RESULT_FIELD field)
{
	BOOL ret = FALSE;
	bing_result* res;
	const char* key;
	if(result)
	{
		//Get the key
		res = (bing_result*)result;
		key = find_field(result_fields, field, FIELD_TYPE_UNKNOWN, res->type, FALSE);

		//Determine if the key is within the result
		ret = hashtable_key_exists(res->data, key);
	}
	return ret;
}

void free_result(bing_result* result)
{
	if(result)
	{
		hashtable_free(result->data);
		result->data = NULL;
		bing_mem_free(result);
	}
}

BOOL result_create(enum BING_SOURCE_TYPE type, bing_result_t* result, bing_response* responseParent, result_creation_func creation, result_additional_result_func additionalResult, int tableSize)
{
	BOOL ret = FALSE;
	bing_result* res;
	if(result && responseParent)
	{
		res = (bing_result*)bing_mem_malloc(sizeof(bing_result));
		if(res)
		{
			//Set variables
			res->type = type;
			res->creation = creation;
			res->additionalResult = additionalResult;

			res->data = hashtable_create(tableSize);
			if(res->data)
			{
				//Add result to response
				if(response_add_result(responseParent, res, RESULT_CREATE_DEFAULT_INTERNAL))
				{
					//Save the result
					res->parent = responseParent;
					*result = res;
					ret = TRUE;
				}
				else
				{
					hashtable_free(res->data);
					bing_mem_free(res);
				}
			}
			else
			{
				bing_mem_free(res);
			}
		}
	}
	return ret;
}

BOOL result_create_raw(const char* type, bing_result_t* result, bing_response* responseParent)
{
	BOOL ret = FALSE;
	bing_result_creator_search* cr;
	result_creation_func creationFunc;
	result_additional_result_func additionalResultFunc;
	unsigned int i;
	if(type && result && responseParent)
	{
		//Check default options
		for(cr = result_def_creator; cr != NULL; cr = cr->next)
		{
			if(strcmp(type, cr->creator.name) == 0)
			{
				break;
			}
		}
		if(cr)
		{
			//Create default options
			ret = result_create(cr->type, result, responseParent, cr->creator.creation, cr->creator.additionalResult, cr->tableCount);
		}
		else
		{
			creationFunc = NULL;
			additionalResultFunc = NULL;

			//Search custom creators
			pthread_mutex_lock(&bingSystem.mutex);

			for(i = 0; i < bingSystem.bingResultCreatorCount; i++)
			{
				if(strcmp(type, bingSystem.bingResultCreators[i].name) == 0)
				{
					creationFunc = bingSystem.bingResultCreators[i].creation;
					additionalResultFunc = bingSystem.bingResultCreators[i].additionalResult;
					break;
				}
			}

			pthread_mutex_unlock(&bingSystem.mutex);

			if(creationFunc)
			{
				//Create the custom result
				if(!additionalResultFunc)
				{
					//The "do nothing" function
					additionalResultFunc = result_def_additional_result;
				}
				ret = result_create(BING_SOURCETYPE_CUSTOM, result, responseParent, creationFunc, additionalResultFunc, -1);
			}
		}
	}
	return ret;
}

int result_get_data(bing_result_t result, enum BING_RESULT_FIELD field, enum FIELD_TYPE type, void* value, size_t size)
{
	BOOL ret = FALSE;
	const char* key;
	if(result && value)
	{
		//Get the key
		key = find_field(result_fields, field, type, bing_result_get_source_type(result), TRUE);

		//Now get the data
		ret = hashtable_get_data_key(((bing_result*)result)->data, key, value, size);
	}
	return ret;
}

int result_get_str_data(bing_result_t result, enum BING_RESULT_FIELD field, enum FIELD_TYPE type, char* value)
{
	int ret = -1;
	const char* key;
	if(result)
	{
		//Get the key
		key = find_field(result_fields, field, type, bing_result_get_source_type(result), TRUE);

		//Now get the data
		ret = bing_result_custom_get_string(result, key, value);
	}
	return ret;
}

int bing_result_get_32bit_int(bing_result_t result, enum BING_RESULT_FIELD field, int* value)
{
	return result_get_data(result, field, FIELD_TYPE_INT, value, sizeof(int));
}

int bing_result_get_64bit_int(bing_result_t result, enum BING_RESULT_FIELD field, long long* value)
{
	return result_get_data(result, field, FIELD_TYPE_LONG, value, sizeof(long long));
}

int bing_result_get_string(bing_result_t result, enum BING_RESULT_FIELD field, char* value)
{
	return result_get_str_data(result, field, FIELD_TYPE_STRING, value);
}

int bing_result_get_double(bing_result_t result, enum BING_RESULT_FIELD field, double* value)
{
	return result_get_data(result, field, FIELD_TYPE_DOUBLE, value, sizeof(double));
}

int bing_result_get_boolean(bing_result_t result, enum BING_RESULT_FIELD field, int* value)
{
	return result_get_data(result, field, FIELD_TYPE_BOOLEAN, value, sizeof(int));
}

int bing_result_get_array(bing_result_t result, enum BING_RESULT_FIELD field, void* value)
{
	return result_get_str_data(result, field, FIELD_TYPE_ARRAY, value);
}

int bing_result_custom_is_field_supported(bing_result_t result, const char* field)
{
	BOOL ret = FALSE;
	bing_result* res;
	if(result && field)
	{
		res = (bing_result*)result;
		ret = hashtable_key_exists(res->data, field);
	}
	return ret;
}

int bing_result_custom_get_32bit_int(bing_result_t result, const char* field, int* value)
{
	return hashtable_get_data_key(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(int));
}

int bing_result_custom_get_64bit_int(bing_result_t result, const char* field, long long* value)
{
	return hashtable_get_data_key(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(long long));
}

int bing_result_custom_get_string(bing_result_t result, const char* field, char* value)
{
	return hashtable_get_string(result ? ((bing_result*)result)->data : NULL, field, value);
}

int bing_result_custom_get_double(bing_result_t result, const char* field, double* value)
{
#if __SIZEOF_DOUBLE__ != __SIZEOF_LONG_LONG__
#error Double size is different than Long Long size
#endif
	return bing_result_custom_get_64bit_int(result, field, (long long*)value);
}

int bing_result_custom_get_boolean(bing_result_t result, const char* field, int* value)
{
	return hashtable_get_data_key(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(int));
}

int bing_result_custom_get_array(bing_result_t result, const char* field, void* value)
{
	return bing_result_custom_get_string(result, field, (char*)value);
}

int bing_result_custom_set_32bit_int(bing_result_t result, const char* field, int value)
{
	return bing_result_custom_set_p_32bit_int(result, field, &value);
}

int bing_result_custom_set_64bit_int(bing_result_t result, const char* field, long long value)
{
	return bing_result_custom_set_p_64bit_int(result, field, &value);
}

int bing_result_custom_set_double(bing_result_t result, const char* field, double value)
{
	return bing_result_custom_set_p_double(result, field, &value);
}

int bing_result_custom_set_boolean(bing_result_t result, const char* field, int value)
{
	return bing_result_custom_set_p_boolean(result, field, &value);
}

int bing_result_custom_set_p_32bit_int(bing_result_t result, const char* field, const int* value)
{
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(int));
}

int bing_result_custom_set_p_64bit_int(bing_result_t result, const char* field, const long long* value)
{
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(long long));
}

int bing_result_custom_set_string(bing_result_t result, const char* field, const char* value)
{
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, value ? (strlen(value) + 1) : 0);
}

int bing_result_custom_set_p_double(bing_result_t result, const char* field, const double* value)
{
#if __SIZEOF_DOUBLE__ != __SIZEOF_LONG_LONG__
#error Double size is different than Long Long size
#endif
	return bing_result_custom_set_p_64bit_int(result, field, (long long*)value);
}

int bing_result_custom_set_p_boolean(bing_result_t result, const char* field, const int* value)
{
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(int));
}

int bing_result_custom_set_array(bing_result_t result, const char* field, const void* value, size_t size)
{
	//This could be a safety hazard but we have no way of checking the size of the data passed in
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, size);
}

void* bing_result_custom_allocation(bing_result_t result, size_t size)
{
	void* ret = NULL;
	bing_result* res;
	if(result)
	{
		res = (bing_result*)result;
		if(res->type == BING_SOURCETYPE_CUSTOM && //We only want to allocate memory for custom types
				(size <= (5 * 1024) && size >= 1)) //We want it to be within a small range since multiple results can be allocated for the same response.
		{
			ret = allocateMemory(size, res->parent);
		}
	}
	return ret;
}

int bing_result_register_result_creator(const char* name, int type, result_creation_func creation_func, result_additional_result_func additional_func)
{
	BOOL ret = FALSE;
	BOOL cont = TRUE;
	unsigned int i;
	bing_result_creator* c;
	bing_result_creator_search* cr;
	char* nName;
	size_t size;
	if(name && creation_func)
	{
		//Check if the name is a standard supported name, if so return
		for(cr = result_def_creator; cr != NULL; cr = cr->next)
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
			while(i < bingSystem.bingResultCreatorCount && cont)
			{
				if(strcmp(bingSystem.bingResultCreators[i++].name, name) == 0)
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
					nName[size - 1] = '\0';

					//Create the new version of the name
					c = (bing_result_creator*)bing_mem_realloc(bingSystem.bingResultCreators, sizeof(bing_result_creator) * (bingSystem.bingResultCreatorCount + 1));

					if(c)
					{
						bingSystem.bingResultCreators = c;

						c[bingSystem.bingResultCreatorCount].name = nName;
						c[bingSystem.bingResultCreatorCount].type = (BOOL)type;
						c[bingSystem.bingResultCreatorCount].creation = creation_func;
						c[bingSystem.bingResultCreatorCount++].additionalResult = additional_func;

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

int bing_result_unregister_result_creator(const char* name)
{
	BOOL ret = FALSE;

	const char* rn;
	unsigned int i;
	bing_result_creator* c;
	if(name && bingSystem.bingResultCreatorCount > 0) //We don't want to run if there is nothing to run on
	{
		pthread_mutex_lock(&bingSystem.mutex);

		//Find the result
		i = 0;
		while(i < bingSystem.bingResultCreatorCount)
		{
			if(strcmp(bingSystem.bingResultCreators[i].name, name) == 0)
			{
				break;
			}
			i++;
		}

		if(i < bingSystem.bingResultCreatorCount)
		{
			//Get the name
			rn = bingSystem.bingResultCreators[i].name;

			//If there is only one creator, simply free everything
			if(bingSystem.bingResultCreatorCount == 1)
			{
				bing_mem_free(bingSystem.bingResultCreators);
				bingSystem.bingResultCreators = NULL;
				bingSystem.bingResultCreatorCount = 0;

				ret = TRUE;
			}
			else
			{
				//We don't want to reallocate because if we fail and the creator was not the last element, then we overwrote it
				c = (bing_result_creator*)bing_mem_malloc(sizeof(bing_result_creator) * (bingSystem.bingResultCreatorCount - 1));

				if(c)
				{
					//If this is the last result then it's easy, we just free the data
					if(i != bingSystem.bingResultCreatorCount - 1)
					{
						memmove(bingSystem.bingResultCreators + i, bingSystem.bingResultCreators + (i + 1), (bingSystem.bingResultCreatorCount - i - 1) * sizeof(bing_result_creator));
					}
					memcpy(c, bingSystem.bingResultCreators, (--bingSystem.bingResultCreatorCount) * sizeof(bing_result_creator));
					bing_mem_free(bingSystem.bingResultCreators);
					bingSystem.bingResultCreators = c;

					ret = TRUE;
				}
			}

			//Free memory if everything else worked
			if(ret)
			{
				bing_mem_free((void*)rn);
			}
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
	return ret;
}
