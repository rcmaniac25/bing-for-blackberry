/*
 * search.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

//XXX Key thing to remember is to add result to another result or response at the end of the execution (this will prevent issues with execution, as some added results are expected to be filled in already)
//XXX Be sure to use response_def_create_standard_responses before using the actual creation function
//XXX Don't forget to manually set query, altered query, and alterations over query

//TODO: search_sync

//TODO: search_async

//TODO: search_event_async
