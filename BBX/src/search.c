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

		//Free result stack (we don't want to free the actual results)
		while(parser->lastResult)
		{
			st = parser->lastResult;
			parser->lastResult = st->prev;

			free(st); //Object
		}

		//Free the element
		while(parser->lastResultElement)
		{
			st = parser->lastResultElement;
			parser->lastResultElement = st->prev;

			free(st->value); //Name
			free(st); //Object
		}

		//Now free the response (no stacks will exist if a response doesn't exist. Allocated memory, internal responses, and all results are associated with the parent response. Freeing the response will free everything.)
		free_response(parser->current);

		//Finally, free any strings
		free((void*)parser->query);
		free((void*)parser->alteredQuery);
		free((void*)parser->alterationOverrideQuery);

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
	pstack* pt = malloc(sizeof(pstack));
	if(pt)
	{
		memset(pt, 0, sizeof(pstack));

		//Add to last result
		pt->prev = parser->lastResult;
		parser->lastResult = pt;
		pt->keepValue = TRUE;
		pt->value = result;

		pt = malloc(sizeof(pstack));
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
						//Wasn't created correctly, free (this isn't a parser error. The creator ran into an error [or something]).
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
					free((void*)parser->query);
					parser->query = malloc(size);
					hashtable_get_string(table, "SearchTerms", (char*)parser->query);
				}
				size = hashtable_get_string(table, "AlteredQuery", NULL);
				if(size > -1)
				{
					free((void*)parser->alteredQuery);
					parser->alteredQuery = malloc(size);
					hashtable_get_string(table, "AlteredQuery", (char*)parser->alteredQuery);
				}
				size = hashtable_get_string(table, "AlterationOverrideQuery", NULL);
				if(size > -1)
				{
					free((void*)parser->alterationOverrideQuery);
					parser->alterationOverrideQuery = malloc(size);
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

						data = malloc(sizeof(long long));
						result_get_64bit_int(result, RESULT_FIELD_CODE, (long long*)data);
						BING_MSG_PRINTOUT("Error Code = %lld\n", *((long long*)data));
						free(data);

						data = malloc(result_get_string(result, RESULT_FIELD_MESSAGE, NULL));
						result_get_string(result, RESULT_FIELD_MESSAGE, data);
						BING_MSG_PRINTOUT("Error Message = %s\n", data);
						free(data);

						data = malloc(result_get_string(result, RESULT_FIELD_PARAMETER, NULL));
						result_get_string(result, RESULT_FIELD_PARAMETER, data);
						BING_MSG_PRINTOUT("Error Parameter = %s\n", data);
						free(data);
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
							data = malloc(sizeof(int));
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
								//Try to free it from the internal result list
								if(!response_remove_result(((bing_result*)result)->parent, (bing_result*)result, TRUE, TRUE))
								{
									//Try to free it from the normal result list
									if(!response_remove_result(((bing_result*)result)->parent, (bing_result*)result, FALSE, TRUE))
									{
										//Just free it, no idea why it wasn't in either list
										free_result((bing_result*)result);
									}
								}
							}
							free(data);
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
						//TODO: Setup remaining values

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
							//This wasn't a bundled response, just free it (otherwise it will never get freed)
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

void search_cleanup(xmlParserCtxtPtr ctx)
{
	//TODO
	searchCount--;
	if(searchCount)
	{
		xmlCleanupParser();
	}
}

//Search functions
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
