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
	assert(sizeof(xmlChar) == sizeof(char)); //Purly for the sake that xmlChar could be a different size, which would mean a different method of string creation is needed

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

int hashtable_key_exists(hashtable_t* table, const char* key)
{
	int ret = -1;
	if(table && key)
	{
		ret = xmlHashLookup(((ht*)table)->table, (xmlChar*)key) != NULL;
	}
	return ret;
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

int hashtable_put_item(hashtable_t* table, const char* key, const void* data, size_t data_size)
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
				if(!resizeHashTable(hash))
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
			if(data)
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
		strlcpy(keys[index], (char*)name, size);
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

int hashtable_get_data_key(hashtable_t* table, const char* key, void* value, size_t size)
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
		ret = hashtable_get_item(table, field, value);
	}
	return ret;
}

int hashtable_set_data(hashtable_t* table, const char* field, const void* value, size_t size)
{
	BOOL ret = FALSE;
	if(table && field)
	{
		if(!value && hashtable_get_item(table, field, NULL) != -1)
		{
			hashtable_remove_item(table, field);
		}
		else if(value)
		{
			if(hashtable_put_item(table, field, value, size) != -1)
			{
				ret =  TRUE;
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
