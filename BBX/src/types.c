/*
 * types.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

enum FIELD_TYPE getParsedTypeByType(const char* type)
{
	if(type)
	{
		if(strcmp(type, "text") == 0 || strcmp(type, "Edm.String") == 0 || //Normal string types
				strcmp(type, "Edm.Guid") == 0) //There is no dedicated GUID type, so simply return it as a string
		{
			return FIELD_TYPE_STRING;
		}
		else if(strcmp(type, "dateTime") == 0 || strcmp(type, "Edm.DateTime") == 0)
		{
			//TODO
		}
		else if(strcmp(type, "Edm.Int32") == 0)
		{
			return FIELD_TYPE_INT;
		}
		else if(strcmp(type, "Edm.Int64") == 0)
		{
			return FIELD_TYPE_LONG;
		}
	}
	return FIELD_TYPE_UNKNOWN;
}

//This will always produce a unique memory element. It will not pass a pointer.
void* parseByType(const char* type, xmlNodePtr node)
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
				tmp = (void*)bing_mem_strdup((const char*)text);

				//Free the contents
				xmlMemFree((void*)text);

				//Return the duplicated string
				return tmp;
			}
		}
		else if(strcmp(type, "dateTime") == 0 || strcmp(type, "Edm.DateTime") == 0)
		{
			//TODO
		}
		else if(strcmp(type, "Edm.Int32") == 0)
		{
			//Get the node contents
			text = xmlNodeGetContent(node);
			if(text)
			{
				//Parse the int
				tmp = (void*)atoi((const char*)text);

				//Free the contents
				xmlMemFree((void*)text);

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
#if __SIZEOF_LONG_LONG__ != 8
#error Long Long not equal to 8
#endif
#if __SIZEOF_POINTER__ == 8
				//We can simply parse, a pointer is the size of a long long
				tmp = (void*)atoll((const char*)text);
#else
				//We need to allocate memory for the long long
				tmp = bing_mem_malloc(sizeof(long long));
				if(tmp)
				{
					*((long long*)tmp) = atoi((const char*)text);
				}
#endif

				//Free the contents
				xmlMemFree((void*)text);

				//Return the int;
				return tmp;
			}
		}
	}
	return NULL;
}

enum FIELD_TYPE getParsedTypeByName(xmlNodePtr node)
{
	const char* name;
	if(node && node->type == XML_ELEMENT_NODE)
	{
		name = (const char*)node->name;

		//We don't have many options right now, so just do this manually
		if(strcmp(name, "id") == 0)
		{
			return FIELD_TYPE_STRING;
		}
		else if(strcmp(name, "updated") == 0)
		{
			return FIELD_TYPE_LONG;
		}
	}
	return FIELD_TYPE_UNKNOWN;
}

void* parseByName(xmlNodePtr node)
{
	//We could parse by type, but if we end up with another type that returns a long, we would end up attempting to parse it as a date, which won't go well.
	const char* name;
	if(node && node->type == XML_ELEMENT_NODE)
	{
		name = (const char*)node->name;

		//We don't have many options right now, so just do this manually
		if(strcmp(name, "id") == 0)
		{
			return parseByType("text", node);
		}
		else if(strcmp(name, "updated") == 0)
		{
			return parseByType("dateTime", node);
		}
	}
	return NULL;
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
