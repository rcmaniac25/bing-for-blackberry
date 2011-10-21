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
int bing_get_domain();

/*
 * Bing search system
 */
unsigned int create_bing(const char* application_ID);

void free_bing(unsigned int bing);

#if defined(BING_DEBUG)
void set_error_return(unsigned int bing, int error);

int get_error_return(unsigned int bing);
#endif

const char* get_app_ID(unsigned int bing);

void set_app_ID(unsigned int bing, const char* appId);

bing_response_t search_sync(unsigned int bing, const char* query, bing_request_t request);

void search_async(unsigned int bing, const char* query, bing_request_t request, receive_bing_response_func response_func);

void search_event_async(unsigned int bing, const char* query, bing_request_t request);

void free_response(bing_response_t response);

const char* request_url(unsigned int bing, const char* query, bing_request_t request);

/*
 * Request functions
 */
//TODO

/*
 * Response functions
 */
//TODO

/*
 * Result functions
 */
//TODO

__END_DECLS

#endif /* BING_H_ */
