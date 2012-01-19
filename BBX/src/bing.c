/*
 * bing.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

static volatile BOOL bing_initialized = FALSE;

void initialize_bing()
{
	if(!bing_initialized)
	{
		bing_initialized = TRUE; //Set this first so if another thread tries to initialize it, it doesn't work

		searchCount = 0;

		memset(&bingSystem, 0, sizeof(bing_system));

		bingSystem.domainID = bps_register_domain();
		bingSystem.bingInstancesCount = 0;
		bingSystem.bingInstances = NULL;
		bingSystem.bingResponseCreatorCount = 0;
		bingSystem.bingResultCreatorCount = 0;
		bingSystem.bingResponseCreators = NULL;
		bingSystem.bingResultCreators = NULL;
		pthread_mutex_init(&bingSystem.mutex, NULL);

		//Set these as a precaution
		bing_malloc = (bing_malloc_handler)malloc;
		bing_calloc = (bing_calloc_handler)calloc;
		bing_realloc = (bing_realloc_handler)realloc;
		bing_free = (bing_free_handler)free;
		bing_strdup = (bing_strdup_handler)strdup;
	}
}

void shutdown_bing()
{
	unsigned int i = 0;
	if(bing_initialized)
	{
		//Free all the creator strings, then the creators themselves
		while(bingSystem.bingResponseCreatorCount > 0)
		{
			bing_free((void*)bingSystem.bingResponseCreators[--bingSystem.bingResponseCreatorCount].name);
		}
		while(bingSystem.bingResultCreatorCount > 0)
		{
			bing_free((void*)bingSystem.bingResultCreators[--bingSystem.bingResultCreatorCount].name);
		}
		bing_free(bingSystem.bingResponseCreators);
		bing_free(bingSystem.bingResultCreators);
		bingSystem.bingResponseCreators = NULL;
		bingSystem.bingResultCreators = NULL;

		//Free and the Bing instances
		while(bingSystem.bingInstancesCount > 0)
		{
			if(bingSystem.bingInstances[i])
			{
				free_bing(++i);
			}
		}

		//Reset the instance system
		bingSystem.bingInstancesCount = 0;
		bing_free(bingSystem.bingInstances);
		bingSystem.bingInstances = NULL;

		pthread_mutex_destroy(&bingSystem.mutex);

		searchCount = 0;

		bing_initialized = FALSE; //Set this last so that it only know's it's uninitialized after everything has been freed
	}
}

int bing_get_domain()
{
	initialize_bing();

	return bingSystem.domainID;
}

int findFreeIndex()
{
	int i;
	if(bingSystem.domainID != -1)
	{
		if(bingSystem.bingInstances)
		{
			for(i = 0; i < bingSystem.bingInstancesCount; i++)
			{
				if(!bingSystem.bingInstances[i])
				{
					return i;
				}
			}
		}
	}
	return -1;
}

unsigned int create_bing(const char* application_ID)
{
	int appIdLen;
	int ret = 0; //Zero is reserved for bad
	int loc;
	bing* bingI;
	bing** in;
	size_t size;

	initialize_bing();

	if(bingSystem.domainID != -1)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		//Reallocate the instance length
		if(bingSystem.bingInstances)
		{
			in = bing_realloc(bingSystem.bingInstances, (bingSystem.bingInstancesCount + 1) * sizeof(bing*));
		}
		else
		{
			in = bing_malloc(sizeof(bing*));
		}
		if(in)
		{
			//Reset instances
			in[bingSystem.bingInstancesCount++] = NULL;
			bingSystem.bingInstances = in;

			//Find a bing_free index
			loc = findFreeIndex();

			//Create Bing instance
			in[loc] = bingI = (bing*)bing_malloc(sizeof(bing));

			if(bingI)
			{
				memset(bingI, 0, sizeof(bing));

				//Copy application ID
				if(application_ID)
				{
					if(bingI->appId)
					{
						bing_free(bingI->appId);
					}
					bingI->appId = bing_strdup(application_ID);
					//If an error occurs, it's up to the developer to make sure that the app ID was copied
				}
#if defined(BING_DEBUG)
				bingI->errorRet = DEFAULT_ERROR_RET;
#endif

				pthread_mutex_init(&bingI->mutex, NULL);

				ret = bingSystem.bingInstancesCount;
			}
			else
			{
				//Didn't work, revert
				if(bingSystem.bingInstancesCount > 1)
				{
					bingSystem.bingInstancesCount--;
					if(loc == bingSystem.bingInstancesCount) //At end of instances, free up some space
					{
						in = bing_realloc(bingSystem.bingInstances, bingSystem.bingInstancesCount * sizeof(bing*));

						if(in)
						{
							bingSystem.bingInstances = in;
						}
					}
					//We don't need to worry about this if it isn't at the end because it means the array element is already NULL and thus hasn't changed.
				}
				else
				{
					bing_free(bingSystem.bingInstances);

					bingSystem.bingInstances = NULL;
					bingSystem.bingInstancesCount = 0;
				}
			}
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
	return ret;
}

void free_bing(unsigned int bingID)
{
	bing* bingI;
	bing** in;

	initialize_bing(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		if(bingSystem.bingInstancesCount >= bingID)
		{
			bingI = bingSystem.bingInstances[bingID - 1];

			//Free the system
			if(bingSystem.bingInstancesCount == 1)
			{
				bing_free(bingSystem.bingInstances);

				bingSystem.bingInstances = NULL;
				bingSystem.bingInstancesCount = 0;
			}
			else
			{
				bingSystem.bingInstancesCount--;
				bingSystem.bingInstances[bingID - 1] = NULL;
			}

			pthread_mutex_lock(&bingI->mutex);

			//Free the responses themselves
			while(bingI->responseCount > 0)
			{
				free_response((bing_response_t)bingI->responses[bingI->responseCount - 1]);
			}
			bing_free(bingI->responses);

			bing_free(bingI->appId);

			pthread_mutex_destroy(&bingI->mutex);

			bing_free(bingI);
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
}

bing* retrieveBing(unsigned int bingID)
{
	bing* bingI = NULL;

	initialize_bing(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		if(bingSystem.bingInstancesCount >= bingID)
		{
			bingI = bingSystem.bingInstances[bingID - 1];
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}

	return bingI;
}

#if defined(BING_DEBUG)

int set_error_return(unsigned int bingID, int error)
{
	bing* bingI = retrieveBing(bingID);

	if(bingI)
	{
		bingI->errorRet = error;

		return TRUE;
	}
	return FALSE;
}

int get_error_return(unsigned int bingID)
{
	int res = FALSE;
	bing* bingI = retrieveBing(bingID);

	if(bingI)
	{
		res = bingI->errorRet;
	}

	return res;
}

#endif

int get_app_ID(unsigned int bingID, char* buffer)
{
	bing* bingI = retrieveBing(bingID);
	int ret = -1;

	if(bingI)
	{
		pthread_mutex_lock(&bingI->mutex);

		ret = strlen(bingI->appId) + 1;

		if(buffer)
		{
			strcpy(buffer, bingI->appId);
		}

		pthread_mutex_unlock(&bingI->mutex);
	}

	return ret;
}

int set_app_ID(unsigned int bingID, const char* appId)
{
	bing* bingI;
	int size;
	int res = FALSE;
	char* preApp;

	if(appId && (size = strlen(appId) + 1) > 1)
	{
		bingI= retrieveBing(bingID);

		if(bingI)
		{
			pthread_mutex_lock(&bingI->mutex);

			preApp = bingI->appId;

			bingI->appId = (char*)bing_malloc(size);
			if(bingI->appId)
			{
				strlcpy(bingI->appId, appId, size);

				bing_free(preApp);

				res = TRUE;
			}
			else
			{
				bingI->appId = preApp;
			}

			pthread_mutex_unlock(&bingI->mutex);
		}
	}

	return res;
}

//Utility functions

const char BING_URL[] = "http://api.bing.net/xml.aspx?";
const char URL_UNRESERVED[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
const char HEX[] = "0123456789ABCDEF";

//requestUrl formats the URL, this encodes it so it is formatted in a manner that can be interpreted properly
const char* encodeUrl(const char* url)
{
	unsigned char* bytes = (unsigned char*)url;
	char* ret = "";
	char* find;
	size_t size;
	size_t pos;
	unsigned char b;
	if(bytes != NULL)
	{
		size = strlen(url) + 1;
		ret = (char*)bing_malloc(size * 3); //We don't know how many of these bytes we will need, better to be safe then sorry
		if(ret != NULL)
		{
			memset(ret, 0, size * 3);
			pos = 0;

			while((b = *bytes))
			{
				find = strchr(URL_UNRESERVED, b);
				if(find)
				{
					//The value is valid for a URL, simply use it
					ret[pos++] = b;
				}
				else
				{
					//The value is not valid for a URL, encode it
					ret[pos++] = '%';
					ret[pos++] = HEX[(b >> 4) & 0x0F];
					ret[pos++] = HEX[b & 0x0F];
				}
				//Move the bytes forward
				bytes++;
			}
			ret[pos] = 0; //Null char
		}
		else
		{
			//Error, back to original
			ret = "";
		}
	}
	return ret;
}

const char* request_url(unsigned int bingID, const char* query, const bing_request_t request)
{
	bing* bingI = retrieveBing(bingID);
	char* ret = NULL;
	const char* queryStr;
	const char* appIdStr;
	const char* requestOptions;
	const char* sourceType;
	bing_request* req = (bing_request*)request;
	size_t urlSize = 46; //This is the length of the URL format

	if(bingI && request)
	{
		//Size of URL (it's constant but it could change so we don't want to hard code the size)
		urlSize += sizeof(BING_URL);

		//Encode the query and get its size
		queryStr = encodeUrl(query);
		urlSize += strlen(queryStr);

		//Get the source type
		sourceType = req->sourceType;
		if(!sourceType)
		{
			//Get bundle source type
			sourceType = request_get_bundle_sourcetype(req);
		}

		//Size of the request source type
		urlSize += strlen(sourceType);

		//Get the request options and types and size
		requestOptions = req->getOptions(request);
		urlSize += strlen(requestOptions);

		//We want to lock it now before we use the application ID (since it can be changed)
		pthread_mutex_lock(&bingI->mutex);

		//Application ID and size
		appIdStr = bingI->appId;
		if(!appIdStr)
		{
			appIdStr = "";
		}
		urlSize += strlen(appIdStr);

		//Allocate the url data
		ret = (char*)bing_calloc(urlSize + 6, sizeof(char)); //The 6 is just for null chars as a precaution.
		if(ret)
		{
			//Now actually create the URL
			if(snprintf(ret, urlSize + 6, "%sxmltype=attributebased&AppId=%s&Query=%s&Sources=%s%s", BING_URL, appIdStr, queryStr, sourceType, requestOptions) < 0)
			{
				//Error
				bing_free(ret);
				ret = NULL;
			}
		}

		//Let the Bing element go back to normal execution
		pthread_mutex_unlock(&bingI->mutex);

		//Free the strings
		if(!req->sourceType)
		{
			//Need to free source type for bundle
			bing_free((void*)sourceType);
		}
		bing_free((void*)requestOptions);
		bing_free((void*)queryStr);
	}
	return ret;
}

const char* find_field(bing_field_search* searchFields, int fieldID, enum FIELD_TYPE type, enum SOURCE_TYPE sourceType, BOOL checkType)
{
	int i;
	//If the field actually has a value then we check it, otherwise skip it. We also don't want to do anything with custom types (since it will fail anyway)
	if(fieldID && sourceType != BING_SOURCETYPE_CUSTOM)
	{
		for(; searchFields; searchFields = searchFields->next)
		{
			//Make sure the variable and type match (we don't want to return a String for something that needs to be a long or double)
			if(searchFields->field.variableValue == fieldID &&
					!(checkType && searchFields->field.type != type))
			{
				//Fields support certain types, see if the type matches
				for(i = 0; i < searchFields->field.sourceTypeCount; i++)
				{
					if(searchFields->field.supportedTypes[i] == sourceType)
					{
						return searchFields->field.name;
					}
				}
				//If we want every field, then it is implicitly supported
				if(searchFields->field.sourceTypeCount == BING_FIELD_SUPPORT_ALL_FIELDS)
				{
					return searchFields->field.name;
				}
			}
		}
	}
	return NULL;
}

void append_data(hashtable_t* table, const char* format, const char* key, void** data, size_t* curDataSize, char** returnData, size_t* returnDataSize)
{
	char buffer[256];
	buffer[0] = '\0';
	char* rett;
	int size = hashtable_get_item(table, key, NULL);
	if(size > 0)
	{
		//Make sure that the data buffer (for getting the data) is large enough (add one for safety)
		size += 1;
		if(size > curDataSize[0])
		{
			data[0] = bing_realloc(data[0], size);
			curDataSize[0] = size;
		}

		//Get the data, format it, return it
		if(data[0])
		{
			hashtable_get_item(table, key, data[0]);

			snprintf(buffer, 256, format, data[0]);

			returnDataSize[0] += strlen(buffer);
			rett = bing_realloc(returnData[0], returnDataSize[0]);
			if(rett)
			{
				strlcat(rett, buffer, returnDataSize[0]);
				returnData[0] = rett;
			}
		}
	}
}

BOOL replace_string_with_longlong(hashtable_t* table, const char* field)
{
	BOOL ret = TRUE; //We want to return true by default as the field might not exist
	char* str;
	long long ll;
	int strLen = hashtable_get_string(table, field, NULL);
	if(strLen > -1)
	{
		str = bing_malloc(strLen);
		if(str)
		{
			hashtable_get_string(table, field, str);
			ll = atoll(str);
			ret = hashtable_set_data(table, field, &ll, sizeof(long long));
			bing_free(str);
		}
		else
		{
			ret = FALSE;
		}
	}
	return ret;
}

BOOL replace_string_with_double(hashtable_t* table, const char* field)
{
	BOOL ret = TRUE; //We want to return true by default as the field might not exist
	char* str;
	double d;
	int strLen = hashtable_get_string(table, field, NULL);
	if(strLen > -1)
	{
		str = bing_malloc(strLen);
		if(str)
		{
			hashtable_get_string(table, field, str);
			d = atof(str);
			ret = hashtable_set_data(table, field, &d, sizeof(double));
			bing_free(str);
		}
		else
		{
			ret = FALSE;
		}
	}
	return ret;
}

BOOL set_bing_memory_handlers(bing_malloc_handler bm, bing_calloc_handler bc, bing_realloc_handler br, bing_free_handler bf, bing_strdup_handler bs)
{
	//We want everything to be set to prevent issues
	if(bm && bc && br && bf && bs)
	{
		bing_malloc = bm;
		bing_calloc = bc;
		bing_realloc = br;
		bing_free = bf;
		bing_strdup = bs;
		return TRUE;
	}
	return FALSE;
}
