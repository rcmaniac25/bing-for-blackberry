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
#include <stdlib.h>
#include <pthread.h>
#include <bps/bps.h>

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

static bing_system bingSystem;

/*
 * Setup the Bing subsystem.
 */
void initialize();

/*
 * Shutdown the Bing subsystem.
 */
void shutdown();

//TODO: Dictionary functions, creation of requests, responses, result, etc.

#endif /* BING_INTERNAL_H_ */
