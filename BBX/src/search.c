/*
 * search.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#include <stdbool.h>
#include <bps/event.h>
#include <bps/netstatus.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <curl/curl.h>
#include <curl/easy.h>

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
	CURL* curl;
	pthread_t thread;
	xmlParserCtxtPtr ctx; //Reciprocal pointer so we can pass the parser to get all the info and still get the context that the parser is contained in
	receive_bing_response_func responseFunc;
	void* userData;
	int bpsChannel;
	BOOL parseError;
#if defined(BING_DEBUG)
	BOOL errorRet;
#endif
} bing_parser;

//Processors
void errorCallback(void *ctx, const char *msg, ...)
{
	bing_parser* parser = (bing_parser*)ctx;
	parser->parseError = TRUE; //We simply mark this as error because on completion we can check this and it will automatically handle all cleanup and we can get if the search completed or not
}

void serrorCallback(void* userData, xmlErrorPtr error)
{
	bing_parser* parser = (bing_parser*)userData;
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

void cleanStacks(bing_parser* parser)
{
	pstack* st;

	//Free result stack (we don't want to free the actual results)
	while(parser->lastResult)
	{
		st = parser->lastResult;
		parser->lastResult = st->prev;

		bing_free(st); //Object
	}

	//Free the element
	while(parser->lastResultElement)
	{
		st = parser->lastResultElement;
		parser->lastResultElement = st->prev;

		bing_free(st->value); //Name
		bing_free(st); //Object
	}
}

BOOL checkForError(bing_parser* parser)
{
	if(parser->parseError)
	{
		//Error, cleanup everything

		//First clean the stacks
		cleanStacks(parser);

		//Now free the response (no stacks will exist if a response doesn't exist. Allocated memory, internal responses, and all results are associated with the parent response. Freeing the response will free everything.)
		free_response(parser->current);

		//Finally, free any strings
		bing_free((void*)parser->query);
		bing_free((void*)parser->alteredQuery);
		bing_free((void*)parser->alterationOverrideQuery);

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

hashtable_t* createHashtableFromAtts(int att_count, const xmlChar** atts)
{
	hashtable_t* table = NULL;
	int i;
	const char* localName;
	const char* value;
	const char* end;
	char* tmp;
	size_t size;
	if(atts && att_count > 0)
	{
		//We don't need a element for each string, we just need it for the data itself
		table = hashtable_create(att_count);
		if(table)
		{
			//Fill table (attributes are in quintuplets: localname/prefix/URI/value/end)
			for(i = 0; i < att_count; i++)
			{
				//Get the attribute components
				localName = (char*)atts[i * 5];		//The name of the attribute
				value = (char*)atts[(i * 5) + 3];	//Pointer to the attribute value (start)
				end = (char*)atts[(i * 5) + 4];		//Pointer to the attribute value (end)

				//Get the value ONLY (we need to do this because if we just copy the data, we will get a non-NULL terminated string and if we add one extra char, we will still get a non-NULL terminated string.)
				tmp = bing_malloc(size = (end - value) + 1);
				if(tmp)
				{
					strlcpy(tmp, value, size);

					hashtable_put_item(table, localName, tmp, size);

					bing_free(tmp);
				}
			}
		}
	}
	return table;
}

void addResultToStack(bing_parser* parser, bing_result* result, const char* name, BOOL internal)
{
	pstack* pt = bing_malloc(sizeof(pstack));
	if(pt)
	{
		memset(pt, 0, sizeof(pstack));

		//Add to last result
		pt->prev = parser->lastResult;
		parser->lastResult = pt;
		pt->keepValue = TRUE;
		pt->value = result;

		pt = bing_malloc(sizeof(pstack));
		if(pt)
		{
			//Add to last result element
			pt->prev = parser->lastResultElement;
			parser->lastResultElement = pt;
			pt->keepValue = TRUE;
			pt->value = bing_strdup(name);
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

const char* getQualifiedName(const xmlChar* localname, const xmlChar* prefix)
{
	size_t size;
	char* qualifiedName;
	if(prefix)
	{
		qualifiedName = bing_malloc(size = strlen((char*)localname) + strlen((char*)prefix) + 2);
		if(qualifiedName)
		{
			snprintf(qualifiedName, size, "%s:%s", prefix, localname);
			qualifiedName[size] = '\0';
		}
	}
	else
	{
		//There is nothing that requires changing
		qualifiedName = bing_strdup((char*)localname);
	}
	return qualifiedName;
}

//The real "meat and potatoes" of the parser. Many if blocks for safety and to increase flexibility (and hopefully readability...)

void startElementNs(void* ctx, const xmlChar* localname, const xmlChar* prefix, const xmlChar* URI, int nb_namespaces, const xmlChar** namespaces, int nb_attributes, int nb_defaulted, const xmlChar** attributes)
{
	bing_result_t result = NULL;
	hashtable_t* table;
	int size;
	char* data;
	bing_parser* parser = (bing_parser*)ctx;
	const char* qualifiedName;

	if(checkForError(parser) &&
			strcmp((char*)localname, "SearchResponse") != 0) //We don't want the document element
	{
		//We need to create the qualified name (as that is what we use for type checks)
		qualifiedName = getQualifiedName(localname, prefix);

		if(qualifiedName) //We need a qualified name
		{
			if(!endsWith(qualifiedName, ":Results")) //We don't want result arrays (we don't want to combine this with the qualified name check so that we don't unfree the qualified name if needed)
			{
				//It's a result type
				if(endsWith(qualifiedName, "Result"))
				{
					if(parser->current)
					{
						if(!result_is_common(qualifiedName) && //First check if the result is "common" which excludes it from this usage (especially since custom results could end in "Result" and be common)
								result_create_raw(qualifiedName, &result, parser->current)) //Try to actually create the result (and adds it to the response)
						{
							//Create the dictionary (if this fails then, o well. Maybe no hashtable is returned because there where no attributes).
							table = createHashtableFromAtts(nb_attributes, attributes);

							//Result created successfully, time to save state
							if(((bing_result*)result)->creation(qualifiedName, result, (data_dictionary_t)table))
							{
								addResultToStack(parser, (bing_result*)result, qualifiedName, RESULT_CREATE_DEFAULT_INTERNAL);
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
				else if(strcmp(qualifiedName, "Query") == 0) //It's the query info
				{
					table = createHashtableFromAtts(nb_attributes, attributes);

					if(table)
					{
						size = hashtable_get_string(table, "SearchTerms", NULL);
						if(size > -1)
						{
							bing_free((void*)parser->query);
							parser->query = bing_malloc(size);
							hashtable_get_string(table, "SearchTerms", (char*)parser->query);
						}
						size = hashtable_get_string(table, "AlteredQuery", NULL);
						if(size > -1)
						{
							bing_free((void*)parser->alteredQuery);
							parser->alteredQuery = bing_malloc(size);
							hashtable_get_string(table, "AlteredQuery", (char*)parser->alteredQuery);
						}
						size = hashtable_get_string(table, "AlterationOverrideQuery", NULL);
						if(size > -1)
						{
							bing_free((void*)parser->alterationOverrideQuery);
							parser->alterationOverrideQuery = bing_malloc(size);
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
				else if(strcmp(qualifiedName, "Error") == 0) //Oops, error
				{
					table = createHashtableFromAtts(nb_attributes, attributes);

					//Can we create an actual error result?
					if(table && parser->current && parser->errorRet)
					{
						if(result_create_raw("Error", &result, parser->current))
						{
							//Run the creation function
							if(((bing_result*)result)->creation("Error", result, (data_dictionary_t)table))
							{
								//Print out the results (do them one at a time to prevent memory leaks that can occur if realloc is used without a backup pointer)b

								data = bing_malloc(sizeof(long long));
								result_get_64bit_int(result, RESULT_FIELD_CODE, (long long*)data);
								BING_MSG_PRINTOUT("Error Code = %lld\n", *((long long*)data));
								bing_free(data);

								data = bing_malloc(result_get_string(result, RESULT_FIELD_MESSAGE, NULL));
								result_get_string(result, RESULT_FIELD_MESSAGE, data);
								BING_MSG_PRINTOUT("Error Message = %s\n", data);
								bing_free(data);

								data = bing_malloc(result_get_string(result, RESULT_FIELD_PARAMETER, NULL));
								result_get_string(result, RESULT_FIELD_PARAMETER, data);
								BING_MSG_PRINTOUT("Error Parameter = %s\n", data);
								bing_free(data);
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
					if(result_is_common(qualifiedName)) //Is this a common data type (as opposed to a response)
					{
						table = createHashtableFromAtts(nb_attributes, attributes);

						if(result_create_raw(qualifiedName, &result, parser->current))
						{
							//Run the creation function
							if(((bing_result*)result)->creation(qualifiedName, result, (data_dictionary_t)table))
							{
								//Make the result a internal result (if this doesn't work, then it means that it's already internal)
								response_swap_result(parser->current, result, RESULT_CREATE_DEFAULT_INTERNAL);

								//If an array, we want to store it so we can get internal values
								if(((bing_result*)result)->array)
								{
									addResultToStack(parser, (bing_result*)result, qualifiedName, TRUE);

									//Determine where it should be added to

									//We can let endElement handle saving it
									if(parser->lastResult)
									{
										if(parser->lastResult->prev)
										{
											parser->lastResult->addToResult = (bing_result*)parser->lastResult->prev->value;
										}
										else
										{
											parser->lastResult->addToResponse = parser->current;
										}
									}
								}
								else
								{
									data = bing_malloc(sizeof(int));
									if(data)
									{
										*((int*)data) = FALSE;

										//Determine where it should be added to
										if(parser->lastResult)
										{
											//This is getting added to a result
											((bing_result*)parser->lastResult->value)->additionalResult(qualifiedName, (bing_result_t)parser->lastResult->value, result, ((int*)data));
										}
										else
										{
											hashtable_free(table);

											//This is getting added to a response
											table = hashtable_create(1);
											if(table)
											{
												if(hashtable_put_item(table, strchr(qualifiedName, ':') + 1, &result, sizeof(bing_result_t)) != -1)
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
									bing_free(data);
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
					else //It's a response
					{
						parser->current = NULL;
						if(response_create_raw(qualifiedName, (bing_response_t*)&parser->current, parser->bing,
								(parser->response != NULL && parser->response->type == BING_SOURCETYPE_BUNDLE) ? parser->response : NULL)) //The general idea is that if there is already a response and it is bundle, it will be the parent. Otherwise add it to Bing
						{
							table = createHashtableFromAtts(nb_attributes, attributes);

							if(response_def_create_standard_responses(parser->current, (data_dictionary_t)table) &&
									parser->current->creation(qualifiedName, (bing_response_t)parser->current, (data_dictionary_t)table))
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

			bing_free((void*)qualifiedName);
		}
		else
		{
			//No qualified name, error
			parser->parseError = TRUE;
		}
	}
}

void endElementNs(void* ctx, const xmlChar* localname, const xmlChar* prefix, const xmlChar* URI)
{
	pstack* st;
	bing_parser* parser = (bing_parser*)ctx;
	hashtable_t* table;
	const char* qualifiedName;

	if(checkForError(parser) && parser->lastResultElement)
	{
		//We need to create the qualified name (as that is what we use for type checks)
		qualifiedName = getQualifiedName(localname, prefix);

		if(qualifiedName) //We need a qualified name
		{
			if(!endsWith(qualifiedName, ":Results") && //We don't want result arrays
					strcmp((char*)parser->lastResultElement->value, qualifiedName) == 0)
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
							if(hashtable_put_item(table, strchr(qualifiedName, ':') + 1, &st->value, sizeof(bing_result_t)) != -1)
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
				bing_free(st); //Object

				//Free the element
				st = parser->lastResultElement;
				parser->lastResultElement = st->prev;

				bing_free(st->value); //Name
				bing_free(st); //Object
			}

			bing_free((void*)qualifiedName);
		}
		else
		{
			//No qualified name, error
			parser->parseError = TRUE;
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
		NULL,			//startElement
		NULL,			//endElement
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
		XML_SAX2_MAGIC,	//initialized
		NULL,			//_private
		startElementNs,	//startElementNs
		endElementNs,	//endElementNs
		serrorCallback	//serror
};

void search_setup()
{
	LIBXML_TEST_VERSION

	if(atomic_add_value(&searchCount, 1) == 0)
	{
		//Setup XML
		xmlGcMemSetup(bing_free, bing_malloc, bing_malloc, bing_realloc, bing_strdup);

		//On first run, setup the parser
		xmlInitParser();

		//Setup cURL
		curl_global_init_mem(CURL_GLOBAL_ALL, bing_malloc, bing_free, bing_realloc, bing_strdup, bing_calloc); //THIS IS NOT THREAD SAFE!!
	}
}

void search_cleanup(bing_parser* parser)
{
	xmlParserCtxtPtr ctx;
	if(parser)
	{
		ctx = parser->ctx;
		ctx->userData = NULL;

		//Shutdown cURL
		curl_easy_cleanup(parser->curl);

		//Now get rid of the bing context
		parser->bing = 0;

		//Close thread (if used)
		parser->thread = NULL; //pthread_exit (called implicitly at end of execution) frees the thread

		//Cleanup stacks
		cleanStacks(parser);

		//Cleanup strings
		bing_free((void*)parser->query);
		bing_free((void*)parser->alteredQuery);
		bing_free((void*)parser->alterationOverrideQuery);

		//Free the bing parser
		bing_free(parser);

		//Free the document
		xmlFreeDoc(ctx->myDoc);

		//Free the actual context
		xmlFreeParserCtxt(ctx);
	}
#if defined(BING_DEBUG)
	else
	{
		BING_MSG_PRINTOUT("Parser cleanup-parser is NULL\n");
	}
#endif

	//Not desired to do this if parser is NULL (as the call shouldn't have happened with a NULL parser), but it's still a cleanup operation
	if(atomic_sub_value(&searchCount, 1) == 1)
	{
		xmlCleanupParser();

		//Cleanup cURL
		curl_global_cleanup(); //THIS IS NOT THREAD SAFE!!
	}
}

size_t getxmldata(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	bing_parser* parser = (bing_parser*)userdata;
	size_t atcsize = size * nmemb;

	//Check if we have a parser, otherwise we need to create one
	if(parser->ctx)
	{
		//Only write data if no error has occurred
		if(!parser->parseError)
		{
			xmlParseChunk(parser->ctx, ptr, atcsize, FALSE);
		}
	}
	else
	{
		//Create parser
		parser->ctx = xmlCreatePushParserCtxt((xmlSAXHandlerPtr)&parserHandler, parser, ptr, atcsize, NULL);
		parser->parseError = !parser->ctx; //If an error occurs, it will ignore writing any data
	}

	//If an error occurred, we want to let cURL know there was an error
	if(parser->parseError)
	{
		atcsize = 0;
	}

	return atcsize;
}

CURL* setupCurl(const char* url, bing_parser* parser)
{
	CURL* ret = curl_easy_init();

	if(ret)
	{
		//Set the URL
		if(curl_easy_setopt(ret, CURLOPT_URL, url) == CURLE_OK && //Check this as it is one of three important options, the rest are optional
				curl_easy_setopt(ret, CURLOPT_WRITEFUNCTION, getxmldata) == CURLE_OK &&
				curl_easy_setopt(ret, CURLOPT_WRITEDATA, (void*)parser) == CURLE_OK)
		{
			//We don't want any progress meters
			curl_easy_setopt(ret, CURLOPT_NOPROGRESS, 1L);
		}
		else
		{
			curl_easy_cleanup(ret);
			ret = NULL;
		}
	}

	return ret;
}

BOOL setupParser(bing_parser* parser, unsigned int bingID, const char* url)
{
	BOOL ret = FALSE;
#if defined(BING_DEBUG)
	bing* bingI;
#endif

	memset(parser, 0, sizeof(bing_parser));

	parser->bing = bingID;
	parser->curl = setupCurl(url, parser);
	if(parser->curl)
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

BOOL check_for_connection()
{
	bool av;
	if(netstatus_get_availability(&av) == BPS_SUCCESS)
	{
		return CPP_BOOL_TO_BOOL(av);
	}
	return FALSE; //Return false as a precaution because if we can't get netstatus, well, we might not be able to use the network in general.
}

//Search functions
bing_response_t search_sync(unsigned int bingID, const char* query, const bing_request_t request)
{
	const char* url;
	bing_parser* parser;
	bing_response_t ret = NULL;
#if defined(BING_DEBUG)
	int curlCode;
#endif

	if(check_for_connection())
	{
		search_setup();

		//Get the url
		url = request_url(bingID, query, request);
		if(url)
		{
			//Create the parser
			parser = bing_malloc(sizeof(bing_parser));
			if(parser)
			{
				//Setup the parser
				if(setupParser(parser, bingID, url))
				{
					if(check_for_connection())
					{
						//Invoke cURL
						if((
#if defined(BING_DEBUG)
								curlCode =
#endif
										curl_easy_perform(parser->curl)) == CURLE_OK)
						{
							//No errors

							//Finish parsing
							xmlParseChunk(parser->ctx, NULL, 0, TRUE);

							//We check for an error, if none exists then we get the response
							if(checkForError(parser))
							{
								ret = parser->response;
							}
#if defined(BING_DEBUG)
							else if(parser->errorRet)
							{
								BING_MSG_PRINTOUT("Parser error\n");
							}
	#endif
						}
#if defined(BING_DEBUG)
						else if(parser->errorRet)
						{
							BING_MSG_PRINTOUT("Error invoking cURL: %s\n", curl_easy_strerror(curlCode));
						}
#endif
					}

					//Cleanup
					search_cleanup(parser);
				}
				else
				{
#if defined(BING_DEBUG)
					if(get_error_return(bingID))
					{
						BING_MSG_PRINTOUT("Could not setup parser\n");
					}
#endif
					//Couldn't setup parser
					bing_free(parser);
				}
			}
#if defined(BING_DEBUG)
			else if(get_error_return(bingID))
			{
				BING_MSG_PRINTOUT("Could not parser\n");
			}
#endif

			//Free URL
			bing_free((void*)url);
		}
#if defined(BING_DEBUG)
		else if(get_error_return(bingID))
		{
			BING_MSG_PRINTOUT("Could not create URL\n");
		}
#endif
	}

	return ret;
}

void* async_search(void* ctx)
{
#if defined(BING_DEBUG)
	int curlCode;
#endif
	receive_bing_response_func responseFunc = NULL;
	bing_response* response;
	void* userData;
	bing_parser* parser = (bing_parser*)ctx;

	if(check_for_connection())
	{
		//Invoke cURL
		if((
#if defined(BING_DEBUG)
				curlCode =
#endif
						curl_easy_perform(parser->curl)) == CURLE_OK)
		{
			//No errors

			//Finish parsing
			xmlParseChunk(parser->ctx, NULL, 0, TRUE);

			//We check for an error
#if defined(BING_DEBUG)
			if(
#endif
				checkForError(parser)
#if defined(BING_DEBUG)
			)
			{
				BING_MSG_PRINTOUT("ASYNC: Parser error\n");
			}
#else
			;
#endif

			//Get response (do this so we can free before any error might occur)
			responseFunc = parser->responseFunc;
			response = parser->response;
			userData = parser->userData;
	}
#if defined(BING_DEBUG)
		else if(parser->errorRet)
		{
			BING_MSG_PRINTOUT("ASYNC: Error invoking cURL: %s\n", curl_easy_strerror(curlCode));
		}
#endif
	}

	//Cleanup
	search_cleanup(parser);

	//Return response (NULL is fine for a response)
	if(responseFunc)
	{
		responseFunc(response, userData);
	}

	return NULL;
}

//We need to free the event because the event could be seen by multiple applications and we don't want them all freeing it
void event_done(bps_event_t *event)
{
	bps_event_payload_t* payload = bps_event_get_payload(event);
	free_response((bing_response_t)payload->data1);
}

void event_invocation(bing_response_t response, void* user_data)
{
	bps_event_t* event = NULL;
	bps_event_payload_t payload;
	bing_parser* parser = (bing_parser*)user_data;
	if(response) //We only want to push an event if we have something to push.
	{
		memset(&payload, 0, sizeof(bps_event_payload_t));
		payload.data1 = (uintptr_t)response;

		//Create the event
		if((bps_event_create(&event, bing_get_domain(), 0, &payload, event_done) | //Create event
				(parser->bpsChannel >= 0 ? bps_channel_push_event(parser->bpsChannel, event) : bps_push_event(event))) != BPS_SUCCESS) //Push event (if we have a BPS channel, use it)
		{
			//Since the event will never be pushed, free it
			if(event)
			{
				bps_event_destroy(event);
			}

			//Since response will never be pushed, free it
			free_response(response);
		}
	}
}

int search_async_in(unsigned int bingID, const char* query, const bing_request_t request, void* user_data, BOOL user_data_is_parser, receive_bing_response_func response_func)
{
	const char* url;
	bing_parser* parser;
	pthread_attr_t thread_atts;
	BOOL ret = FALSE;

	if(check_for_connection())
	{
		search_setup();

		//Get the url
		url = request_url(bingID, query, request);
		if(url)
		{
			//Create the parser
			parser = bing_malloc(sizeof(bing_parser));
			if(parser)
			{
				//Setup the parser
				if(setupParser(parser, bingID, url))
				{
					//Setup callback functions
					parser->responseFunc = response_func;
					parser->userData = user_data_is_parser ? parser : user_data;
					parser->bpsChannel = user_data_is_parser ? bps_channel_get_active() : -1;

					//Setup thread attributes
					pthread_attr_init(&thread_atts);
					pthread_attr_setdetachstate(&thread_atts, PTHREAD_CREATE_DETACHED); //XXX This is how we want the thread to act, but we should do some extra checks as it could go wrong if the calling application exits before the thread returns. But if we leave it as joinable, the only thing we know that will happen is the application will hang as pthread_join would have to be called in search_shutdown, which is running on the same thread.

					//Create thread
					ret = pthread_create(&parser->thread, &thread_atts, async_search, parser) == EOK;

					//Cleanup attributes
					pthread_attr_destroy(&thread_atts);
				}
				else
				{
#if defined(BING_DEBUG)
					if(get_error_return(bingID))
					{
						BING_MSG_PRINTOUT("Could not setup parser\n");
					}
#endif
					//Couldn't setup parser
					bing_free(parser);
				}
			}
#if defined(BING_DEBUG)
			else if(get_error_return(bingID))
			{
				BING_MSG_PRINTOUT("Could not parser\n");
			}
#endif

			//Free URL
			bing_free((void*)url);
		}
#if defined(BING_DEBUG)
		else if(get_error_return(bingID))
		{
			BING_MSG_PRINTOUT("Could not create URL\n");
		}
#endif
	}

	return ret;
}

int search_async(unsigned int bingID, const char* query, const bing_request_t request, void* user_data, receive_bing_response_func response_func)
{
	return search_async_in(bingID, query, request, user_data, FALSE, response_func);
}

int search_event_async(unsigned int bingID, const char* query, const bing_request_t request)
{
	return search_async_in(bingID, query, request, NULL, TRUE, event_invocation);
}
