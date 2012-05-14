/*
 * bing_cpp.h
 *
 *  Created on: Dec 26, 2011
 *      Author: Vincent Simonetti
 */

#ifndef BING_CPP_H_
#define BING_CPP_H_

#include <bing.h>

#if defined (BING_Qt)
#include <QtCore/QString>
#endif

#if defined (__cplusplus) || defined(__CPLUSPLUS__)

#if !defined(_CPP_LIB_DECL)
#define _CPP_LIB_DECL extern "C++" {
#endif

_CPP_LIB_DECL

/*
 * Notes:
 *
 * All text is in UTF8 format.
 *
 * Default "new" and "delete" operators are used.
 */

namespace bing_cpp //Not really the greatest name, but getting errors compiling otherwise.
{
	//Class forwards
	class bing_service_request;
	class bing_service_response;

	/*
	 * Function delegates
	 */

	typedef void (*receive_bing_cpp_response_func) (bing_service_response response, void* user_data);

	/**
	 * A Bing service object that can be used to search using Microsoft's Bing services.
	 */
	class bing_service
	{
		unsigned int ID;

	public:
		/**
		 * @brief Create a new Bing service.
		 *
		 * The @cpp bing_service() constructor allows developers to allocate a Bing service object to
		 * perform search operations using Microsoft's Bing services.
		 *
		 * @param bingService Another instance of a Bing service element in which to copy the App ID from.
		 */
		bing_service(const bing_service& bingService);

		/**
		 * @brief Create a new Bing service.
		 *
		 * The @cpp bing_service() constructor allows developers to allocate a Bing service object to
		 * perform search operations using Microsoft's Bing services.
		 *
		 * @param application_ID The application ID which allows a developer to access
		 * 	Microsoft Bing services. If this string is not NULL, it is copied for use by
		 * 	the service. So the developer can free the memory when he is done.
		 */
		bing_service(const char* application_ID);

		//TODO: QString constructor (don't forget reference constructor)

		/**
		 * @brief Destroy and free a Bing service.
		 *
		 * The @cpp ~bing_service() destructor allows developers to free a Bing service object.
		 */
		~bing_service();

#if defined(BING_DEBUG)

		/**
		 * @brief Set if the internal parser should return on search errors.
		 *
		 * The @cpp set_error_return() function allows a developer to explicitly handle errors
		 * that might show up from the service. Examples of such are bad Application IDs,
		 * unsupported search requests, etc.
		 *
		 * If this is set to a non-zero value (true), then it will return a special response
		 * that specifies error information. If it is a zero value (false) and an error
		 * occurs, it will simply stop execution and clean up the IO connections.
		 *
		 * @param error A boolean indicating if error cases should be returned.
		 *
		 * @return A boolean indicating if the value was set or not.
		 */
		bool error_return(bool error);

		/**
		 * @brief Get if the internal parser should return on search errors.
		 *
		 * If this is set to a non-zero value (true), then it will return a special response
		 * that specifies error information. If it is a zero value (false) and an error
		 * occurs, it will simply stop execution and clean up the IO connections.
		 *
		 * @return A boolean indicating if error cases should be returned.
		 */
		bool error_return() const;

#endif

		/**
		 * @brief Get a Bing service's application ID.
		 *
		 * The @cpp app_ID() function allows developers to get the service's current
		 * application ID.
		 *
		 * @param buffer The buffer to copy the application ID to. If this is NULL, then
		 * 	the application ID length (plus NULL char) is returned.
		 *
		 * @return The length of the application ID, or -1 if an error occurred.
		 */
		int get_app_ID(char* buffer) const;

		/**
		 * @brief Set a Bing service's application ID.
		 *
		 * The @cpp app_ID() function allows developers to set the service's
		 * application ID. Allowing for different IDs to be used for different
		 * searches.
		 *
		 * @param appId The application ID to set. Only if this is not NULL and
		 * 	has a non-zero length (not including NULL char) will the app ID be
		 * 	copied to the Bing service. If an error occurs with copying then the
		 * 	original app ID remains unchanged. The ID is copied so the developer
		 * 	can free the data when the function returns.
		 *
		 * @return A boolean value specifying if the function completed successfully.
		 */
		bool set_app_ID(const char* appId);

		//TODO: QString get/set_app_ID (don't forget reference functions)

		/**
		 * @brief Get a Bing service's unique ID.
		 *
		 * The @cpp unique_bing_id() function allows developers to get the service's current
		 * unique ID, this allows for a developer to use this ID with the C Bing functions.
		 *
		 * @return The unique Bing service ID.
		 */
		unsigned int unique_bing_id() const;

		/**
		 * @brief Perform a synchronous search.
		 *
		 * The @c search_sync() function allows developers to perform a blocking
		 * search operation that will return when the search is complete.
		 *
		 * @param query The search query to perform. If this is NULL, then the function
		 * 	returns immediately with no response.
		 * @param request The type of search to perform. This determines the response
		 * 	that will be returned. If this is NULL, then the function returns
		 * 	immediately with no response.
		 *
		 * @return A response to the search query and request. This object, when not
		 * 	NULL for errors, is allocated and should be freed using the delete operator.
		 */
		bing_service_response* search_sync(const char* query, const bing_service_request* request);

		/**
		 * @brief Perform a synchronous search.
		 *
		 * The @c search_sync() function allows developers to perform a blocking
		 * search operation that will return when the search is complete.
		 *
		 * @param query The search query to perform. If this is NULL, then the function
		 * 	returns immediately with no response.
		 * @param request The type of search to perform. This determines the response
		 * 	that will be returned. If this is NULL, then the function returns
		 * 	immediately with no response.
		 *
		 * @return A response to the search query and request. This object, when not
		 * 	NULL for errors, is allocated and should be freed using the delete operator.
		 */
		const bing_service_response& search_sync(const char* query, const bing_service_request& request);

		//TODO: Search functionality (async)
	};

	/**
	 * Base class for all request objects.
	 */
	class bing_service_request
	{
	protected:
		bing_request_t request;
		bing_service_request();

	public:
		~bing_service_request();

		/**
		 * @brief Create a Bing service request from a C Bing request.
		 *
		 * The @cpp create_from_request function takes a C Bing request and makes
		 * a Bing service request from it.
		 *
		 * @param request The C Bing request to wrap.
		 *
		 * @return The wrapped C Bing request.
		 */
		const bing_service_request* create_from_request(const bing_request_t request);

		/**
		 * @brief Get a Bing service request.
		 *
		 * The @cpp get_request() function allows developers to get the service request
		 * current C request, this allows for a developer to use this request with the
		 * C Bing functions.
		 *
		 * @return The Bing service request.
		 */
		bing_request_t get_request() const;

		//TODO
	};

	/**
	 * Base class for all Response objects.
	 */
	class bing_service_response
	{
		bing_response_t response;

		//TODO
	};

	/**
	 * Base class for all Result objects.
	 */
	class bing_service_result
	{
		bing_result_t result;

		//TODO
	};
};

__END_DECLS

#endif

#endif /* BING_CPP_H_ */
