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

void bing_initialize()
{
	if(atomic_set_value((unsigned int*)&bing_initialized, TRUE) == FALSE) //Set this first so if another thread tries to initialize it, it doesn't work
	{
		atomic_clr(&searchCount, sizeof(unsigned int));

		memset(&bingSystem, 0, sizeof(bing_system));

		pthread_mutex_init(&bingSystem.mutex, NULL);
		pthread_mutex_lock(&bingSystem.mutex);

		bingSystem.domainID = bps_register_domain();
		bingSystem.bingInstancesCount = 0;
		bingSystem.bingInstances = NULL;
		bingSystem.bingResponseCreatorCount = 0;
		bingSystem.bingResultCreatorCount = 0;
		bingSystem.bingResponseCreators = NULL;
		bingSystem.bingResultCreators = NULL;

#if !defined(BING_NO_MEM_HANDLERS)
		//Set these as a precaution
		bing_mem_malloc = (bing_malloc_handler)malloc;
		bing_mem_calloc = (bing_calloc_handler)calloc;
		bing_mem_realloc = (bing_realloc_handler)realloc;
		bing_mem_free = (bing_free_handler)free;
		bing_mem_strdup = (bing_strdup_handler)strdup;
#endif

		pthread_mutex_unlock(&bingSystem.mutex);
	}
}

int bing_shutdown()
{
	BOOL ret = FALSE;

	unsigned int i;
	if(atomic_set_value((unsigned int*)&bing_initialized, FALSE) == TRUE) //FALSE = 0 so it is the equivalent of ORing 0 (which doesn't do anything). This guarantees that nothing else will edit the value, so we know we will get the value
	{
		pthread_mutex_lock(&bingSystem.mutex);

		if(!atomic_set_value(&searchCount, 0))
		{
			//Free all the creator strings, then the creators themselves
			while(bingSystem.bingResponseCreatorCount > 0)
			{
				bing_mem_free((void*)bingSystem.bingResponseCreators[--bingSystem.bingResponseCreatorCount].dedicatedName);
				bing_mem_free((void*)bingSystem.bingResponseCreators[--bingSystem.bingResponseCreatorCount].bundleName);
			}
			while(bingSystem.bingResultCreatorCount > 0)
			{
				bing_mem_free((void*)bingSystem.bingResultCreators[--bingSystem.bingResultCreatorCount].name);
			}
			bing_mem_free(bingSystem.bingResponseCreators);
			bing_mem_free(bingSystem.bingResultCreators);
			bingSystem.bingResponseCreators = NULL;
			bingSystem.bingResultCreators = NULL;

			//Free and the Bing instances
			i = 0;
			while(bingSystem.bingInstancesCount > 0)
			{
				if(bingSystem.bingInstances[i])
				{
					bing_free(++i);
				}
			}

			//Reset the instance system
			bingSystem.bingInstancesCount = 0;
			bing_mem_free(bingSystem.bingInstances);
			bingSystem.bingInstances = NULL;

			pthread_mutex_destroy(&bingSystem.mutex);

			atomic_clr(&searchCount, sizeof(unsigned int));

			atomic_clr((unsigned int*)&bing_initialized, sizeof(BOOL)); //Set this last so that it only know's it's uninitialized after everything has been freed

			ret = TRUE;
		}
		else
		{
			pthread_mutex_unlock(&bingSystem.mutex);
		}
	}

	return ret;
}

int bing_get_domain()
{
	bing_initialize();

	return bingSystem.domainID;
}

int findFreeIndex()
{
	unsigned int i;
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

unsigned int bing_create(const char* application_ID)
{
	int ret = 0; //Zero is reserved for bad
	int loc;
	bing* bingI;
	bing** in;

	bing_initialize();

	if(bingSystem.domainID != -1)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		//Reallocate the instance length
		if(bingSystem.bingInstances)
		{
			in = bing_mem_realloc(bingSystem.bingInstances, (bingSystem.bingInstancesCount + 1) * sizeof(bing*));
		}
		else
		{
			in = bing_mem_malloc(sizeof(bing*));
		}
		if(in)
		{
			//Reset instances
			in[bingSystem.bingInstancesCount++] = NULL;
			bingSystem.bingInstances = in;

			//Find a bing_free index
			loc = findFreeIndex();

			//Create Bing instance
			in[loc] = bingI = (bing*)bing_mem_malloc(sizeof(bing));

			if(bingI)
			{
				memset(bingI, 0, sizeof(bing));

				//Copy application ID
				if(application_ID)
				{
					if(bingI->appId)
					{
						bing_mem_free(bingI->appId);
					}
					bingI->appId = bing_mem_strdup(application_ID);
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
						in = bing_mem_realloc(bingSystem.bingInstances, bingSystem.bingInstancesCount * sizeof(bing*));

						if(in)
						{
							bingSystem.bingInstances = in;
						}
					}
					//We don't need to worry about this if it isn't at the end because it means the array element is already NULL and thus hasn't changed.
				}
				else
				{
					bing_mem_free(bingSystem.bingInstances);

					bingSystem.bingInstances = NULL;
					bingSystem.bingInstancesCount = 0;
				}
			}
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
	return ret;
}

void bing_free(unsigned int bingID)
{
	bing* bingI;

	bing_initialize(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		if(bingSystem.bingInstancesCount >= bingID)
		{
			bingI = bingSystem.bingInstances[bingID - 1];

			//Free the system
			if(bingSystem.bingInstancesCount == 1)
			{
				bing_mem_free(bingSystem.bingInstances);

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
				bing_response_free((bing_response_t)bingI->responses[bingI->responseCount - 1]);
			}
			bing_mem_free(bingI->responses);

			bing_mem_free(bingI->appId);

			pthread_mutex_destroy(&bingI->mutex);

			bing_mem_free(bingI);
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
}

bing* retrieveBing(unsigned int bingID)
{
	bing* bingI = NULL;

	bing_initialize(); //This shouldn't be here but put it here for safety

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

int bing_set_error_return(unsigned int bingID, int error)
{
	bing* bingI = retrieveBing(bingID);

	if(bingI)
	{
		pthread_mutex_lock(&bingI->mutex);

		bingI->errorRet = error;

		pthread_mutex_unlock(&bingI->mutex);

		return TRUE;
	}
	return FALSE;
}

int bing_get_error_return(unsigned int bingID)
{
	int res = FALSE;
	bing* bingI = retrieveBing(bingID);

	if(bingI)
	{
		pthread_mutex_lock(&bingI->mutex);

		res = bingI->errorRet;

		pthread_mutex_unlock(&bingI->mutex);
	}

	return res;
}

#endif

int bing_get_app_ID(unsigned int bingID, char* buffer)
{
	bing* bingI = retrieveBing(bingID);
	int ret = -1;

	if(bingI)
	{
		pthread_mutex_lock(&bingI->mutex);

		ret = strlen(bingI->appId) + 1;

		if(buffer)
		{
			strlcpy(buffer, bingI->appId, ret);
			buffer[ret - 1] = '\0';
		}

		pthread_mutex_unlock(&bingI->mutex);
	}

	return ret;
}

int bing_set_app_ID(unsigned int bingID, const char* appId)
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

			bingI->appId = (char*)bing_mem_malloc(size);
			if(bingI->appId)
			{
				strlcpy(bingI->appId, appId, size);
				bingI->appId[size - 1] = '\0';

				bing_mem_free(preApp);

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

const char BING_URL[] = "https://api.datamarket.azure.com/Bing/Search/";
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
		ret = (char*)bing_mem_malloc(size * 3); //We don't know how many of these bytes we will need, better to be safe then sorry
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

const char* bing_request_url(const char* query, const bing_request_t request)
{
	char* ret = NULL;
	const char* queryStr;
	const char* appIdStr;
	const char* requestOptions;
	const char* sourceType;
	char* sourceTypeTmp;
	bing_request* req = (bing_request*)request;
	size_t urlSize = 26 + 1; //This is the length of the URL format and null char. We don't include '?' because when the size of sourceType is taken, it will include that

	//TODO Modify to handle translation requests (cannot be in bundles, at least this function won't allow it. Can modify search functions to do it)

	if(request)
	{
		//Size of URL (it's constant but it could change so we don't want to hard code the size)
		urlSize += sizeof(BING_URL);

		//Encode the query and get its size
		queryStr = encodeUrl(query);
		urlSize += strlen(queryStr);

		//Get the source type
		sourceType = req->sourceType;
		if(sourceType)
		{
			sourceTypeTmp = bing_mem_malloc(strlen(sourceType) + 2); //Includes the ? and null char
			if(sourceTypeTmp)
			{
				//We need to add ? so it is a valid URL
				if(snprintf(sourceTypeTmp, strlen(sourceType) + 2, "%s?", sourceType) < 0)
				{
					free(sourceTypeTmp);
					sourceTypeTmp = NULL; //Error (this will set sourceType to NULL, which will prevent it from executing)
				}
				sourceType = sourceTypeTmp;
			}
			else
			{
				//Prevent from running if we can't make source type string
				sourceType = NULL;
			}
		}
		else
		{
			//Get bundle source type
			sourceType = request_get_bundle_sourcetype(req);

			//Format for the URL
			sourceTypeTmp = bing_mem_malloc(strlen(sourceType) + 26); //Includes format, ?, and null char
			if(sourceTypeTmp)
			{
				//Format it so it is correct for a composite request
				if(snprintf(sourceTypeTmp, strlen(sourceType) + 26, "Composite?Sources=%%27%s%%27", sourceType) < 0)
				{
					free(sourceTypeTmp);
					sourceTypeTmp = NULL; //Error (this will set sourceType to NULL, which will prevent it from executing)
				}
				sourceType = sourceTypeTmp;
			}
			else
			{
				//Prevent from running if we can't make source type string
				sourceType = NULL;
			}
		}

		if(sourceType)
		{
			//Size of the request source type
			urlSize += strlen(sourceType);

			//Get the request options and types and size
			requestOptions = req->getOptions(request);
			urlSize += strlen(requestOptions);

			//Allocate the url data
			ret = (char*)bing_mem_calloc(urlSize, sizeof(char));
			if(ret)
			{
				//Now actually create the URL
				if(snprintf(ret, urlSize, "%s%sQuery=%%27%s%%27&$format=ATOM%s", BING_URL, sourceType, queryStr, requestOptions) < 0)
				{
					//Error
					bing_mem_free(ret);
					ret = NULL;
				}
			}

			//Free the strings
			bing_mem_free((void*)requestOptions);
			bing_mem_free((void*)sourceType);
		}
		bing_mem_free((void*)queryStr);
	}
	return ret;
}

const char* find_field(bing_field_search* searchFields, int fieldID, enum FIELD_TYPE type, enum BING_SOURCE_TYPE sourceType, BOOL checkType)
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
	char* rett;
	size_t size;

	buffer[0] = '\0';
	size = hashtable_get_item(table, key, NULL);
	if(size > 0)
	{
		//Make sure that the data buffer (for getting the data) is large enough (add one for safety)
		size += 1;
		if(size > curDataSize[0])
		{
			data[0] = bing_mem_realloc(data[0], size);
			curDataSize[0] = size;
		}

		//Get the data, format it, return it
		if(data[0])
		{
			hashtable_get_item(table, key, data[0]);

			snprintf(buffer, 256, format, data[0]);
			buffer[255] = '\0';

			returnDataSize[0] += strlen(buffer);
			rett = bing_mem_realloc(returnData[0], returnDataSize[0]);
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
	if(strLen > 0)
	{
		str = bing_mem_malloc(strLen);
		if(str)
		{
			hashtable_get_string(table, field, str);
			ll = atoll(str);
			ret = hashtable_set_data(table, field, &ll, sizeof(long long));
			bing_mem_free(str);
		}
		else
		{
			ret = FALSE;
		}
	}
	return ret;
}

BOOL replace_string_with_int(hashtable_t* table, const char* field)
{
	BOOL ret = TRUE; //We want to return true by default as the field might not exist
	char* str;
	int i;
	int strLen = hashtable_get_string(table, field, NULL);
	if(strLen > 0)
	{
		str = bing_mem_malloc(strLen);
		if(str)
		{
			hashtable_get_string(table, field, str);
			i = atoi(str);
			ret = hashtable_set_data(table, field, &i, sizeof(int));
			bing_mem_free(str);
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
	if(strLen > 0)
	{
		str = bing_mem_malloc(strLen);
		if(str)
		{
			hashtable_get_string(table, field, str);
			d = atof(str);
			ret = hashtable_set_data(table, field, &d, sizeof(double));
			bing_mem_free(str);
		}
		else
		{
			ret = FALSE;
		}
	}
	return ret;
}

BOOL bing_set_memory_handlers(bing_malloc_handler bm, bing_calloc_handler bc, bing_realloc_handler br, bing_free_handler bf, bing_strdup_handler bs)
{
#if defined(BING_NO_MEM_HANDLERS)
	return FALSE;
#else
	bing_initialize();

	//We want everything to be set to prevent issues
	if(bm && bc && br && bf && bs)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		bing_mem_malloc = bm;
		bing_mem_calloc = bc;
		bing_mem_realloc = br;
		bing_mem_free = bf;
		bing_mem_strdup = bs;

		pthread_mutex_unlock(&bingSystem.mutex);

		return TRUE;
	}
	return FALSE;
#endif
}
