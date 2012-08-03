/*
 * hashtable.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#include <assert.h>

#include <libxml/xmlstring.h>
#include <libxml/hash.h>

//This is a "wrapper" for the actual hashtable which comes from libxml

#define MIN_ALLOC 4

typedef struct hashTable
{
	int alloc;
	xmlHashTablePtr table;
} ht;

//Internal functions
hashtable_t* hashtable_create(int size)
{
	ht* hash;

	assert(sizeof(xmlChar) == sizeof(char)); //Purely for the sake that xmlChar could be a different size, which would mean a different method of string creation is needed. We do this at runtime since xmlChar is not a system type

	hash = (ht*)bing_mem_malloc(sizeof(ht));
	if(hash)
	{
		hash->alloc = size < MIN_ALLOC ? MIN_ALLOC : size;
		hash->table = xmlHashCreate(hash->alloc);
		if(!hash->table)
		{
			bing_mem_free(hash);
			hash = NULL;
		}
	}
	return (hashtable_t*)hash;
}

void ht_data_deallocator(void* payload, xmlChar* name)
{
	bing_mem_free(payload);
}

void hashtable_free(hashtable_t* table)
{
	ht* hash;
	if(table)
	{
		hash = (ht*)table;
		xmlHashFree(hash->table, ht_data_deallocator);
		hash->table = NULL;
		hash->alloc = 0;
		bing_mem_free(hash);
	}
}

BOOL hashtable_key_exists(hashtable_t* table, const char* key)
{
	BOOL ret = FALSE;
	if(table && key)
	{
		ret = xmlHashLookup(((ht*)table)->table, (xmlChar*)key) != NULL;
	}
	return ret;
}

void* ht_copy(void* payload, xmlChar* name)
{
	void* nd = NULL;

	if(payload)
	{
		//Duplicate the data
		size_t size = *((size_t*)payload) + sizeof(size_t); //We want the size of the data then we want to add the size of a "size_t".
		nd = bing_mem_malloc(size);
		memcpy(nd, payload, size);
	}

	return nd;
}

void ht_dup_value(void* payload, void* data, xmlChar* name)
{
	xmlHashTablePtr nTable = (xmlHashTablePtr)data;

	void* nd = ht_copy(payload, name);

	//Add new value
	xmlHashAddEntry(nTable, name, nd);
}

BOOL hashtable_copy(hashtable_t* dstTable, const hashtable_t* srcTable)
{
	ht* dst;
	xmlHashTablePtr src;
	int c;
	BOOL ret = FALSE;
	if(dstTable && srcTable)
	{
		dst = (ht*)dstTable;
		src = ((ht*)srcTable)->table;

		c = xmlHashSize(src);
		//Duplicate the table
		if(c > dst->alloc)
		{
			//Need to expand the table, free the old one and create the new one

			//First create the new one
			src = xmlHashCopy(src, ht_copy);
			if(src)
			{
				//Free the old one
				xmlHashFree(dst->table, ht_data_deallocator);

				dst->table = src;
				dst->alloc = xmlHashSize(dst->table); //Get the actual size

				ret = TRUE;
			}
			else
			{
				//Copy it manually
				src = xmlHashCreate(c);

				if(src)
				{
					//Duplicate the table
					xmlHashScan(((ht*)srcTable)->table, ht_dup_value, src);

					dst->alloc = xmlHashSize(dst->table); //Get the actual size

					ret = TRUE;
				}
			}
		}
		else
		{
			//Erase the current table (no existing function to clear it)
			c = xmlHashSize(dst->table);
			if(c > 0)
			{
				//Elements exist, need to erase it
				xmlHashFree(dst->table, ht_data_deallocator);
				dst->table = xmlHashCreate(c);
			}

			//Can simply copy the table
			xmlHashScan(src, ht_dup_value, dst->table);

			ret = TRUE;
		}
	}
	return ret;
}

BOOL resizeHashTableSize(ht* hash, int nAlloc)
{
	//Create hashtable
	xmlHashTablePtr nTable = xmlHashCreate(nAlloc);
	if(!nTable)
	{
		return FALSE;
	}

	//Duplicate the table
	xmlHashScan(hash->table, ht_dup_value, nTable);

	//Free the old table
	xmlHashFree(hash->table, ht_data_deallocator);

	//Set the new values
	hash->table = nTable;
	hash->alloc = nAlloc;

	return TRUE;
}

BOOL resizeHashTable(ht* hash)
{
	return resizeHashTableSize(hash, hash->alloc * 2);
}

BOOL hashtable_compact(hashtable_t* table)
{
	ht* hash;
	if(table)
	{
		hash = (ht*)table;

		//If the allocated size of the table is larger then the actual size, reduce it
		if(hash->alloc > xmlHashSize(hash->table))
		{
			return resizeHashTableSize(hash, xmlHashSize(hash->table));
		}
		return TRUE;
	}
	return FALSE;
}

BOOL hashtable_put_item(hashtable_t* table, const char* key, const void* data, size_t data_size)
{
	BOOL ret = FALSE;
	void* ud = NULL;
	ht* hash;
	if(table && key && data && data_size > 0)
	{
		ud = bing_mem_malloc(data_size + sizeof(size_t));
		if(ud)
		{
			*((size_t*)ud) = data_size;
			memcpy(ud + sizeof(size_t), data, data_size);
#if defined(BING_DEBUG)
			if(memcmp(ud + sizeof(size_t), data, data_size) != 0)
			{
				bing_mem_free(ud);
				ud = NULL;
			}
			else
			{
#endif

			hash = (ht*)table;
			if(hash->alloc <= xmlHashSize(hash->table))
			{
				//We need to resize the table
				if(!resizeHashTable(hash))
				{
					bing_mem_free(ud);
					return ret;
				}
			}

			ret = xmlHashUpdateEntry(hash->table, (xmlChar*)key, ud, ht_data_deallocator) == 0;
			if(!ret)
			{
				bing_mem_free(ud);
			}
#if defined(BING_DEBUG)
			}
#endif
		}
	}
	return ret;
}

size_t hashtable_get_item(hashtable_t* table, const char* name, void* data)
{
	size_t ret = 0;
	void* dat = NULL;
	if(table && name)
	{
		dat = xmlHashLookup(((ht*)table)->table, (xmlChar*)name);
		if(dat)
		{
			ret = *((size_t*)dat);
			if(data)
			{
				memcpy(data, dat + sizeof(size_t), ret);
#if defined(BING_DEBUG)
				if(memcmp(data, dat + sizeof(size_t), ret) != 0)
				{
					ret = 0;
				}
#endif
			}
		}
	}
	return ret;
}

BOOL hashtable_remove_item(hashtable_t* table, const char* key)
{
	BOOL ret = FALSE;
	if(table && key)
	{
		ret = xmlHashRemoveEntry(((ht*)table)->table, (xmlChar*)key, ht_data_deallocator) == 0;
	}
	return ret;
}

void ht_get_name(void* payload, void* data, xmlChar* name)
{
	int index = *((int*)data);
	char** keys = *((char***)(data + sizeof(int)));
	int size = strlen((char*)name) + 1;

	keys[index] = (char*)bing_mem_calloc(size, sizeof(char));
	if(keys[index])
	{
		strlcpy(keys[index], (char*)name, size);
	}

	*((int*)data) = index - 1;
}

int hashtable_get_keys(hashtable_t* table, char** keys)
{
	int ret = -1;
	ht* hash;
	void* dat;

	if(table)
	{
		//Get the number of names
		hash = (ht*)table;
		ret = xmlHashSize(hash->table);
		if(keys && ret > 0)
		{
			dat = bing_mem_malloc(sizeof(char**) + sizeof(int));
			if(dat)
			{
				//Make a simple structure for data
				*((int*)dat) = ret - 1; //Index, so decrement by 1
				*((char***)(dat + sizeof(int))) = keys;

				//Get the names
				xmlHashScan(hash->table, ht_get_name, dat);

				bing_mem_free(dat);
			}
			else
			{
				ret = -1;
			}
		}
	}
	return ret;
}

BOOL hashtable_get_data_key(hashtable_t* table, const char* key, void* value, size_t size)
{
	BOOL ret = FALSE;
	if(table && value)
	{
		//Now get the data (we want to check sizes first so we don't copy something bigger into something smaller)
		if(hashtable_get_item(table, key, NULL) == size)
		{
			hashtable_get_item(table, key, value);
			ret = TRUE;
		}
	}
	return ret;
}

int hashtable_get_string(hashtable_t* table, const char* field, char* value)
{
	int ret = -1;
	if(table)
	{
		//Now get the data
		ret = (int)hashtable_get_item(table, field, value);
		if(ret == 0)
		{
			//There is no content to the string, don't say it even exists
			ret = -1;
		}
	}
	return ret;
}

BOOL hashtable_set_data(hashtable_t* table, const char* field, const void* value, size_t size)
{
	BOOL ret = FALSE;
	if(table && field)
	{
		if(!value && hashtable_get_item(table, field, NULL) > 0)
		{
			hashtable_remove_item(table, field);
		}
		else if(value)
		{
			ret = hashtable_put_item(table, field, value, size);
		}
	}
	return ret;
}

//Public functions
int bing_dictionary_get_data(data_dictionary_t dict, const char* name, void* data)
{
	size_t ret = hashtable_get_item((hashtable_t*)dict, name, data);
	return ret > 0 ? (int)ret : -1;
}

int bing_dictionary_get_element_names(data_dictionary_t dict, char** names)
{
	return hashtable_get_keys((hashtable_t*)dict, names);
}
