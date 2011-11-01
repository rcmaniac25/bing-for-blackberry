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
#include <bps/bps.h>

/*
 * Defines
 */

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

typedef struct BING_SYSTEM_S
{
	int domainID;
	pthread_mutex_t mutex;

	int bingInstancesCount;
	bing** bingInstances;
} bing_system;

typedef enum
{
	FIELD_TYPE_UNKNOWN,
	FIELD_TYPE_LONG,
	FIELD_TYPE_STRING,
	FIELD_TYPE_DOUBLE,
	FIELD_TYPE_BOOLEAN,
	FIELD_TYPE_ARRAY
} FIELD_TYPE;

typedef struct BING_FIELD_SUPPORT_S
{
	//This is the built in variable value (REQUEST_FIELD, RESULT_FIELD)
	int variableValue;
	FIELD_TYPE type;
	const char* name;

	int sourceTypeCount;
	SOURCE_TYPE supportedTypes[BING_SOURCETYPE_COUNT];
} bing_field_support;

typedef struct BING_FIELD_SEARCH_S
{
	bing_field_support field;
	struct BING_FIELD_SEARCH_S* next;
} bing_field_search;

typedef struct BING_REQUEST_S
{
	const char* sourceType;

	//These will never be NULL
	request_get_options_func getOptions;
	request_finish_get_options_func finishGetOptions;
} bing_request;

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

const char* find_field(bing_field_search* searchFields, int fieldID, FIELD_TYPE type, SOURCE_TYPE sourceType);

//TODO: Dictionary functions (hash_map), creation of requests, responses, result, etc.

#endif /* BING_INTERNAL_H_ */
