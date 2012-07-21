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

#include <curl/curl.h>

//Defines for parsing
#define DEFAULT_HASHTABLE_SIZE 8
#define PARSE_PROPERTY_TYPE ((xmlChar*)"type")
#define PARSE_PROPERTY_MTYPE ((xmlChar*)"m:type")

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

/* XXX
typedef struct PARSER_STACK_S
{
	void* value;
	BOOL keepValue;
	bing_result* addToResult;
	bing_response* addToResponse;
	struct PARSER_STACK_S* prev;
} pstack;
*/

typedef struct PARSER_STACK2_S
{
	void* value;
	struct PARSER_STACK2_S* prev;
} pstack2;

typedef struct BING_PARSER_S
{
	//Freed on error
	bing_response* response;
	bing_response* current;
	/* XXX
	pstack* lastResult; //XXX Remove
	pstack* lastResultElement; //XXX Remove
	const char* query; //XXX Remove
	const char* alteredQuery; //XXX Remove
	const char* alterationOverrideQuery; //XXX Remove
	*/

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

const char* getQualifiedName(const xmlChar* localname, const xmlChar* prefix) //XXX Actually, might not be needed
{
	size_t size;
	char* qualifiedName;
	if(prefix)
	{
		qualifiedName = bing_mem_malloc(size = strlen((char*)localname) + strlen((char*)prefix) + 2);
		if(qualifiedName)
		{
			snprintf(qualifiedName, size, "%s:%s", prefix, localname);
			qualifiedName[size - 1] = '\0';
		}
	}
	else
	{
		//There is nothing that requires changing
		qualifiedName = bing_mem_strdup((char*)localname);
	}
	return qualifiedName;
}

bing_result* parseResult(xmlNodePtr resultNode, BOOL type, bing_response* parent, xmlFreeFunc xmlFree)
{
	//Not really the greatest names, could probably change
	bing_result* res = NULL;
	bing_result* tres;
	xmlNodePtr node;
	const xmlChar* xmlText;
	char* text;
	pstack2* additionalProcessing = NULL;
	pstack2* tStack;
	hashtable_t* data = hashtable_create(DEFAULT_HASHTABLE_SIZE);
	int size;
	BOOL keep;

	//Get data
	if(!type)
	{
		//Go through all the nodes to get data
		for(node = resultNode->children; node != NULL; node = node->next)
		{
			//We want to stop on content, we process that later
			if(strcmp((char*)node->name, "content") == 0)
			{
				break;
			}

			//If we have a node with a type property, it makes it easy for us
			if(xmlHasProp(node, PARSE_PROPERTY_TYPE))
			{
				xmlText = xmlGetProp(node, PARSE_PROPERTY_TYPE);
				if(xmlText)
				{
					//Parse the data
					if(parseToHashtableByType((char*)xmlText, node, data, xmlFree))
					{
						//Check to see if this is a composite response
						if(strcmp((char*)node->name, PARSE_KEY_TITLE) == 0)
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
				if(xmlHasProp(node, PARSE_PROPERTY_MTYPE))
				{
					xmlText = xmlGetProp(node, PARSE_PROPERTY_MTYPE);
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
				else if(strcmp((char*)node->name, "link") == 0)
				{
					xmlText = xmlGetProp(node, (xmlChar*)"rel");
					if(xmlText)
					{
						if(strcmp((char*)xmlText, "next") == 0)
						{
							xmlFree((void*)xmlText);

							xmlText = xmlGetProp(node, (xmlChar*)"href");
							//XXX Special assignment
							if(!hashtable_put_item(data, PARSE_NEXT_LINK, xmlText, strlen((char*)xmlText) + 1))
							{
								//XXX Error
							}
						}
						else if(strcmp((char*)xmlText, "self") == 0)
						{
							xmlFree((void*)xmlText);

							xmlText = xmlGetProp(node, (xmlChar*)"href");
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
		}

		//If this is an empty result, ignore it
		if (!node)
		{
			hashtable_free(data);
			return NULL;
		}

		//If content is not the expected type, ignore it.
		if(xmlHasProp(node, PARSE_PROPERTY_TYPE))
		{
			xmlText = xmlGetProp(node, PARSE_PROPERTY_TYPE);
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
		xmlText = PARSE_PROPERTY_TYPE;
		if(!xmlHasProp(node, xmlText))
		{
			xmlText = PARSE_PROPERTY_MTYPE;
			if(!xmlHasProp(node, xmlText))
			{
				xmlText = NULL;
			}
		}

		//Process the node
		if(xmlText)
		{
			//... as stated before, pointers will be embedded in lib so no need to free them.
			xmlText = xmlGetProp(node, PARSE_PROPERTY_TYPE);
			if(xmlText)
			{
				if(isComplex((char*)xmlText))
				{
					//Push a new value onto the stack (order doesn't matter)
					tStack = bing_mem_malloc(sizeof(pstack2));
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
		xmlText = xmlGetProp(resultNode, PARSE_PROPERTY_MTYPE);
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
			tres = parseResult(node, TRUE, parent, xmlFree);
			if(tres)
			{
				keep = FALSE;
				res->additionalResult((char*)node->name, res, tres, &keep);
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
	hashtable_t* data = hashtable_create(DEFAULT_HASHTABLE_SIZE);
	size_t size;
	BOOL subResComp;

	//Get general data
	for(node = responseNode->children; node != NULL; node = node->next)
	{
		//We want to stop on content, we process that later
		if(strcmp((char*)node->name, "entry") == 0)
		{
			break;
		}

		//If we have a node with a type property, it makes it easy for us
		if(xmlHasProp(node, PARSE_PROPERTY_TYPE))
		{
			xmlText = xmlGetProp(node, PARSE_PROPERTY_TYPE);
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
			if(strcmp((char*)node->name, "link") == 0)
			{
				xmlText = xmlGetProp(node, (xmlChar*)"rel");
				if(xmlText)
				{
					if(strcmp((char*)xmlText, "next") == 0)
					{
						xmlFree((void*)xmlText);

						xmlText = xmlGetProp(node, (xmlChar*)"href");
						//XXX Special assignment
						if(!hashtable_put_item(data, PARSE_NEXT_LINK, xmlText, strlen((char*)xmlText) + 1))
						{
							//XXX Error
						}
					}
					else if(strcmp((char*)xmlText, "self") == 0)
					{
						xmlFree((void*)xmlText);

						xmlText = xmlGetProp(node, (xmlChar*)"href");
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
			if(strcmp((char*)node->name, "entry") == 0)
			{
				//Result automatically added to response
				if(!parseResult(node, FALSE, parser->current, xmlFree))
				{
					//Check if composite (we find out first before processing because if it isn't, we have no way to... react. We also want to check a "link" node which requires additional checking)
					subResComp = FALSE;
					for(node2 = node->children; node2 != NULL; node2 = node2->next)
					{
						//Check the "title"
						if(strcmp((char*)node2->name, PARSE_KEY_TITLE) == 0)
						{
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
					}
					if(subResComp)
					{
						for(node2 = node->children; node2 != NULL; node2 = node2->next)
						{
							//Find the "link" node
							if(strcmp((char*)node2->name, "link") == 0)
							{
								//Get the "type" property of the link (if it's a composite, it will have a "type" property. Check anyway)
								if(xmlHasProp(node2, PARSE_PROPERTY_TYPE))
								{
									xmlText = xmlGetProp(node2, PARSE_PROPERTY_TYPE);
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

/* XXX Old

//Processors

//*sigh* C doesn't have an equivalent function of endWith unless it's a character.
BOOL endsWith(const char* str1, const char* str2)
{
	//See if we can find str2

	//First we need the length of the string (we can ignore the null char)
	int len = str2 ? strlen(str2) : 0;

	//If we can find the last instance of the string, we can check length and compare
	const char* sub = NULL;
	if(len > 0)
	{
		sub = strrchr(str1, str2[0]);
		if(sub)
		{
			//We do this by first checking length, if it is the same length then it could be it..
			if(strlen(sub) == len)
			{
				//strcmp will tell us if it matches, if it doesn't match the string would be too short to run anyway so just return
				return strcmp(sub, str2) == 0;
			}
		}
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

		bing_mem_free(st); //Object
	}

	//Free the element
	while(parser->lastResultElement)
	{
		st = parser->lastResultElement;
		parser->lastResultElement = st->prev;

		bing_mem_free(st->value); //Name
		bing_mem_free(st); //Object
	}
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
				tmp = bing_mem_malloc(size = (end - value) + 1);
				if(tmp)
				{
					strlcpy(tmp, value, size);

					hashtable_put_item(table, localName, tmp, size);

					bing_mem_free(tmp);
				}
			}
		}
	}
	return table;
}

void addResultToStack(bing_parser* parser, bing_result* result, const char* name, BOOL internal)
{
	pstack* pt = bing_mem_malloc(sizeof(pstack));
	if(pt)
	{
		memset(pt, 0, sizeof(pstack));

		//Add to last result
		pt->prev = parser->lastResult;
		parser->lastResult = pt;
		pt->keepValue = TRUE;
		pt->value = result;

		pt = bing_mem_malloc(sizeof(pstack));
		if(pt)
		{
			//Add to last result element
			pt->prev = parser->lastResultElement;
			parser->lastResultElement = pt;
			pt->keepValue = TRUE;
			pt->value = bing_mem_strdup(name);
			if(!pt->value)
			{
				//Darn, so close again
				parser->parseError = PE_ADD_RESULT_STACK_STRDUB_FAIL;

				response_remove_result(parser->current, result, internal, TRUE);
			}
		}
		else
		{
			//That's not good (darn, so close. We where able to allocate the first stack but not the second)
			parser->parseError = PE_ADD_RESULT_STACK_NAME_PSTACK_NOALLOC;

			response_remove_result(parser->current, result, internal, TRUE);
		}
	}
	else
	{
		//That's not good
		parser->parseError = PE_ADD_RESULT_STACK_RESULT_PSTACK_NOALLOC;

		response_remove_result(parser->current, result, internal, TRUE);
	}
}

//The real "meat and potatoes" of the parser. Many if blocks for safety and to increase flexibility (and hopefully readability...)

//XXX Rewrite
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
						if(size > 0)
						{
							bing_mem_free((void*)parser->query);
							parser->query = bing_mem_malloc(size);
							hashtable_get_string(table, "SearchTerms", (char*)parser->query);
						}
						size = hashtable_get_string(table, "AlteredQuery", NULL);
						if(size > 0)
						{
							bing_mem_free((void*)parser->alteredQuery);
							parser->alteredQuery = bing_mem_malloc(size);
							hashtable_get_string(table, "AlteredQuery", (char*)parser->alteredQuery);
						}
						size = hashtable_get_string(table, "AlterationOverrideQuery", NULL);
						if(size > 0)
						{
							bing_mem_free((void*)parser->alterationOverrideQuery);
							parser->alterationOverrideQuery = bing_mem_malloc(size);
							hashtable_get_string(table, "AlterationOverrideQuery", (char*)parser->alterationOverrideQuery);
						}
					}
					else
					{
						//If the table is NULL then we have an issue as we need that table for query
						parser->parseError = PE_START_ELE_QUERY_HASHTABLE_CREATE_FAIL;
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

								data = bing_mem_malloc(sizeof(long long));
								bing_result_get_64bit_int(result, BING_RESULT_FIELD_CODE, (long long*)data);
								BING_MSG_PRINTOUT("Error Code = %lld\n", *((long long*)data));
								bing_mem_free(data);

								data = bing_mem_malloc(bing_result_get_string(result, BING_RESULT_FIELD_MESSAGE, NULL));
								bing_result_get_string(result, BING_RESULT_FIELD_MESSAGE, data);
								BING_MSG_PRINTOUT("Error Message = %s\n", data);
								bing_mem_free(data);

								data = bing_mem_malloc(bing_result_get_string(result, BING_RESULT_FIELD_PARAMETER, NULL));
								bing_result_get_string(result, BING_RESULT_FIELD_PARAMETER, data);
								BING_MSG_PRINTOUT("Error Parameter = %s\n", data);
								bing_mem_free(data);
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
									data = bing_mem_malloc(sizeof(int));
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
									bing_mem_free(data);
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
											bing_response_free(parser->current);
											parser->parseError = PE_START_ELE_RESPONSE_BUNDLE_CREATE_FAIL; //See parser error below for info why this is an actual error
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
									bing_response_free(parser->current);
								}
								parser->parseError = PE_START_ELE_RESPONSE_CREATION_CALLBACK_FAIL; //See parser error below for info why this is an actual error
							}

							hashtable_free(table);
						}
						else
						{
							//Unlike "results" where missing one is unfortunate, the "response" type helps maintain the tree hierarchy and skipping one would mess up everything (possibly)
							parser->parseError = PE_START_ELE_RESPONSE_CREATE_FAIL;
						}
					}
				}
			}

			bing_mem_free((void*)qualifiedName);
		}
		else
		{
			//No qualified name, error
			parser->parseError = PE_START_ELE_NO_QNAME;
		}
	}
}

//XXX Rewrite
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
				bing_mem_free(st); //Object

				//Free the element
				st = parser->lastResultElement;
				parser->lastResultElement = st->prev;

				bing_mem_free(st->value); //Name
				bing_mem_free(st); //Object
			}

			bing_mem_free((void*)qualifiedName);
		}
		else
		{
			//No qualified name, error
			parser->parseError = PE_END_ELE_NO_QNAME;
		}
	}
}
*/

BOOL checkForError(bing_parser* parser)
{
	if(parser->parseError != PE_NO_ERROR)
	{
		//Error, cleanup everything

		//First clean the stacks
		//cleanStacks(parser); //XXX Not needed

		//Now free the response (no stacks will exist if a response doesn't exist. Allocated memory, internal responses, and all results are associated with the parent response. Freeing the response will free everything.)
		bing_response_free(parser->current);

		/* XXX Not needed
		//Finally, free any strings
		bing_mem_free((void*)parser->query);
		bing_mem_free((void*)parser->alteredQuery); //XXX Remove
		bing_mem_free((void*)parser->alterationOverrideQuery); //XXX Remove
		*/

		//Mark everything as NULL to prevent errors later
		parser->response = NULL;
		parser->current = NULL;
		/* XXX Not needed
		parser->lastResult = NULL;
		parser->lastResultElement = NULL;
		parser->query = NULL;
		parser->alteredQuery = NULL;
		parser->alterationOverrideQuery = NULL;
		*/

		return FALSE; //Don't continue
	}
	return TRUE; //No error, continue
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
		NULL,			//startElementNs
		NULL,			//endElementNs
		serrorCallback	//serror
};

void search_setup()
{
	LIBXML_TEST_VERSION

	if(atomic_add_value(&searchCount, 1) == 0)
	{
		//Setup XML
		xmlGcMemSetup(bing_mem_free, bing_mem_malloc, bing_mem_malloc, bing_mem_realloc, bing_mem_strdup);

		//On first run, setup the parser
		xmlInitParser();

		//Setup cURL
		curl_global_init_mem(CURL_GLOBAL_ALL, bing_mem_malloc, bing_mem_free, bing_mem_realloc, bing_mem_strdup, bing_mem_calloc); //THIS IS NOT THREAD SAFE!!
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

		/*
		//Cleanup stacks
		cleanStacks(parser);

		//Cleanup strings
		bing_mem_free((void*)parser->query);
		bing_mem_free((void*)parser->alteredQuery); //XXX Remove
		bing_mem_free((void*)parser->alterationOverrideQuery); //XXX Remove
		*/

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
		parser->ctx = xmlCreatePushParserCtxt((xmlSAXHandlerPtr)&parserHandler, parser, ptr, atcsize, NULL);
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
						parseResponse(parser->ctx->myDoc->children, FALSE, parser, xmlFree);

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
		search_setup();

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

			//Parse document
			parseResponse(parser->ctx->myDoc->children, FALSE, parser, xmlFree);

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
		search_setup();

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
