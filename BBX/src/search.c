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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

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
	//Freed on error
	bing_response* response;
	bing_response* current;
	pstack* lastResult;
	pstack* lastResultElement;
	const char* query;
	const char* alteredQuery;
	const char* alterationOverrideQuery;

	//State info
	unsigned int bing;
	int socket;
	receive_bing_response_func responseFunc;
	void* userData;
	BOOL parseError;
#if defined(BING_DEBUG)
	BOOL errorRet;
#endif
} bing_parser;

//Processors
void errorCallback(void *ctx, const char *msg, ...)
{
	bing_parser* parser = (bing_parser*)((xmlParserCtxtPtr)ctx)->userData;
	parser->parseError = TRUE; //We simply mark this as error because on completion we can check this and it will automatically handle all cleanup and we can get if the search completed or not
}

//*sigh* C doesn't have an equivalent function of endWith unless it's a character.
BOOL endsWith(const char* str1, const char* str2)
{
	//See if we can find str2

	//First we need the length of the string (we can ignore the null char)
	int len = str2 ? strlen(str2) : 0;

	//Next we need to see if we can find an initial instance of the string
	const char* sub = NULL;
	if(len > 0)
	{
		sub = strchr(str1, str2[0]);
	}

	//If we found an instance, see if it is the correct instance...
	while(sub)
	{
		//We do this by first checking length, if it is the same length then it could be it..
		if(strlen(sub) == len)
		{
			//strcmp will tell us if it matches, if it doesn't match the string would be too short to run anyway so just return
			return strcmp(sub, str2) == 0;
		}
		//String was too big, try again (don't forget to skip to the next char)
		sub = strchr(sub + 1, str2[0]);
	}

	//Didn't find anything
	return FALSE;
}

BOOL checkForError(bing_parser* parser)
{
	pstack* st;
	if(parser->parseError)
	{
		//Error, cleanup everything

		//First clean the stacks

		//Free result stack (we don't want to BING_FREE the actual results)
		while(parser->lastResult)
		{
			st = parser->lastResult;
			parser->lastResult = st->prev;

			BING_FREE(st); //Object
		}

		//Free the element
		while(parser->lastResultElement)
		{
			st = parser->lastResultElement;
			parser->lastResultElement = st->prev;

			BING_FREE(st->value); //Name
			BING_FREE(st); //Object
		}

		//Now BING_FREE the response (no stacks will exist if a response doesn't exist. Allocated memory, internal responses, and all results are associated with the parent response. Freeing the response will BING_FREE everything.)
		free_response(parser->current);

		//Finally, BING_FREE any strings
		BING_FREE((void*)parser->query);
		BING_FREE((void*)parser->alteredQuery);
		BING_FREE((void*)parser->alterationOverrideQuery);

		//Mark everything as NULL to prevent errors later
		parser->response = NULL;
		parser->current = NULL;
		parser->lastResult = NULL;
		parser->lastResultElement = NULL;
		parser->query = NULL;
		parser->alteredQuery = NULL;
		parser->alterationOverrideQuery = NULL;

		return FALSE; //Don't continue
	}
	return TRUE; //No error, continue
}

hashtable_t* createHashtableFromAtts(const xmlChar** atts)
{
	hashtable_t* table = NULL;
	int count;
	if(atts)
	{
		//First count the number of elements
		for(count = 0; atts[count]; count++)
		{
			//Attributes are name/value pairs, so if we have a value at 0, then we have one at 1. One at 2 means one at 3 as well. Etc.
			count++;
		}

		table = hashtable_create(count);
		if(table)
		{
			//Fill table
			for(count = 0; atts[count]; count++)
			{
				hashtable_put_item(table,
						(char*)atts[count],
						atts[count + 1],
						strlen((char*)atts[count + 1]) + 1);
				count++;
			}
		}
	}
	return table;
}

void addResultToStack(bing_parser* parser, bing_result* result, const char* name, BOOL internal)
{
	pstack* pt = BING_MALLOC(sizeof(pstack));
	if(pt)
	{
		memset(pt, 0, sizeof(pstack));

		//Add to last result
		pt->prev = parser->lastResult;
		parser->lastResult = pt;
		pt->keepValue = TRUE;
		pt->value = result;

		pt = BING_MALLOC(sizeof(pstack));
		if(pt)
		{
			//Add to last result element
			pt->prev = parser->lastResultElement;
			parser->lastResultElement = pt;
			pt->keepValue = TRUE;
			pt->value = strdup(name);
			if(!pt->value)
			{
				//Darn, so close again
				parser->parseError = TRUE;

				response_remove_result(parser->current, result, internal, TRUE);
			}
		}
		else
		{
			//That's not good (darn, so close. We where able to allocate the first stack but not the second)
			parser->parseError = TRUE;

			response_remove_result(parser->current, result, internal, TRUE);
		}
	}
	else
	{
		//That's not good
		parser->parseError = TRUE;

		response_remove_result(parser->current, result, internal, TRUE);
	}
}

void startElement(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	bing_result_t result = NULL;
	hashtable_t* table;
	int size;
	char* data;
	bing_parser* parser = (bing_parser*)((xmlParserCtxtPtr)ctx)->userData;
	pstack* pt;

	if(checkForError(parser))
	{
		//It's a result type
		if(endsWith((char*)name, "Result"))
		{
			if(parser->current)
			{
				if(!result_is_common((char*)name) && //First check if the result is "common" which excludes it from this usage (especially since custom results could end in "Result" and be common)
						result_create_raw((char*)name, &result, parser->current)) //Try to actually create the result (and adds it to the response)
				{
					//Create the dictionary (if this fails then, o well. Maybe no hashtable is returned because there where no attributes).
					table = createHashtableFromAtts(atts);

					//Result created successfully, time to save state
					if(((bing_result*)result)->creation((char*)name, result, (data_dictionary_t)table))
					{
						addResultToStack(parser, (bing_result*)result, (char*)name, RESULT_CREATE_DEFAULT_INTERNAL);
					}
					else
					{
						//Wasn't created correctly, BING_FREE (this isn't a parser error. The creator ran into an error [or something]).
						response_remove_result(parser->current, (bing_result*)result, RESULT_CREATE_DEFAULT_INTERNAL, TRUE);
					}
					hashtable_free(table);
				}
			}
		}
		else if(strcmp((char*)name, "Query") == 0)
		{
			table = createHashtableFromAtts(atts);

			if(table)
			{
				size = hashtable_get_string(table, "SearchTerms", NULL);
				if(size > -1)
				{
					BING_FREE((void*)parser->query);
					parser->query = BING_MALLOC(size);
					hashtable_get_string(table, "SearchTerms", (char*)parser->query);
				}
				size = hashtable_get_string(table, "AlteredQuery", NULL);
				if(size > -1)
				{
					BING_FREE((void*)parser->alteredQuery);
					parser->alteredQuery = BING_MALLOC(size);
					hashtable_get_string(table, "AlteredQuery", (char*)parser->alteredQuery);
				}
				size = hashtable_get_string(table, "AlterationOverrideQuery", NULL);
				if(size > -1)
				{
					BING_FREE((void*)parser->alterationOverrideQuery);
					parser->alterationOverrideQuery = BING_MALLOC(size);
					hashtable_get_string(table, "AlterationOverrideQuery", (char*)parser->alterationOverrideQuery);
				}
			}
			else
			{
				//If the table is NULL then we have an issue as we need that table for query
				parser->parseError = TRUE;
			}

			hashtable_free(table);
		}
#if defined(BING_DEBUG)
		else if(strcmp((char*)name, "Error") == 0)
		{
			table = createHashtableFromAtts(atts);

			//Can we create an actual error result?
			if(table && parser->current && parser->errorRet)
			{
				if(result_create_raw("Error", &result, parser->current))
				{
					//Run the creation function
					if(((bing_result*)result)->creation("Error", result, (data_dictionary_t)table))
					{
						//Print out the results (do them one at a time to prevent memory leaks that can occur if realloc is used without a backup pointer)b

						data = BING_MALLOC(sizeof(long long));
						result_get_64bit_int(result, RESULT_FIELD_CODE, (long long*)data);
						BING_MSG_PRINTOUT("Error Code = %lld\n", *((long long*)data));
						BING_FREE(data);

						data = BING_MALLOC(result_get_string(result, RESULT_FIELD_MESSAGE, NULL));
						result_get_string(result, RESULT_FIELD_MESSAGE, data);
						BING_MSG_PRINTOUT("Error Message = %s\n", data);
						BING_FREE(data);

						data = BING_MALLOC(result_get_string(result, RESULT_FIELD_PARAMETER, NULL));
						result_get_string(result, RESULT_FIELD_PARAMETER, data);
						BING_MSG_PRINTOUT("Error Parameter = %s\n", data);
						BING_FREE(data);
					}
					else
					{
						response_remove_result(parser->current, (bing_result*)result, RESULT_CREATE_DEFAULT_INTERNAL, TRUE);
					}
				}
			}
			hashtable_free(table);
		}
#endif
		else
		{
			if(result_is_common((char*)name))
			{
				table = createHashtableFromAtts(atts);

				if(result_create_raw((char*)name, &result, parser->current))
				{
					//Run the creation function
					if(((bing_result*)result)->creation((char*)name, result, (data_dictionary_t)table))
					{
						//Make the result a internal result (if this doesn't work, then it means that it's already internal)
						response_swap_result(parser->current, result, RESULT_CREATE_DEFAULT_INTERNAL);

						//If an array, we want to store it so we can get internal values
						if(((bing_result*)result)->array)
						{
							addResultToStack(parser, (bing_result*)result, (char*)name, TRUE);

							//Determine where it should be added to

							//We can let endElement handle saving it
							if(parser->lastResult && parser->lastResult->prev)
							{
								parser->lastResult->addToResult = (bing_result*)parser->lastResult->prev->value;
							}
							else
							{
								parser->lastResult->addToResponse = parser->current;
							}
						}
						else
						{
							data = BING_MALLOC(sizeof(int));
							if(data)
							{
								*((int*)data) = FALSE;

								//Determine where it should be added to
								if(parser->lastResult)
								{
									//This is getting added to a result
									((bing_result*)parser->lastResult->value)->additionalResult((char*)name, (bing_result_t)parser->lastResult->value, result, ((int*)data));
								}
								else
								{
									hashtable_free(table);

									//This is getting added to a response
									table = hashtable_create(1);
									if(table)
									{
										if(hashtable_put_item(table, strchr((char*)name, ':') + 1, &result, sizeof(bing_result_t)) != -1)
										{
											((bing_response*)parser->current)->additionalData((bing_response_t)parser->current, (data_dictionary_t)table);
										}
									}
								}
							}

							//Free the result (if we can)
							if(!data || !(*((int*)data)))
							{
								//Try to BING_FREE it from the internal result list
								if(!response_remove_result(((bing_result*)result)->parent, (bing_result*)result, TRUE, TRUE))
								{
									//Try to BING_FREE it from the normal result list
									if(!response_remove_result(((bing_result*)result)->parent, (bing_result*)result, FALSE, TRUE))
									{
										//Just BING_FREE it, no idea why it wasn't in either list
										free_result((bing_result*)result);
									}
								}
							}
							BING_FREE(data);
						}
					}
					else
					{
						//Error, remove
						response_remove_result(parser->current, (bing_result*)result, RESULT_CREATE_DEFAULT_INTERNAL, TRUE);
					}
				}

				hashtable_free(table);
			}
			else
			{
				parser->current = NULL;
				if(response_create_raw((char*)name, (bing_response_t*)&parser->current, parser->bing,
						(parser->response != NULL && parser->response->type == BING_SOURCETYPE_BUNDLE) ? parser->response : NULL)) //The general idea is that if there is already a response and it is bundle, it will be the parent. Otherwise add it to Bing
				{
					table = createHashtableFromAtts(atts);

					if(response_def_create_standard_responses(parser->current, (data_dictionary_t)table) &&
							parser->current->creation((char*)name, (bing_response_t)parser->current, (data_dictionary_t)table))
					{
						//Set remaining values
						if(parser->query)
						{
							hashtable_set_data(parser->current->data, RESPONSE_QUERY_STR, parser->query, sizeof(parser->query) + 1);
						}
						if(parser->alteredQuery)
						{
							hashtable_set_data(parser->current->data, RESPONSE_ALTERED_QUERY_STR, parser->alteredQuery, sizeof(parser->alteredQuery) + 1);
						}
						if(parser->alterationOverrideQuery)
						{
							hashtable_set_data(parser->current->data, RESPONSE_ALTERATIONS_OVER_QUERY_STR, parser->alterationOverrideQuery, sizeof(parser->alterationOverrideQuery) + 1);
						}

						//Should we do any extra processing on the response?
						if(parser->response)
						{
							//Response already exists. If it is a bundle then it is already added, otherwise we need to replace it
							if(parser->response->type != BING_SOURCETYPE_BUNDLE)
							{
								//Save this temporarily
								data = (char*)parser->response;

								if(response_create_raw("bundle", (bing_response_t*)&parser->response, parser->bing, NULL))
								{
									//We need to take the original response and make it a child of the new bundle response
									response_swap_response((bing_response*)data, parser->response);

									//We also need the new current response to be a child of the new bundle response
									response_swap_response(parser->current, parser->response);
								}
								else
								{
									//Darn it, that failed
									free_response(parser->current);
									parser->parseError = TRUE; //See parser error below for info why this is an actual error
								}
							}
						}
						else if(parser->current)
						{
							//Response doesn't exist, make current
							parser->response = parser->current;
						}
					}
					else
					{
						//Darn it, that failed
						if(!(parser->response != NULL && parser->response->type == BING_SOURCETYPE_BUNDLE))
						{
							//This wasn't a bundled response, just BING_FREE it (otherwise it will never get freed)
							free_response(parser->current);
						}
						parser->parseError = TRUE; //See parser error below for info why this is an actual error
					}

					hashtable_free(table);
				}
				else
				{
					//Unlike "results" where missing one is unfortunate, the "response" type helps maintain the tree hierarchy and skipping one would mess up everything (possibly)
					parser->parseError = TRUE;
				}
			}
		}
	}
}

void endElement(void *ctx, const xmlChar *name)
{
	pstack* st;
	bing_parser* parser = (bing_parser*)((xmlParserCtxtPtr)ctx)->userData;
	hashtable_t* table;
	if(checkForError(parser) && parser->lastResultElement)
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
						if(hashtable_put_item(table, strchr((char*)name, ':') + 1, &st->value, sizeof(bing_result_t)) != -1)
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
				//Try to BING_FREE it from the normal result list
				if(!response_remove_result(((bing_result*)st->value)->parent, (bing_result*)st->value, FALSE, TRUE))
				{
					//Try to BING_FREE it from the internal result list
					if(!response_remove_result(((bing_result*)st->value)->parent, (bing_result*)st->value, TRUE, TRUE))
					{
						//Just BING_FREE it, no idea why it wasn't in either list
						free_result((bing_result*)st->value);
					}
				}
			}
			BING_FREE(st); //Object

			//Free the element
			st = parser->lastResultElement;
			parser->lastResultElement = st->prev;

			BING_FREE(st->value); //Name
			BING_FREE(st); //Object
		}
	}
}

static const xmlSAXHandler parserHandler=
{
		NULL,			//internalSubset
		NULL,			//isStandalone
		NULL,			//hasInternalSubset
		NULL,			//hasExternalSubset
		NULL,			//resolveEntity
		NULL,			//getEntity
		NULL,			//entityDecl
		NULL,			//notationDecl
		NULL,			//attributeDecl
		NULL,			//elementDecl
		NULL,			//unparsedEntityDecl
		NULL,			//setDocumentLocator
		NULL,			//startDocument
		NULL,			//endDocument
		startElement,	//startElement
		endElement,		//endElement
		NULL,			//reference
		NULL,			//characters
		NULL,			//ignorableWhitespace
		NULL,			//processingInstruction
		NULL,			//comment
		NULL,			//warning
		errorCallback,	//error
		NULL,			//fatalError
		NULL,			//getParameterEntity
		NULL,			//cdataBlock
		NULL,			//externalSubset
		FALSE,			//initialized
		NULL,			//_private
		NULL,			//startElementNs
		NULL,			//endElementNs
		NULL			//serror
};

void search_setup()
{
	LIBXML_TEST_VERSION

	searchCount++;

	if(searchCount == 1)
	{
		//On first run, setup the parser
		xmlInitParser();
	}
}

void search_cleanup(xmlParserCtxtPtr ctx)
{
	bing_parser* parser = (bing_parser*)ctx->userData;

	//First shutdown the connection
	shutdown(parser->socket, SHUT_RDWR);

	//Next close the socket
	close(parser->socket);

	//Now get rid of the bing context
	parser->bing = 0;

	//Close thread (if used)
	//TODO

	//Cleanup strings
	BING_FREE((void*)parser->query);
	BING_FREE((void*)parser->alteredQuery);
	BING_FREE((void*)parser->alterationOverrideQuery);

	//Free the bing parser
	BING_FREE(parser);
	ctx->userData = NULL;

	//Free the document
	xmlFreeDoc(ctx->myDoc);

	//Free the actual context
	xmlFreeParserCtxt(ctx);

	searchCount--;
	if(searchCount)
	{
		xmlCleanupParser();
	}
}

int openUrl(const char* url)
{
	//Open's a socket and returns address info
	int ret = -1;
	int err;
	struct addrinfo hint, *adRoot, *ad;

	//Setup hints
	memset(&hint, 0, sizeof(struct addrinfo));
	hint.ai_socktype = SOCK_STREAM; //This is a standard HTTP connection
	hint.ai_protocol = IPPROTO_TCP; //We need data in order

	//DNS lookup
	if((err = getaddrinfo(url, "80", &hint, &adRoot)) == 0)
	{
		for(ad = adRoot; ad; ad = ad->ai_next)
		{
			//Create a socket
			if((ret = socket(ad->ai_family, ad->ai_socktype, ad->ai_protocol)) == -1)
			{
				//Error opening socket
				continue;
			}

			//Establish a connection
			if(connect(ret, ad->ai_addr, ad->ai_addrlen) == -1)
			{
				//Error connecting
				close(ret);
				continue;
			}

			//We made a connection, stop running
			break;
		}
		if(!ad)
		{
			ret = -1;
		}
		freeaddrinfo(adRoot);
	}
#if defined(BING_DEBUG)
	else
	{
		BING_MSG_PRINTOUT(gai_strerror(err));
	}
#endif

	return ret;
}

BOOL setupParser(bing_parser* parser, unsigned int bingID, const char* url)
{
	BOOL ret = FALSE;
	bing* bingI;

	memset(parser, 0, sizeof(bing_parser));

	parser->bing = bingID;
	parser->socket = openUrl(url);
	if(parser->socket != -1)
	{
#if defined(BING_DEBUG)
		bingI = retrieveBing(bingID);
		if(bingI)
		{
			pthread_mutex_lock(&bingI->mutex);

			parser->errorRet = bingI->errorRet;

			pthread_mutex_unlock(&bingI->mutex);

			ret = TRUE;
		}
#else
		ret = TRUE;
#endif
	}

	return ret;
}

//Search functions
bing_response_t search_sync(unsigned int bingID, const char* query, const bing_request_t request)
{
	const char* url;
	bing_parser* parser;
	bing_response_t ret = NULL;
	char buffer[256];
	ssize_t count;
	xmlParserCtxtPtr ctx;

	search_setup();

	//Get the url
	url = request_url(bingID, query, request);
	if(url)
	{
		//Create the parser
		parser = BING_MALLOC(sizeof(bing_parser));
		if(parser)
		{
			//Setup the parser
			if(setupParser(parser, bingID, url))
			{
				//Read the first 4 bytes of the xml (used to determine encoding)
				count = read(parser->socket, buffer, 4);
				if(count == 4)
				{
					//Create parser (we want a push parser so as we get data, we can push it to the parser)
					ctx = xmlCreatePushParserCtxt((xmlSAXHandlerPtr)&parserHandler, parser, buffer, 4, NULL);
					if(ctx)
					{
						//Read data
						while ((count = read(parser->socket, buffer, 256)) > 0)
						{
							xmlParseChunk(ctx, buffer, count, FALSE);
						}

						//Finish parsing
						xmlParseChunk(ctx, buffer, 0, TRUE);

						//We check for an error, if none exists then we get the response
						if(checkForError(parser))
						{
							ret = parser->response;
						}

						//Cleanup
						search_cleanup(ctx);
					}
					else
					{
						//Error, shutdown connection
						shutdown(parser->socket, SHUT_RDWR);
						close(parser->socket);
						BING_FREE(parser);
					}
				}
				else
				{
					//Error, shutdown connection
					shutdown(parser->socket, SHUT_RDWR);
					close(parser->socket);
					BING_FREE(parser);
				}
			}
			else
			{
				//Couldn't setup parser
				BING_FREE(parser);
			}
		}

		//Free URL
		BING_FREE((void*)url);
	}

	return ret;
}

int search_async(unsigned int bingID, const char* query, const bing_request_t request, void* user_data, receive_bing_response_func response_func)
{
	const char* url;
	bing_parser* parser;
	BOOL ret = FALSE;

	search_setup();

	//Get the url
	url = request_url(bingID, query, request);
	if(url)
	{
		//Create the parser
		parser = BING_MALLOC(sizeof(bing_parser));
		if(parser)
		{
			//Setup the parser
			if(setupParser(parser, bingID, url))
			{
				parser->responseFunc = response_func;
				//TODO: create thread, start thread
				//TODO: Within thread: read initial data, create context, read all data (check each time for error), get response and pass to response_func, search_cleanup
			}
			else
			{
				//Couldn't setup parser
				BING_FREE(parser);
			}
		}

		//Free URL
		BING_FREE((void*)url);
	}

	return ret;
}

//TODO: event invocation function

int search_event_async(unsigned int bingID, const char* query, const bing_request_t request)
{
	const char* url;
	bing_parser* parser;
	BOOL ret = FALSE;

	search_setup();

	//Get the url
	url = request_url(bingID, query, request);
	if(url)
	{
		//Create the parser
		parser = BING_MALLOC(sizeof(bing_parser));
		if(parser)
		{
			//Setup the parser
			if(setupParser(parser, bingID, url))
			{
				//TODO: Set response func to event invocation function
				//TODO: create thread, start thread
				//TODO: Within thread: read initial data, create context, read all data (check each time for error), get response and pass to event invocation function, search_cleanup
			}
			else
			{
				//Couldn't setup parser
				BING_FREE(parser);
			}
		}

		//Free URL
		BING_FREE((void*)url);
	}

	return ret;
}

//XXX: xmlCreatePushParserCtxt
