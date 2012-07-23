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
#include <libxml/SAX2.h>

#include <curl/curl.h>

//Defines for parsing
#define DEFAULT_HASHTABLE_SIZE 8
#define PARSE_PROPERTY_TYPE "type"
#define PARSE_PROPERTY_MTYPE "m:type"

#define PARSE_KEY_TITLE "title"
#define PARSE_KEY_SUBTITLE "subtitle"

#define PARSE_THIS_LINK "#thisLink"

//Defines for cURL
#define CURL_TRUE 1L
#define CURL_FALSE 0L
#define CURL_EMPTY_STRING ""

//XXX New error codes once updated
enum PARSER_ERROR
{
	PE_NO_ERROR,

	//Error callbacks
	PE_ERROR_CALLBACK,
	PE_SERROR_CALLBACK,

	//addResultToStack
	PE_ADD_RESULT_STACK_STRDUB_FAIL,
	PE_ADD_RESULT_STACK_NAME_PSTACK_NOALLOC,
	PE_ADD_RESULT_STACK_RESULT_PSTACK_NOALLOC,

	//startElementNs
	PE_START_ELE_QUERY_HASHTABLE_CREATE_FAIL,
	PE_START_ELE_RESPONSE_BUNDLE_CREATE_FAIL,
	PE_START_ELE_RESPONSE_CREATION_CALLBACK_FAIL,
	PE_START_ELE_RESPONSE_CREATE_FAIL,
	PE_START_ELE_NO_QNAME,

	//endElementNs
	PE_END_ELE_NO_QNAME,

	//getxmldata
	PE_GETXMLDATA_CTX_CREATE_FAIL
};

typedef struct PARSER_STACK_S
{
	void* value;
	struct PARSER_STACK_S* prev;
} pstack;

typedef struct BING_PARSER_S
{
	//Freed on error
	bing_response* response;
	bing_response* current;

	//State info
	unsigned int bing;
	CURL* curl;
	pthread_t thread;
	xmlParserCtxtPtr ctx; //Reciprocal pointer so we can pass the parser to get all the info and still get the context that the parser is contained in
	receive_bing_response_func responseFunc;
	const void* userData;
	int bpsChannel;
	enum PARSER_ERROR parseError;
#if defined(BING_DEBUG)
	BOOL errorRet;
#endif
} bing_parser;

//Helper functions to handle namespaces
BOOL nsXmlHasProp(xmlNodePtr node, const char* name)
{
	const char* xmlName = strchr(name, ':');
	char* tname;
	BOOL ret = FALSE;
	if(xmlName)
	{
		xmlName = bing_mem_strdup(name);

		tname = strchr(xmlName, ':');
		*(tname++) = '\0';

		//Has namespace
		ret = xmlHasNsProp(node, (xmlChar*)tname, (xmlChar*)xmlName) != NULL;

		bing_mem_free((void*)xmlName);
	}
	else
	{
		//No namespace, handle like normal
		ret = xmlHasNsProp(node, (xmlChar*)name, NULL) != NULL;
	}
	return ret;
}

const xmlChar* nsXmlGetProp(xmlNodePtr node, const char* name)
{
	const char* xmlName = strchr(name, ':');
	char* tname;
	xmlChar* ret = NULL;
	if(xmlName)
	{
		xmlName = bing_mem_strdup(name);

		tname = strchr(xmlName, ':');
		*(tname++) = '\0';

		//Has namespace
		ret = xmlGetNsProp(node, (xmlChar*)tname, (xmlChar*)xmlName);

		bing_mem_free((void*)xmlName);
	}
	else
	{
		//No namespace, handle like normal
		ret = xmlGetNsProp(node, (xmlChar*)name, NULL);
	}
	return ret;
}

const char* xmlGetQualifiedName(xmlNodePtr node)
{
	size_t size;
	char* qualifiedName;
	if(node->ns && node->ns->prefix)
	{
		//A prefix exists, produce the qualified name
		qualifiedName = bing_mem_malloc(size = strlen((char*)node->name) + strlen((char*)node->ns->prefix) + 2);
		if(qualifiedName)
		{
			snprintf(qualifiedName, size, "%s:%s", node->ns->prefix, node->name);
			qualifiedName[size - 1] = '\0';
		}
	}
	else
	{
		//There is nothing that requires changing
		qualifiedName = bing_mem_strdup((char*)node->name);
	}
	return qualifiedName;
}

//Parse functions
bing_result* parseResult(xmlNodePtr resultNode, BOOL type, bing_response* parent, xmlFreeFunc xmlFree)
{
	//Not really the greatest names, could probably change
	bing_result* res = NULL;
	bing_result* tres;
	xmlNodePtr node;
	const xmlChar* xmlText;
	char* text;
	const char* nodeName;
	pstack* additionalProcessing = NULL;
	pstack* tStack;
	hashtable_t* data = hashtable_create(DEFAULT_HASHTABLE_SIZE);
	int size;
	BOOL keep;

	//Get data
	if(!type)
	{
		//Go through all the nodes to get data
		for(node = resultNode->children; node != NULL; node = node->next)
		{
			nodeName = xmlGetQualifiedName(node);
			if(nodeName)
			{
				//We want to stop on content, we process that later
				if(strcmp(nodeName, "content") == 0)
				{
					break;
				}

				//If we have a node with a type property, it makes it easy for us
				if(nsXmlHasProp(node, PARSE_PROPERTY_TYPE))
				{
					xmlText = nsXmlGetProp(node, PARSE_PROPERTY_TYPE);
					if(xmlText)
					{
						//Parse the data
						if(parseToHashtableByType((char*)xmlText, node, data, xmlFree))
						{
							//Check to see if this is a composite response
							if(strcmp(nodeName, PARSE_KEY_TITLE) == 0)
							{
								size = hashtable_get_string(data, PARSE_KEY_TITLE, NULL);
								if(size > 0)
								{
									text = bing_mem_malloc(size);
									if(text)
									{
										hashtable_get_string(data, PARSE_KEY_TITLE, text);
										text[size - 1] = '\0';
										if(strcmp(text, "ExpandableSearchResult") == 0)
										{
											//This is a composite response
											hashtable_free(data);
											return NULL;
										}
										bing_mem_free(text);
									}
									else
									{
										//XXX Error
									}
								}
							}
						}
						else
						{
							//XXX Error
						}
						xmlFree((void*)xmlText);
					}
					else
					{
						//XXX Error
					}
				}
				else
				{
					if(nsXmlHasProp(node, PARSE_PROPERTY_MTYPE))
					{
						xmlText = nsXmlGetProp(node, PARSE_PROPERTY_MTYPE);
						if(xmlText)
						{
							if(!parseToHashtableByType((char*)xmlText, node, data, xmlFree))
							{
								//XXX Error
							}
							xmlFree((void*)xmlText);
						}
						else
						{
							//XXX Error
						}
					}
					else if(strcmp(nodeName, "link") == 0)
					{
						xmlText = nsXmlGetProp(node, "rel");
						if(xmlText)
						{
							if(strcmp((char*)xmlText, "next") == 0)
							{
								xmlFree((void*)xmlText);

								xmlText = nsXmlGetProp(node, "href");
								//XXX Special assignment
								if(!hashtable_put_item(data, PARSE_NEXT_LINK, xmlText, strlen((char*)xmlText) + 1))
								{
									//XXX Error
								}
							}
							else if(strcmp((char*)xmlText, "self") == 0)
							{
								xmlFree((void*)xmlText);

								xmlText = nsXmlGetProp(node, "href");
								//XXX Special assignment
								if(!hashtable_put_item(data, PARSE_THIS_LINK, xmlText, strlen((char*)xmlText) + 1))
								{
									//XXX Error
								}
							}
							else
							{
								//TODO (never encountered, unsure if there is something else)
							}
							xmlFree((void*)xmlText);
						}
						else
						{
							//XXX Error
						}
					}
					else
					{
						parseToHashtableByName(node, data, xmlFree);
					}
				}

				bing_mem_free((void*)nodeName);
			}
			else
			{
				//XXX Error
			}
		}

		//If this is an empty result, ignore it
		if (!node)
		{
			hashtable_free(data);
			return NULL;
		}

		//If content is not the expected type, ignore it.
		if(nsXmlHasProp(node, PARSE_PROPERTY_TYPE))
		{
			xmlText = nsXmlGetProp(node, PARSE_PROPERTY_TYPE);
			if(xmlText)
			{
				if(strcmp((char*)xmlText, "application/xml") != 0)
				{
					xmlFree((void*)xmlText); //Would rather only have this once in the if block, but then I have to use gotos
					hashtable_free(data);
					return NULL;
				}
				xmlFree((void*)xmlText);
			}
		}

		//Prepare for parse content
		resultNode = node->children;
	}

	//Parse content
	for(node = resultNode->children; node != NULL; node = node->next)
	{
		//Determine if we have a node with a type (pointers will be embedded in lib so no need to free them)
		text = PARSE_PROPERTY_TYPE;
		if(!nsXmlHasProp(node, text))
		{
			text = PARSE_PROPERTY_MTYPE;
			if(!nsXmlHasProp(node, text))
			{
				text = NULL;
			}
		}

		//Process the node
		if(text)
		{
			//... as stated before, pointers will be embedded in lib so no need to free them.
			xmlText = nsXmlGetProp(node, text);
			if(xmlText)
			{
				if(isComplex((char*)xmlText))
				{
					//Push a new value onto the stack (order doesn't matter)
					tStack = bing_mem_malloc(sizeof(pstack));
					if(!tStack)
					{
						//XXX Error
					}
					tStack->prev = additionalProcessing;
					tStack->value = node;
					additionalProcessing = tStack;
				}
				else
				{
					if(!parseToHashtableByType((char*)xmlText, node, data, xmlFree))
					{
						//XXX Error
					}
				}
				xmlFree((void*)xmlText);
			}
		}
		else
		{
			if(!parseToHashtableByName(node, data, xmlFree))
			{
				//XXX Error
			}
		}
	}

	//Create (this will also retrieve the name used by both the creation function and the the creation callbacks)
	if(type)
	{
		xmlText = nsXmlGetProp(resultNode, PARSE_PROPERTY_MTYPE);
		if(xmlText)
		{
			if(result_create_raw((char*)xmlText, (bing_result_t*)&res, parent))
			{
				//Make this an internal result
				response_swap_result(parent, res, RESULT_CREATE_DEFAULT_INTERNAL);

				//Run creation callback
				if(!res->creation((char*)xmlText, res, (data_dictionary_t)data))
				{
					//XXX Error
				}
			}
			else
			{
				//XXX Error
			}
			xmlFree((void*)xmlText);
		}
		else
		{
			//XXX Error
		}
	}
	else
	{
		size = hashtable_get_item(data, PARSE_KEY_TITLE, NULL);
		if(size > 0)
		{
			text = bing_mem_malloc(size);
			if(text)
			{
				hashtable_get_item(data, PARSE_KEY_TITLE, text);
				if(result_create_raw(text, (bing_result_t*)&res, parent))
				{
					if(!res->creation(text, res, (data_dictionary_t)data))
					{
						//XXX Error
					}
				}
				else
				{
					//XXX Error
				}
				bing_mem_free((void*)text);
			}
		}
	}

	//Additional content processing
	while(additionalProcessing)
	{
		//TODO Check for error, don't execute processing on error (let loop continue)
		//Only run if there is a result to run on (we still do the loop so we can free the stack)
		if(res)
		{
			node = (xmlNodePtr)additionalProcessing->value;
			tres = parseResult(node, TRUE, parent, xmlFree);
			if(tres)
			{
				keep = FALSE;
				nodeName = xmlGetQualifiedName(node);
				if(nodeName)
				{
					res->additionalResult(nodeName, res, tres, &keep);
					bing_mem_free((void*)nodeName);
				}
				else
				{
					//XXX Error
				}
				if(!keep)
				{
					//The result shouldn't be kept
					if(type)
					{
						//Free from internal
						if(response_remove_result(parent, tres, TRUE, TRUE))
						{
							tres = NULL;
						}
					}
					else
					{
						//Free from public
						if(response_remove_result(parent, tres, FALSE, TRUE))
						{
							tres = NULL;
						}
					}
					if(tres)
					{
						//Couldn't remove the response, just free it
						free_result(tres);
					}
				}
			}
			else
			{
				//TODO (never encountered, unsure if this would be an error or not)
			}
		}

		//Move to next stack element
		tStack = additionalProcessing;
		additionalProcessing = additionalProcessing->prev;

		//Free stack element
		bing_mem_free((void*)tStack);
	}

	//Cleanup table
	hashtable_free(data);

	return res;
}

bing_response* parseResponse(xmlNodePtr responseNode, BOOL composite, bing_parser* parser, xmlFreeFunc xmlFree)
{
	//Not really the greatest names, could probably change
	bing_response* tmp;
	xmlNodePtr node;
	xmlNodePtr node2;
	const xmlChar* xmlText;
	char* text;
	const char* nodeName;
	hashtable_t* data = hashtable_create(DEFAULT_HASHTABLE_SIZE);
	size_t size;
	BOOL subResComp;

	//Get general data
	for(node = responseNode->children; node != NULL; node = node->next)
	{
		nodeName = xmlGetQualifiedName(node);
		if(nodeName)
		{
			//We want to stop on content, we process that later
			if(strcmp(nodeName, "entry") == 0)
			{
				break;
			}

			//If we have a node with a type property, it makes it easy for us
			if(nsXmlHasProp(node, PARSE_PROPERTY_TYPE))
			{
				xmlText = nsXmlGetProp(node, PARSE_PROPERTY_TYPE);
				if(xmlText)
				{
					//Parse the data
					if(!parseToHashtableByType((char*)xmlText, node, data, xmlFree))
					{
						//XXX Error
					}
					xmlFree((void*)xmlText);
				}
				else
				{
					//XXX Error
				}
			}
			else
			{
				if(strcmp(nodeName, "link") == 0)
				{
					xmlText = nsXmlGetProp(node, "rel");
					if(xmlText)
					{
						if(strcmp((char*)xmlText, "next") == 0)
						{
							xmlFree((void*)xmlText);

							xmlText = nsXmlGetProp(node, "href");
							//XXX Special assignment
							if(!hashtable_put_item(data, PARSE_NEXT_LINK, xmlText, strlen((char*)xmlText) + 1))
							{
								//XXX Error
							}
						}
						else if(strcmp((char*)xmlText, "self") == 0)
						{
							xmlFree((void*)xmlText);

							xmlText = nsXmlGetProp(node, "href");
							//XXX Special assignment
							if(!hashtable_put_item(data, PARSE_THIS_LINK, xmlText, strlen((char*)xmlText) + 1))
							{
								//XXX Error
							}
						}
						else
						{
							//TODO (never encountered, unsure if there is something else)
						}
						xmlFree((void*)xmlText);
					}
					else
					{
						//XXX Error
					}
				}
				else
				{
					parseToHashtableByName(node, data, xmlFree);
				}
			}

			bing_mem_free((void*)nodeName);
		}
		else
		{
			//XXX Error
		}
	}

	//If this is an empty response, ignore it
	if (!node)
	{
		hashtable_free(data);
		return NULL;
	}

	//Get the name in preparation for response creation
	text = NULL;
	size = hashtable_get_item(data, (composite ? PARSE_KEY_TITLE : PARSE_KEY_SUBTITLE), NULL);
	if(size > 0)
	{
		text = bing_mem_malloc(size);
		if(text)
		{
			hashtable_get_item(data, (composite ? PARSE_KEY_TITLE : PARSE_KEY_SUBTITLE), text);

			//Create response
			parser->current = NULL;
			if(response_create_raw(text, (bing_response_t*)&parser->current, parser->bing,
					(parser->response != NULL && parser->response->type == BING_SOURCETYPE_COMPOSITE) ? parser->response : NULL)) //The general idea is that if there is already a response and it is bundle, it will be the parent. Otherwise add it to Bing
			{
				//Run creation functions
				if(response_def_create_standard_responses(parser->current, (data_dictionary_t)data) &&
						parser->current->creation(text, (bing_response_t)parser->current, (data_dictionary_t)data))
				{
					//Should we do any extra processing on the response?
					if(parser->response)
					{
						//Response already exists. If it is a composite then it is already added, otherwise we need to replace it
						if(parser->response->type != BING_SOURCETYPE_COMPOSITE)
						{
							//Save it temporarily
							tmp = parser->response;

							if(response_create_raw(RESPONSE_COMPOSITE, (bing_response_t*)&parser->response, parser->bing, NULL))
							{
								//We need to take the original response and make it a child of the new composite response
								response_swap_response(tmp, parser->response);

								//We also need the new current response to be a child of the new composite response
								response_swap_response(parser->current, parser->response);
							}
							else
							{
								//Darn it, that failed
								bing_response_free(parser->current);
								parser->parseError = PE_START_ELE_RESPONSE_BUNDLE_CREATE_FAIL; //XXX Rename
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
					if(!(parser->response != NULL && parser->response->type == BING_SOURCETYPE_COMPOSITE))
					{
						//This wasn't a composited response, just free it (otherwise it will never get freed)
						bing_response_free(parser->current);
					}
					parser->parseError = PE_START_ELE_RESPONSE_CREATION_CALLBACK_FAIL; //XXX Rename
				}
			}
			else
			{
				parser->parseError = PE_START_ELE_RESPONSE_CREATE_FAIL; //XXX Rename
			}

			bing_mem_free((void*)text);
		}
	}

	if(parser->current)
	{
		//Parse entries
		for(; node != NULL; node = node->next)
		{
			//TODO Check for error, stop if error

			nodeName = xmlGetQualifiedName(node);
			if(!nodeName)
			{
				//XXX Error
			}

			if(strcmp(nodeName, "entry") == 0)
			{
				bing_mem_free((void*)nodeName);

				//Result automatically added to response
				if(!parseResult(node, FALSE, parser->current, xmlFree))
				{
					//Check if composite (we find out first before processing because if it isn't, we have no way to... react. We also want to check a "link" node which requires additional checking)
					subResComp = FALSE;
					for(node2 = node->children; node2 != NULL; node2 = node2->next)
					{
						nodeName = xmlGetQualifiedName(node2);
						if(!nodeName)
						{
							//XXX Error
						}

						//Check the "title"
						if(strcmp(nodeName, PARSE_KEY_TITLE) == 0)
						{
							bing_mem_free((void*)nodeName);

							//Get the inner text
							xmlText = xmlNodeGetContent(node);
							if(xmlText)
							{
								if(strcmp((char*)xmlText, "ExpandableSearchResult") == 0)
								{
									//Yep, it's a composite
									subResComp = TRUE;
								}
								xmlFree((char*)xmlText);
							}
							break;
						}

						bing_mem_free((void*)nodeName);
					}
					if(subResComp)
					{
						for(node2 = node->children; node2 != NULL; node2 = node2->next)
						{
							nodeName = xmlGetQualifiedName(node2);
							if(nodeName)
							{
								//Find the "link" node
								if(strcmp(nodeName, "link") == 0)
								{
									//Get the "type" property of the link (if it's a composite, it will have a "type" property. Check anyway)
									if(nsXmlHasProp(node2, PARSE_PROPERTY_TYPE))
									{
										xmlText = nsXmlGetProp(node2, PARSE_PROPERTY_TYPE);
										if(xmlText)
										{
											if(strcmp((char*)xmlText, "application/atom+xml;type=feed") == 0)
											{
												//Yep, this is a composite node (node2->children->children gets us straight to the internal response [as opposed to the "container" of the response])
												if(!parseResponse(node2->children->children, TRUE, parser, xmlFree))
												{
													//XXX Error
												}
											}
											else
											{
												//XXX Error
											}
											xmlFree((void*)xmlText);
										}
										else
										{
											//XXX Error
										}
									}
								}

								bing_mem_free((void*)nodeName);
							}
							else
							{
								//XXX Error
							}
						}
					}
					else
					{
						//TODO (never encountered, unsure what to do)
					}
				}
			}
			else
			{
				bing_mem_free((void*)nodeName);

				//TODO (never encountered, unsure what to do)
			}
		}
	}

	//Cleanup table
	hashtable_free(data);

	return parser->current;
}

void errorCallback(void *ctx, const char *msg, ...)
{
	bing_parser* parser = (bing_parser*)ctx;
	parser->parseError = PE_ERROR_CALLBACK; //We simply mark this as error because on completion we can check this and it will automatically handle all cleanup and we can get if the search completed or not
}

void serrorCallback(void* userData, xmlErrorPtr error)
{
	bing_parser* parser = (bing_parser*)userData;
	parser->parseError = PE_SERROR_CALLBACK; //We simply mark this as error because on completion we can check this and it will automatically handle all cleanup and we can get if the search completed or not
}

BOOL checkForError(bing_parser* parser)
{
	if(parser->parseError != PE_NO_ERROR)
	{
		//Error, cleanup everything

		//Now free the response (no stacks will exist if a response doesn't exist. Allocated memory, internal responses, and all results are associated with the parent response. Freeing the response will free everything.)
		bing_response_free(parser->current);

		//Mark everything as NULL to prevent errors later
		parser->response = NULL;
		parser->current = NULL;

		return FALSE; //Don't continue
	}
	return TRUE; //No error, continue
}

static xmlSAXHandler parserHandler;

void search_setup()
{
	xmlSAXHandler* handler;

	LIBXML_TEST_VERSION

	if(atomic_add_value(&searchCount, 1) == 0)
	{
		//Setup XML
		xmlGcMemSetup(bing_mem_free, bing_mem_malloc, bing_mem_malloc, bing_mem_realloc, bing_mem_strdup);

		//On first run, setup the parser
		xmlInitParser();

		//Setup cURL
		curl_global_init_mem(CURL_GLOBAL_ALL, bing_mem_malloc, bing_mem_free, bing_mem_realloc, bing_mem_strdup, bing_mem_calloc); //THIS IS NOT THREAD SAFE!!

		//Setup parser
		xmlSAXVersion(&parserHandler, 2);
		//parserHandler.error = errorCallback;
		//parserHandler.fatalError = errorCallback;
		//parserHandler.serror = serrorCallback;
	}
}

void search_cleanup(bing_parser* parser)
{
	xmlParserCtxtPtr ctx;
	if(parser)
	{
#if defined(BING_DEBUG)
		lastErrorCode = (int)parser->parseError; //For devs
		if(parser->parseError != PE_NO_ERROR)
		{
			BING_MSG_PRINTOUT("Parser error: %d\n", (int)parser->parseError);
		}
#endif

		ctx = parser->ctx;
		if(ctx)
		{
			ctx->userData = NULL;
		}

		//Shutdown cURL
		curl_easy_cleanup(parser->curl);

		//Now get rid of the bing context
		parser->bing = 0;

		//Close thread (if used)
		parser->thread = NULL; //pthread_exit (called implicitly at end of execution) frees the thread

		//Free the bing parser
		bing_mem_free(parser);

		//Free the document
		if(ctx)
		{
			xmlFreeDoc(ctx->myDoc);
		}

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
		if(parser->parseError == PE_NO_ERROR)
		{
			xmlParseChunk(parser->ctx, ptr, atcsize, FALSE);
		}
	}
	else
	{
		//Create parser
		parser->ctx = xmlCreatePushParserCtxt(/*(xmlSAXHandlerPtr)&parserHandler*/NULL, parser, ptr, atcsize, NULL);
		parser->parseError = PE_NO_ERROR;
		if(!parser->ctx)
		{
			//If an error occurs, it will ignore writing any data
			parser->parseError = PE_GETXMLDATA_CTX_CREATE_FAIL;
		}
	}

	//If an error occurred, we want to let cURL know there was an error
	if(parser->parseError != PE_NO_ERROR)
	{
		atcsize = 0;
	}

	return atcsize;
}

CURL* setupCurl(bing* bingI, const char* url, bing_parser* parser)
{
	CURL* ret;
	if(bingI)
	{
		pthread_mutex_lock(&bingI->mutex);

		if(bingI->accountKey)
		{
			ret = curl_easy_init();

			if(ret)
			{
				//Set the URL
				if(curl_easy_setopt(ret, CURLOPT_URL, url) == CURLE_OK &&							//The URL, required
						curl_easy_setopt(ret, CURLOPT_WRITEFUNCTION, getxmldata) == CURLE_OK &&		//The function to handle the data, required
						curl_easy_setopt(ret, CURLOPT_WRITEDATA, (void*)parser) == CURLE_OK &&		//The "userdata" to be passed into the write function, required
						curl_easy_setopt(ret, CURLOPT_SSL_VERIFYPEER, CURL_FALSE) == CURLE_OK &&	//We don't have a SSL cert for verification, so skip it
						curl_easy_setopt(ret, CURLOPT_USERNAME, CURL_EMPTY_STRING) == CURLE_OK &&	//For basic HTTP authentication, this is the "user ID", which is ignored right now
						curl_easy_setopt(ret, CURLOPT_PASSWORD, bingI->accountKey) == CURLE_OK)		//For basic HTTP authentication, this is the "account key"
				{
					//We don't want any progress meters
					curl_easy_setopt(ret, CURLOPT_NOPROGRESS, CURL_TRUE);
				}
				else
				{
					curl_easy_cleanup(ret);
					ret = NULL;
				}
			}
		}
		pthread_mutex_unlock(&bingI->mutex);
	}

	return ret;
}

BOOL setupParser(bing_parser* parser, unsigned int bingID, const char* url)
{
	BOOL ret = FALSE;
	bing* bingI = retrieveBing(bingID);
	if(bingI)
	{
		memset(parser, 0, sizeof(bing_parser));

		parser->bing = bingID;
		parser->curl = setupCurl(bingI, url, parser);
		if(parser->curl)
		{
#if defined(BING_DEBUG)
			pthread_mutex_lock(&bingI->mutex);

			parser->errorRet = bingI->errorRet;

			pthread_mutex_unlock(&bingI->mutex);
#endif
			ret = TRUE;
		}
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
bing_response_t bing_search_url_sync(unsigned int bingID, const char* url)
{
	bing_parser* parser;
	bing_response_t ret = NULL;
#if defined(BING_DEBUG)
	int curlCode;
#endif
	xmlFreeFunc xmlFreeF;

	//TODO: Need to figure out how to get translation APIs to work when in a composite (will need to do two separate searches)

	if(check_for_connection() && url)
	{
		search_setup();

		//Create the parser
		parser = bing_mem_malloc(sizeof(bing_parser));
		if(parser)
		{
			//Setup the parser
			if(setupParser(parser, bingID, url))
			{
				if(check_for_connection())
				{
					//Get memory function
					xmlGcMemGet(&xmlFreeF, NULL, NULL, NULL, NULL);

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

						//Parse document
						parseResponse(parser->ctx->myDoc->children, FALSE, parser, xmlFreeF);

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
				if(bing_get_error_return(bingID))
				{
					BING_MSG_PRINTOUT("Could not setup parser\n");
				}
#endif
				//Couldn't setup parser
				bing_mem_free(parser);
			}
		}
#if defined(BING_DEBUG)
		else if(bing_get_error_return(bingID))
		{
			BING_MSG_PRINTOUT("Could not parser\n");
		}
#endif
	}

	return ret;
}

bing_response_t bing_search_sync(unsigned int bingID, const char* query, const bing_request_t request)
{
	const char* url;
	bing_response_t ret = NULL;

	if(check_for_connection())
	{
		//Get the URL
		url = bing_request_url(query, request);
		if(url)
		{
			ret = bing_search_url_sync(bingID, url);

			//Free URL
			bing_mem_free((void*)url);
		}
#if defined(BING_DEBUG)
		else if(bing_get_error_return(bingID))
		{
			BING_MSG_PRINTOUT("Could not create URL\n");
		}
#endif
	}

	return ret;
}

bing_response_t bing_search_next_sync(const bing_response_t pre_response)
{
	bing_response_t ret = NULL;
	bing_response* res = (bing_response*)pre_response;

	if(check_for_connection() && bing_response_has_next_results(pre_response))
	{
		ret = bing_search_url_sync(res->bing, res->nextUrl);
	}

	return ret;
}

void* async_search(void* ctx)
{
#if defined(BING_DEBUG)
	int curlCode;
#endif
	receive_bing_response_func responseFunc = NULL;
	bing_response* response = NULL;
	const void* userData = NULL;
	bing_parser* parser = (bing_parser*)ctx;
	xmlFreeFunc xmlFreeF;

	if(check_for_connection())
	{
		//Get memory function
		xmlGcMemGet(&xmlFreeF, NULL, NULL, NULL, NULL);

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

			//Parse document
			parseResponse(parser->ctx->myDoc->children, FALSE, parser, xmlFreeF);

			//We check for an error
#if defined(BING_DEBUG)
			if(!
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


	//Return response (NULL is fine for a response)
	if(responseFunc)
	{
		responseFunc(response, userData);
	}

	//Cleanup
	search_cleanup(parser);

	return NULL;
}

//We need to free the event because the event could be seen by multiple applications and we don't want them all freeing it
void event_done(bps_event_t *event)
{
	bps_event_payload_t* payload = bps_event_get_payload(event);
	bing_response_free((bing_response_t)payload->data1);
	payload->data1 = NULL;
}

void event_invocation(bing_response_t response, const void* user_data)
{
	bps_event_t* event = NULL;
	bps_event_payload_t payload;
	bing_parser* parser = (bing_parser*)user_data;
	if(response) //We only want to push an event if we have something to push.
	{
		memset(&payload, 0, sizeof(bps_event_payload_t));
		payload.data1 = (uintptr_t)response;

		//Create the event
		if((bps_event_create(&event, bing_get_domain(), 0, &payload, event_done) != BPS_SUCCESS | //Create event
				(parser->bpsChannel >= 0 ? bps_channel_push_event(parser->bpsChannel, event) : bps_push_event(event))) != BPS_SUCCESS) //Push event (if we have a BPS channel, use it)
		{
			//Since the event will never be pushed, free it
			if(event)
			{
				//If the event was created, the "event_done" function will free the event...
				bps_event_destroy(event);
			}
			else
			{
				//otherwise we can simply free the response
				bing_response_free(response);
			}
		}
	}
}

int search_async_url_in(unsigned int bingID, const char* url, const void* user_data, BOOL user_data_is_parser, receive_bing_response_func response_func)
{
	bing_parser* parser;
	pthread_attr_t thread_atts;
	BOOL ret = FALSE;

	//TODO: Need to figure out how to get translation APIs to work when in a composite (will need to do two separate searches)

	if(check_for_connection() && url)
	{
		search_setup();

		//Create the parser
		parser = bing_mem_malloc(sizeof(bing_parser));
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
				pthread_attr_setdetachstate(&thread_atts, PTHREAD_CREATE_DETACHED);

				//Create thread
				ret = pthread_create(&parser->thread, &thread_atts, async_search, parser) == EOK;

				//Cleanup attributes
				pthread_attr_destroy(&thread_atts);
			}
			else
			{
#if defined(BING_DEBUG)
				if(bing_get_error_return(bingID))
				{
					BING_MSG_PRINTOUT("Could not setup parser\n");
				}
#endif
				//Couldn't setup parser
				bing_mem_free(parser);
			}
		}
#if defined(BING_DEBUG)
		else if(bing_get_error_return(bingID))
		{
			BING_MSG_PRINTOUT("Could not parser\n");
		}
#endif
	}

	return ret;
}

int search_async_in(unsigned int bingID, const char* query, const bing_request_t request, const void* user_data, BOOL user_data_is_parser, receive_bing_response_func response_func)
{
	const char* url;
	BOOL ret = FALSE;

	if(check_for_connection())
	{
		//Get the URL
		url = bing_request_url(query, request);
		if(url)
		{
			ret = search_async_url_in(bingID, url, user_data, user_data_is_parser, response_func);

			//Free URL
			bing_mem_free((void*)url);
		}
#if defined(BING_DEBUG)
		else if(bing_get_error_return(bingID))
		{
			BING_MSG_PRINTOUT("Could not create URL\n");
		}
#endif
	}

	return ret;
}

int bing_search_async(unsigned int bingID, const char* query, const bing_request_t request, const void* user_data, receive_bing_response_func response_func)
{
	return search_async_in(bingID, query, request, user_data, FALSE, response_func);
}

int bing_search_next_async(const bing_response_t pre_response, const void* user_data, receive_bing_response_func response_func)
{
	BOOL ret = FALSE;
	bing_response* res = (bing_response*)pre_response;

	if(check_for_connection() && bing_response_has_next_results(pre_response))
	{
		ret = search_async_url_in(res->bing, res->nextUrl, user_data, FALSE, response_func);
	}

	return ret;
}

int bing_search_event_async(unsigned int bingID, const char* query, const bing_request_t request)
{
	return search_async_in(bingID, query, request, NULL, TRUE, event_invocation);
}

int bing_search_event_next_async(const bing_response_t pre_response)
{
	BOOL ret = FALSE;
	bing_response* res = (bing_response*)pre_response;

	if(check_for_connection() && bing_response_has_next_results(pre_response))
	{
		ret = search_async_url_in(res->bing, res->nextUrl, NULL, TRUE, event_invocation);
	}

	return ret;
}

#if defined(BING_DEBUG)
int bing_get_last_error_code()
{
	return lastErrorCode;
}
#endif
