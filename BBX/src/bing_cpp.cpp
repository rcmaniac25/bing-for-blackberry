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

#if !defined(BOOL)
#define BOOL int

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif
#endif

#define BOOL_TO_CPP_BOOL(x) (x) != FALSE
#define CPP_BOOL_TO_BOOL(x) (x) ? TRUE : FALSE

//bing_service

bing_service::bing_service(const char* application_ID)
{
	ID = create_bing(application_ID);
}

bing_service::~bing_service()
{
	free_bing(ID);
	ID = 0;
}

#if defined(BING_DEBUG)

bool bing_service::error_return(bool error)
{
	return BOOL_TO_CPP_BOOL(set_error_return(ID, CPP_BOOL_TO_BOOL(error)));
}

bool bing_service::error_return()
{
	return BOOL_TO_CPP_BOOL(get_error_return(ID));
}

#endif

int bing_service::app_ID(char* buffer)
{
	return get_app_ID(ID, buffer);
}

bool bing_service::app_ID(const char* appId)
{
	return BOOL_TO_CPP_BOOL(set_app_ID(ID, appId));
}

#endif
