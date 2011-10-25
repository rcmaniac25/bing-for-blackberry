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

#define BING_VERSION 2.2

/*
 * Structures
 */

typedef void* bing_result_t;
typedef void* bing_response_t;
typedef void* bing_request_t;

typedef void* data_dictionary_t;

typedef struct _bing_news_collection
{
	const char* name;
	unsigned int news_article_count;
	const bing_result_t* news_articles;
} bing_news_collection_s, *bing_news_collection_t;

typedef struct _bing_thumbnail
{
	const char* url;
	const char* content_type;
	long long height;
	long long width;
	long long file_size;
} bing_thumbnail_s, *bing_thumbnail_t;

typedef struct _bing_title_url
{
	const char* title;
	const char* url;
} bing_deep_link_s, bing_related_search_s, *bing_deep_link_t, *bing_related_search_t;

typedef struct _bing_search_tag
{
	const char* name;
	const char* value;
} bing_search_tag_s, *bing_search_tag_t;

/*
 * Function delegates
 */

typedef void (*receive_bing_response_func) (bing_response_t response);
typedef int (*response_creation_func)(const char* name, bing_response_t response, data_dictionary_t dictionary);
typedef void (*response_additional_data_func)(bing_response_t response, data_dictionary_t dictionary);
typedef int (*result_creation_func)(const char* name, bing_result_t result, data_dictionary_t dictionary);
typedef void (*result_additional_result_func)(const char* name, bing_result_t result, bing_result_t new_result);

/*
 * Dictionary functions
 */

/**
 * @brief Get data from a dictionary.
 *
 * The @c dictionary_get_data() function allows developers to retrieve data from a
 * dictionary.
 *
 * @param dict The dictionary to retrieve data from.
 * @param name The name of the field to retrieve from the dictionary.
 * @param data The buffer to copy the data into.
 *
 * @return The size of the data in bytes, or 0 if an error has occurred, dict is NULL,
 * 	or the name doesn't exist.
 */
int dictionary_get_data(data_dictionary_t dict, const char* name, void* data);

/**
 * @brief Get all the names within a dictionary.
 *
 * The @c dictionary_get_element_names() function allows developers to retrieve
 * an array of all names within a dictionary.
 *
 * @param dict The dictionary to retrieve the names from.
 * @param name The array to copy all the names into. malloc will be used
 * 	to allocate each individual string within the names array. It's up
 * 	to the developer to free each name string.
 *
 * @return The number of names within the dictionary, or -1 if an error occurred
 * or dict was NULL.
 */
int dictionary_get_element_names(data_dictionary_t dict, char** names);

/*
 * Event handling functions
 */

/**
 * @brief Get the Bing service's unique domain ID for use in event processing.
 *
 * @return A domain ID for a Bing service. Returning -1 indicates an error.
 */
int bing_get_domain();

//TODO: So an event shows up with the same domain... How does one get the response from it?

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
bing_response_t search_sync(unsigned int bing, const char* query, const bing_request_t request);

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
int search_async(unsigned int bing, const char* query, const bing_request_t request, receive_bing_response_func response_func);

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
int search_event_async(unsigned int bing, const char* query, const bing_request_t request);

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
const char* request_url(unsigned int bing, const char* query, const bing_request_t request);

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

//Standard functions

/**
 * @brief Get the Bing request type.
 *
 * The @c request_get_type() functions allows developers to retrieve the
 * type of Bing request that is passed in.
 *
 * @param request The Bing request to get the type of.
 *
 * @return The Bing request type, or BING_REQUEST_UNKNOWN if NULL is passed in.
 */
REQUEST_TYPE request_get_type(bing_request_t request);

//TODO: Figure out data functions necessary

/**
 * @brief Free a Bing request from memory.
 *
 * The @c free_request() function allows developers to free
 * entire Bing request.
 *
 * @param request The Bing request to free.
 *
 * @return Nothing is returned.
 */
void free_request(bing_request_t request);

//Custom functions

//TODO: Figure out data functions necessary

/* TODO
register a request creator
-source type (only non-standard)
-options callback
--*request
free a request creator
-source type (only custom)
 */

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

//Standard operations

/**
 * @brief Get the Bing response type.
 *
 * The @c response_get_type() function allows developers to retrieve the
 * type of Bing response that is passed in.
 *
 * @param response The Bing response to get the type of.
 *
 * @return The Bing response type, or BING_RESPONSE_UNKNOWN if NULL is passed in.
 */
RESPONSE_TYPE response_get_type(bing_response_t response);

/**
 * @brief Get the estimated total number of results for a Bing response.
 *
 * The @c response_get_total() function allows developers to retrieve the
 * estimated total number of results that response can get.
 *
 * This is not how any results that a response will have. This is the
 * "5 million hits" total.
 *
 * @param response The Bing response to get total size of.
 *
 * @return The estimated Bing response total.
 */
long long response_get_total(bing_response_t response);

/**
 * @brief Get the offset within the results for a Bing response.
 *
 * The @c response_get_offset() function allows developers to retrieve the
 * zero-based offset within the results results that response can get.
 * This allows for easier searching through many results.
 *
 * @param response The Bing response to get offset of.
 *
 * @return The Bing response offset.
 */
long long response_get_offset(bing_response_t response);

/**
 * @brief Get the query used to search for this Bing response.
 *
 * The @c response_get_query() function allows developers to retrieve the
 * actual query used to get this Bing response.
 *
 * @param response The Bing response to get the query of.
 * @param buffer The buffer to copy the query into.
 *
 * @return The size of the Bing response query in bytes, or -1
 * 	if an error occurred.
 */
int response_get_query(bing_response_t response, char* buffer);

/**
 * @brief Get the altered query used to search for this Bing response.
 *
 * The @c response_get_altered_query() function allows developers
 * to retrieve the altered search query, if appropriate, to get
 * this Bing response.
 *
 * @param response The Bing response to get the altered query of.
 * @param buffer The buffer to copy the altered query into.
 *
 * @return The size of the Bing response altered query in bytes, or
 * 	-1 if an error occurred.
 */
int response_get_altered_query(bing_response_t response, char* buffer);

/**
 * @brief Get the unaltered query used to search for this Bing response.
 *
 * The @c response_get_alterations_override_query() function allows
 * developers to retrieve the original, unaltered search query that
 * can be used to research with that won't be altered.
 *
 * @param response The Bing response to get the altered query of.
 * @param buffer The buffer to copy the unaltered query into.
 *
 * @return The size of the Bing response unaltered query in bytes, or
 * 	-1 if an error occurred.
 */
int response_get_alterations_override_query(bing_response_t response, char* buffer);

/**
 * @brief Get the results from a Bing response.
 *
 * The @c response_get_results() function allows developers to
 * get the actual search results.
 *
 * @param response The Bing response to get the results of.
 * @param results The array of results to copy into.
 *
 * @return The Bing response result count, or -1 if an error
 * 	occurred.
 */
int response_get_results(bing_response_t response, bing_result_t* results);

/**
 * @brief Free a Bing response from memory.
 *
 * The @c free_response() function allows developers to free
 * entire Bing response.
 *
 * This frees the response itself, the results the response
 * contains, and the memory allocated by the response and
 * results to allow the retrieval of some data.
 *
 * @param response The Bing response to free.
 *
 * @return Nothing is returned.
 */
void free_response(bing_response_t response);

//Specific functions

/**
 * @brief Get the Ad API version for an Bing Ad response.
 *
 * The @c response_get_ad_api_version() function allows developers to
 * get the API version of a Bing Ad response.
 *
 * @param response The Bing response to get the Ad API version from.
 * @param buffer The buffer to copy the Ad API version into.
 *
 * @return The size of the Bing Ad response API version in bytes, or
 * 	-1 if an error occurred or if the response is not an Ad type.
 */
int response_get_ad_api_version(bing_response_t response, char* buffer);

/**
 * @brief Get the Ad page number for an Bing Ad response.
 *
 * The @c response_get_ad_page_number() function allows developers to
 * get the Ad page number of a Bing Ad response.
 *
 * @param response The Bing response to get the Ad page number from.
 *
 * @return The Bing Ad page number, or -1 if an error occurred or
 * 	if the response is not an Ad type.
 */
long long response_get_ad_page_number(bing_response_t response);

/**
 * @brief Get the responses from a Bing Bundle response.
 *
 * The @c response_get_bundle_responses() function allows developers to
 * get the bundled Bing responses.
 *
 * @param response The Bing response to get the reponses of.
 * @param responses The array of responses to copy into.
 *
 * @return The Bing response "responses" count, or -1 if an error
 * 	occurred or if the response is not a Bundle type.
 */
int response_get_bundle_responses(bing_response_t response, bing_response_t* responses);

/**
 * @brief Get the total for an Bing MobileWeb or Spell response.
 *
 * The @c response_get_mobile_web_spell_total() function allows developers
 * to get the total of a Bing MobileWeb or Spell response.
 *
 * @param response The Bing response to get the total from.
 *
 * @return The total, or -1 if an error occurred or if the response is not
 * 	a MobileWeb or Spell type.
 */
long long response_get_mobile_web_spell_total(bing_response_t response);

/**
 * @brief Get the offset for an Bing MobileWeb response.
 *
 * The @c response_get_mobile_web_offset() function allows developers
 * to get the offset of a Bing MobileWeb response.
 *
 * @param response The Bing response to get the offset from.
 *
 * @return The total, or -1 if an error occurred or if the response is not
 * 	an MobileWeb type.
 */
long long response_get_mobile_web_offset(bing_response_t response);

/**
 * @brief Get the title for an Bing Phonebook response.
 *
 * The @c response_get_phonebook_title() function allows developers to
 * get the title of a Bing Phonebook response.
 *
 * @param response The Bing response to get the title from.
 * @param buffer The buffer to copy the title into.
 *
 * @return The size of the Bing Phonebook title in bytes, or -1 if an error
 * 	occurred or if the response is not an Phonebook type.
 */
int response_get_phonebook_title(bing_response_t response, char* buffer);

/**
 * @brief Get the URL to the local search repository for an Bing Phonebook response.
 *
 * The @c response_get_phonebook_local_serp_url() function allows developers to
 * get the URL to the local search repository of a Bing Phonebook response.
 *
 * @param response The Bing response to get the URL from.
 * @param buffer The buffer to copy the URL into.
 *
 * @return The size of the Bing Phonebook URL in bytes, or -1 if an error
 * 	occurred or if the response is not an Phonebook type.
 */
int response_get_phonebook_local_serp_url(bing_response_t response, char* buffer);

/**
 * @brief Get the related searches for an Bing News response.
 *
 * The @c response_get_news_related_searches() function allows developers to
 * get the related searches to an Bing News response.
 *
 * @param response The Bing response to get the related search from.
 * @param searches The array of related searches to copy into.
 *
 * @return The Bing response "related searches" count, or -1 if an error
 * 	occurred or if the response is not a News type.
 */
int response_get_news_related_searches(bing_response_t response, bing_related_search_t searches);

//Custom functions

/**
 * @brief Get a custom value from a Bing response.
 *
 * The @c response_custom_get_*() functions allows developers to retrieve
 * values from a Bing response. All values are self contained and will be
 * copied to the value parameter. These functions work on all response
 * types but allow for retrieval of custom result values.
 *
 * In the case of string and array, the return type is the amount of data,
 * in bytes. If value is NULL then nothing is copied.
 *
 * For array types, the actual data is copied, not pointers to the data.
 * Also note that if array elements contain arrays, buffers, or strings
 * themselves that these will be allocated internally and will be freed
 * when the result is freed (which happens when the response is freed).
 *
 * @param response The Bing response to retrieve data from.
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

int response_custom_get_64bit_int(bing_response_t response, const char* field, long long* value);
int response_custom_get_string(bing_response_t response, const char* field, char* value);
int response_custom_get_double(bing_response_t response, const char* field, double* value);
int response_custom_get_boolean(bing_response_t response, const char* field, int* value);
int response_custom_get_array(bing_response_t response, const char* field, void* value);

/**
 * @brief Set a custom value for a Bing response.
 *
 * The @c response_custom_set_*() functions allows developers to set
 * values to a custom Bing response. If the result is not the custom type
 * then the function will fail. All values are self contained and will be
 * copied from the value parameter.
 *
 * In the case of string and array, the entire data amount is copied
 * using strlen for string or the size parameter for array.
 *
 * For array types, the actual data is copied, not pointers to the data.
 * Also note that if array elements contain arrays, buffers, or strings
 * themselves that these will be copied as well and the copies will be
 * freed when the response is freed.
 *
 * If the field does not exist then it will be created, if and only if
 * value is not NULL. If the value is NULL and the field exists, it will
 * be removed.
 *
 * @param response The Bing response to set data to.
 * @param field The field name to get the data of. If the field already
 * 	exists, the data will be replaced. If the field doesn't exist and
 * 	the value is not NULL, then the field will be created. If the field
 * 	exists and the value is NULL, the field is removed.
 * @param value The value to copy data from. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing response.
 * 	If this is NULL then no effect occurs unless the field exists, in which
 * 	case the field is removed.
 * @param size The size of the array data in bytes, so if an array of 3 int
 * 	are passed in, size would be (sizeof(int) * 3).
 *
 * @return A boolean value which is non-zero for a successful data set,
 * 	otherwise zero on error.
 */

int response_custom_set_64bit_int(bing_response_t response, const char* field, long long* value);
int response_custom_set_string(bing_response_t response, const char* field, const char* value);
int response_custom_set_double(bing_response_t response, const char* field, double* value);
int response_custom_set_boolean(bing_response_t response, const char* field, int* value);
int response_custom_set_array(bing_response_t response, const char* field, const void* value, size_t size);

/**
 * @brief Register a new response creator.
 *
 * The @c result_register_result_creator() function allows developers to
 * register a set of callbacks and a name for a, as of now, unsupported
 * Bing response.
 *
 * The creation function is provided the internally allocated response,
 * the name the response is associated with, and a dictionary with all
 * the attributes that were passed with result.
 *
 * Some responses can actually contain additional data. That's where
 * the additional data function comes in. When additional data
 * is loaded, this function gets called so the data can be handled in
 * whatever manner is deemed appropriate.
 *
 * @param name The name associated with the response. Only unsupported
 * 	names can be registered. For example, the name "web:Web"
 * 	is for a web response type. If this was passed in, it would fail.
 * 	Names are the XML names that returned by the Bing service.
 * @param creation_func The function that handles any data passed in to
 * 	a response. This function is required.
 * @param additional_func The function that handles any additional
 * 	data that are passed in to another response. This is optional.
 * 	If this is NULL and additional data is found, it is ignored.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.
 */
int response_register_response_creator(const char* name, response_creation_func creation_func, response_additional_data_func additional_func);

/**
 * @brief Unregister a response creator.
 *
 * The @c response_unregister_response_creator() function allows developers to
 * unregister a set of response creator callbacks.
 *
 * @param name The name associated with the response. This is the same
 * 	as the name passed into response_register_response_creator.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.
 */
int response_unregister_response_creator(const char* name);

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
	RESULT_FIELD_ADLINK_URL,
	//64bit integer
	RESULT_FIELD_CODE,
	RESULT_FIELD_MESSAGE,
	RESULT_FIELD_HELP_URL,
	RESULT_FIELD_PARAMETER,
	RESULT_FIELD_SOURCE_TYPE,
	//64bit integer
	RESULT_FIELD_SOURCE_TYPE_ERROR_CORE,
	RESULT_FIELD_VALUE,
	//64bit integer
	RESULT_FIELD_HEIGHT,
	//64bit integer
	RESULT_FIELD_WIDTH,
	//64bit integer
	RESULT_FIELD_FILE_SIZE,
	RESULT_FIELD_MEDIA_URL,
	RESULT_FIELD_URL,
	RESULT_FIELD_CONTENT_TYPE,
	//bing_thumbnail_t
	RESULT_FIELD_THUMBNAIL,
	RESULT_FIELD_ATTRIBUTION,
	RESULT_FIELD_INSTANT_ANWSER_SPECIFIC_DATA,
	RESULT_FIELD_DATE_TIME,
	//boolean
	RESULT_FIELD_BREAKING_NEWS,
	RESULT_FIELD_DATE,
	RESULT_FIELD_SNIPPET,
	RESULT_FIELD_SOURCE,
	//bing_news_collection_s
	RESULT_FIELD_NEWSCOLLECTION,
	//double
	RESULT_FIELD_LATITUDE,
	//double
	RESULT_FIELD_LONGITUDE,
	//double
	RESULT_FIELD_USER_RATING,
	//64bit integer
	RESULT_FIELD_REVIEW_COUNT,
	RESULT_FIELD_BUSINESS_URL,
	RESULT_FIELD_CITY,
	RESULT_FIELD_COUNTRY_OR_REGION,
	RESULT_FIELD_PHONE_NUMBER,
	RESULT_FIELD_POSTAL_CODE,
	RESULT_FIELD_STATE_OR_PROVINCE,
	RESULT_FIELD_UNIQUE_ID,
	RESULT_FIELD_BUSINESS,
	RESULT_FIELD_ADDRESS,
	RESULT_FIELD_TRANSLATED_TERM,
	RESULT_FIELD_SOURCE_TITLE,
	RESULT_FIELD_RUN_TIME,
	RESULT_FIELD_PLAY_URL,
	RESULT_FIELD_CLICK_THROUGH_PAGE_URL,
	//bing_thumbnail_t
	RESULT_FIELD_STATIC_THUMBNAIL,
	RESULT_FIELD_CACHE_URL,
	//bing_deep_link_s
	RESULT_FIELD_DEEP_LINKS,
	//bing_search_tag_s
	RESULT_FIELD_SEARCH_TAGS
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
 * Also note that if array elements contain arrays, buffers, or strings
 * themselves that these will be allocated internally and will be freed
 * when the result is freed (which happens when the response is freed).
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
int result_get_array(bing_result_t result, RESULT_FIELD field, void* value);

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
 * Also note that if array elements contain arrays, buffers, or strings
 * themselves that these will be allocated internally and will be freed
 * when the result is freed (which happens when the response is freed).
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
int result_custom_get_array(bing_result_t result, const char* field, void* value);

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
 * Also note that if array elements contain arrays, buffers, or strings
 * themselves that these will be copied as well and the copies will be
 * freed when the result is freed (which happens when the response is freed).
 *
 * If the field does not exist then it will be created, if and only if
 * value is not NULL. If the value is NULL and the field exists, it will
 * be removed.
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
 * @brief Register a new result creator.
 *
 * The @c result_register_result_creator() function allows developers to
 * register a set of callbacks and a name for a, as of now, unsupported
 * Bing result.
 *
 * The creation function is provided the internally allocated result,
 * the name the result is associated with, and a dictionary with all
 * the attributes that were passed with result.
 *
 * Some results can actually contain additional results. That's where
 * the additional result function comes in. When an additional result
 * is loaded, this function gets called so the result can be handled in
 * whatever manner is deemed appropriate. The name passed in is the
 * name of the additional result. Results registered with this function
 * can be additional results.
 *
 * @param name The name associated with the result. Only unsupported
 * 	names can be registered. For example, the name "web:WebResult"
 * 	is for a web result type. If this was passed in, it would fail.
 * 	Names are the XML names that returned by the Bing service.
 * @param creation_func The function that handles any data passed in to
 * 	a result. This function is required.
 * @param additional_func The function that handles any additional
 * 	results that are passed in to another result. This is optional.
 * 	If this is NULL and an additional result is found, it is ignored.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.
 */
int result_register_result_creator(const char* name, result_creation_func creation_func, result_additional_result_func additional_func);

/**
 * @brief Unregister a result creator.
 *
 * The @c result_unregister_result_creator() function allows developers to
 * unregister a set of result creator callbacks.
 *
 * @param name The name associated with the result. This is the same
 * 	as the name passed into result_register_result_creator.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.S
 */
int result_unregister_result_creator(const char* name);

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
