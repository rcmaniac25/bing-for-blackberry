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

void initialize();

void shutdown();

//TODO: Dictionary functions, creation of requests, responses, result, etc.

#endif /* BING_INTERNAL_H_ */
