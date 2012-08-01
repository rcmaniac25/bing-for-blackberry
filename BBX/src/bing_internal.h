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

#include "bing_cpp.h" //Use this version since it includes bing.h

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <atomic.h>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

/*
 * Defines
 */

__BEGIN_DECLS

//Defines for processing and setup
#if !defined(BOOL)
#define BOOL int

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif
#endif

#define RESULT_CREATE_DEFAULT_INTERNAL FALSE

//Helper macros
#define BOOL_TO_CPP_BOOL(x) (x) != FALSE
#define CPP_BOOL_TO_BOOL(x) (x) ? TRUE : FALSE

//Internal keys
#define REQUEST_COMPOSITE_SUBREQ_STR "bb_req_composite_sub-requests"
#define RESPONSE_COMPOSITE_SUBRES_STR "bb_res_composite_sub-responses"

#define RESPONSE_MAX_TOTAL_STR "bb_res_max_total"
#define RESPONSE_OFFSET_STR "bb_res_offset"
#define RESPONSE_QUERY_STR "bb_res_query"

//Composite define
#define RESPONSE_COMPOSITE "composite"

//Parsing defines
#define PARSE_LINK_NEXT_KEY "#nextLink"
#define PARSE_NAME_TITLE "title"

#define PARSE_COMPOSITE_IDENT "ExpandableSearchResult"

/**
 * The print out function to use for messages.
 * void printFunc(const char* msg, ...);
 */
#define BING_MSG_PRINTOUT printf

/**
 * Memory functions
 *
 * Tradeoff between flexibility and speed. This isn't a realtime library, it doesn't have to be extremely fast.
 * The faster way to do it would be to have static variables, as it calls the function directly. But if understanding of errors (such as where a error occurred) is necessary, this doesn't work.
 */

//Memory types
typedef void* (*bing_malloc_handler)(size_t);
typedef void* (*bing_calloc_handler)(size_t,size_t);
typedef void* (*bing_realloc_handler)(void*,size_t);
typedef void (*bing_free_handler)(void*);
typedef char* (*bing_strdup_handler)(const char*);

//Memory handlers
#if defined(BING_NO_MEM_HANDLERS)
#define bing_mem_malloc malloc
#define bing_mem_calloc calloc
#define bing_mem_realloc realloc
#define bing_mem_free free
#define bing_mem_strdup strdup
#else
static bing_malloc_handler bing_mem_malloc = (bing_malloc_handler)malloc;
static bing_calloc_handler bing_mem_calloc = (bing_calloc_handler)calloc;
static bing_realloc_handler bing_mem_realloc = (bing_realloc_handler)realloc;
static bing_free_handler bing_mem_free = (bing_free_handler)free;
static bing_strdup_handler bing_mem_strdup = (bing_strdup_handler)strdup;
#endif

/*
 * Structures
 */

enum FIELD_TYPE
{
	FIELD_TYPE_UNKNOWN,
	FIELD_TYPE_LONG,
	FIELD_TYPE_INT,
	FIELD_TYPE_STRING,
	FIELD_TYPE_DOUBLE,
	FIELD_TYPE_BOOLEAN,
	FIELD_TYPE_ARRAY
};

typedef struct BING_FIELD_SUPPORT_S
{
	//This is the built in variable value (BING_REQUEST_FIELD, BING_RESULT_FIELD)
	int variableValue;
	enum FIELD_TYPE type;
	const char* name;

	int sourceTypeCount;
	enum BING_SOURCE_TYPE supportedTypes[BING_SOURCETYPE_COMPOSITE_COUNT];
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
	const char* compositeSourceType;
	BOOL custom;
	request_get_options_func uGetOptions;
	request_finish_get_options_func uFinishGetOptions;

	//This is a counter that allows us to determine if it has been added to a composite, while still allowing it to be added to multiple composite types
	int compositeUse;

	//These will never be NULL
	request_get_options_func getOptions;
	hashtable_t* data;
} bing_request;

typedef struct BING_RESULT_S
{
	enum BING_SOURCE_TYPE type;

	//These will never be NULL
	struct BING_RESPONSE_S* parent;
	result_creation_func creation;
	result_additional_result_func additionalResult;
	hashtable_t* data;
} bing_result;

typedef struct BING_RESPONSE_S
{
	enum BING_SOURCE_TYPE type;
	unsigned int bing;

	//These will never be NULL
	response_creation_func creation;
	hashtable_t* data;

	const char* nextUrl;

	unsigned int resultCount;
	bing_result_t* results;

	unsigned int internalResultCount;
	bing_result_t* internalResults;

	unsigned int allocatedMemoryCount;
	void** allocatedMemory;
} bing_response;

typedef struct BING_S
{
	pthread_mutex_t mutex;

	char* accountKey;

	unsigned int responseCount;
	bing_response** responses;
} bing;

typedef struct BING_RESPONSE_CREATOR_S
{
	const char* dedicatedName;
	const char* compositeName;
	response_creation_func creation;
} bing_response_creator;

typedef struct BING_RESULT_CREATOR_S
{
	const char* name;
	BOOL type;
	result_creation_func creation;
	result_additional_result_func additionalResult;
} bing_result_creator;

typedef struct BING_SYSTEM_S
{
	int domainID;
	pthread_mutex_t mutex;

	unsigned int bingInstancesCount;
	bing** bingInstances;

	unsigned int bingResponseCreatorCount;
	bing_response_creator* bingResponseCreators;

	unsigned int bingResultCreatorCount;
	bing_result_creator* bingResultCreators;
} bing_system;

typedef struct list_s
{
	unsigned int count;
	unsigned int cap;
	void* listElements;
} list;

#define LIST_ELEMENTS(l, t) ((t*)l->listElements)
#define LIST_ELEMENT(l, i, t) (((t*)l->listElements)[i])

/*
 * Variables
 */

static bing_system bingSystem;
static volatile unsigned int searchCount;
#if defined(BING_DEBUG)
static int lastErrorCode = 0;
#endif

/*
 * Functions
 */

const char* find_field(bing_field_search* searchFields, int fieldID, enum FIELD_TYPE type, enum BING_SOURCE_TYPE sourceType, BOOL checkType);
void append_data(hashtable_t* table, const char* format, const char* key, void** data, size_t* curDataSize, char** returnData, size_t* returnDataSize);
const char* xmlGetQualifiedName(xmlNodePtr node);

//Dictionary functions
hashtable_t* hashtable_create(int size);
void hashtable_free(hashtable_t* table);
BOOL hashtable_copy(hashtable_t* dstTable, const hashtable_t* srcTable);
BOOL hashtable_compact(hashtable_t* table);
BOOL hashtable_key_exists(hashtable_t* table, const char* key);
BOOL hashtable_put_item(hashtable_t* table, const char* key, const void* data, size_t data_size);
size_t hashtable_get_item(hashtable_t* table, const char* name, void* data);
BOOL hashtable_remove_item(hashtable_t* table, const char* key);
int hashtable_get_keys(hashtable_t* table, char** keys); //Returns the number of keys
//-Helper dictionary functions
BOOL hashtable_get_data_key(hashtable_t* table, const char* key, void* value, size_t size);
int hashtable_get_string(hashtable_t* table, const char* field, char* value);
BOOL hashtable_set_data(hashtable_t* table, const char* field, const void* value, size_t size);

//Bing functions
bing* retrieveBing(unsigned int bingID);

//Type functions
BOOL isComplex(const char* name);
enum FIELD_TYPE getParsedTypeByType(const char* type);
enum FIELD_TYPE getParsedTypeByName(xmlNodePtr node);
void* parseByType(const char* type, xmlNodePtr node, xmlFreeFunc xmlFree);
void* parseByName(xmlNodePtr node, xmlFreeFunc xmlFree);
BOOL parseToHashtableByType(const char* type, xmlNodePtr node, hashtable_t* table, xmlFreeFunc xmlFree);
BOOL parseToHashtableByName(xmlNodePtr node, hashtable_t* table, xmlFreeFunc xmlFree);
long long parseTime(const char* stime);

//Request functions
const char* request_get_composite_sourcetype(bing_request* composite);
BOOL response_def_create_standard_responses(bing_response_t response, data_dictionary_t dictionary);
BOOL response_create_raw(const char* type, bing_response_t* response, unsigned int bing, bing_response* responseParent);
BOOL response_add_result(bing_response* response, bing_result* result, BOOL internal);
BOOL response_remove_result(bing_response* response, bing_result* result, BOOL internal, BOOL freeResult);
BOOL response_swap_result(bing_response* response, bing_result* result, BOOL internal);
BOOL response_swap_response(bing_response* response, bing_response* responseParent);

//Result functions
BOOL result_create_raw(const char* type, bing_result_t* result, bing_response* responseParent);
void free_result(bing_result* result);

//Memory functions
void* allocateMemory(size_t size, bing_response* response);
void* rallocateMemory(void* ptr, size_t size, bing_response* response);
void freeMemory(void* ptr, bing_response* response);

__END_DECLS

#endif /* BING_INTERNAL_H_ */
