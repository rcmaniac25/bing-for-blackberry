/*
 * Bing.h
 *
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Authors: Microsoft, Vincent Simonetti
 */

#ifndef BING_H_
#define BING_H_

#include <sys/cdefs.h>
#include <process.h>

__BEGIN_DECLS

/*
 * Structures
 */

struct _bing_result;
struct _bing_response;
struct _bing_request;

typedef struct _bing_result* bing_result_t;
typedef struct _bing_response* bing_response_t;
typedef struct _bing_request* bing_request_t;

typedef void (*receive_bing_response_func) (bing_response_t response);

/*
 * Event handling functions
 */

/**
 * @brief Get the Bing service's unique domain ID for use in event processing.
 *
 * @return A domain ID for a Bing service. Returning -1 indicates an error.
 */
int bing_get_domain();

/*
 * Bing search system
 */

/**
 * @brief Create a new Bing service.
 *
 * The @c create_bing() function allows developers to allocate a Bing service object to
 * perform search operations using Microsoft's Bing services.
 *
 * @param application_ID The application ID which allows a developer to access
 * 	Microsoft Bing services. If this string is not NULL, it is copied for use by
 * 	the service. So the developer can free the memory when he is done.
 *
 * @return A unique Bing service ID that is used to perform searches. If this value is
 * 	zero then an error has occurred and no service has been allocated.
 */
unsigned int create_bing(const char* application_ID);

/**
 * @brief Free a Bing service.
 *
 * The @c free_bing() function allows developers to free allocated memory used
 * 	for a Bing service.
 *
 * @param bing The unique Bing ID for each application ID.
 *
 * @return Nothing is returned.
 */
void free_bing(unsigned int bing);

#if defined(BING_DEBUG)

/**
 * @brief Set if the internal parser should return on search errors.
 *
 * The @c set_error_return() function allows a developer to explicitly handle errors
 * that might show up from the service. Examples of such are bad Application IDs,
 * unsupported search requests, etc.
 *
 * If this is set to a non-zero value (true), then it will return a special response
 * that specifies error information. If it is a zero value (false) and an error
 * occurs, it will simply stop execution and clean up the IO connections.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param error A boolean integer indicating if error cases should be returned.
 * 	Zero is false, non-zero is true.
 *
 * @return Nothing is returned.
 */
int set_error_return(unsigned int bing, int error);

/**
 * @brief Get if the internal parser should return on search errors.
 *
 * If this is set to a non-zero value (true), then it will return a special response
 * that specifies error information. If it is a zero value (false) and an error
 * occurs, it will simply stop execution and clean up the IO connections.
 *
 * @param bing The unique Bing ID for each application ID.
 *
 * @return A boolean integer indicating if error cases should be returned. Zero is false,
 * non-zero is true.
 */
int get_error_return(unsigned int bing);

#endif

/**
 * @brief Get a Bing service's application ID.
 *
 * The @c get_app_ID() function allows developers to get the service's current
 * application ID.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param buffer The buffer to copy the application ID to. If this is NULL, then
 * 	the application ID length (plus NULL char) is copied into the len parameter.
 * @param len The length of the allocated buffer. If this is NULL, no operation
 * 	will take place. If buffer is NULL, then this will be set to the application
 * 	ID length. If buffer is not null but len states that it is not big enough to
 * 	hold the application ID, then it the operation fails and NULL will be
 * 	returned by the function.
 *
 * @return A copy of the buffer parameter. If the buffer is not NULL and an error
 * 	occurs, then this will be NULL.
 */
const char* get_app_ID(unsigned int bing, char* buffer, int* len);

/**
 * @brief Set a Bing service's application ID.
 *
 * The @c set_app_ID() function allows developers to set the service's
 * application ID. Allowing for different IDs to be used for different
 * searches.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param appId The application ID to set. Only if this is not NULL and
 * 	has a non-zero length (not including NULL char) will the app ID be
 * 	copied to the Bing service. If an error occurs with copying then the
 * 	original app ID remains unchanged. The ID is copied so the developer
 * 	can free the data when the function returns.
 *
 * @return A boolean value specifying if the function completed successfully.
 * 	If this is a non-zero value then the operation completed. Otherwise it
 * 	failed.
 */
int set_app_ID(unsigned int bing, const char* appId);

/**
 * @brief Perform a synchronous search.
 *
 * The @c search_sync() function allows developers to perform a blocking
 * search operation that will return when the search is complete.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param query The search query to perform. If this is NULL, then the function
 * 	returns immediately with no response.
 * @param request The type of search to perform. This determines the response
 * 	that will be returned. If this is NULL, then the function returns
 * 	immediately with no response.
 *
 * @return A response to the search query and request. This object, when not
 * 	NULL for errors, is allocated and should be freed using the free_response
 * 	function to prevent memory leaks.
 */
bing_response_t search_sync(unsigned int bing, const char* query, bing_request_t request);

/**
 * @brief Perform a asynchronous search.
 *
 * The @c search_async() function allows developers to perform a non-blocking
 * search operation that will return immediately and call the specified callback
 * function with the response.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param query The search query to perform. If this is NULL, then the function
 * 	returns a zero (false) value.
 * @param request The type of search to perform. This determines the response
 * 	that will be returned. If this is NULL, then the function function returns
 * 	a zero (false) value.
 * @param response_func The function that will be called with a response from
 * 	the search. If this is NULL, then the function returns a zero (false) value.
 *
 * @return A boolean result which is non-zero for a successful query, otherwise
 * 	zero on error or bad query.
 */
int search_async(unsigned int bing, const char* query, bing_request_t request, receive_bing_response_func response_func);

/**
 * @brief Perform a asynchronous search but returns with an event.
 *
 * The @c search_event_async() function allows developers to perform a non-blocking
 * search operation that will return immediately and delegate an event with
 * the response.
 *
 * To determine if the event is a Bing response, use the bing_get_domain() function.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param query The search query to perform. If this is NULL, then the function
 * 	returns a zero (false) value.
 * @param request The type of search to perform. This determines the response
 * 	that will be returned. If this is NULL, then the function function returns
 * 	a zero (false) value.
 *
 * @return A boolean result which is non-zero for a successful query, otherwise
 * 	zero on error or bad query.
 */
int search_event_async(unsigned int bing, const char* query, bing_request_t request);

/**
 * @brief Get a URL that can invoke a Bing search request.
 *
 * The @c request_url() function allows developers to create a URL for performing
 * 	a custom search query. This is not the same as a custom request, but can allow
 * 	a developer to custom tailor the search URL and handle the result in whatever
 * 	manner they deem necessary.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param query The search query to perform. If this is NULL, then the function
 * 	returns a URL with an empty query.
 * @param request The type of search to perform. This determines the response
 * 	that will be returned. If this is NULL, then the function function returns
 * 	NULL.
 *
 * @return A URL string that can be used for searches. This function allocates
 * 	memory with malloc and it is up to the developer to free it to avoid
 * 	memory leaks.
 */
const char* request_url(unsigned int bing, const char* query, bing_request_t request);

/*
 * Request functions
 */

enum
{
	BING_REQUEST_UNKNOWN,

	//A custom request
	BING_REQUEST_CUSTOM,

	//A bundled request
	BING_REQUEST_BUNDLE,

	//Standard requests
	BING_REQUEST_AD,
	BING_REQUEST_IMAGE,
	BING_REQUEST_INSTANT_ANWSER,
	BING_REQUEST_MOBILE_WEB,
	BING_REQUEST_NEWS,
	BING_REQUEST_PHONEBOOK,
	BING_REQUEST_RELATED_SEARCH,
	BING_REQUEST_SPELL,
	BING_REQUEST_TRANSLATION,
	BING_REQUEST_VIDEO,
	BING_REQUEST_WEB
};

//TODO

void free_request(bing_request_t request);

/*
 * Response functions
 */

enum
{
	BING_RESPONSE_UNKNOWN,

	//A custom response
	BING_RESPONSE_CUSTOM,

	//A bundled response
	BING_RESPONSE_BUNDLE,

	//Standard responses
	BING_RESPONSE_AD,
	BING_RESPONSE_IMAGE,
	BING_RESPONSE_INSTANT_ANWSER,
	BING_RESPONSE_MOBILE_WEB,
	BING_RESPONSE_NEWS,
	BING_RESPONSE_PHONEBOOK,
	BING_RESPONSE_RELATED_SEARCH,
	BING_RESPONSE_SPELL,
	BING_RESPONSE_TRANSLATION,
	BING_RESPONSE_VIDEO,
	BING_RESPONSE_WEB
};

//TODO

void free_response(bing_response_t response);

/*
 * Result functions
 */

enum
{
	BING_RESULT_UNKNOWN,

	//A custom result
	BING_RESULT_CUSTOM,

	//A error result
	BING_RESULT_ERROR,

	//Standard result
	BING_RESULT_AD,
	BING_RESULT_IMAGE,
	BING_RESULT_INSTANT_ANWSER,
	BING_RESULT_MOBILE_WEB,
	BING_RESULT_NEWS,
	BING_RESULT_PHONEBOOK,
	BING_RESULT_RELATED_SEARCH,
	BING_RESULT_SPELL,
	BING_RESULT_TRANSLATION,
	BING_RESULT_VIDEO,
	BING_RESULT_WEB
};

//TODO

__END_DECLS

#endif /* BING_H_ */
