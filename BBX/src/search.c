/*
 * search.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"
#include <libxml/parser.h>

//XXX Key thing to remember is to add result to another result or response at the end of the execution (this will prevent issues with execution, as some added results are expected to be filled in already)
//XXX Be sure to use response_def_create_standard_responses before using the actual creation function for the response

typedef struct PARSER_STACK_S
{
	void* value;
	BOOL keepValue;
	bing_result* addToResult;
	bing_response* addToResponse;
	struct PARSER_STACK_S* prev;
} pstack;

typedef struct BING_PARSER_S
{
	bing_response* response;
	bing_response* current;
	pstack* lastResult;
	pstack* lastResultElement;
	const char* query;
	const char* alteredQuery;
	const char* alterationOverrideQuery;
	BOOL errorRet;

	unsigned int bing;
	BOOL parseError;
} bing_parser;

//Processors
void errorCallback(void *ctx, const char *msg, ...)
{
	bing_parser* parser = (bing_parser*)((xmlParserCtxtPtr)ctx)->userData;
	//TODO
}

void startElement(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	bing_parser* parser = (bing_parser*)((xmlParserCtxtPtr)ctx)->userData;
	//TODO
}

void endElement(void *ctx, const xmlChar *name)
{
	pstack* st;
	bing_parser* parser = (bing_parser*)((xmlParserCtxtPtr)ctx)->userData;
	hashtable_t* table;
	if(parser->lastResultElement)
	{
		if(strcmp((char*)parser->lastResultElement->value, (char*)name) == 0)
		{
			//Free result
			st = parser->lastResult;
			parser->lastResult = st->prev;

			if(st->keepValue) //Well we where told to keep the result, but it might be special...
			{
				//The result might supposed to be added to another result/response.

				st->keepValue = FALSE; //Just for the sake of it, we are going to say we don't want to keep the result...

				if(st->addToResult)
				{
					st->addToResult->additionalResult((char*)parser->lastResultElement->value, (bing_result_t)st->addToResult, (bing_result_t)st->value, &st->keepValue); //Because the addition function might want get rid of it
				}
				else if(st->addToResponse)
				{
					//Response is a little harder to do as it takes a dictionary (hashtable). We need to create the hashtable and store the result within it before passing it to the return
					table = hashtable_create(1);
					if(table)
					{
						if(hashtable_put_item(table, strchr((char*)name, ':') + 1, st->value, sizeof(bing_result_t)) != -1)
						{
							st->addToResponse->additionalData((bing_response_t)st->addToResponse, (data_dictionary_t)table);
						}
						hashtable_free(table);
					}
				}
				else
				{
					//Nothing to add the result to, we might as well keep the value (it will get freed with it's parent  [hopefully])
					st->keepValue = TRUE;
				}
			}
			//Free the result if desired
			if(!st->keepValue)
			{
				//Try to free it from the normal result list
				if(!response_remove_result(((bing_result*)st->value)->parent, (bing_result*)st->value, FALSE, TRUE))
				{
					//Try to free it from the internal result list
					if(!response_remove_result(((bing_result*)st->value)->parent, (bing_result*)st->value, TRUE, TRUE))
					{
						//Just free it, no idea why it wasn't in either list
						free_result((bing_result*)st->value);
					}
				}
			}
			free(st); //Object

			//Free the element
			st = parser->lastResultElement;
			parser->lastResultElement = st->prev;

			free(st->value); //Name
			free(st); //Object
		}
	}
}

xmlSAXHandler parserHandler=
{
		//XXX Document so this isn't painful to look at
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		startElement,
		endElement,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		errorCallback,
		NULL,
		NULL,
		NULL,
		NULL,
		FALSE,
		NULL,
		NULL,
		NULL,
		NULL
};

void serach_setup()
{
	LIBXML_TEST_VERSION

	searchCount++;

	if(searchCount == 1)
	{
		//On first run, setup the parser
		xmlInitParser();
	}
}

bing_response_t search_sync(unsigned int bingID, const char* query, const bing_request_t request)
{
	const char* url;
	bing_response_t ret = NULL;

	serach_setup();

	url = request_url(bingID, query, request);
	if(url)
	{
		//TODO

		free((void*)url);
	}

	return ret;
}

int search_async(unsigned int bingID, const char* query, const bing_request_t request, receive_bing_response_func response_func)
{
	const char* url;
	BOOL ret = FALSE;

	serach_setup();

	url = request_url(bingID, query, request);
	if(url)
	{
		//TODO

		free((void*)url);
	}

	return ret;
}

int search_event_async(unsigned int bingID, const char* query, const bing_request_t request)
{
	const char* url;
	BOOL ret = FALSE;

	serach_setup();

	url = request_url(bingID, query, request);
	if(url)
	{
		//TODO

		free((void*)url);
	}

	return ret;
}

//XXX: xmlCreatePushParserCtxt

void search_cleanup(xmlParserCtxtPtr ctx)
{
	//TODO
	searchCount--;
	if(searchCount)
	{
		xmlCleanupParser();
	}
}
