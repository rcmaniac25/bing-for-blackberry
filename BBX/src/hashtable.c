/*
 * hashtable.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#include <libxml/xmlstring.h>
#include <libxml/hash.h>

//This is a "wrapper" for the actual hashtable which comes from libxml

#define MIN_ALLOC 10

typedef struct hashTable
{
	int alloc;
	xmlHashTablePtr table;
} ht;

//Internal functions
hashtable_t* hashtable_create(int size)
{
	ht* hash = (ht*)malloc(sizeof(ht));
	if(hash)
	{
		hash->alloc = size < MIN_ALLOC ? MIN_ALLOC : size;
		hash->table = xmlHashCreate(hash->alloc);
		if(!hash->table)
		{
			free(hash);
			hash = NULL;
		}
	}
	return (hashtable_t*)hash;
}

void ht_data_deallocator(void* payload, xmlChar* name)
{
	free(payload);
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
		free(hash);
	}
}

void ht_dup_value(void* payload, void* data, xmlChar* name)
{
	xmlHashTablePtr nTable = (xmlHashTablePtr)data;

	//Duplicate the data
	size_t size = *((size_t*)payload) + sizeof(size_t);
	void* nd = malloc(size);
	memcpy(nd, payload, size);

	//Add new value
	xmlHashAddEntry(nTable, name, nd);
}

int resizeHashTable(ht* hash)
{
	int nAlloc = hash->alloc * 2;
	xmlHashTablePtr nTable = xmlHashCreate(nAlloc);
	if(!nTable)
	{
		return -1;
	}

	//Duplicate the table
	xmlHashScan(hash->table, ht_dup_value, nTable);

	//Free the old table
	xmlHashFree(hash->table, ht_data_deallocator);

	//Set the new values
	hash->table = nTable;
	hash->alloc = nAlloc;

	return 0;
}

int hashtable_put_item(hashtable_t* table, const char* key, void* data, size_t data_size)
{
	int ret = -1;
	void* ud = NULL;
	ht* hash;
	if(table && key && data && data_size > 0)
	{
		ud = malloc(data_size + sizeof(size_t));
		if(ud)
		{
			*((size_t*)ud) = data_size;
			memcpy(ud + sizeof(size_t), data, data_size);

			hash = (ht*)table;
			if(hash->alloc <= xmlHashSize(hash->table))
			{
				//We need to resize the table
				if(resizeHashTable(hash) == -1)
				{
					free(ud);
					return ret;
				}
			}

			ret = xmlHashUpdateEntry(hash->table, (xmlChar*)key, ud, ht_data_deallocator);
			if(ret == -1)
			{
				free(ud);
			}
		}
	}
	return ret;
}

int hashtable_get_item(hashtable_t* table, const char* name, void* data)
{
	int ret = -1;
	void* dat = NULL;
	if(table && name)
	{
		dat = xmlHashLookup(((ht*)table)->table, (xmlChar*)name);
		if(dat)
		{
			ret = (int)(*((size_t*)dat));
			if(dat)
			{
				memcpy(data, dat + sizeof(size_t), *((size_t*)dat));
			}
		}
	}
	return ret;
}

int hashtable_remove_item(hashtable_t* table, const char* key)
{
	int ret = -1;
	int keyIndex;
	if(table && key)
	{
		ret = xmlHashRemoveEntry(((ht*)table)->table, (xmlChar*)key, ht_data_deallocator);
	}
	return ret;
}

void ht_get_name(void* payload, void* data, xmlChar* name)
{
	int index = *((int*)data);
	char** keys = *((char***)data);
	int size = strlen((char*)name);

	keys[index] = (char*)calloc(size, sizeof(char));
	if(keys[index])
	{
		strcpy(keys[index], (char*)name);
	}

	*((int*)data) = index + 1;
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
		if(keys)
		{
			dat = malloc(sizeof(char***) + sizeof(int));
			if(!dat)
			{
				ret = -1;
			}
			else
			{
				//Make a simple structure for data
				*((int*)dat) = ret;
				*((char***)(dat + sizeof(int))) = keys;

				//Get the names
				xmlHashScan(hash->table, ht_get_name, dat);

				free(dat);
			}
		}
	}
	return ret;
}

//Public functions
int dictionary_get_data(data_dictionary_t dict, const char* name, void* data)
{
	return hashtable_get_item((hashtable_t*)dict, name, data);
}

int dictionary_get_element_names(data_dictionary_t dict, char** names)
{
	return hashtable_get_keys((hashtable_t*)dict, names);
}
