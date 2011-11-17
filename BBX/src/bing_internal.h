/*
 * bing_internal.h
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#ifndef BING_INTERNAL_H_
#define BING_INTERNAL_H_

#include "bing.h"

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <bps/bps.h>

/*
 * Defines
 */

__BEGIN_DECLS

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

#if defined (__cplusplus) || defined(__CPLUSPLUS__)
#define BOOL_TO_CPP_BOOL(x) (x) != FALSE
#define CPP_BOOL_TO_BOOL(x) (x) ? TRUE : FALSE
#endif

/*
 * Structures
 */

typedef struct BING_S
{
	pthread_mutex_t mutex;

	char* appId;
#if defined(BING_DEBUG)
	BOOL errorRet;
#endif
} bing;

enum FIELD_TYPE
{
	FIELD_TYPE_UNKNOWN,
	FIELD_TYPE_LONG,
	FIELD_TYPE_STRING,
	FIELD_TYPE_DOUBLE,
	FIELD_TYPE_BOOLEAN,
	FIELD_TYPE_ARRAY
};

typedef struct BING_FIELD_SUPPORT_S
{
	//This is the built in variable value (REQUEST_FIELD, RESULT_FIELD)
	int variableValue;
	enum FIELD_TYPE type;
	const char* name;

	int sourceTypeCount;
	enum SOURCE_TYPE supportedTypes[15]; //BING_SOURCETYPE_COUNT
} bing_field_support;

#define BING_FIELD_SUPPORT_ALL_FIELDS -2

typedef struct BING_FIELD_SEARCH_S
{
	bing_field_support field;
	struct BING_FIELD_SEARCH_S* next;
} bing_field_search;

typedef struct hashtable_s hashtable_t;

typedef struct BING_REQUEST_S
{
	const char* sourceType;

	//These will never be NULL
	request_get_options_func getOptions;
	request_finish_get_options_func finishGetOptions;
	hashtable_t* data;
} bing_request;

typedef struct BING_RESULT_S
{
	//These will never be NULL
	result_creation_func creation;
	result_additional_result_func additionalResult;
	hashtable_t* data;
} bing_result;

typedef struct BING_RESPONSE_S
{
	//These will never be NULL
	response_creation_func creation;
	response_additional_data_func additionalData;
	hashtable_t* data;
	int resultCount;
	bing_result* results;
} bing_response;

typedef struct BING_RESPONSE_CREATOR_S
{
	const char* name;
	response_creation_func creation;
	response_additional_data_func additionalData;
} bing_response_creator;

typedef struct BING_RESULT_CREATOR_S
{
	const char* name;
	result_creation_func creation;
	result_additional_result_func additionalResult;
} bing_result_creator;

typedef struct BING_SYSTEM_S
{
	int domainID;
	pthread_mutex_t mutex;

	int bingInstancesCount;
	bing** bingInstances;
} bing_system;

/*
 * Variables
 */

static bing_system bingSystem;

/*
 * Functions
 */

/*
 * Setup the Bing subsystem.
 */
void initialize();

/*
 * Shutdown the Bing subsystem.
 */
//TODO: Not sure how this will be called
void shutdown();

const char* find_field(bing_field_search* searchFields, int fieldID, enum FIELD_TYPE type, enum SOURCE_TYPE sourceType);

//Dictionary functions
hashtable_t* hashtable_create();
void hashtable_free(hashtable_t* table);
int hashtable_key_exists(hashtable_t* table, const char* key);
int hashtable_put_item(hashtable_t* table, const char* key, void* data, size_t data_size);
int hashtable_get_item(hashtable_t* table, const char* name, void* data);
int hashtable_remove_item(hashtable_t* table, const char* key);
int hashtable_get_keys(hashtable_t* table, char** keys);

//TODO: creation of requests, responses, result, etc.

__END_DECLS

#endif /* BING_INTERNAL_H_ */
