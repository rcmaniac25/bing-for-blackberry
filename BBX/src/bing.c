/*
 * bing.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * @author Vincent Simonetti
 */

#include "bing.h"
#include <string.h>
#include <stdlib.h>
#include <bps/bps.h>

#if !defined(BOOL)
#define BOOL int

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif
#endif

#define DEFAULT_ERROR_RET TRUE

typedef struct BING_S
{
	char* appId;
#if defined(BING_DEBUG)
	BOOL errorRet;
#endif
} bing;

typedef struct BING_SYSTEM_S
{
	int domainID;

	int count;
	bing** bingInstances;
} bing_system;

static bing_system bingSystem;
static BOOL initialized = FALSE;

void initialize()
{
	if(!initialized)
	{
		memset(&bingSystem, 0, sizeof(bing_system));

		bingSystem.domainID = bps_register_domain();
		bingSystem.count = 0;
		bingSystem.bingInstances = NULL;

		initialized = TRUE;
	}
}

int bing_get_domain()
{
	initialize();

	return bingSystem.domainID;
}

unsigned int create_bing(const char* application_ID)
{
	int appIdLen;
	int ret = 0; //Zero is reserved for bad
	bing* bingI;
	bing** in;

	initialize();

	if(bingSystem.domainID != -1)
	{
		//TODO: Need to lock

		//Reallocate the instance length
		if(bingSystem.bingInstances == NULL)
		{
			in = malloc(sizeof(bing*));
		}
		else
		{
			in = realloc(bingSystem.bingInstances, (bingSystem.count + 1) * sizeof(bing*));
		}
		if(in != NULL)
		{
			//Reset instances
			bingSystem.bingInstances = in;
			bingSystem.count++;

			//Create Bing instance
			in[bingSystem.count - 1] = bingI = (bing*)malloc(sizeof(bing));

			if(bingI == NULL)
			{
				//Didn't work, revert
				if(bingSystem.count > 1)
				{
					in = realloc(bingSystem.bingInstances, (bingSystem.count - 1) * sizeof(bing*));

					bingSystem.bingInstances = in;
					bingSystem.count--;
				}
				else
				{
					free(bingSystem.bingInstances);

					bingSystem.bingInstances = NULL;
					bingSystem.count = 0;
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

				ret = bingSystem.count;
			}
		}

		//TODO: Need to unlock
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
		//TODO: Need to lock

		if(bingSystem.count >= bingID)
		{
			bingI = bingSystem.bingInstances[bingID - 1];

			//Free the system
			if(bingSystem.count == 1)
			{
				free(bingSystem.bingInstances);

				bingSystem.bingInstances = NULL;
				bingSystem.count = 0;
			}
			else
			{
				bingSystem.count--;
				in = (bing**)malloc(sizeof(bing*) * bingSystem.count);

				if(in != NULL) //Hopefully this is always true, otherwise a memory leak will occur and it won't be pretty
				{
					if(bingID == 1)
					{
						memcpy(in, bingSystem.bingInstances, sizeof(bing*) * bingSystem.count);
					}
					else
					{
						memcpy(in, bingSystem.bingInstances, sizeof(bing*) * (bingID - 1));
						memcpy(in + (bingID - 1), bingSystem.bingInstances + bingID, sizeof(bing*) * (bingSystem.count - (bingID - 1)));
					}

					free(bingSystem.bingInstances);

					bingSystem.bingInstances = in;
				}
			}

			free(bingI->appId);
			free(bingI);
		}

		//TODO: Need to unlock
	}
}

#if defined(BING_DEBUG)
void set_error_return(unsigned int bingID, int error)
{
	bing* bingI = NULL;

	initialize(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		//TODO: Need to lock

		if(bingSystem.count >= bingID)
		{
			bingI = bingSystem.bingInstances[bingID - 1];
		}

		//TODO: Need to unlock
	}

	if(bingI != NULL)
	{
		bingI->errorRet = error;
	}
}

int get_error_return(unsigned int bingID)
{
	int res = FALSE;

	initialize(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		//TODO: Need to lock

		if(bingSystem.count >= bingID)
		{
			res = bingSystem.bingInstances[bingID - 1]->errorRet;
		}

		//TODO: Need to unlock
	}

	return res;
}
#endif

const char* get_app_ID(unsigned int bingID)
{
	const char* res = NULL;

	initialize(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		//TODO: Need to lock

		if(bingSystem.count >= bingID)
		{
			res = bingSystem.bingInstances[bingID - 1]->appId;
		}

		//TODO: Need to unlock
	}

	return res;
}

void set_app_ID(unsigned int bingID, const char* appId)
{
	bing* bingI;

	initialize(); //This shouldn't be here but put it here for safety

	if(bingSystem.domainID != -1 && bingID > 0)
	{
		//TODO: Need to lock

		if(bingSystem.count >= bingID)
		{
			bingI = bingSystem.bingInstances[bingID - 1];

			free(bingI->appId);
			bingI->appId = (char*)malloc(strlen(appId) + 1);
			if(bingI->appId != NULL)
			{
				bingI->appId = strcpy(bingI->appId, appId);
			}
		}

		//TODO: Need to unlock
	}
}
