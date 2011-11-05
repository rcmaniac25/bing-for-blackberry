/*
 * hashtable.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

//Partially based off http://elliottback.com/wp/hashmap-implementation-in-c/, as well as my own personal hash table (written for a class)

//Note: This is not an "expert" hashtable, this is very basic and probably poorly written and designed. It gets the job done but it would be much more preferable to use a better written at some point.

typedef struct hashtable_keyvalue_s
{
	const char* key;
	const void* data;
} hashtable_keyvalue_t;

struct hashtable_s
{
	int capacity;
	int size;
	hashtable_keyvalue_t* data;
	pthread_mutex_t mutex;
};

#define MIN_CAP 10

//Internal functions
hashtable_t* hashtable_create()
{
	hashtable_t* table = (hashtable_t*)calloc(1, sizeof(hashtable_t));
	if(table != NULL)
	{
		table->data = (hashtable_keyvalue_t*)calloc(MIN_CAP, sizeof(hashtable_keyvalue_t)); //Makes all values NULL so it is safe to do "NULL" comparision checks on
		if(table->data != NULL)
		{
			table->capacity = MIN_CAP;
			table->size = 0;
			pthread_mutex_init(&table->mutex, NULL);
		}
		else
		{
			free(table);
			table = NULL;
		}
	}
	return table;
}

void hashtable_free(hashtable_t* table)
{
	int i;
	if(table != NULL)
	{
		if(table->size > 0)
		{
			for(i = 0; i < table->capacity; i++)
			{
				if(table->data[i].key != NULL)
				{
					free((void*)table->data[i].key);
					free((void*)table->data[i].data);
				}
			}
		}
		pthread_mutex_destroy(&table->mutex);
		free(table->data);
		free(table);
	}
}

int hashtable_key_exists(hashtable_t* table, const char* key)
{
	int ret = -1;
	if(table != NULL && key != NULL)
	{
		//TODO
	}
	return ret;
}

int hashtable_put_item(hashtable_t* table, const char* key, void* data, size_t data_size)
{
	int ret = -1;
	if(table != NULL && key != NULL && data != NULL && data_size > 0)
	{
		//TODO
	}
	return ret;
}

int hashtable_get_item(hashtable_t* table, const char* name, void* data)
{
	int ret = -1;
	if(table != NULL && name != NULL)
	{
		//TODO
	}
	return ret;
}

int hashtable_remove_item(hashtable_t* table, const char* key)
{
	int ret = -1;
	int keyIndex;
	if(table != NULL && key != NULL)
	{
		keyIndex = hashtable_key_exists(table, key);
		if(keyIndex != -1)
		{
			//TODO
		}
	}
	return ret;
}

int hashtable_get_keys(hashtable_t* table, char** keys)
{
	int ret = -1;
	if(table != NULL && keys != NULL)
	{
		//TODO
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
