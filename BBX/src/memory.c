/*
 * memory.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

void set_bing_memory_handlers(bing_malloc_handler bm, bing_calloc_handler bc, bing_realloc_handler br, bing_free_handler bf, bing_strdup_handler bs)
{
	if(bm && bc && br && bf && bs)
	{
		bing_malloc = bm;
		bing_calloc = bc;
		bing_realloc = br;
		bing_free = bf;
		bing_strdup = bs;
	}
}

//XML/cURL memory handlers
void* xml_curl_malloc(size_t size)
{
	return bing_malloc(size);
}

void xml_curl_free(void* ptr)
{
	bing_free(ptr);
}

void* xml_curl_realloc(void* ptr, size_t size)
{
	return bing_realloc(ptr, size);
}

void* xml_curl_calloc(size_t nmemb, size_t size)
{
	return bing_calloc(nmemb, size);
}

char* xml_curl_strdup(const char* str)
{
	return bing_strdup(str);
}
