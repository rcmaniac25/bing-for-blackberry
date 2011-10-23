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
#include <stdlib.h>

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
 * @return A boolean integer indicating if the value was set or not. Zero is false,
 * 	non-zero is true.
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
 * 	the application ID length (plus NULL char) is returned.
 *
 * @return The length of the application ID, or -1 if an error occurred.
 */
int get_app_ID(unsigned int bing, char* buffer);

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

typedef enum
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
} REQUEST_TYPE;

//TODO

void free_request(bing_request_t request);

/*
 * Response functions
 */

typedef enum
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
} RESPONSE_TYPE;

//TODO

void free_response(bing_response_t response);

/*
 * Result functions
 */

typedef enum
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
} RESULT_TYPE;

typedef enum
{
	//All values are strings unless otherwise noted.

	RESULT_FIELD_UNKNOWN,

	//64bit integer
	RESULT_FIELD_RANK,
	RESULT_FIELD_POSITION,
	RESULT_FIELD_TITLE,
	RESULT_FIELD_DESCRIPTION,
	RESULT_FIELD_DISPLAY_URL,
	RESULT_FIELD_ADLINK_URL
} RESULT_FIELD;

//Standard operations

/**
 * @brief Get the Bing result type.
 *
 * The @c result_get_type() functions allows developers to retrieve the
 * type of Bing result that is passed in.
 *
 * @param result The Bing result to get the type of.
 *
 * @return The Bing result type, or BING_RESULT_UNKNOWN if NULL is passed in.
 */
RESULT_TYPE result_get_type(bing_result_t result);

/**
 * @brief Get a value from a Bing result.
 *
 * The @c result_get_*() functions allows developers to retrieve values from
 * a Bing result. All values are self contained and will be copied to
 * the value parameter.
 *
 * In the case of string and array, the return type is the amount of data,
 * in bytes. If value is NULL then nothing is copied.
 *
 * For array types, the actual data is copied, not pointers to the data.
 *
 * @param result The Bing result to retrieve data from.
 * @param field The field to get the data of. If the field doesn't support
 * 	the data type that the function specifies or the field isn't
 * 	supported, then the function fails.
 * @param value The value to copy data into. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing result.
 *
 * @return A boolean value which is non-zero for a successful data retrieval,
 * 	otherwise zero on error or invalid field. Note that for array and string
 * 	types, the length of the data in bytes is returned.
 */

int result_get_64bit_int(bing_result_t result, RESULT_FIELD field, long long* value);
int result_get_string(bing_result_t result, RESULT_FIELD field, char* value);
int result_get_double(bing_result_t result, RESULT_FIELD field, double* value);
int result_get_boolean(bing_result_t result, RESULT_FIELD field, int* value);
int result_get_array(bing_result_t result, RESULT_FIELD field, void** value);

//Custom operations

/**
 * @brief Get a custom value from a Bing result.
 *
 * The @c result_custom_get_*() functions allows developers to retrieve
 * values from a Bing result. All values are self contained and will be
 * copied to the value parameter. These are the same functions as
 * result_get_* but with the actual field name passed. These functions
 * work on all result types but allow for retrieval of custom result
 * values.
 *
 * In the case of string and array, the return type is the amount of data,
 * in bytes. If value is NULL then nothing is copied.
 *
 * For array types, the actual data is copied, not pointers to the data.
 *
 * @param result The Bing result to retrieve data from.
 * @param field The field name to get the data of. If the field doesn't
 * 	support the data type that the function specifies or the field isn't
 * 	supported, then the function fails.
 * @param value The value to copy data into. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing result.
 *
 * @return A boolean value which is non-zero for a successful data retrieval,
 * 	otherwise zero on error or invalid field. Note that for array and string
 * 	types, the length of the data in bytes is returned.
 */

int result_custom_get_64bit_int(bing_result_t result, const char* field, long long* value);
int result_custom_get_string(bing_result_t result, const char* field, char* value);
int result_custom_get_double(bing_result_t result, const char* field, double* value);
int result_custom_get_boolean(bing_result_t result, const char* field, int* value);
int result_custom_get_array(bing_result_t result, const char* field, void** value);

/**
 * @brief Set a custom value for a Bing result.
 *
 * The @c result_custom_set_*() functions allows developers to set
 * values to a custom Bing result. If the result is not the custom type
 * then the function will fail. All values are self contained and will be
 * copied from the value parameter.
 *
 * In the case of string and array, the entire data amount is copied
 * using strlen for string or the size parameter for array.
 *
 * For array types, the actual data is copied, not pointers to the data.
 *
 * If the field does not exist then it will be created, if and only if
 * value is not NULL. If the value is NULL and the field exists, it will
 * be removed.
 *
 * If the result has been made immutable, then this function will fail.
 *
 * @param result The Bing result to set data to.
 * @param field The field name to get the data of. If the field already
 * 	exists, the data will be replaced. If the field doesn't exist and
 * 	the value is not NULL, then the field will be created. If the field
 * 	exists and the value is NULL, the field is removed.
 * @param value The value to copy data from. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing result.
 * 	If this is NULL then no effect occurs unless the field exists, in which
 * 	case the field is removed.
 * @param size The size of the array data in bytes, so if an array of 3 int
 * 	are passed in, size would be (sizeof(int) * 3).
 *
 * @return A boolean value which is non-zero for a successful data set,
 * 	otherwise zero on error.
 */

int result_custom_set_64bit_int(bing_result_t result, const char* field, long long* value);
int result_custom_set_string(bing_result_t result, const char* field, const char* value);
int result_custom_set_double(bing_result_t result, const char* field, double* value);
int result_custom_set_boolean(bing_result_t result, const char* field, int* value);
int result_custom_set_array(bing_result_t result, const char* field, const void* value, size_t size);

/**
 * @brief Create a custom Bing result.
 *
 * The @c result_custom_create() functions allows developers to create
 * custom Bing results. These results will remain with the Bing response
 * until the Bing response if freed.
 *
 * Results are mutable by default so result_custom_set_* functions can be used
 * to modify them.
 *
 * The result is automatically added to the Bing response which will manage it's
 * memory.
 *
 * @param resultResponse The Bing response to have a Bing result added to.
 * 	If this is NULL or the response is of any type other then BING_RESPONSE_CUSTOM
 * 	then the function will fail.
 * @param result A pointer to where the returned result. Though it is not
 * 	necessary for this to be set (it can be NULL), the developer must go through
 * 	all response results to find the mutable one which was created.
 *
 * @return A boolean value which is non-zero for a successful operation,
 * 	otherwise zero on error.
 */
int result_custom_create(bing_response_t resultResponse, bing_result_t* result);

/**
 * @brief Get if the Bign result is mutable or not.
 *
 * The @c result_is_mutable() functions allows developers to determine
 * if a result is mutable or not.
 *
 * @param result The custom Bing result to determine if it is
 * 	mutable or not.
 *
 * @return A boolean value which is non-zero for a successful operation,
 * 	otherwise zero on error, NULL result, or a standard Bing result type
 * 	(not BING_RESPONSE_CUSTOM).
 */
int result_is_mutable(bing_result_t result);

/**
 * @brief Finish creating a custom Bing result.
 *
 * The @c result_custom_creation_done() functions allows developers to
 * set the custom Bing result as done in creation and makes the result
 * immutable so result_custom_set_* functions will fail.
 *
 * @param result The custom Bing result to make immutable.
 *
 * @return A boolean value which is non-zero for a successful operation,
 * 	otherwise zero on error.
 */
int result_custom_creation_done(bing_result_t result);

__END_DECLS

#if defined (__cplusplus) || defined(__CPLUSPLUS__)

#if !defined(_CPP_LIB_DECL)
#define _CPP_LIB_DECL extern "C++" {
#endif

_CPP_LIB_DECL

namespace bing_cpp //Not really the greatest name, but getting errors compiling otherwise.
{
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
		 * @param application_ID The application ID which allows a developer to access
		 * 	Microsoft Bing services. If this string is not NULL, it is copied for use by
		 * 	the service. So the developer can free the memory when he is done.
		 */
		bing_service(const char* application_ID);

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
		bool error_return();

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
		int app_ID(char* buffer);

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
		bool app_ID(const char* appId);

		//TODO
	};
};

__END_DECLS

#endif

#endif /* BING_H_ */
