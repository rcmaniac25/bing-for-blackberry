/*
 * types.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#include <time.h>

long long parseTime(const char* stime)
{
	struct tm ptime;
	time_t t = time(NULL);
	if(stime)
	{
		//Clear time structure
		memset(&ptime, 0, sizeof(struct tm));

		//Parse time
		if (strptime(stime, "%Y-%m-%dT%H:%M:%SZ", &ptime) != NULL)
		{
			//Not set by strptime(); tells mktime() to determine whether daylight saving time is in effect
			ptime.tm_isdst = -1;

			//Get time as epoch
			t = mktime(&ptime);
		}
	}
	return (long long)t;
}

enum FIELD_TYPE getParsedTypeByType(const char* type)
{
	if(type)
	{
		if(strcmp(type, "text") == 0 || strcmp(type, "Edm.String") == 0 || //Normal string types
				strcmp(type, "Edm.Guid") == 0) //There is no dedicated GUID type, so simply return it as a string
		{
			return FIELD_TYPE_STRING;
		}
		else if(strcmp(type, "dateTime") == 0 || strcmp(type, "Edm.DateTime") == 0 || strcmp(type, "Edm.Int64") == 0)
		{
			return FIELD_TYPE_LONG;
		}
		else if(strcmp(type, "Edm.Int32") == 0)
		{
			return FIELD_TYPE_INT;
		}
	}
	return FIELD_TYPE_UNKNOWN;
}

//Helper macro for saving a long long
#define SAVE_LONG_LONG(x) \
	tmp = bing_mem_malloc(sizeof(long long)); \
	if(tmp) \
	{ \
		*((long long*)tmp) = (x); \
	}

//This will always produce a unique memory element. It will not pass a pointer.
void* parseByType(const char* type, xmlNodePtr node, xmlFreeFunc xmlFree)
{
	const xmlChar* text;
	void* tmp;
	if(type && node)
	{
		//Could possibly change to an array of types-to-parsing types. Similar to what happens with finding the right result, response, etc. {type, FIELD_TYPE, parser func}

		if(strcmp(type, "text") == 0 || strcmp(type, "Edm.String") == 0 || //Normal string types
				strcmp(type, "Edm.Guid") == 0) //There is no dedicated GUID type, so simply return it as a string
		{
			//Get the node contents
			text = xmlNodeGetContent(node);
			if(text)
			{
				//Duplicate the string
				tmp = (void*)bing_mem_strdup((char*)text);

				//Free the contents
				xmlFree((void*)text);

				//Return the duplicated string
				return tmp;
			}
		}
		else if(strcmp(type, "dateTime") == 0 || strcmp(type, "Edm.DateTime") == 0)
		{
			//Get the node contents
			text = xmlNodeGetContent(node);
			if(text)
			{
				//We need to allocate memory for the long long
				SAVE_LONG_LONG(parseTime((char*)text))

				//Free the contents
				xmlFree((void*)text);

				//Return the value
				return tmp;
			}
		}
		else if(strcmp(type, "Edm.Int32") == 0)
		{
			//Get the node contents
			text = xmlNodeGetContent(node);
			if(text)
			{
				//Parse the int
				tmp = (void*)atoi((char*)text);

				//Free the contents
				xmlFree((void*)text);

				//Return the int;
				return tmp;
			}
		}
		else if(strcmp(type, "Edm.Int64") == 0)
		{
			//Get the node contents
			text = xmlNodeGetContent(node);
			if(text)
			{
				//Parse the long long
#if __SIZEOF_POINTER__ == 8
				//We can simply parse, a pointer is the size of a long long
				tmp = (void*)atoll((char*)text);
#else
				//We need to allocate memory for the long long
				SAVE_LONG_LONG(atoll((char*)text))
#endif

				//Free the contents
				xmlFree((void*)text);

				//Return the value
				return tmp;
			}
		}
	}
	return NULL;
}

enum FIELD_TYPE getParsedTypeByName(xmlNodePtr node)
{
	const char* name;
	enum FIELD_TYPE ret = FIELD_TYPE_UNKNOWN;
	if(node && node->type == XML_ELEMENT_NODE)
	{
		name = xmlGetQualifiedName(node);
		if(name)
		{
			//We don't have many options right now, so just do this manually
			if(strcmp(name, "id") == 0)
			{
				ret = FIELD_TYPE_STRING;
			}
			else if(strcmp(name, "updated") == 0)
			{
				ret = FIELD_TYPE_LONG;
			}

			bing_mem_free((void*)name);
		}
	}
	return ret;
}

void* parseByName(xmlNodePtr node, xmlFreeFunc xmlFree)
{
	//We could parse by type, but if we end up with another type that returns a long, we would end up attempting to parse it as a date, which won't go well.
	const char* name;
	void* ret = NULL;
	if(node && node->type == XML_ELEMENT_NODE)
	{
		name = xmlGetQualifiedName(node);
		if(name)
		{
			//We don't have many options right now, so just do this manually
			if(strcmp(name, "id") == 0)
			{
				ret = parseByType("text", node, xmlFree);
			}
			else if(strcmp(name, "updated") == 0)
			{
				ret = parseByType("dateTime", node, xmlFree);
			}

			bing_mem_free((void*)name);
		}
	}
	return ret;
}

BOOL isComplex(const char* name)
{
	if(name)
	{
		//We only have one complex type right now
		return strcmp(name, "Bing.Thumbnail") == 0;
	}
	return FALSE;
}

//Take the parsed data and store it in the table under the node's qualified name
BOOL handleParsedData(xmlNodePtr node, const void* parsedData, enum FIELD_TYPE type, hashtable_t* table)
{
	size_t size;
	BOOL res = FALSE;
	const char* name;
	const void* ptr;

	if(type != FIELD_TYPE_UNKNOWN)
	{
//If the data is the size of a pointer, we will take it since the parsed int/long value of zero will be a NULL pointer anyway
#if __SIZEOF_POINTER__ == 8
		if(parsedData || type == FIELD_TYPE_INT || type == FIELD_TYPE_LONG)
#else
		if(parsedData || type == FIELD_TYPE_INT)
#endif
		{
			//Determine size
			switch(type)
			{
				case FIELD_TYPE_STRING:
					size = strlen((char*)parsedData) + 1;
					break;
				case FIELD_TYPE_LONG:
					size = sizeof(long long);
					break;
				case FIELD_TYPE_INT:
					size = sizeof(int);
					break;
				case FIELD_TYPE_DOUBLE:
					size = sizeof(double);
					break;
				case FIELD_TYPE_BOOLEAN:
					size = sizeof(BOOL);
					break;
				case FIELD_TYPE_ARRAY:
					size = *((size_t*)parsedData);
					parsedData = parsedData + sizeof(size_t); //Move the data position
					break;
				default:
					size = 0;
					break;
			}
			if(size > 0)
			{
				//Add to table
#if __SIZEOF_POINTER__ == 8
				//If type is int or long long, the value is stored in the pointer itself instead of a memory position the pointer points to. We need to get a pointer so hashtable can copy it
				if(type == FIELD_TYPE_INT || type == FIELD_TYPE_LONG)
#else
				//If type is int, the value is stored in the pointer itself instead of a memory position the pointer points to. We need to get a pointer so hashtable can copy it
				if(type == FIELD_TYPE_INT)
#endif
				{
					ptr = &parsedData;
				}
				else
				{
					ptr = parsedData;
				}
				name = xmlGetQualifiedName(node);
				if(name)
				{
					res = hashtable_put_item(table, name, ptr, size);
					bing_mem_free((void*)name);
				}
			}
#if __SIZEOF_POINTER__ == 8
			//If data is not an int or long long, we need to free it
			if(!(type == FIELD_TYPE_INT || type == FIELD_TYPE_LONG))
#else
			//If data is not an int, we need to free it
			if(type != FIELD_TYPE_INT)
#endif
			{
				bing_mem_free((void*)parsedData);
			}
		}
	}
	return res;
}

//Parse to a table

BOOL parseToHashtableByType(const char* stype, xmlNodePtr node, hashtable_t* table, xmlFreeFunc xmlFree)
{
	//table.Add(node.Name, ParseByType(stype, node));
	enum FIELD_TYPE type = getParsedTypeByType(stype);
	if(type != FIELD_TYPE_UNKNOWN)
	{
		return handleParsedData(node, parseByType(stype, node, xmlFree), type, table);
	}
	return FALSE;
}

BOOL parseToHashtableByName(xmlNodePtr node, hashtable_t* table, xmlFreeFunc xmlFree)
{
	//table.Add(node.Name, ParseByName(node));
	enum FIELD_TYPE type = getParsedTypeByName(node);
	if(type != FIELD_TYPE_UNKNOWN)
	{
		return handleParsedData(node, parseByName(node, xmlFree), type, table);
	}
	return FALSE;
}
