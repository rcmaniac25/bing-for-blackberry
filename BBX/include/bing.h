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

#include <sys/platform.h>
#include <bps/bps.h>

#include <stdlib.h>

__BEGIN_DECLS

/*
 * The full Bing library.
 *
 * Notes:
 * Text-All text is in UTF8 format.
 *
 * Arrays-The data itself will be copied, not just the pointer to the element.
 * But, if the element contains pointers, the pointers will be references. Internal
 * pointers are Response-dependent and will only effect usage of the response.
 */

#define BING_VERSION 2.2 //XXX This no longer is needed. As "Bing 2.0" has been replaced by the Azure version
#define BBING_VERSION 2.0

/*
 * Structures
 */

typedef void* bing_result_t;
typedef void* bing_response_t;
typedef void* bing_request_t;

typedef void* data_dictionary_t;

typedef struct _bing_news_collection //XXX Not Used
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

typedef struct _bing_title_url //XXX Not Used
{
	const char* title;
	const char* url;
} bing_deep_link_s, bing_related_search_s, *bing_deep_link_t, *bing_related_search_t;

typedef struct _bing_search_tag //XXX Not Used
{
	const char* name;
	const char* value;
} bing_search_tag_s, *bing_search_tag_t;

enum BING_SOURCE_TYPE
{
	BING_SOURCETYPE_UNKNOWN,

	//A custom source type
	BING_SOURCETYPE_CUSTOM,

	//A bundled source type (not used for result)
	BING_SOURCETYPE_BUNDLE,

	//A error result (only used for result)
	BING_RESULT_ERROR,

	//Standard source types
	BING_SOURCETYPE_AD,				//XXX Remove
	BING_SOURCETYPE_IMAGE,
	BING_SOURCETYPE_INSTANT_ANWSER,	//XXX Remove
	BING_SOURCETYPE_MOBILE_WEB,		//XXX Remove
	BING_SOURCETYPE_NEWS,
	BING_SOURCETYPE_RELATED_SEARCH,
	BING_SOURCETYPE_SPELL,
	BING_SOURCETYPE_TRANSLATION,
	BING_SOURCETYPE_VIDEO,
	BING_SOURCETYPE_WEB,

	//Common result types
	BING_RESULT_COMMON,

	BING_SOURCETYPE_COUNT
};

#define BING_RESULT_COMMON_TYPE "bb_result-common_type"

/*
 * Function delegates
 */

typedef void (*receive_bing_response_func) (bing_response_t response, const void* user_data);
typedef const char* (*request_get_options_func)(bing_request_t request);
typedef void (*request_finish_get_options_func)(bing_request_t request, const char* options);
typedef int (*response_creation_func)(const char* name, bing_response_t response, data_dictionary_t dictionary);
typedef void (*response_additional_data_func)(bing_response_t response, data_dictionary_t dictionary);
typedef int (*result_creation_func)(const char* name, bing_result_t result, data_dictionary_t dictionary);
typedef void (*result_additional_result_func)(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult);

/*
 * Dictionary functions
 */

/**
 * @brief Get data from a dictionary.
 *
 * The @c bing_dictionary_get_data() function allows developers to retrieve data from a
 * dictionary.
 *
 * @param dict The dictionary to retrieve data from.
 * @param name The name of the field to retrieve from the dictionary.
 * @param data The buffer to copy the data into.
 *
 * @return The size of the data in bytes, or -1 if an error has occurred, dict is NULL,
 * 	or the name doesn't exist.
 */
int bing_dictionary_get_data(data_dictionary_t dict, const char* name, void* data);

/**
 * @brief Get all the names within a dictionary.
 *
 * The @c bing_dictionary_get_element_names() function allows developers to retrieve
 * an array of all names within a dictionary.
 *
 * @param dict The dictionary to retrieve the names from.
 * @param name The array to copy all the names into. The memory handler set for use
 * 	(default is malloc) will be used to allocate each individual string within the names
 * 	array. It's up to the developer to free each name string.
 *
 * @return The number of names within the dictionary, or -1 if an error occurred
 * or dict was NULL.
 */
int bing_dictionary_get_element_names(data_dictionary_t dict, char** names);

/*
 * Event handling functions
 */

/**
 * @brief Get the Bing service's unique domain ID for use in event processing.
 *
 * @return A domain ID for a Bing service. Returning -1 indicates an error.
 */
int bing_get_domain();

/**
 * @brief Get the Bing response located within an Event.
 *
 * The @c bing_event_get_response() function allows developers to retrieve
 * the Bing response from an event.
 *
 * The event does not free the Bing response unless it is never retrieved.
 * If it was retrieved, then it is up to the developer to free it.
 *
 * @param event The event to retrieve the response from.
 * @param response A pointer to a Bing response which will store the
 * 	actual response that can be used.
 *
 * @return Nothing is returned.
 */
void bing_event_get_response(bps_event_t* event, bing_response_t* response);

/*
 * Bing search system
 */

/**
 * @brief Initialize the Bing subsystem.
 *
 * This can be called multiple times because it will only run once. This will
 * be automatically called internally if not called already.
 *
 * @return Nothing is returned.
 */
void bing_initialize();

/**
 * @brief Shutdown the Bing subsystem.
 *
 * This will shutdown and clean up all Bing instances, results, and responses.
 * Requests will still need to be freed manually. Be careful when you call
 * this because it will clean up everything.
 *
 * @return A boolean integer indicating if the subsystem was shutdown or not.
 * 	Zero is false, non-zero is true. Possible reasons for not shutting down
 * 	include the subsystem was not initialized in the first place and the
 * 	that searches are still occurring when this is called.
 */
int bing_shutdown();

/**
 * @brief Create a new Bing service.
 *
 * The @c bing_create() function allows developers to allocate a Bing service object to
 * perform search operations using Microsoft's Bing services.
 *
 * @param application_ID The application ID which allows a developer to access
 * 	Microsoft Bing services. If this string is not NULL, it is copied for use by
 * 	the service. So the developer can free the memory when he is done.
 *
 * @return A unique Bing service ID that is used to perform searches. If this value is
 * 	zero then an error has occurred and no service has been allocated.
 */
unsigned int bing_create(const char* application_ID);

/**
 * @brief Free a Bing service.
 *
 * The @c bing_free() function allows developers to free allocated memory used
 * 	for a Bing service.
 *
 * @param bing The unique Bing ID for each application ID.
 *
 * @return Nothing is returned.
 */
void bing_free(unsigned int bing);

#if defined(BING_DEBUG)

/**
 * @brief Set if the internal parser should return search errors.
 *
 * The @c bing_set_error_return() function allows a developer to explicitly handle errors
 * that might show up from the service. Examples of such are bad Application IDs,
 * unsupported search requests, etc.
 *
 * If this is set to a non-zero value (true), then it will return a special response
 * that specifies error information. If it is a zero value (false) and an error
 * occurs, it will simply stop execution and clean up the IO connections.
 *
 * In addition, if the library was compiled in debug mode and this is set, it
 * acts like a verbose option and returns where it failed.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param error A boolean integer indicating if error cases should be returned.
 * 	Zero is false, non-zero is true.
 *
 * @return A boolean integer indicating if the value was set or not. Zero is false,
 * 	non-zero is true.
 */
int bing_set_error_return(unsigned int bing, int error); //XXX Reword if no error type is returned

/**
 * @brief Get if the internal parser should return search errors.
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
int bing_get_error_return(unsigned int bing); //XXX Reword if no error type is returned

/**
 * @brief A non-threadsafe way to find the last error that occurred, if one happened at all.
 *
 * Possible error values
 * 00. No error
 * 01. Default error callback (LibXML2)
 * 02. Default structured error callback (LibXML2)
 * 03. Name duplication failed (internal structure for parsing response requires duplicating the Bing result's name)
 * 04. Stack allocation failed (internal structure for parsing response requires creating a stack. This failed for the Bing result name)
 * 05. Stack allocation failed (internal structure for parsing response requires creating a stack. This failed for the Bing result itself)
 * 06. Could not create the internal table to store query information (start of element)
 * 07. Could not create a response for a bundle (start of element)
 * 08. Could not run creation callback for a newly created response (start of element)
 * 09. Could not create a response (start of element)
 * 10. Could not create a qualified name (start of element)
 * 11. Could not create a qualified name (end of element)
 * 12. Could not create a LibXML2 context (read network data)
 *
 * @return A integer defining the last error code to have occurred after a search.
 */
int bing_get_last_error_code(); //XXX Will need to be updated once search is finished

#endif

/**
 * @brief Get a Bing service's application ID.
 *
 * The @c bing_get_app_ID() function allows developers to get the service's current
 * application ID.
 *
 * @param bing The unique Bing ID for each application ID.
 * @param buffer The buffer to copy the application ID to. If this is NULL, then
 * 	the application ID length (plus NULL char) is returned.
 *
 * @return The length of the application ID, or -1 if an error occurred.
 */
int bing_get_app_ID(unsigned int bing, char* buffer); //XXX Rename when done (account key)

/**
 * @brief Set a Bing service's application ID.
 *
 * The @c bing_set_app_ID() function allows developers to set the service's
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
int bing_set_app_ID(unsigned int bing, const char* appId); //XXX Rename when done (account key)

/**
 * @brief Perform a synchronous search.
 *
 * The @c bing_search_sync() function allows developers to perform a blocking
 * search operation that will return when the search is complete.
 *
 * @param bing The unique Bing ID to perform a search with.
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
bing_response_t bing_search_sync(unsigned int bing, const char* query, const bing_request_t request);

/**
 * @brief Perform a synchronous search.
 *
 * The @c bing_search_next_sync() function allows developers to perform a blocking
 * search operation that will return when the search is complete. This is the same
 * as bing_search_sync but it takes the previous response so it can search for the
 * next set of search results.
 *
 * @param pre_response The previous search response.
 *
 * @return A response to the next set of results for the original search query.
 * 	This object, when not NULL for errors or for lack of a next page, is allocated
 * 	and should be freed using the free_response function to prevent memory leaks.
 */
bing_response_t bing_search_next_sync(const bing_response_t pre_response); //TODO

/**
 * @brief Perform a asynchronous search.
 *
 * The @c bing_search_async() function allows developers to perform a non-blocking
 * search operation that will return immediately and call the specified callback
 * function with the response. Remember, the callback will be called by a
 * different thread other than the one calling this function. So plan synchronization
 * out properly with your callback function.
 *
 * @param bing The unique Bing ID to perform a search with.
 * @param query The search query to perform. If this is NULL, then the function
 * 	returns a zero (false) value.
 * @param request The type of search to perform. This determines the response
 * 	that will be returned. If this is NULL, then the function function returns
 * 	a zero (false) value.
 * @param user_data Any user data that will be passed to the response function.
 * @param response_func The function that will be called with a response from
 * 	the search. If this is NULL, then the function returns a zero (false) value.
 *
 * @return A boolean result which is non-zero for a successful query, otherwise
 * 	zero on error or bad query.
 */
int bing_search_async(unsigned int bing, const char* query, const bing_request_t request, const void* user_data, receive_bing_response_func response_func);

/**
 * @brief Perform a asynchronous search.
 *
 * The @c bing_search_next_async() function allows developers to perform a non-blocking
 * search operation that will return immediately and call the specified callback
 * function with the response. This is the same as bing_search_async but it takes the
 * previous response so it can search for the next set of search results. Remember,
 * the callback will be called by a different thread other than the one calling this
 * function. So plan synchronization out properly with your callback function.
 *
 * @param pre_response The previous search response.
 * @param user_data Any user data that will be passed to the response function.
 * @param response_func The function that will be called with a response from
 * 	the search. If this is NULL, then the function returns a zero (false) value.
 *
 * @return A boolean result which is non-zero for a successful query, otherwise
 * 	zero on error, bad query, or lack of next set of results.
 */
int bing_search_next_async(const bing_response_t pre_response, const void* user_data, receive_bing_response_func response_func); //TODO

/**
 * @brief Perform a asynchronous search but returns with an event.
 *
 * The @c bing_search_event_async() function allows developers to perform a non-blocking
 * search operation that will return immediately and delegate an event with
 * the response. The response should NOT be freed by any receiving functions.
 *
 * To determine if the event is a Bing response, use the bing_get_domain() function.
 *
 * @param bing The unique Bing ID to perform a search with.
 * @param query The search query to perform. If this is NULL, then the function
 * 	returns a zero (false) value.
 * @param request The type of search to perform. This determines the response
 * 	that will be returned. If this is NULL, then the function function returns
 * 	a zero (false) value.
 *
 * @return A boolean result which is non-zero for a successful query, otherwise
 * 	zero on error or bad query.
 */
int bing_search_event_async(unsigned int bing, const char* query, const bing_request_t request);

/**
 * @brief Perform a asynchronous search but returns with an event.
 *
 * The @c bing_search_event_next_async() function allows developers to perform a
 * non-blocking search operation that will return immediately and delegate an
 * event with the response. This is the same as bing_search_event_async but it takes
 * the previous response so it can search for the next set of search results.
 * The response should NOT be freed by any receiving functions.
 *
 * @param pre_response The previous search response.
 *
 * @return A boolean result which is non-zero for a successful query, otherwise
 * 	zero on error, bad query, or lack of next set of results.
 */
int bing_search_event_next_async(const bing_response_t pre_response); //TODO

/**
 * @brief Get a URL that can invoke a Bing search request.
 *
 * The @c bing_request_url() function allows developers to create a URL for performing
 * a custom search query. This is not the same as a custom request, but can allow
 * a developer to custom tailor the search URL and handle the result in whatever
 * manner they deem necessary.
 *
 * @param bing The unique Bing ID to generate a URL for.
 * @param query The search query to perform. If this is NULL, then the function
 * 	returns a URL with an empty query.
 * @param request The type of search to perform. This determines the response
 * 	that will be returned. If this is NULL, then the function function returns
 * 	NULL.
 *
 * @return A URL string that can be used for searches, or NULL on error. This
 * 	function allocates memory with malloc and it is up to the developer to
 * 	free it to avoid memory leaks.
 */
const char* bing_request_url(unsigned int bing, const char* query, const bing_request_t request);

/**
 * @brief Set memory handlers to be used by Bing.
 *
 * The @c bing_set_memory_handlers function allows developers to set the memory
 * handler functions to use in place of normal malloc/free/etc. handlers. All
 * handlers must be set for them to be used.
 *
 * Be careful to only set this when you start using your program otherwise
 * there is a risk of memory issues of using two or more memory handlers
 * through out execution of the program.
 *
 * @param bing_malloc The "malloc"-like memory handler.
 * @param bing_calloc The "calloc"-like memory handler.
 * @param bing_realloc The "realloc"-like memory handler.
 * @param bing_free The "free"-like memory handler.
 * @param bing_strdup The "strdup"-like memory handler.
 *
 * @return A boolean result which is non-zero for a successful set, otherwise
 * 	zero if not every field is set.
 */
int bing_set_memory_handlers(void* (*bing_malloc)(size_t), void* (*bing_calloc)(size_t,size_t), void* (*bing_realloc)(void*,size_t), void (*bing_free)(void*), char* (*bing_strdup)(const char*));

/*
 * Request functions
 */

enum BING_REQUEST_FIELD
{
	//All values are strings unless otherwise noted.

	BING_REQUEST_FIELD_UNKNOWN,

	BING_REQUEST_FIELD_VERSION, //XXX Not used
	BING_REQUEST_FIELD_MARKET,
	BING_REQUEST_FIELD_ADULT,
	BING_REQUEST_FIELD_OPTIONS,
	//double
	BING_REQUEST_FIELD_LATITUDE,
	//double
	BING_REQUEST_FIELD_LONGITUDE,
	BING_REQUEST_FIELD_LANGUAGE, //XXX Not used
	//double
	BING_REQUEST_FIELD_RADIUS, //XXX Not used
	//64bit integer
	BING_REQUEST_FIELD_PAGE_NUMBER, //XXX Not used
	//64bit integer
	BING_REQUEST_FIELD_AD_UNIT_ID, //XXX Not used
	//64bit integer
	BING_REQUEST_FIELD_PROPERTY_ID, //XXX Not used
	//64bit integer
	BING_REQUEST_FIELD_CHANNEL_ID, //XXX Not used
	//64bit integer
	BING_REQUEST_FIELD_MAINLINE_AD_COUNT, //XXX Not used
	//64bit integer
	BING_REQUEST_FIELD_SIDEBAR_AD_COUNT, //XXX Not used
	//64bit integer
	BING_REQUEST_FIELD_COUNT,
	//64bit integer
	BING_REQUEST_FIELD_OFFSET,
	BING_REQUEST_FIELD_FILTERS,
	BING_REQUEST_FIELD_MOBILE_WEB_OPTIONS, //XXX Not used
	BING_REQUEST_FIELD_CATEGORY,
	BING_REQUEST_FIELD_LOCATION_OVERRIDE,
	BING_REQUEST_FIELD_SORT_BY,
	BING_REQUEST_FIELD_FILE_TYPE,
	BING_REQUEST_FIELD_LOC_ID, //XXX Not used
	BING_REQUEST_FIELD_SOURCE_LANGUAGE, //XXX Not used
	BING_REQUEST_FIELD_TARGET_LANGUAGE, //XXX Not used
	BING_REQUEST_FIELD_WEB_OPTIONS, //XXX Not used
	BING_REQUEST_FIELD_TO,
	BING_REQUEST_FILED_FROM
};

//Helper functions to make the end result better, from http://stackoverflow.com/questions/195975/how-to-make-a-char-string-from-a-c-macros-value
#if defined(__STRING)
#define VALUE_NAME(name) __STRING(name)
#else
#define STR_VALUE(arg) #arg
#define VALUE_NAME(name) STR_VALUE(name)
#endif

#define BING_DEFAULT_SEARCH_MARKET "en-US"
#define BING_DEFAULT_API_VERSION VALUE_NAME(BING_VERSION) //XXX This no longer is needed

#define BING_OPTION_SEPERATOR "+"

//Used for the BING_REQUEST_FIELD_ADULT field
#define BING_ADULT_OPTIONS_OFF "Off"
#define BING_ADULT_OPTIONS_MODERATE "Moderate"
#define BING_ADULT_OPTIONS_STRICT "Strict"

//Used for the BING_REQUEST_FIELD_OPTIONS field (separated with BING_SEARCH_OPTIONS_SEPERATOR)
#define BING_SEARCH_OPTIONS_SEPERATOR OPTION_SEPERATOR
#define BING_SEARCH_OPTIONS_DISABLE_LOCATION_DETECTION "DisableLocationDetection"
#define BING_SEARCH_OPTIONS_ENABLE_HIGHLIGHTING "EnableHighlighting"

//Used for the BING_REQUEST_FIELD_FILTERS field (separated with BING_IMAGE_FILTERS_SEPERATOR). You cannot include more than one value for duration in the same request.
#define BING_IMAGE_FILTERS_SEPERATOR OPTION_SEPERATOR
#define BING_IMAGE_FILTERS_SIZE_SMALL "Size:Small"
#define BING_IMAGE_FILTERS_SIZE_MEDIUM "Size:Medium"
#define BING_IMAGE_FILTERS_SIZE_LARGE "Size:Large"
//Append on to this the desired height of the image as an unsigned integer
#define BING_IMAGE_FILTERS_SIZE_HEIGHT "Size:Height:"
//Append on to this the desired width of the image as an unsigned integer
#define BING_IMAGE_FILTERS_SIZE_WIDTH "Size:Width:"
#define BING_IMAGE_FILTERS_ASPECT_SQUARE "Aspect:Square"
#define BING_IMAGE_FILTERS_ASPECT_WIDE "Aspect:Wide"
#define BING_IMAGE_FILTERS_ASPECT_TALL "Aspect:Tall"
#define BING_IMAGE_FILTERS_COLOR_COLOR "Color:Color"
#define BING_IMAGE_FILTERS_COLOR_MONOCHROME "Color:Monochrome"
#define BING_IMAGE_FILTERS_STYLE_PHOTO "Style:Photo"
#define BING_IMAGE_FILTERS_STYLE_GRAPHICS "Style:Graphics"
#define BING_IMAGE_FILTERS_FACE_FACE "Face:Face"
#define BING_IMAGE_FILTERS_FACE_PORTRAIT "Face:Portrait"
#define BING_IMAGE_FILTERS_FACE_OTHER "Face:Other"

//Used for the BING_REQUEST_FIELD_CATEGORY field when used on a news source type
#define BING_NEWS_CATEGORY_BUSINESS "rt_Business"
#define BING_NEWS_CATEGORY_ENTERTAINMENT "rt_Entertainment"
#define BING_NEWS_CATEGORY_HEALTH "rt_Health"
#define BING_NEWS_CATEGORY_POLITICS "rt_Politics"
#define BING_NEWS_CATEGORY_SPORTS "rt_Sports"
#define BING_NEWS_CATEGORY_US "rt_US"
#define BING_NEWS_CATEGORY_WORLD "rt_World"
#define BING_NEWS_CATEGORY_SCITECH "rt_ScienceAndTechnology"

//Used for the BING_REQUEST_FIELD_SORT_BY field when used on a news source type
#define BING_NEWS_SORT_OPTIONS_DATE "Date"
#define BING_NEWS_SORT_OPTIONS_RELEVANCE "Relevance"

//Used for the BING_REQUEST_FIELD_FILTERS field (separated with BING_VIDEO_FILTERS_SEPERATOR). You cannot include more than one value for duration in the same request.
#define BING_VIDEO_FILTERS_SEPERATOR OPTION_SEPERATOR
#define BING_VIDEO_FILTERS_DURATION_SHORT "Duration:Short"
#define BING_VIDEO_FILTERS_DURATION_MEDIUM "Duration:Medium"
#define BING_VIDEO_FILTERS_DURATION_LONG "Duration:Long"
#define BING_VIDEO_FILTERS_ASPECT_STANDARD "Aspect:Standard"
#define BING_VIDEO_FILTERS_ASPECT_WIDESCREEN "Aspect:Widescreen"
#define BING_VIDEO_FILTERS_RESOLUTION_LOW "Resolution:Low"
#define BING_VIDEO_FILTERS_RESOLUTION_MEDIUM "Resolution:Medium"
#define BING_VIDEO_FILTERS_RESOLUTION_HIGH "Resolution:High"

//Used for the BING_REQUEST_FIELD_SORT_BY field when used on a video source type
#define BING_VIDEO_SORT_OPTION_DATE "Date"
#define BING_VIDEO_SORT_OPTION_RELEVANCE "Relevance"

//Used for the BING_REQUEST_FIELD_FILE_TYPE field when used on a web source type
#define BING_WEB_FILE_TYPE_DOC "DOC"
#define BING_WEB_FILE_TYPE_DWF "DWF"
#define BING_WEB_FILE_TYPE_RSS "FEED"
#define BING_WEB_FILE_TYPE_HTM "HTM"
#define BING_WEB_FILE_TYPE_HTML "HTML"
#define BING_WEB_FILE_TYPE_PDF "PDF"
#define BING_WEB_FILE_TYPE_PPT "PPT"
#define BING_WEB_FILE_TYPE_RTF "RTF"
#define BING_WEB_FILE_TYPE_TEXT "TEXT"
#define BING_WEB_FILE_TYPE_TXT "TXT"
#define BING_WEB_FILE_TYPE_XLS "XLS"

//Standard functions

/**
 * @brief Get the Bing request source type.
 *
 * The @c bing_request_get_source_type() functions allows developers to retrieve the
 * source type of Bing request that is passed in.
 *
 * @param request The Bing request to get the source type of.
 *
 * @return The Bing request source type, or BING_SOURCETYPE_UNKNOWN if NULL is passed in.
 */
enum BING_SOURCE_TYPE bing_request_get_source_type(bing_request_t request);

/**
 * @brief Create a standard Bing request.
 *
 * The @c bing_request_create() functions allows developers to create a new
 * Bing request that can be used to search. It is up to the dev to free this
 * request using free_request() to prevent a memory leak.
 *
 * @param source_type The sourcetype that the new Bing request should be. The
 * 	BING_RESULT_ERROR type and BING_SOURCETYPE_UNKNOWN type are not valid and
 * 	will cause the function to fail.
 * @param request A pointer to the newly created Bing request.
 *
 * @return A boolean value which is non-zero for a successful creation,
 * 	otherwise zero on error, invalid source types, or NULL request pointer.
 */
int bing_request_create(enum BING_SOURCE_TYPE source_type, bing_request_t* request);

/**
 * @brief Check if the Bing request type is supported.
 *
 * The @c bing_request_is_field_supported() functions allows developers to determine
 * if a field is supported.
 *
 * @param request The Bing request to check for a field.
 * @param field The field to check for.
 *
 * @return A boolean value which is non-zero if the field is supported
 * 	within the specified Bing request, otherwise zero on error or NULL request.
 */
int bing_request_is_field_supported(bing_request_t request, enum BING_REQUEST_FIELD field);

/**
 * @brief Get a value from a Bing request.
 *
 * The @c bing_request_get_*() functions allows developers to retrieve values from
 * a Bing request. All values are self contained and will be copied to
 * the value parameter.
 *
 * In the case of string, the return type is the amount of data, in bytes. If
 * value is NULL then nothing is copied.
 *
 * @param request The Bing request to retrieve data from.
 * @param field The field to get the data of. If the field doesn't support
 * 	the data type that the function specifies or the field isn't
 * 	supported, then the function fails.
 * @param value The value to copy data into. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing request.
 *
 * @return A boolean value which is non-zero for a successful data retrieval,
 * 	otherwise zero on error or invalid field. Note that for string types, the
 * 	length of the data in bytes is returned. If an error occurs on a string
 * 	type then the result is -1.
 */

int bing_request_get_64bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, long long* value);
int bing_request_get_string(bing_request_t request, enum BING_REQUEST_FIELD field, char* value);
int bing_request_get_double(bing_request_t request, enum BING_REQUEST_FIELD field, double* value);

/**
 * @brief Set a value for a Bing request.
 *
 * The @c bing_request_set_*() functions allows developers to set values to a
 * Bing request. All values are self contained and will be copied to the value
 * parameter. All values are self contained and will be copied from the value
 * parameter.
 *
 * In the case of string, the entire data amount is copied using strlen for
 * string.
 *
 * If the field does not exist then it will be created, if and only if
 * value is not NULL. If the value is NULL and the field exists, it will
 * be removed.
 *
 * @param request The Bing request to set data to.
 * @param field The field to set the data to. If the field already
 * 	exists, the data will be replaced. If the field doesn't exist and
 * 	the value is not NULL, then the field will be created. If the field
 * 	exists and the value is NULL, the field is removed.
 * @param value The value to copy data from. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing request.
 * 	If this is NULL then no effect occurs unless the field exists, in which
 * 	case the field is removed.
 *
 * @return A boolean value which is non-zero for a successful data set,
 * 	otherwise zero on error.
 */

int bing_request_set_64bit_int(bing_request_t request, enum BING_REQUEST_FIELD field, const long long* value);
int bing_request_set_string(bing_request_t request, enum BING_REQUEST_FIELD field, const char* value);
int bing_request_set_double(bing_request_t request, enum BING_REQUEST_FIELD field, const double* value);

/**
 * @brief Add a request to a bundle request.
 *
 * The @c bing_request_bundle_add_request() functions allows developers to add a different
 * Bing request to add to a bundle request.
 *
 * Added requests don't have to be freed as they will be freed when the parent request
 * is freed.
 *
 * @param request The Bing request to add a request to.
 * @param request_to_add The Bing request to add to the request.
 *
 * @return A boolean value which is non-zero if the the request has been added
 * 	successfully, otherwise zero on error, NULL request, NULL additional
 * 	request, or if the request and request_to_add is the same.
 */
int bing_request_bundle_add_request(bing_request_t request, bing_request_t request_to_add);

/**
 * @brief Free a Bing request from memory.
 *
 * The @c bing_request_free() function allows developers to free
 * entire Bing request.
 *
 * @param request The Bing request to free.
 *
 * @return Nothing is returned.
 */
void bing_request_free(bing_request_t request);

//Custom functions

/**
 * @brief Check if the Bing request type is supported.
 *
 * The @c bing_request_is_field_supported() functions allows developers to determine
 * if a field is supported.
 *
 * @param request The Bing request to check for a field.
 * @param field The field string to check for.
 *
 * @return A boolean value which is non-zero if the field is supported
 * 	within the specified Bing request, otherwise zero on error, NULL request,
 * 	or NULL field string.
 */
int bing_request_custom_is_field_supported(bing_request_t request, const char* field);

/**
 * @brief Get a custom value from a Bing request.
 *
 * The @c bing_request_custom_get_*() functions allows developers to retrieve
 * values from a Bing request. All values are self contained and will be
 * copied to the value parameter. These are the same functions as
 * request_get_* but with the actual field name passed. These functions
 * work on all result types but allow for retrieval of custom result
 * values.
 *
 * In the case of string, the return type is the amount of data, in bytes.
 * If value is NULL then nothing is copied.
 *
 * @param request The Bing request to retrieve data from.
 * @param field The field name to get the data of. If the field doesn't
 * 	support the data type that the function specifies or the field isn't
 * 	supported, then the function fails.
 * @param value The value to copy data into. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing request.
 *
 * @return A boolean value which is non-zero for a successful data retrieval,
 * 	otherwise zero on error or invalid field. Note that for string types, the
 * 	length of the data in bytes is returned. If an error occurs on a string
 * 	type then the result is -1.
 */

int bing_request_custom_get_64bit_int(bing_request_t request, const char* field, long long* value);
int bing_request_custom_get_string(bing_request_t request, const char* field, char* value);
int bing_request_custom_get_double(bing_request_t request, const char* field, double* value);

/**
 * @brief Set a custom value for a Bing request.
 *
 * The @c bing_request_custom_set_*() functions allows developers to set
 * values to a custom Bing request. If the result is not the custom type
 * then the function will fail. All values are self contained and will be
 * copied from the value parameter.
 *
 * In the case of string, the entire data amount is copied using strlen for
 * string or the size parameter for array.
 *
 * If the field does not exist then it will be created, if and only if
 * value is not NULL. If the value is NULL and the field exists, it will
 * be removed.
 *
 * @param request The Bing request to set data to.
 * @param field The field name to set the data to. If the field already
 * 	exists, the data will be replaced. If the field doesn't exist and
 * 	the value is not NULL, then the field will be created. If the field
 * 	exists and the value is NULL, the field is removed.
 * @param value The value to copy data from. Note that no data is passed,
 * 	all is copied. So changing any values will not effect the Bing request.
 * 	If this is NULL then no effect occurs unless the field exists, in which
 * 	case the field is removed.
 *
 * @return A boolean value which is non-zero for a successful data set,
 * 	otherwise zero on error.
 */

int bing_request_custom_set_64bit_int(bing_request_t request, const char* field, const long long* value);
int bing_request_custom_set_string(bing_request_t request, const char* field, const char* value);
int bing_request_custom_set_double(bing_request_t request, const char* field, const double* value);

/**
 * @brief Create a custom request.
 *
 * The @c bing_request_create_custom_request() function allows developers to
 * create a custom Bing request. It is up to the dev to free this request
 * using free_request() to prevent a memory leak.
 *
 * Requests contain a callback function to get the options to be written
 * to the Bing service, and a callback function to free the options string.
 *
 * @param source_type The source type of the custom request. Only
 * 	non-standard request source types can be created. For example the
 * 	source type "web" would cause the function to fail as it is the
 * 	source type for web requests.
 * @param request A pointer to a location where the source request will
 * 	be created.
 * @param get_options_func An optional function pointer that will get
 * 	any options that could be desired to be sent along with the general
 * 	search query. Default options are always passed out so if this is
 * 	NULL then only the default options are passed to the Bing service,
 * 	otherwise the returned option string will be appended to the
 * 	default options. If this is not NULL but a NULL string is returned,
 * 	the results are undefined.
 * @param get_options_done_func An optional function pointer that, if the
 * 	result from get_options_func is not NULL, will be passed the options
 * 	string so it can be freed. Only the string returned by
 * 	get_options_func will be passed in to this function. If this is NULL
 * 	and get_options_func is not NULL, then a memory leak could occur.
 *
 * @return A boolean value which is non-zero for a successful creation,
 * 	otherwise zero on error.
 */
int bing_request_create_custom_request(const char* source_type, bing_request_t* request, request_get_options_func get_options_func, request_finish_get_options_func get_options_done_func); //XXX Modify based on how "source_type" is determined/handled

/*
 * Response functions
 */

//Standard operations

/**
 * @brief Determine if the specified Bing response has a "next page" of results.
 *
 * The @c bing_response_has_next_results() function allows developers to find out
 * if there is a next set of results that can be queried on.
 *
 * @param response The Bing response to check for a next set of results.
 *
 * @return A boolean value which is non-zero if more results exist,
 * 	otherwise zero when no more results exist.
 */
int bing_response_has_next_results(bing_response_t response); //TODO

/**
 * @brief Get the Bing response source type.
 *
 * The @c bing_response_get_source_type() function allows developers to retrieve the
 * source type of Bing response that is passed in.
 *
 * @param response The Bing response to get the source type of.
 *
 * @return The Bing response source type, or BING_SOURCETYPE_UNKNOWN if NULL is passed in.
 */
enum BING_SOURCE_TYPE bing_response_get_source_type(bing_response_t response);

/**
 * @brief Get the estimated total number of results for a Bing response.
 *
 * The @c bing_response_get_total() function allows developers to retrieve the
 * estimated total number of results that response can get.
 *
 * This is not how any results that a response will have. This is the
 * "5 million hits" total.
 *
 * @param response The Bing response to get total size of.
 *
 * @return The estimated Bing response total.
 */
long long bing_response_get_total(bing_response_t response); //XXX Not used anymore, remove

/**
 * @brief Get the offset within the results for a Bing response.
 *
 * The @c bing_response_get_offset() function allows developers to retrieve the
 * zero-based offset within the results results that response can get.
 * This allows for easier searching through many results.
 *
 * @param response The Bing response to get offset of.
 *
 * @return The Bing response offset.
 */
long long bing_response_get_offset(bing_response_t response); //XXX Not used anymore but could be something to retrieve and parse so custom offsets can be used.

/**
 * @brief Get the query used to search for this Bing response.
 *
 * The @c bing_response_get_query() function allows developers to retrieve the
 * actual query used to get this Bing response.
 *
 * @param response The Bing response to get the query of.
 * @param buffer The buffer to copy the query into.
 *
 * @return The size of the Bing response query in bytes, or -1
 * 	if an error occurred.
 */
int bing_response_get_query(bing_response_t response, char* buffer); //XXX Not used anymore, remove

/**
 * @brief Get the altered query used to search for this Bing response.
 *
 * The @c bing_response_get_altered_query() function allows developers
 * to retrieve the altered search query, if appropriate, to get
 * this Bing response.
 *
 * @param response The Bing response to get the altered query of.
 * @param buffer The buffer to copy the altered query into.
 *
 * @return The size of the Bing response altered query in bytes, or
 * 	-1 if an error occurred.
 */
int bing_response_get_altered_query(bing_response_t response, char* buffer); //XXX Not used anymore, remove

/**
 * @brief Get the unaltered query used to search for this Bing response.
 *
 * The @c bing_response_get_alterations_override_query() function allows
 * developers to retrieve the original, unaltered search query that
 * can be used to research with that won't be altered.
 *
 * @param response The Bing response to get the altered query of.
 * @param buffer The buffer to copy the unaltered query into.
 *
 * @return The size of the Bing response unaltered query in bytes, or
 * 	-1 if an error occurred.
 */
int bing_response_get_alterations_override_query(bing_response_t response, char* buffer); //XXX Not used anymore, remove

/**
 * @brief Get the results from a Bing response.
 *
 * The @c bing_response_get_results() function allows developers to
 * get the actual search results.
 *
 * @param response The Bing response to get the results of.
 * @param results The array of results to copy into.
 *
 * @return The Bing response result count, or -1 if an error
 * 	occurred.
 */
int bing_response_get_results(bing_response_t response, bing_result_t* results);

/**
 * @brief Free a Bing response from memory.
 *
 * The @c bing_response_free() function allows developers to free
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
void bing_response_free(bing_response_t response);

//Specific functions

/**
 * @brief Get the Ad API version for an Bing Ad response.
 *
 * The @c bing_response_get_ad_api_version() function allows developers to
 * get the API version of a Bing Ad response.
 *
 * @param response The Bing response to get the Ad API version from.
 * @param buffer The buffer to copy the Ad API version into.
 *
 * @return The size of the Bing Ad response API version in bytes, or
 * 	-1 if an error occurred or if the response is not an Ad type.
 */
int bing_response_get_ad_api_version(bing_response_t response, char* buffer); //XXX Not used anymore, remove

/**
 * @brief Get the Ad page number for an Bing Ad response.
 *
 * The @c bing_response_get_ad_page_number() function allows developers to
 * get the Ad page number of a Bing Ad response.
 *
 * @param response The Bing response to get the Ad page number from.
 *
 * @return The Bing Ad page number, or -1 if an error occurred or
 * 	if the response is not an Ad type.
 */
long long bing_response_get_ad_page_number(bing_response_t response); //XXX Not used anymore, remove

/**
 * @brief Get the responses from a Bing Bundle response.
 *
 * The @c bing_response_get_bundle_responses() function allows developers to
 * get the bundled Bing responses.
 *
 * @param response The Bing response to get the responses of.
 * @param responses The array of responses to copy into.
 *
 * @return The Bing response "responses" count, or -1 if an error
 * 	occurred or if the response is not a Bundle type.
 */
int bing_response_get_bundle_responses(bing_response_t response, bing_response_t* responses);

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
int bing_response_get_news_related_searches(bing_response_t response, bing_related_search_t searches); //XXX Not used anymore, remove

//Custom functions

/**
 * @brief Check if the Bing response filed type is supported.
 *
 * The @c bing_response_custom_is_field_supported() functions allows developers to determine
 * if a field is supported.
 *
 * @param response The Bing response to to check for a field.
 * @param field The field string to check for.
 *
 * @return A boolean value which is non-zero if the field is supported
 * 	within the specified Bing response, otherwise zero on error, NULL to,
 * 	or NULL field string.
 */
int bing_response_custom_is_field_supported(bing_response_t response, const char* field);

/**
 * @brief Get a custom value from a Bing response.
 *
 * The @c bing_response_custom_get_*() functions allows developers to retrieve
 * values from a Bing response. All values are self contained and will be
 * copied to the value parameter. These functions work on all response
 * types but allow for retrieval of custom result values.
 *
 * In the case of string and array, the return type is the amount of data,
 * in bytes. If value is NULL then nothing is copied.
 *
 * For array types, the actual data is copied, not pointers to the data.
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

int bing_response_custom_get_64bit_int(bing_response_t response, const char* field, long long* value);
int bing_response_custom_get_string(bing_response_t response, const char* field, char* value);
int bing_response_custom_get_double(bing_response_t response, const char* field, double* value);
int bing_response_custom_get_boolean(bing_response_t response, const char* field, int* value);
int bing_response_custom_get_array(bing_response_t response, const char* field, void* value);

/**
 * @brief Set a custom value for a Bing response.
 *
 * The @c bing_response_custom_set_*() functions allows developers to set
 * values to a custom Bing response. If the result is not the custom type
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

int bing_response_custom_set_64bit_int(bing_response_t response, const char* field, const long long* value);
int bing_response_custom_set_string(bing_response_t response, const char* field, const char* value);
int bing_response_custom_set_double(bing_response_t response, const char* field, const double* value);
int bing_response_custom_set_boolean(bing_response_t response, const char* field, const int* value);
int bing_response_custom_set_array(bing_response_t response, const char* field, const void* value, size_t size);

/**
 * @brief Allocate memory that will be freed when responses are freed.
 *
 * The @c bing_response_custom_allocation() function allows developers to
 * allocate any amount of memory between 1 byte and 10 KiB. This
 * memory will be freed when free_response is called for the same
 * response.
 *
 * It is recommended that this function not be used for anything but
 * internal pointers (a pointer within a structure).
 *
 * It is also not recommended to free the memory returned as it can
 * cause an exception when free_response is called.
 *
 * This is equivalent to malloc where the memory is not zeroed
 * on allocation.
 *
 * @param response The Bing response to allocate memory from.
 * @param size The size of the memory block to allocate.
 *
 * @return A pointer of the allocated memory will be returned, or
 * 	NULL if allocation failed or the size was above the allowed limit.
 */
void* bing_response_custom_allocation(bing_response_t response, size_t size);

/**
 * @brief Register a new response creator.
 *
 * The @c bing_result_register_result_creator() function allows developers to
 * register a set of callbacks and a name for a, as of now, unsupported
 * Bing response within this library.
 *
 * The creation function is provided the internally allocated response,
 * the name the response is associated with, and a dictionary with all
 * the attributes that were passed with result. This can indicate if
 * creation has run successfully or not based on the return value
 * where a non-zero value means it ran successfully and zero means
 * it failed (and thus will not be returned). Standard response values
 * are handled regardless of what creation function is passed in.
 *
 * Some responses can actually contain additional data. That's where
 * the additional data function comes in. When additional data
 * is loaded, this function gets called so the data can be handled in
 * whatever manner is deemed appropriate. The dictionary will be freed
 * when the callback function returns. If a result is contained
 * within the dictionary, it will be freed on return from the function.
 *
 * The dictionaries that are passed in can be NULL.
 *
 * @param name The name associated with the response. Only unsupported
 * 	names can be registered. For example, the name "web:Web"
 * 	is for a web response type. If this was passed in, it would fail.
 * 	Names are the XML names that returned by the Bing service. If the
 * 	name already exists then this function fails.
 * @param creation_func The function that handles any data passed in to
 * 	a response. This is optional.
 * @param additional_func The function that handles any additional
 * 	data that are passed in to another response. This is optional.
 * 	If this is NULL and additional data is found, it is ignored.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.
 */
int bing_response_register_response_creator(const char* name, response_creation_func creation_func, response_additional_data_func additional_func); //XXX Modify based on how "name" is determined/handled

/**
 * @brief Unregister a response creator.
 *
 * The @c bing_response_unregister_response_creator() function allows developers to
 * unregister a set of response creator callbacks.
 *
 * @param name The name associated with the response. This is the same
 * 	as the name passed into response_register_response_creator.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.
 */
int bing_response_unregister_response_creator(const char* name); //XXX Modify based on how "name" is determined/handled

/*
 * Result functions
 */

enum BING_RESULT_FIELD
{
	//All values are strings unless otherwise noted.

	BING_RESULT_FIELD_UNKNOWN,

	//64bit integer
	BING_RESULT_FIELD_RANK, //XXX Not used
	BING_RESULT_FIELD_POSITION, //XXX Not used
	BING_RESULT_FIELD_TITLE,
	BING_RESULT_FIELD_DESCRIPTION,
	BING_RESULT_FIELD_DISPLAY_URL,
	BING_RESULT_FIELD_ADLINK_URL, //XXX Not used
	//64bit integer
	BING_RESULT_FIELD_CODE, //XXX Not used
	BING_RESULT_FIELD_MESSAGE, //XXX Not used
	BING_RESULT_FIELD_HELP_URL, //XXX Not used
	BING_RESULT_FIELD_PARAMETER, //XXX Not used
	BING_RESULT_FIELD_SOURCE_TYPE, //XXX Not used
	//64bit integer
	BING_RESULT_FIELD_SOURCE_TYPE_ERROR_CODE, //XXX Not used
	BING_RESULT_FIELD_VALUE,
	//64bit integer
	BING_RESULT_FIELD_HEIGHT,
	//64bit integer
	BING_RESULT_FIELD_WIDTH,
	//64bit integer
	BING_RESULT_FIELD_FILE_SIZE,
	BING_RESULT_FIELD_MEDIA_URL,
	BING_RESULT_FIELD_URL,
	BING_RESULT_FIELD_CONTENT_TYPE,
	//bing_thumbnail_t
	BING_RESULT_FIELD_THUMBNAIL,
	BING_RESULT_FIELD_ATTRIBUTION, //XXX Not used
	BING_RESULT_FIELD_INSTANT_ANWSER_SPECIFIC_DATA, //XXX Not used
	BING_RESULT_FIELD_DATE_TIME, //XXX Not used
	//boolean
	BING_RESULT_FIELD_BREAKING_NEWS, //XXX Not used
	//64bit integer (represents number of milliseconds since epoch)
	BING_RESULT_FIELD_DATE,
	BING_RESULT_FIELD_SNIPPET, //XXX Not used
	BING_RESULT_FIELD_SOURCE,
	//bing_news_collection_s
	BING_RESULT_FIELD_NEWSCOLLECTION, //XXX Not used
	BING_RESULT_FIELD_TRANSLATED_TERM, //XXX Not used
	BING_RESULT_FIELD_SOURCE_TITLE, //XXX Not used
	BING_RESULT_FIELD_RUN_TIME, //XXX Not used
	BING_RESULT_FIELD_PLAY_URL, //XXX Not used
	BING_RESULT_FIELD_CLICK_THROUGH_PAGE_URL, //XXX Not used
	//bing_thumbnail_t
	BING_RESULT_FIELD_STATIC_THUMBNAIL, //XXX Not used
	BING_RESULT_FIELD_CACHE_URL, //XXX Not used
	//bing_deep_link_s
	BING_RESULT_FIELD_DEEP_LINKS, //XXX Not used
	//bing_search_tag_s
	BING_RESULT_FIELD_SEARCH_TAGS, //XXX Not used
	BING_RESULT_FIELD_ID,
	BING_RESULT_FIELD_SOURCE_URL,
	//64bit integer
	BING_RESULT_FIELD_RUN_TIME_LENGTH,
	BING_RESULT_FIELD_BING_URL,
	BING_RESULT_FIELD_TEXT,
	BING_RESULT_FIELD_RESULT
};

//Standard operations

/**
 * @brief Get the Bing result source type.
 *
 * The @c bing_result_get_source_type() functions allows developers to retrieve the
 * source type of Bing result that is passed in.
 *
 * @param result The Bing result to get the source type of.
 *
 * @return The Bing result source type, or BING_SOURCETYPE_UNKNOWN if NULL is passed in.
 */
enum BING_SOURCE_TYPE bing_result_get_source_type(bing_result_t result);

/**
 * @brief Check if the Bing result type is supported.
 *
 * The @c bing_result_is_field_supported() functions allows developers to determine
 * if a field is supported.
 *
 * @param result The Bing result to check for a field.
 * @param field The field to check for.
 *
 * @return A boolean value which is non-zero if the field is supported
 * 	within the specified Bing result, otherwise zero on error or NULL result.
 */
int bing_result_is_field_supported(bing_result_t result, enum BING_RESULT_FIELD field);

/**
 * @brief Get a value from a Bing result.
 *
 * The @c bing_result_get_*() functions allows developers to retrieve values from
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

int bing_result_get_64bit_int(bing_result_t result, enum BING_RESULT_FIELD field, long long* value);
int bing_result_get_string(bing_result_t result, enum BING_RESULT_FIELD field, char* value);
int bing_result_get_double(bing_result_t result, enum BING_RESULT_FIELD field, double* value);
int bing_result_get_boolean(bing_result_t result, enum BING_RESULT_FIELD field, int* value);
int bing_result_get_array(bing_result_t result, enum BING_RESULT_FIELD field, void* value);

//Custom operations

/**
 * @brief Check if the Bing to type is supported.
 *
 * The @c bing_result_custom_is_field_supported() functions allows developers to determine
 * if a field is supported.
 *
 * @param result The Bing result to check for a field.
 * @param field The field string to check for.
 *
 * @return A boolean value which is non-zero if the field is supported
 * 	within the specified Bing result, otherwise zero on error, NULL to,
 * 	or NULL field string.
 */
int bing_result_custom_is_field_supported(bing_result_t result, const char* field);

/**
 * @brief Get a custom value from a Bing result.
 *
 * The @c bing_result_custom_get_*() functions allows developers to retrieve
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

int bing_result_custom_get_64bit_int(bing_result_t result, const char* field, long long* value);
int bing_result_custom_get_string(bing_result_t result, const char* field, char* value);
int bing_result_custom_get_double(bing_result_t result, const char* field, double* value);
int bing_result_custom_get_boolean(bing_result_t result, const char* field, int* value);
int bing_result_custom_get_array(bing_result_t result, const char* field, void* value);

/**
 * @brief Set a custom value for a Bing result.
 *
 * The @c bing_result_custom_set_*() functions allows developers to set
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

int bing_result_custom_set_64bit_int(bing_result_t result, const char* field, const long long* value);
int bing_result_custom_set_string(bing_result_t result, const char* field, const char* value);
int bing_result_custom_set_double(bing_result_t result, const char* field, const double* value);
int bing_result_custom_set_boolean(bing_result_t result, const char* field, const int* value);
int bing_result_custom_set_array(bing_result_t result, const char* field, const void* value, size_t size);

/**
 * @brief Allocate memory that will be freed when the parent response
 * is freed.
 *
 * The @c bing_result_custom_allocation() function allows developers to
 * allocate any amount of memory between 1 byte and 5 KiB. This
 * memory will be freed when free_response is called for the
 * parent response that the result came from.
 *
 * It is recommended that this function not be used for anything but
 * internal pointers (a pointer within a structure).
 *
 * It is also not recommended to free the memory returned as it can
 * cause an exception when free_response is called.
 *
 * This is equivalent to malloc where the memory is not zeroed
 * on allocation.
 *
 * @param result The Bing result to allocate memory from.
 * @param size The size of the memory block to allocate.
 *
 * @return A pointer of the allocated memory will be returned, or
 * 	NULL if allocation failed or the size was above the allowed limit.
 */
void* bing_result_custom_allocation(bing_result_t result, size_t size);

/**
 * @brief Register a new result creator.
 *
 * The @c bing_result_register_result_creator() function allows developers to
 * register a set of callbacks and a name for a, as of now, unsupported
 * Bing result.
 *
 * The creation function is provided the internally allocated result,
 * the name the result is associated with, and a dictionary with all
 * the attributes that were passed with result. This can indicate if
 * creation has run successfully or not based on the return value
 * where a non-zero value means it ran successfully and zero means
 * it failed (and thus will not be returned). The dictionary that is
 * passed in can be NULL.
 *
 * Some results can actually contain additional results. That's where
 * the additional result function comes in. When an additional result
 * is loaded, this function gets called so the result can be handled in
 * whatever manner is deemed appropriate. The name passed in is the
 * name of the additional result. Results registered with this function
 * can be additional results. Be sure to set if the result is should be
 * kept or not (non-zero means TRUE, zero means FALSE) to prevent memory
 * leaks from occurring. This way a result can simply be stored or it can
 * be removed when no longer needed.
 *
 * @param name The name associated with the result. Only unsupported
 * 	names can be registered. For example, the name "web:WebResult"
 * 	is for a web result type. If this was passed in, it would fail.
 * 	Names are the XML names that returned by the Bing service. If the
 * 	name already exists then this function fails.
 * @param array A non-zero value if the result should be considered an
 * 	array (meaning it can contain multiple other results). A zero value
 * 	means that it is not an array.
 * @param common A non-zero value if the result is a common value as
 * 	opposed to an individual result. For example, a web request would
 * 	be responded to with a non-common result. But the data types it
 * 	might contain would be common types.
 * @param creation_func The function that handles any data passed in to
 * 	a result. This function is required.
 * @param additional_func The function that handles any additional
 * 	results that are passed in to another result. This is optional.
 * 	If this is NULL and an additional result is found, it is ignored.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.
 */
int bing_result_register_result_creator(const char* name, int array, int common, result_creation_func creation_func, result_additional_result_func additional_func); //XXX Modify based on how "name" is determined/handled

/**
 * @brief Unregister a result creator.
 *
 * The @c bing_result_unregister_result_creator() function allows developers to
 * unregister a set of result creator callbacks.
 *
 * @param name The name associated with the result. This is the same
 * 	as the name passed into result_register_result_creator.
 *
 * @return A boolean value which is non-zero for a successful registration,
 * 	otherwise zero on error.
 */
int bing_result_unregister_result_creator(const char* name); //XXX Modify based on how "name" is determined/handled

__END_DECLS

#endif /* BING_H_ */
