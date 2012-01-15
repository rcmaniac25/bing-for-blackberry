/*
 * memory.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

#if (defined(BING_MEM_TRACK) || defined(BING_STR_MEM_TRACK)) && defined(BING_DEBUG)
size_t getMemoryAllocated()
{
	size_t totalSize = 0;
	struct memory_alloc* m;

	initialize_bing();

	m = (struct memory_alloc*)memAlloc;
	while(m)
	{
		totalSize = m->endPtr - m->ptr;
		m = m->prev;
	}
	return totalSize;
}

#if defined(BING_STR_MEM_TRACK)
void* malloc_dbg_track(size_t size, const char* file, int line)
#else
void* malloc_dbg_track(size_t size)
#endif
{
	struct memory_alloc* m;
	void* ret = malloc(size);
	if(ret)
	{
		m = (struct memory_alloc*)malloc(sizeof(struct memory_alloc*));
		if(m)
		{
			m->prev = (struct memory_alloc*)memAlloc;
			m->ptr = ret;
			m->endPtr = ret + size;
#if defined(BING_STR_MEM_TRACK)
			m->file = file;
			m->line = line;
#endif
			memAlloc = m;
		}
		else
		{
			free(ret);
			ret = NULL;
		}
	}
	return ret;
}

#if defined(BING_STR_MEM_TRACK)
void* calloc_dbg_track(size_t nmemb, size_t size, const char* file, int line)
{
	return malloc_dbg_track(nmemb * size, file, line);
}
#else
void* calloc_dbg_track(size_t nmemb, size_t size)
{
	return malloc_dbg_track(nmemb * size);
}
#endif

#if defined(BING_STR_MEM_TRACK)
void* realloc_dbg_track(void* ptr, size_t size, const char* file, int line)
#else
void* realloc_dbg_track(void* ptr, size_t size)
#endif
{
	struct memory_alloc* m;

	void* ret = NULL;
	if(ptr)
	{
		m = (struct memory_alloc*)memAlloc;
		while(m)
		{
			if(m->ptr <= ptr && m->endPtr >= ptr)
			{
				ret = realloc(ptr, size);
				if(ret)
				{
					m->ptr = ret;
					m->endPtr = ret + size;
				}
				break;
			}
			m = m->prev;
		}
	}
	else
	{
#if defined(BING_STR_MEM_TRACK)
		ret = malloc_dbg_track(size, file, line);
#else
		ret = malloc_dbg_track(size);
#endif
	}
	return ret;
}

void free_dbg_track(void* ptr)
{
	struct memory_alloc* m;
	struct memory_alloc* n;

	if(ptr)
	{
		m = n = (struct memory_alloc*)memAlloc;
		if(m)
		{
			if(m->ptr <= ptr && m->endPtr >= ptr)
			{
				memAlloc = m->prev;
				free(m);
			}
			else
			{
				m = m->prev;
				while(m)
				{
					if(m->ptr <= ptr && m->endPtr >= ptr)
					{
						if(m->prev)
						{
							n->prev = m->prev;
						}
						else
						{
							n->prev = NULL;
						}
						free(m);
						break;
					}
					m = m->prev;
					n = n->prev;
				}
			}
		}
	}
	free(ptr);
}

#if defined(BING_STR_MEM_TRACK)
char* strdup_dbg_track(const char* str, const char* file, int line)
#else
char* strdup_dbg_track(const char* str)
#endif
{
	char* ret = NULL;
	size_t size;
	if(str)
	{
#if defined(BING_STR_MEM_TRACK)
		ret = malloc_dbg_track(size = strlen(str) + 1, file, line);
#else
		ret = malloc_dbg_track(size = strlen(str) + 1);
#endif
		if(ret)
		{
			memcpy(ret, str, size);
		}
	}
	return ret;
}
#else
size_t getMemoryAllocated()
{
	return 0;
}

//TODO: Make replaceable
void* bing_malloc(size_t size)
{
	return malloc(size);
}

void* bing_calloc(size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
}

void* bing_realloc(void* ptr, size_t size)
{
	return realloc(ptr, size);
}

void bing_free(void* ptr)
{
	free(ptr);
}

char* bing_strdup(const char* str)
{
	return strdup(str);
}
#endif

//XML/cURL memory handlers (we wrap the functions so that debug functions can be used)
void* xml_curl_malloc(size_t size)
{
	return BING_MALLOC(size);
}

void xml_curl_free(void* ptr)
{
	BING_FREE(ptr);
}

void* xml_curl_realloc(void* ptr, size_t size)
{
	return BING_REALLOC(ptr, size);
}

void* xml_curl_calloc(size_t nmemb, size_t size)
{
	return BING_CALLOC(nmemb, size);
}

char* xml_curl_strdup(const char* str)
{
	return BING_STRDUP(str);
}
