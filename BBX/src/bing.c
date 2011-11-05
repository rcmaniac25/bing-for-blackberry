/*
 * bing.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

static volatile BOOL initialized = FALSE;

void initialize()
{
	if(!initialized)
	{
		initialized = TRUE; //Set this first so if another thread tries to initialize it, it doesn't work

		memset(&bingSystem, 0, sizeof(bing_system));

		bingSystem.domainID = bps_register_domain();
		bingSystem.bingInstancesCount = 0;
		bingSystem.bingInstances = NULL;
		pthread_mutex_init(&bingSystem.mutex, NULL);
	}
}

void shutdown()
{
	if(initialized)
	{
		if(bingSystem.bingInstancesCount > 0)
		{
			//TODO: GO through all services and free them
			bingSystem.bingInstancesCount = 0;
			free(bingSystem.bingInstances);
			bingSystem.bingInstances = NULL;
		}

		pthread_mutex_destroy(&bingSystem.mutex);

		initialized = FALSE; //Set this last so that it only know's it's uninitialized after everything has been freed
	}
}

int bing_get_domain()
{
	initialize();

	return bingSystem.domainID;
}

int findFreeIndex()
{
	int i;
	if(bingSystem.domainID != -1)
	{
		if(bingSystem.bingInstances != NULL)
		{
			for(i = 0; i < bingSystem.bingInstancesCount; i++)
			{
				if(bingSystem.bingInstances[i] == NULL)
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

	initialize();

	if(bingSystem.domainID != -1)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		//Reallocate the instance length
		if(bingSystem.bingInstances == NULL)
		{
			in = malloc(sizeof(bing*));
		}
		else
		{
			in = realloc(bingSystem.bingInstances, (bingSystem.bingInstancesCount + 1) * sizeof(bing*));
		}
		if(in != NULL)
		{
			//Reset instances
			in[bingSystem.bingInstancesCount - 1] = NULL;
			bingSystem.bingInstances = in;
			bingSystem.bingInstancesCount++;

			//Find a free index
			loc = findFreeIndex();

			//Create Bing instance
			in[loc] = bingI = (bing*)malloc(sizeof(bing));

			if(bingI == NULL)
			{
				//Didn't work, revert
				if(bingSystem.bingInstancesCount > 1)
				{
					bingSystem.bingInstancesCount--;
					if(loc == bingSystem.bingInstancesCount) //At end of instances, free up some space
					{
						in = realloc(bingSystem.bingInstances, bingSystem.bingInstancesCount * sizeof(bing*));

						if(in != NULL)
						{
							bingSystem.bingInstances = in;
						}
					}
				}
				else
				{
					free(bingSystem.bingInstances);

					bingSystem.bingInstances = NULL;
					bingSystem.bingInstancesCount = 0;
				}
			}
			else
			{
				memset(bingI, 0, sizeof(bing));

				//Copy application ID
				if(application_ID != NULL)
				{
					bingI->appId = (char*)malloc(strlen(application_ID) + 1);
					if(bingI->appId != NULL)
					{
						bingI->appId = strcpy(bingI->appId, application_ID);
					}
					//If an error occurs, it's up to the developer to make sure that the app ID was copied
				}
#if defined(BING_DEBUG)
				bingI->errorRet = DEFAULT_ERROR_RET;
#endif

				pthread_mutex_init(&bingI->mutex, NULL);

				ret = bingSystem.bingInstancesCount;
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

	initialize(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		pthread_mutex_lock(&bingSystem.mutex);

		if(bingSystem.bingInstancesCount >= bingID)
		{
			bingI = bingSystem.bingInstances[bingID - 1];

			//Free the system
			if(bingSystem.bingInstancesCount == 1)
			{
				free(bingSystem.bingInstances);

				bingSystem.bingInstances = NULL;
				bingSystem.bingInstancesCount = 0;
			}
			else
			{
				bingSystem.bingInstancesCount--;
				bingSystem.bingInstances[bingID - 1] = NULL;
			}

			pthread_mutex_lock(&bingI->mutex);

			//TODO: Free all responses created for this that have not been freed already
			//TODO: Free all registered result, response, and request creators
			free(bingI->appId);

			pthread_mutex_destroy(&bingI->mutex);

			free(bingI);
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
}

bing* retrieveBing(unsigned int bingID)
{
	bing* bingI = NULL;

	initialize(); //This shouldn't be here but put it here for safety

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

	if(bingI != NULL)
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

	if(bingI != NULL)
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

	if(bingI != NULL)
	{
		pthread_mutex_lock(&bingI->mutex);

		ret = strlen(bingI->appId) + 1;

		if(buffer != NULL)
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

	if(appId != NULL && (size = strlen(appId) + 1) > 1)
	{
		bingI= retrieveBing(bingID);

		if(bingI != NULL)
		{
			pthread_mutex_lock(&bingI->mutex);

			preApp = bingI->appId;

			bingI->appId = (char*)malloc(size);
			if(bingI->appId != NULL)
			{
				bingI->appId = strcpy(bingI->appId, appId);

				free(preApp);

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
		size = sizeof(url) + 1;
		ret = (char*)malloc(size * 3); //We don't know how many of these bytes we will need, better to be safe then sorry
		if(ret != NULL)
		{
			memset(ret, 0, size * 3);
			pos = 0;

			while((b = *bytes) != 0)
			{
				find = strchr(URL_UNRESERVED, b);
				if(find != NULL)
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
	const char* rquestOptions;
	bing_request* req = (bing_request*)request;
	size_t urlSize = 46; //This is the length of the URL format

	if(bingI != NULL && request != NULL)
	{
		//Size of URL (it's constant but it could change so we don't want to hard code the size)
		urlSize += sizeof(BING_URL);

		//Encode the query and get its size
		queryStr = encodeUrl(query);
		urlSize += strlen(queryStr);

		//Size of the request source type
		urlSize += strlen(req->sourceType);

		//Get the request options and types and size
		rquestOptions = req->getOptions(request);
		urlSize += strlen(rquestOptions);

		//We want to lock it now before we use the application ID (since it can be changed)
		pthread_mutex_lock(&bingI->mutex);

		//Application ID and size
		appIdStr = bingI->appId;
		if(appIdStr == NULL)
		{
			appIdStr = "";
		}
		urlSize += strlen(appIdStr);

		//Allocate the url data
		ret = (char*)malloc(urlSize + 6); //The 6 is just for null chars as a precaution.
		if(ret != NULL)
		{
			//Zero everything (safer that way... Can never be too safe)
			memset(ret, 0, urlSize + 6);

			//Now actually create the URL
			if(sprintf(ret, "%sxmltype=attributebased&AppId=%s&Query=%s&Sources=%s%s", BING_URL, appIdStr, queryStr, req->sourceType, rquestOptions) < 0)
			{
				//Error
				free(ret);
				ret = NULL;
			}
		}

		//Let the Bing element go back to normal execution
		pthread_mutex_unlock(&bingI->mutex);

		//Free the strings
		req->finishGetOptions(request, rquestOptions);
		free((void*)queryStr);
	}
	return ret;
}

const char* find_field(bing_field_search* searchFields, int fieldID, enum FIELD_TYPE type, enum SOURCE_TYPE sourceType)
{
	int i;
	//If the field actually has a value then we check it, otherwise skip it. We also don't want to do anything with custom types (since it will fail anyway)
	if(fieldID && sourceType != BING_SOURCETYPE_CUSTOM)
	{
		for(; searchFields != NULL; searchFields = searchFields->next)
		{
			//Make sure the variable and type match (we don't want to return a String for something that needs to be a long or double)
			if(searchFields->field.variableValue == fieldID &&
					searchFields->field.type == type)
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
