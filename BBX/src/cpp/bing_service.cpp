/*
 * bing_service.cpp
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "../bing_internal.h"

#if defined (__cplusplus) || defined(__CPLUSPLUS__)

using namespace bing_cpp;

bing_service::bing_service(const bing_service& bingService)
{
	char* IDstr;
	int len;

	ID = 0;

	//Copy the App ID
	len = bing_get_app_ID(bingService.ID, NULL);
	if(len > 0)
	{
		IDstr = (char*)bing_mem_malloc(len);
		if(IDstr)
		{
			bing_get_app_ID(bingService.ID, IDstr);

			ID = bing_create(IDstr);

			bing_mem_free(IDstr);
		}
	}

	//Create this if it could not be created before
	if(ID == 0)
	{
		ID = bing_create(NULL);
	}

#if defined(BING_DEBUG)
	//Copy the error_return value
	bing_set_error_return(ID, bing_get_error_return(bingService.ID));
#endif
}

bing_service::bing_service(const char* application_ID)
{
	ID = bing_create(application_ID);
}

bing_service::~bing_service()
{
	bing_free(ID);
	ID = 0;
}

#if defined(BING_DEBUG)

bool bing_service::error_return(bool error)
{
	return BOOL_TO_CPP_BOOL(bing_set_error_return(ID, CPP_BOOL_TO_BOOL(error)));
}

bool bing_service::error_return() const
{
	return BOOL_TO_CPP_BOOL(bing_get_error_return(ID));
}

#endif

int bing_service::get_app_ID(char* buffer) const
{
	return bing_get_app_ID(ID, buffer);
}

bool bing_service::set_app_ID(const char* appId)
{
	return BOOL_TO_CPP_BOOL(bing_set_app_ID(ID, appId));
}

unsigned int bing_service::unique_bing_id() const
{
	return ID;
}

bing_service_response* bing_service::search_sync(const char* query, const bing_service_request* request)
{
	bing_response_t res = bing_search_sync(ID, query, request ? request->get_request() : NULL);

	//TODO: create response
	return NULL;
}

#endif
