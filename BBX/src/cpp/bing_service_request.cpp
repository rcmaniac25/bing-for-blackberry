/*
 * bing_service_request.cpp
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "../bing_internal.h"

#if defined (__cplusplus) || defined(__CPLUSPLUS__)

using namespace bing_cpp;

//TODO: constructor

bing_service_request::~bing_service_request()
{
	bing_request_free(request);
	request = NULL;
}

const bing_service_request* bing_service_request::create_from_request(const bing_request_t request)
{
	if(request)
	{
		//TODO
	}
	return NULL;
}

bing_request_t bing_service_request::get_request() const
{
	return request;
}

#endif
