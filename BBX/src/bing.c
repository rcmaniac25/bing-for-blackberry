/*
 * bing.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"
#include <ctype.h>

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
				bing_mem_free((void*)bingSystem.bingResponseCreators[--bingSystem.bingResponseCreatorCount].compositeName);
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

unsigned int bing_create(const char* account_key)
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
				if(account_key)
				{
					if(bingI->accountKey)
					{
						bing_mem_free(bingI->accountKey);
					}
					bingI->accountKey = bing_mem_strdup(account_key);
					//If an error occurs, it's up to the developer to make sure that the app ID was copied
				}

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

			bing_mem_free(bingI->accountKey);

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

int bing_get_account_key(unsigned int bingID, char* buffer)
{
	bing* bingI = retrieveBing(bingID);
	int ret = -1;

	if(bingI)
	{
		pthread_mutex_lock(&bingI->mutex);

		ret = strlen(bingI->accountKey) + 1;

		if(buffer)
		{
			strlcpy(buffer, bingI->accountKey, ret);
			buffer[ret - 1] = '\0';
		}

		pthread_mutex_unlock(&bingI->mutex);
	}

	return ret;
}

int bing_set_account_key(unsigned int bingID, const char* account_key)
{
	bing* bingI;
	int size;
	int res = FALSE;
	char* preKey;

	if(account_key && (size = strlen(account_key) + 1) > 1)
	{
		bingI= retrieveBing(bingID);

		if(bingI)
		{
			pthread_mutex_lock(&bingI->mutex);

			preKey = bingI->accountKey;

			bingI->accountKey = (char*)bing_mem_malloc(size);
			if(bingI->accountKey)
			{
				strlcpy(bingI->accountKey, account_key, size);
				bingI->accountKey[size - 1] = '\0';

				bing_mem_free(preKey);

				res = TRUE;
			}
			else
			{
				bingI->accountKey = preKey;
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
	char* ret = NULL;
	char* find;
	size_t size;
	size_t pos;
	unsigned char b;
	if(bytes)
	{
		size = strlen(url) + 1;
		ret = (char*)bing_mem_malloc(size * 3); //We don't know how many of these bytes we will need, better to be safe then sorry
		if(ret)
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
	}
	if(!ret)
	{
		ret = (char*)bing_mem_calloc(1, sizeof(char));
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

	//TODO: If the request is a translation request, convert to URL. If the request is composite, and contains a translation URL, convert both to URLs, then get the index of the translation request, specify it and space-seperate the URLs. So translation is 2nd request: "normal_url 1 translation_url"

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
					bing_mem_free((void*)sourceTypeTmp);
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
			//Get composite source type
			sourceType = request_get_composite_sourcetype(req);

			//Format for the URL
			sourceTypeTmp = bing_mem_malloc(strlen(sourceType) + 27); //Includes format, ?, &, and null char
			if(sourceTypeTmp)
			{
				//Format it so it is correct for a composite request
				if(snprintf(sourceTypeTmp, strlen(sourceType) + 27, "Composite?Sources=%%27%s%%27&", sourceType) < 0)
				{
					bing_mem_free((void*)sourceTypeTmp);
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
	//If the field actually has a value then we check it, otherwise skip it.
	if(fieldID)
	{
		for(; searchFields != NULL; searchFields = searchFields->next)
		{
			//Make sure the variable and type match (we don't want to return a String for something that needs to be a long or double)
			if(searchFields->field.variableValue == fieldID &&
					!(checkType && searchFields->field.type != type))
			{
				if(sourceType == BING_SOURCETYPE_CUSTOM)
				{
					//If the type is custom, anything goes.
					return searchFields->field.name;
				}
				else if(searchFields->field.sourceTypeCount == BING_FIELD_SUPPORT_ALL_FIELDS)
				{
					//If every field type is supported, it's good.
					return searchFields->field.name;
				}
				else
				{
					//Fields support certain types, see if the type matches
					for(i = 0; i < searchFields->field.sourceTypeCount; i++)
					{
						if(searchFields->field.supportedTypes[i] == sourceType)
						{
							return searchFields->field.name;
						}
					}
				}
			}
		}
	}
	return NULL;
}

#define SNPRINTF_CAST(x) snprintf(buffer, BUFFER_SIZE, format, *((x*)data[0]))
#define SNPRINTF_NO_CAST snprintf(buffer, BUFFER_SIZE, format, data[0])

char getFormatComp(const char* format, BOOL getLength)
{
	char* c = (char*)format;
	char* tmp;
	while(*c)
	{
		//First find the format
		tmp = strchr(c, '%');
		if(!tmp)
		{
			//If format isn't found, stop
			break;
		}

		//If the format is %%, move one up
		if(*(tmp + 1) == '%')
		{
			tmp++;
		}
		else
		{
			//Go to the next char after format indicator
			tmp++;

			//Find the format (we ignore all chars but letters)
			while(*tmp)
			{
				if(isalpha(*tmp))
				{
					//We found letters, they might be length chars
					if(*tmp == 'h' || *tmp == 'l' || *tmp == 'L' || *tmp == 'j' || *tmp == 't' || *tmp == 'z')
					{
						if(getLength)
						{
							break;
						}
					}
					else if(!getLength)
					{
						break;
					}
				}
				else if(isspace(*tmp))
				{
					//Found whitespace, means we went past the format
					return '\0';
				}
				tmp++;
			}

			//Did we get to the end of the string? If so, then reset so that the "main loop" will end
			if(*tmp == '\0')
			{
				tmp--;
			}
			else
			{
				//Found the format, end
				return *tmp;
			}
		}
		c = tmp + 1;
	}
	return '\0';
}

void append_data(hashtable_t* table, const char* format, const char* key, void** data, size_t* curDataSize, char** returnData, size_t* returnDataSize)
{
#define BUFFER_SIZE 256

	char buffer[BUFFER_SIZE];
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

			//A relatively terrible way to format the data, but it allows us to have a generic method of getting data and formatting it, at least from outside the function
			switch(getFormatComp(format, FALSE))
			{
				case 'c':
					SNPRINTF_CAST(unsigned char);
					break;
				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'X':
					if(getFormatComp(format, TRUE) == 'l')
					{
						SNPRINTF_CAST(long long);
					}
					else
					{
						SNPRINTF_CAST(int);
					}
					break;
				case 'a':
				case 'A':
				case 'e':
				case 'E':
				case 'f':
				case 'F':
				case 'g':
				case 'G':
					SNPRINTF_CAST(double);
					break;
				default:
					SNPRINTF_NO_CAST;
					break;
			}
			buffer[BUFFER_SIZE - 1] = '\0';

			returnDataSize[0] += strlen(buffer);
			rett = bing_mem_realloc(returnData[0], returnDataSize[0]);
			if(rett)
			{
				strlcat(rett, buffer, returnDataSize[0]);
				returnData[0] = rett;
			}
		}
	}

#undef BUFFER_SIZE
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
