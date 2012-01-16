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
//This wacky memory management system is primarily so that memory that is not the beginning of the pointer can be passed into free (as can already be done with the C-libraries free function... I think).

size_t getMemoryAllocated()
{
	/*
	size_t totalSize = 0;
	struct memory_alloc* m;

	initialize_bing();

	m = (struct memory_alloc*)memAlloc;
	while(m)
	{
		totalSize = m->endPtr - (void*)m - sizeof(struct memory_alloc); //Get the last valid memory location, the current (with header) memory location, then subtract header size
		m = m->prev;
	}
	return totalSize;
	*/
	initialize_bing();

	return memAlloc;
}

#if defined(BING_STR_MEM_TRACK)
void* malloc_dbg_track(size_t size, const char* file, int line)
#else
void* malloc_dbg_track(size_t size)
#endif
{
	/*
	struct memory_alloc* m;
	void* ret = NULL;
	if(size > 0) //Don't allocate anything that isn't greater then or equal to 1 byte
	{
		ret = malloc(size + sizeof(struct memory_alloc));
	}
	if(ret)
	{
		m = (struct memory_alloc*)ret;
		ret += sizeof(struct memory_alloc); //Make sure memory returned doesn't include header

		m->prev = (struct memory_alloc*)memAlloc;
		m->endPtr = ret + size;
#if defined(BING_STR_MEM_TRACK)
		m->file = file;
		m->line = line;
#endif
		memAlloc = m;
	}
	return ret;
	*/
	void* ret = malloc(size + sizeof(size_t));
	if(ret)
	{
		*((size_t*)ret) = size;
		memAlloc += size;
		ret += sizeof(size_t);
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
	/*
	struct memory_alloc* m;
	struct memory_alloc** d;

	void* ret = NULL;
	if(ptr)
	{
		//Find anything pointing to this memory block, because the memory address might change
		ptr -= sizeof(struct memory_alloc);
		m = (struct memory_alloc*)ptr;
		d = NULL;
		if(memAlloc == m)
		{
			d = (struct memory_alloc**)&memAlloc;
		}
		else
		{
			m = (struct memory_alloc*)memAlloc;
			while(m->prev)
			{
				if(m->prev == ptr)
				{
					d = &m->prev;
					break;
				}
				m = m->prev;
			}
		}

		//Reallocate
		ret = realloc(ptr, size);
		if(ret)
		{
			m = (struct memory_alloc*)ret;
			if(d) //If he have something pointing to this then make sure it is changed (and thus points to the correct memory location)
			{
				*d = m;
			}
			ret += sizeof(struct memory_alloc);

			m->endPtr = ret + size;
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
	*/
	if(ptr)
	{
		ptr -= sizeof(size_t);
	}
	void* ret = realloc(ptr, size + sizeof(size_t));
	if(ret)
	{
		if(ptr)
		{
			memAlloc -= *((size_t*)ret);
		}
		*((size_t*)ret) = size;
		memAlloc += size;
		ret += sizeof(size_t);
	}
	return ret;
}

void free_dbg_track(void* ptr)
{
	/*
	struct memory_alloc* m;
	struct memory_alloc* n;
	BOOL freeMem = FALSE;

	if(ptr)
	{
		ptr -= sizeof(struct memory_alloc);
		m = (struct memory_alloc*)ptr;

		//Find the "next" memory block, this way we can set it's previous to our block's previous
		if(memAlloc == m)
		{
			memAlloc = m->prev;
			freeMem = TRUE;
		}
		else
		{
			n = (struct memory_alloc*)memAlloc;
			while(n->prev)
			{
				if(n->prev == m)
				{
					n->prev = m->prev;
					freeMem = TRUE;
					break;
				}
				n = n->prev;
			}
		}

		//Only free memory if we know it (yes this can cause memory leaks)
		if(freeMem)
		{
			free(ptr);
		}
	}
	*/
	if(ptr)
	{
		ptr -= sizeof(size_t);
		memAlloc -= *((size_t*)ptr);
		free(ptr);
	}
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

void set_bing_memory_handlers(void* (*bing_malloc)(size_t), void* (*bing_calloc)(size_t,size_t), void* (*bing_realloc)(void*,size_t), void (*bing_free)(void*), char* (*bing_strdup)(const char*))
{
	//Don't do anything
}
#else
size_t getMemoryAllocated()
{
	//Unused for normal memory management
	return 0;
}

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
