/*
 * bing_cpp.cpp
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#if defined (__cplusplus) || defined(__CPLUSPLUS__)

using namespace bing_cpp;

//bing_service

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

bool bing_service::error_return()
{
	return BOOL_TO_CPP_BOOL(bing_get_error_return(ID));
}

#endif

int bing_service::app_ID(char* buffer)
{
	return bing_get_app_ID(ID, buffer);
}

bool bing_service::app_ID(const char* appId)
{
	return BOOL_TO_CPP_BOOL(bing_set_app_ID(ID, appId));
}

unsigned int bing_service::unique_bing_id()
{
	return ID;
}

#endif
