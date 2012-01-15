/*
 * result.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

//Nmaes used in multiple functions
#define RES_AD_RANK "Rank"
#define RES_AD_TITLE "Title"

#define RES_ERROR_CODE "Code"
#define RES_ERROR_ERRORCODE "SourceTypeErrorCode"
#define RES_ERROR_VALUE "Value"

#define RES_IMAGE_HEIGHT "Height"
#define RES_IMAGE_WIDTH "Width"
#define RES_IMAGE_FILESIZE "FileSize"
#define RES_IMAGE_CONTENTTYPE "ContentType"
#define RES_IMAGE_URL "Url"
#define RES_IMAGE_THUMBNAIL "Thumbnail"

#define RES_VIDEO_STATICTHUMBNAIL "StaticThumbnail"

#define RES_PHONE_LAT "Latitude"
#define RES_PHONE_LONG "Longitude"
#define RES_PHONE_RATING "UserRating"
#define RES_PHONE_REVW_COUNT "ReviewCount"

#define RES_NEWS_BREAKINGNEWS "BreakingNews"
#define RES_NEWS_COLLECTION "NewsCollection"

#define RES_WEB_DEEPLINKS "DeepLink"
#define RES_WEB_SEARCHTAGS "SearchTag"

//Common name conversions
#define RES_COM_DEEPLINK_ARRAY "web:DeepLinks"
#define RES_COM_DEEPLINK_ARRAY_NAME "DeepLinkArray"
#define RES_COM_DEEPLINK "web:DeepLink"
#define RES_COM_DEEPLINK_NAME RES_WEB_DEEPLINKS
#define RES_COM_NEWS_ARRAY "news:NewsArticles"
#define RES_COM_NEWS_ARRAY_NAME "NewsArticleArray"
#define RES_COM_NEWSCOL "news:NewsCollection"
#define RES_COM_NEWSCOL_NAME "NewsCollection"
#define RES_COM_NEWSCOL_ARRAY "news:NewsCollections"
#define RES_COM_NEWSCOL_ARRAY_NAME "NewsCollectionArray"
#define RES_COM_RELSEARCH_ARRAY "news:RelatedSearches"
#define RES_COM_RELSEARCH_ARRAY_NAME "RelatedSearchArray"
#define RES_COM_RELSEARCH "news:NewsRelatedSearch"
#define RES_COM_RELSEARCH_NAME "RelatedSearch"
#define RES_COM_SEARCHTAG "web:WebSearchTag"
#define RES_COM_SEARCHTAG_NAME RES_WEB_SEARCHTAGS
#define RES_COM_SEARCHTAG_ARRAY "web:SearchTags"
#define RES_COM_SEARCHTAG_ARRAY_NAME "SearchTagArray"
#define RES_COM_THUMBNAIL "mms:Thumbnail"
#define RES_COM_STATICTHUMBNAIL "mms:StaticThumbnail"
#define RES_COM_THUMBNAIL_NAME "Thumbnail"

//Internal common names
#define RES_COM_NEWS_COLLECTION "NewsCollection"

#define RES_COM_NEWS_COL_NAME "Name"
#define RES_COM_NEWS_COL_ARTICLES "NewsArticles"

#define RES_COM_WEB_DEEPLINK_ARRAY "DeepArray"

#define RES_COM_WEB_RELATEDSEARCH RES_COM_RELSEARCH_NAME
#define RES_COM_WEB_SEARCHTAGS RES_WEB_SEARCHTAGS

//Creation/update functions

//Creation
int result_def_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	//Default response is to duplicate the dictionary
	if(!dictionary)
	{
		//If NULL, then everything is good, carry on.
		return TRUE;
	}
	return hashtable_copy(((bing_result*)result)->data, (hashtable_t*)dictionary);
}

int result_phonebook_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	hashtable_t* table;
	BOOL ret = (BOOL)result_def_create(name, result, dictionary);
	if(ret)
	{
		table = ((bing_result*)result)->data;
		ret = replace_string_with_double(table, RES_PHONE_LAT);
		ret &= replace_string_with_double(table, RES_PHONE_LONG);
		ret &= replace_string_with_double(table, RES_PHONE_RATING);
		ret &= replace_string_with_longlong(table, RES_PHONE_REVW_COUNT);
	}
	return ret;
}

int result_image_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	hashtable_t* table;
	BOOL ret = (BOOL)result_def_create(name, result, dictionary);
	if(ret)
	{
		table = ((bing_result*)result)->data;
		ret = replace_string_with_longlong(table, RES_IMAGE_HEIGHT);
		ret &= replace_string_with_longlong(table, RES_IMAGE_WIDTH);
		ret &= replace_string_with_longlong(table, RES_IMAGE_FILESIZE);
	}
	return ret;
}

int result_news_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	int strLen;
	char* str;
	int b;
	hashtable_t* table;
	BOOL ret = (BOOL)result_def_create(name, result, dictionary);
	if(ret)
	{
		table = ((bing_result*)result)->data;
		strLen = hashtable_get_string(table, RES_NEWS_BREAKINGNEWS, NULL);
		if(strLen > -1)
		{
			str = BING_MALLOC(strLen);
			if(str)
			{
				hashtable_get_string(table, RES_NEWS_BREAKINGNEWS, str);
				b = str[0] == '1';
				ret = hashtable_set_data(table, RES_NEWS_BREAKINGNEWS, &b, sizeof(int));
				BING_FREE(str);
			}
			else
			{
				ret = FALSE;
			}
		}
		ret = replace_string_with_longlong(((bing_result*)result)->data, RES_AD_RANK);
	}
	return ret;
}

int result_ad_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	BOOL ret = (BOOL)result_def_create(name, result, dictionary);
	if(ret)
	{
		ret = replace_string_with_longlong(((bing_result*)result)->data, RES_AD_RANK);
	}
	return ret;
}

int result_error_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	hashtable_t* table;
	BOOL ret = (BOOL)result_def_create(name, result, dictionary);
	if(ret)
	{
		table = ((bing_result*)result)->data;
		ret = replace_string_with_longlong(table, RES_ERROR_CODE);
		ret &= replace_string_with_longlong(table, RES_ERROR_ERRORCODE);
	}
	return ret;
}

int result_def_common_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	hashtable_t* table;
	const char* str = NULL;
	BOOL ret = (BOOL)result_def_create(name, result, dictionary);
	if(ret)
	{
		//We need to set expected names for the element
		table = ((bing_result*)result)->data;
		if(strcmp(name, RES_COM_DEEPLINK_ARRAY) == 0)
		{
			str = RES_COM_DEEPLINK_ARRAY_NAME;
		}
		else if(strcmp(name, RES_COM_DEEPLINK) == 0)
		{
			str = RES_COM_DEEPLINK_NAME;
		}
		else if(strcmp(name, RES_COM_NEWS_ARRAY) == 0)
		{
			str = RES_COM_NEWS_ARRAY_NAME;
		}
		else if(strcmp(name, RES_COM_NEWSCOL) == 0)
		{
			str = RES_COM_NEWSCOL_NAME;
		}
		else if(strcmp(name, RES_COM_NEWSCOL_ARRAY) == 0)
		{
			str = RES_COM_NEWSCOL_ARRAY_NAME;
		}
		else if(strcmp(name, RES_COM_RELSEARCH_ARRAY) == 0)
		{
			str = RES_COM_RELSEARCH_ARRAY_NAME;
		}
		else if(strcmp(name, RES_COM_RELSEARCH) == 0)
		{
			str = RES_COM_RELSEARCH_NAME;
		}
		else if(strcmp(name, RES_COM_SEARCHTAG) == 0)
		{
			str = RES_COM_SEARCHTAG_NAME;
		}
		else if(strcmp(name, RES_COM_SEARCHTAG_ARRAY) == 0)
		{
			str = RES_COM_SEARCHTAG_ARRAY_NAME;
		}
		else if(strcmp(name, RES_COM_THUMBNAIL) == 0 || strcmp(name, RES_COM_STATICTHUMBNAIL) == 0)
		{
			str = RES_COM_THUMBNAIL_NAME;
		}
		if(str)
		{
			hashtable_set_data(table, BING_RESULT_COMMON_TYPE, str, strlen(str) + 1);
		}
	}
	return ret;
}

int result_common_thumbnail_create(const char* name, bing_result_t result, data_dictionary_t dictionary)
{
	hashtable_t* table;
	BOOL ret = (BOOL)result_def_common_create(name, result, dictionary);
	if(ret)
	{
		table = ((bing_result*)result)->data;
		ret = replace_string_with_longlong(table, RES_IMAGE_HEIGHT);
		ret &= replace_string_with_longlong(table, RES_IMAGE_WIDTH);
		ret &= replace_string_with_longlong(table, RES_IMAGE_FILESIZE);
	}
	return ret;
}

//Additional results
void result_def_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
}

void result_additional_result_helper(bing_result_t result, bing_result_t new_result, const char* commonType, void* data, void (*specificProcessing)(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data))
{
	int size;
	char* str;
	bing_result* res = (bing_result*)new_result;
	bing_response* pres = ((bing_result*)result)->parent;
	if(res->type == BING_RESULT_COMMON) //Is this a "common" type?
	{
		size = hashtable_get_string(res->data, BING_RESULT_COMMON_TYPE, NULL);
		if(size > 0) //Does it contain a common type?
		{
			str = BING_MALLOC(size);
			if(str)
			{
				hashtable_get_string(res->data, BING_RESULT_COMMON_TYPE, str);
				if(strcmp(str, commonType) == 0) //Is it the correct common type?
				{
					BING_FREE(str);

					specificProcessing(((bing_result*)result)->data, res->data, pres, data);
				}
				else
				{
					BING_FREE(str);
				}
			}
		}
	}
}

void copyArray(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	const char* name = (char*)data;
	int size = hashtable_get_string(new_resultData, name, NULL);
	if(size > 0)
	{
		data = BING_MALLOC(size); //We want a new item, so we add one to an existing array or create a new one
		hashtable_get_string(new_resultData, name, (char*)data); //If the collection doesn't exist, then nothing happens

		//Save the array
		hashtable_set_data(resultData, name, data, size);

		BING_FREE(data);
	}
}

void result_web_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_DEEPLINK_ARRAY_NAME, RES_COM_WEB_DEEPLINK_ARRAY, copyArray);
	result_additional_result_helper(result, new_result, RES_COM_SEARCHTAG_ARRAY_NAME, RES_COM_WEB_SEARCHTAGS, copyArray);
}

void loadThumbnail(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	int size;
	char* str;

	//Convert to thumbnail
	bing_thumbnail_t thumbnail = (bing_thumbnail_t)BING_MALLOC(sizeof(bing_thumbnail_s));
	if(thumbnail)
	{
		//Setup the thumbnail
		size = hashtable_get_string(new_resultData, RES_IMAGE_URL, NULL);
		if(size > 0)
		{
			thumbnail->url = str = allocateMemory(size, pres);
			if(str)
			{
				hashtable_get_string(new_resultData, RES_IMAGE_URL, str);
			}
		}
		else
		{
			thumbnail->url = NULL;
		}

		size = hashtable_get_string(new_resultData, RES_IMAGE_CONTENTTYPE, NULL);
		if(size > 0)
		{
			thumbnail->content_type = str = allocateMemory(size, pres);
			if(str)
			{
				hashtable_get_string(new_resultData, RES_IMAGE_CONTENTTYPE, str);
			}
		}
		else
		{
			thumbnail->content_type = NULL;
		}

		hashtable_get_data_key(new_resultData, RES_IMAGE_HEIGHT, &thumbnail->height, sizeof(long long));

		hashtable_get_data_key(new_resultData, RES_IMAGE_WIDTH, &thumbnail->width, sizeof(long long));

		hashtable_get_data_key(new_resultData, RES_IMAGE_FILESIZE, &thumbnail->file_size, sizeof(long long));

		//Save the thumbnail
		hashtable_set_data(resultData, (const char*)data, thumbnail, sizeof(bing_thumbnail_s));

		//Free the thumbnail
		BING_FREE(thumbnail);
	}
}

void result_video_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_THUMBNAIL_NAME, RES_VIDEO_STATICTHUMBNAIL, loadThumbnail);
}

void result_image_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_THUMBNAIL_NAME, RES_IMAGE_THUMBNAIL, loadThumbnail);
}

void addNewsArray(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	//Would perfer to use "copyArray" but it doesn't support different src and dst
	int size = hashtable_get_string(new_resultData, RES_COM_NEWS_COLLECTION, NULL);
	if(size > 0)
	{
		data = BING_MALLOC(size); //We want a new item, so we add one to an existing array or create a new one
		hashtable_get_string(new_resultData, RES_COM_NEWS_COLLECTION, (char*)data); //If the collection doesn't exist, then nothing happens

		//Save the array
		hashtable_set_data(resultData, RES_NEWS_COLLECTION, data, size);

		BING_FREE(data);
	}
}

void result_news_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_NEWSCOL_ARRAY_NAME, NULL, addNewsArray);
}

void addDeepLink(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	const char* name = (const char*)data;
	bing_deep_link_t deeplinks;
	int i;
	int size = hashtable_get_string(resultData, name, NULL);

	deeplinks = (bing_deep_link_t)BING_MALLOC(size = max(size + sizeof(bing_deep_link_s), sizeof(bing_deep_link_s))); //We want a new item, so we add one to an existing array or create a new one
	hashtable_get_string(resultData, name, (char*)deeplinks); //If the collection doesn't exist, then nothing happens

	i = max(size / sizeof(bing_deep_link_s), 1) - 1; //Get the new element index (remember, we increased the element size at malloc)

	//Get the deep link
	size = hashtable_get_string(new_resultData, RES_AD_TITLE, NULL);
	deeplinks[i].title = allocateMemory(size, pres);
	hashtable_get_string(new_resultData, RES_AD_TITLE, (char*)deeplinks[i].title);

	size = hashtable_get_string(new_resultData, RES_IMAGE_URL, NULL);
	deeplinks[i].url = allocateMemory(size, pres);
	hashtable_get_string(new_resultData, RES_IMAGE_URL, (char*)deeplinks[i].url);

	//Save the element (it will overwrite the old array)
	hashtable_set_data(resultData, name, deeplinks, (i + 1) * sizeof(bing_deep_link_s));

	BING_FREE(deeplinks);
}

void result_common_deeplink_array_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_DEEPLINK_NAME, RES_COM_WEB_DEEPLINK_ARRAY, addDeepLink);
}

void result_common_news_array_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	int size;
	int i;
	char* str;
	bing_result_t* articles;
	bing_result* res = (bing_result*)new_result;
	if(res->type == BING_SOURCETYPE_NEWS) //Is this a "common" type?
	{
		//Get the array and size
		size = hashtable_get_string(((bing_result*)result)->data, RES_COM_NEWS_COL_ARTICLES, NULL);

		articles = (bing_result_t*)BING_MALLOC(size = max(size + sizeof(bing_result_t), sizeof(bing_result_t))); //We want a new item, so we add one to an existing array or create a new one
		hashtable_get_string(((bing_result*)result)->data, RES_COM_NEWS_COL_ARTICLES, (char*)articles); //If the collection doesn't exist, then nothing happens

		i = max(size / sizeof(bing_result_t), 1) - 1; //Get the new element index (remember, we increased the element size at malloc)

		//Add the new array element
		articles[i] = new_result;

		//Save the element (it will overwrite the old array)
		hashtable_set_data(((bing_result*)result)->data, RES_COM_NEWS_COL_ARTICLES, articles, size);

		BING_FREE(articles);
	}
	keepResult[0] = TRUE;
}

void result_common_news_col_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_NEWS_ARRAY_NAME, RES_COM_NEWS_COL_ARTICLES, copyArray);
}

void addNewsCollection(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	int i;
	bing_news_collection_t col;
	int size = hashtable_get_string(resultData, RES_COM_NEWS_COLLECTION, NULL);

	col = (bing_news_collection_t)BING_MALLOC(size = max(size + sizeof(bing_news_collection_s), sizeof(bing_news_collection_s))); //We want a new item, so we add one to an existing array or create a new one
	hashtable_get_string(resultData, RES_COM_NEWS_COLLECTION, (char*)col); //If the collection doesn't exist, then nothing happens

	i = max(size / sizeof(bing_news_collection_s), 1) - 1; //Get the new element index (remember, we increased the element size at malloc)

	//First get the name
	size = hashtable_get_string(new_resultData, RES_COM_NEWS_COL_NAME, NULL);
	if(size > 0)
	{
		col[i].name = allocateMemory(size, pres);
		hashtable_get_string(new_resultData, RES_COM_NEWS_COL_NAME, (char*)col[i].name);
	}
	else
	{
		col[i].name = NULL;
	}

	//Now get the array
	col[i].news_article_count = hashtable_get_string(new_resultData, RES_COM_NEWS_COL_ARTICLES, NULL) / sizeof(bing_result_t);
	col[i].news_articles = allocateMemory(col[i].news_article_count * sizeof(bing_result_t), pres);
	hashtable_get_string(new_resultData, RES_COM_NEWS_COL_ARTICLES, (char*)col[i].news_articles);

	//Save the element (it will overwrite the old array)
	hashtable_set_data(resultData, RES_COM_NEWS_COLLECTION, col, (i + 1) * sizeof(bing_news_collection_s));

	BING_FREE(col);
}

void result_common_news_col_array_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_NEWSCOL_NAME, NULL, addNewsCollection);
}

void result_common_related_search_array_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	//DeepLink and RelatedSearch are the same (just different names and uses)
	result_additional_result_helper(result, new_result, RES_COM_RELSEARCH_NAME, RES_COM_WEB_RELATEDSEARCH, addDeepLink);
}

void addSearchTag(hashtable_t* resultData, hashtable_t* new_resultData, bing_response* pres, void* data)
{
	bing_search_tag_t tags;
	int i;
	int size = hashtable_get_string(resultData, RES_COM_WEB_SEARCHTAGS, NULL);

	tags = (bing_search_tag_t)BING_MALLOC(size = max(size + sizeof(bing_search_tag_s), sizeof(bing_search_tag_s))); //We want a new item, so we add one to an existing array or create a new one
	hashtable_get_string(resultData, RES_COM_WEB_SEARCHTAGS, (char*)tags); //If the collection doesn't exist, then nothing happens

	i = max(size / sizeof(bing_search_tag_s), 1) - 1; //Get the new element index (remember, we increased the element size at malloc)

	//Get the search tag
	size = hashtable_get_string(new_resultData, RES_COM_NEWS_COL_NAME, NULL);
	tags[i].name = allocateMemory(size, pres);
	hashtable_get_string(new_resultData, RES_COM_NEWS_COL_NAME, (char*)tags[i].name);

	size = hashtable_get_string(new_resultData, RES_ERROR_VALUE, NULL);
	tags[i].value = allocateMemory(size, pres);
	hashtable_get_string(new_resultData, RES_ERROR_VALUE, (char*)tags[i].value);

	//Save the element (it will overwrite the old array)
	hashtable_set_data(resultData, RES_COM_WEB_SEARCHTAGS, tags, (i + 1) * sizeof(bing_search_tag_s));

	BING_FREE(tags);
}

void result_common_search_tag_array_additional_result(const char* name, bing_result_t result, bing_result_t new_result, int* keepResult)
{
	result_additional_result_helper(result, new_result, RES_COM_SEARCHTAG_NAME, NULL, addSearchTag);
}

//Search structure

typedef struct BING_RESULT_CREATOR_SEARCH_S
{
	bing_result_creator creator;
	enum SOURCE_TYPE type;
	int tableCount;
	struct BING_RESULT_CREATOR_SEARCH_S* next;
} bing_result_creator_search;

static bing_result_creator_search result_def_creator[]=
{
		//Results
		{{"web:WebResult",			FALSE,	FALSE,	result_def_create,			result_web_additional_result},		BING_SOURCETYPE_WEB,				8,	&result_def_creator[1]},
		{{"pho:PhonebookResult",	FALSE,	FALSE,	result_phonebook_create,	result_def_additional_result},		BING_SOURCETYPE_PHONEBOOK,			16,	&result_def_creator[2]},
		{{"mms:VideoResult",		FALSE,	FALSE,	result_def_create,			result_video_additional_result},	BING_SOURCETYPE_VIDEO, 				6,	&result_def_creator[3]},
		{{"mms:ImageResult",		FALSE,	FALSE,	result_image_create,		result_image_additional_result},	BING_SOURCETYPE_IMAGE,				9,	&result_def_creator[4]},
		{{"news:NewsResult",		FALSE,	FALSE,	result_news_create,			result_news_additional_result},		BING_SOURCETYPE_NEWS,				7,	&result_def_creator[5]},
		{{"news:NewsArticle",		FALSE,	TRUE,	result_news_create,			result_news_additional_result},		BING_SOURCETYPE_NEWS,				7,	&result_def_creator[6]}, //This is a special extra, same as news type but with different name
		{{"ads:AdResult",			FALSE,	FALSE,	result_ad_create,			result_def_additional_result},		BING_SOURCETYPE_AD, 				6,	&result_def_creator[7]},
		{{"rs:RelatedSearchResult",	FALSE,	FALSE,	result_def_create,			result_def_additional_result},		BING_SOURCETYPE_RELATED_SEARCH,		2,	&result_def_creator[8]},
		{{"tra:TranslationResult",	FALSE,	FALSE,	result_def_create,			result_def_additional_result},		BING_SOURCETYPE_TRANSLATION,		1,	&result_def_creator[9]},
		{{"spl:SpellResult",		FALSE,	FALSE,	result_def_create,			result_def_additional_result},		BING_SOURCETYPE_SPELL,				1,	&result_def_creator[10]},
		{{"mw:MobileWebResult",		FALSE,	FALSE,	result_def_create,			result_def_additional_result},		BING_SOURCETYPE_MOBILE_WEB,			5,	&result_def_creator[11]},
		{{"ia:InstantAnswerResult",	FALSE,	FALSE,	result_def_create,			result_def_additional_result},		BING_SOURCETYPE_INSTANT_ANWSER,		5,	&result_def_creator[12]},
		{{"Error",					FALSE,	FALSE,	result_error_create,		result_def_additional_result},		BING_RESULT_ERROR,					7,	&result_def_creator[13]},

		//Common
		{{RES_COM_DEEPLINK_ARRAY,	TRUE,	TRUE,	result_def_common_create,		result_common_deeplink_array_additional_result},		BING_RESULT_COMMON,	2,	&result_def_creator[14]},
		{{RES_COM_DEEPLINK,			FALSE,	TRUE,	result_def_common_create,		result_def_additional_result},							BING_RESULT_COMMON,	3,	&result_def_creator[15]},
		{{RES_COM_NEWS_ARRAY,		TRUE,	TRUE,	result_def_common_create,		result_common_news_array_additional_result},			BING_RESULT_COMMON,	2,	&result_def_creator[16]},
		{{RES_COM_NEWSCOL,			TRUE,	TRUE,	result_def_common_create,		result_common_news_col_additional_result},				BING_RESULT_COMMON,	3,	&result_def_creator[17]},
		{{RES_COM_NEWSCOL_ARRAY,	TRUE,	TRUE,	result_def_common_create,		result_common_news_col_array_additional_result},		BING_RESULT_COMMON,	2,	&result_def_creator[18]},
		{{RES_COM_RELSEARCH_ARRAY,	TRUE,	TRUE,	result_def_common_create,		result_common_related_search_array_additional_result},	BING_RESULT_COMMON,	2,	&result_def_creator[19]},
		{{RES_COM_RELSEARCH,		FALSE,	TRUE,	result_def_common_create,		result_def_additional_result},							BING_RESULT_COMMON,	3,	&result_def_creator[20]},
		{{RES_COM_SEARCHTAG,		FALSE,	TRUE,	result_def_common_create,		result_def_additional_result},							BING_RESULT_COMMON,	3,	&result_def_creator[21]},
		{{RES_COM_SEARCHTAG_ARRAY,	TRUE,	TRUE,	result_def_common_create,		result_common_search_tag_array_additional_result},		BING_RESULT_COMMON,	2,	&result_def_creator[22]},
		{{RES_COM_THUMBNAIL,		FALSE,	TRUE,	result_common_thumbnail_create,	result_def_additional_result},							BING_RESULT_COMMON,	6,	&result_def_creator[23]},
		{{RES_COM_STATICTHUMBNAIL,	FALSE,	TRUE,	result_common_thumbnail_create,	result_def_additional_result},							BING_RESULT_COMMON,	6,	NULL}
};

static bing_field_search result_fields[] =
{
		//Ad
		{{RESULT_FIELD_RANK,							FIELD_TYPE_LONG,	RES_AD_RANK,					1,	{BING_SOURCETYPE_AD}},						&result_fields[1]},
		{{RESULT_FIELD_POSITION,						FIELD_TYPE_STRING,	"Position",						1,	{BING_SOURCETYPE_AD}},						&result_fields[2]},
		{{RESULT_FIELD_TITLE,							FIELD_TYPE_STRING,	RES_AD_TITLE,					9,	{BING_SOURCETYPE_AD, BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER, BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},																&result_fields[3]},
		{{RESULT_FIELD_DESCRIPTION,						FIELD_TYPE_STRING,	"Description",					3,	{BING_SOURCETYPE_AD,
				BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_WEB}},																							&result_fields[4]},
		{{RESULT_FIELD_DISPLAY_URL,						FIELD_TYPE_STRING,	"DisplayUrl",					5,	{BING_SOURCETYPE_AD, BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_WEB}},																&result_fields[5]},
		{{RESULT_FIELD_ADLINK_URL,						FIELD_TYPE_STRING,	"AdlinkURL",					1,	{BING_SOURCETYPE_AD}},						&result_fields[6]},

		//Error
		{{RESULT_FIELD_CODE,							FIELD_TYPE_LONG,	RES_ERROR_CODE,					1,	{BING_RESULT_ERROR}},						&result_fields[7]},
		{{RESULT_FIELD_MESSAGE,							FIELD_TYPE_STRING,	"Message",						1,	{BING_RESULT_ERROR}},						&result_fields[8]},
		{{RESULT_FIELD_HELP_URL,						FIELD_TYPE_STRING,	"HelpUrl",						1,	{BING_RESULT_ERROR}},						&result_fields[9]},
		{{RESULT_FIELD_PARAMETER,						FIELD_TYPE_STRING,	"Parameter",					1,	{BING_RESULT_ERROR}},						&result_fields[10]},
		{{RESULT_FIELD_SOURCE_TYPE,						FIELD_TYPE_STRING,	"SourceType",					1,	{BING_RESULT_ERROR}},						&result_fields[11]},
		{{RESULT_FIELD_SOURCE_TYPE_ERROR_CODE,			FIELD_TYPE_LONG,	RES_ERROR_ERRORCODE,			1,	{BING_RESULT_ERROR}},						&result_fields[12]},
		{{RESULT_FIELD_VALUE,							FIELD_TYPE_STRING,	RES_ERROR_VALUE,				2,	{BING_RESULT_ERROR, BING_SOURCETYPE_SPELL}},&result_fields[13]},

		//Image
		{{RESULT_FIELD_HEIGHT,							FIELD_TYPE_LONG,	RES_IMAGE_HEIGHT,				1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[14]},
		{{RESULT_FIELD_WIDTH,							FIELD_TYPE_LONG,	RES_IMAGE_WIDTH,				1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[15]},
		{{RESULT_FIELD_FILE_SIZE,						FIELD_TYPE_LONG,	RES_IMAGE_FILESIZE,				1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[16]},
		{{RESULT_FIELD_MEDIA_URL,						FIELD_TYPE_STRING,	"MediaUrl",						1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[17]},
		{{RESULT_FIELD_URL,								FIELD_TYPE_STRING,	RES_IMAGE_URL,					7,	{BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER, BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_WEB}},																						&result_fields[18]},
		{{RESULT_FIELD_CONTENT_TYPE,					FIELD_TYPE_STRING,	RES_IMAGE_CONTENTTYPE,			2,	{BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER}},																											&result_fields[19]},
		{{RESULT_FIELD_THUMBNAIL,						FIELD_TYPE_ARRAY,	RES_IMAGE_THUMBNAIL,			1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[20]},

		//InstantAnswer
		{{RESULT_FIELD_ATTRIBUTION,						FIELD_TYPE_STRING,	"Attribution",					1,	{BING_SOURCETYPE_INSTANT_ANWSER}},			&result_fields[21]},
		{{RESULT_FIELD_INSTANT_ANWSER_SPECIFIC_DATA,	FIELD_TYPE_STRING,	"InstantAnswerSpecificData",	1,	{BING_SOURCETYPE_INSTANT_ANWSER}},			&result_fields[22]},

		//MobileWeb
		{{RESULT_FIELD_DATE_TIME,						FIELD_TYPE_STRING,	"DateTime",						2,	{BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_WEB}},																														&result_fields[23]},

		//News
		{{RESULT_FIELD_BREAKING_NEWS,					FIELD_TYPE_BOOLEAN,	RES_NEWS_BREAKINGNEWS,			1,	{BING_SOURCETYPE_NEWS}},					&result_fields[24]},
		{{RESULT_FIELD_DATE,							FIELD_TYPE_STRING,	"Date",							1,	{BING_SOURCETYPE_NEWS}},					&result_fields[25]},
		{{RESULT_FIELD_SNIPPET,							FIELD_TYPE_STRING,	"Snippet",						1,	{BING_SOURCETYPE_NEWS}},					&result_fields[26]},
		{{RESULT_FIELD_SOURCE,							FIELD_TYPE_STRING,	"Source",						1,	{BING_SOURCETYPE_NEWS}},					&result_fields[27]},
		{{RESULT_FIELD_NEWSCOLLECTION,					FIELD_TYPE_ARRAY,	RES_NEWS_COLLECTION,			1,	{BING_SOURCETYPE_NEWS}},					&result_fields[28]},

		//PhoneBook
		{{RESULT_FIELD_LATITUDE,						FIELD_TYPE_DOUBLE,	RES_PHONE_LAT,					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[29]},
		{{RESULT_FIELD_LONGITUDE,						FIELD_TYPE_DOUBLE,	RES_PHONE_LONG,					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[30]},
		{{RESULT_FIELD_USER_RATING,						FIELD_TYPE_DOUBLE,	RES_PHONE_RATING,				1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[31]},
		{{RESULT_FIELD_REVIEW_COUNT,					FIELD_TYPE_LONG,	RES_PHONE_REVW_COUNT,			1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[32]},
		{{RESULT_FIELD_BUSINESS_URL,					FIELD_TYPE_STRING,	"BusinessUrl",					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[33]},
		{{RESULT_FIELD_CITY,							FIELD_TYPE_STRING,	"City",							1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[34]},
		{{RESULT_FIELD_COUNTRY_OR_REGION,				FIELD_TYPE_STRING,	"CountryOrRegion",				1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[35]},
		{{RESULT_FIELD_PHONE_NUMBER,					FIELD_TYPE_STRING,	"PhoneNumber",					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[36]},
		{{RESULT_FIELD_POSTAL_CODE,						FIELD_TYPE_STRING,	"PostalCode",					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[37]},
		{{RESULT_FIELD_STATE_OR_PROVINCE,				FIELD_TYPE_STRING,	"StateOrProvince",				1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[38]},
		{{RESULT_FIELD_UNIQUE_ID,						FIELD_TYPE_STRING,	"UniqueId",						1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[39]},
		{{RESULT_FIELD_BUSINESS,						FIELD_TYPE_STRING,	"Business",						1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[40]},
		{{RESULT_FIELD_ADDRESS,							FIELD_TYPE_STRING,	"Address",						1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[41]},

		//Translation
		{{RESULT_FIELD_TRANSLATED_TERM,					FIELD_TYPE_STRING,	"TranslatedTerm",				1,	{BING_SOURCETYPE_TRANSLATION}},				&result_fields[42]},

		//Video
		{{RESULT_FIELD_SOURCE_TITLE,					FIELD_TYPE_STRING,	"SourceTitle",					1,	{BING_SOURCETYPE_VIDEO}},					&result_fields[43]},
		{{RESULT_FIELD_RUN_TIME,						FIELD_TYPE_STRING,	"RunTime",						1,	{BING_SOURCETYPE_VIDEO}},					&result_fields[44]},
		{{RESULT_FIELD_PLAY_URL,						FIELD_TYPE_STRING,	"PlayUrl",						1,	{BING_SOURCETYPE_VIDEO}},					&result_fields[45]},
		{{RESULT_FIELD_CLICK_THROUGH_PAGE_URL,			FIELD_TYPE_STRING,	"ClickThroughPageUrl",			1,	{BING_SOURCETYPE_VIDEO}},					&result_fields[46]},
		{{RESULT_FIELD_STATIC_THUMBNAIL,				FIELD_TYPE_ARRAY,	RES_VIDEO_STATICTHUMBNAIL,		1,	{BING_SOURCETYPE_VIDEO}},					&result_fields[47]},

		//Web
		{{RESULT_FIELD_CACHE_URL,						FIELD_TYPE_STRING,	"CacheUrl",						1,	{BING_SOURCETYPE_WEB}},						&result_fields[48]},
		{{RESULT_FIELD_DEEP_LINKS,						FIELD_TYPE_ARRAY,	RES_WEB_DEEPLINKS,				1,	{BING_SOURCETYPE_WEB}},						&result_fields[49]},
		{{RESULT_FIELD_SEARCH_TAGS,						FIELD_TYPE_ARRAY,	RES_WEB_SEARCHTAGS,				1,	{BING_SOURCETYPE_WEB}},						NULL}
};

//Functions

enum SOURCE_TYPE result_get_source_type(bing_result_t result)
{
	enum SOURCE_TYPE t = BING_SOURCETYPE_UNKNOWN;
	bing_result* res;
	if(result)
	{
		res = (bing_result*)result;
		t = res->type;
	}
	return t;
}

int result_is_field_supported(bing_result_t result, enum RESULT_FIELD field)
{
	BOOL ret = FALSE;
	bing_result* res;
	const char* key;
	if(result)
	{
		//Get the key
		res = (bing_result*)result;
		key = find_field(result_fields, field, FIELD_TYPE_UNKNOWN, res->type, FALSE);

		//Determine if the key is within the result
		ret = hashtable_key_exists(res->data, key) != -1;
	}
	return ret;
}

void free_result(bing_result* result)
{
	if(result)
	{
		hashtable_free(result->data);
		BING_FREE(result);
	}
}

BOOL result_create(enum SOURCE_TYPE type, bing_result_t* result, bing_response* responseParent, BOOL array, result_creation_func creation, result_additional_result_func additionalResult, int tableSize)
{
	BOOL ret = FALSE;
	bing_result* res;
	if(result && responseParent)
	{
		res = (bing_result*)BING_MALLOC(sizeof(bing_result));
		if(res)
		{
			//Set variables
			res->type = type;
			res->array = array;
			res->creation = creation;
			res->additionalResult = additionalResult;

			res->data = hashtable_create(tableSize);
			if(res->data)
			{
				//Add result to response
				if(response_add_result(responseParent, res, RESULT_CREATE_DEFAULT_INTERNAL))
				{
					//Save the result
					res->parent = responseParent;
					result[0] = res;
					ret = TRUE;
				}
				else
				{
					hashtable_free(res->data);
					BING_FREE(res);
				}
			}
			else
			{
				BING_FREE(res);
			}
		}
	}
	return ret;
}

BOOL result_is_common(const char* type)
{
	bing_result_creator_search* cr;
	int i;

	//Check if it is common so it is a nice easy check, before we do the longer-to-run name comparison.

	//First check built in types
	for(cr = result_def_creator; cr != NULL; cr = cr->next)
	{
		if(cr->creator.common && strcmp(type, cr->creator.name) == 0)
		{
			return TRUE;
		}
	}

	//Next check for custom types
	pthread_mutex_lock(&bingSystem.mutex);

	for(i = 0; i < bingSystem.bingResultCreatorCount; i++)
	{
		if(bingSystem.bingResultCreators[i].common && strcmp(type, bingSystem.bingResultCreators[i].name) == 0)
		{
			return TRUE;
		}
	}

	pthread_mutex_unlock(&bingSystem.mutex);

	//Not found
	return FALSE;
}

BOOL result_create_raw(const char* type, bing_result_t* result, bing_response* responseParent)
{
	BOOL ret = FALSE;
	bing_result_creator_search* cr;
	result_creation_func creationFunc;
	result_additional_result_func additionalResultFunc;
	BOOL array;
	int i;
	if(type && result && responseParent)
	{
		//Check default options
		for(cr = result_def_creator; cr != NULL; cr = cr->next)
		{
			if(strcmp(type, cr->creator.name) == 0)
			{
				break;
			}
		}
		if(cr)
		{
			//Create default options
			ret = result_create(cr->type, result, responseParent, cr->creator.array, cr->creator.creation, cr->creator.additionalResult, cr->tableCount);
		}
		else
		{
			creationFunc = NULL;
			additionalResultFunc = NULL;

			//Search custom creators
			pthread_mutex_lock(&bingSystem.mutex);

			for(i = 0; i < bingSystem.bingResultCreatorCount; i++)
			{
				if(strcmp(type, bingSystem.bingResultCreators[i].name) == 0)
				{
					creationFunc = bingSystem.bingResultCreators[i].creation;
					additionalResultFunc = bingSystem.bingResultCreators[i].additionalResult;
					array = bingSystem.bingResultCreators[i].array;
					break;
				}
			}

			pthread_mutex_unlock(&bingSystem.mutex);

			if(creationFunc)
			{
				//Create the custom result
				if(!additionalResultFunc)
				{
					//The "do nothing" function
					additionalResultFunc = result_def_additional_result;
				}
				ret = result_create(BING_SOURCETYPE_CUSTOM, result, responseParent, array, creationFunc, additionalResultFunc, -1);
			}
		}
	}
	return ret;
}

int result_get_data(bing_result_t result, enum RESULT_FIELD field, enum FIELD_TYPE type, void* value, size_t size)
{
	BOOL ret = FALSE;
	const char* key;
	if(result && value)
	{
		//Get the key
		key = find_field(result_fields, field, type, result_get_source_type(result), TRUE);

		//Now get the data
		ret = hashtable_get_data_key(((bing_result*)result)->data, key, value, size);
	}
	return ret;
}

int result_get_str_data(bing_result_t result, enum RESULT_FIELD field, enum FIELD_TYPE type, char* value)
{
	int ret = -1;
	const char* key;
	if(result)
	{
		//Get the key
		key = find_field(result_fields, field, type, result_get_source_type(result), TRUE);

		//Now get the data
		ret = result_custom_get_string(result, key, value);
	}
	return ret;
}

int result_get_64bit_int(bing_result_t result, enum RESULT_FIELD field, long long* value)
{
	return result_get_data(result, field, FIELD_TYPE_LONG, value, sizeof(long long));
}

int result_get_string(bing_result_t result, enum RESULT_FIELD field, char* value)
{
	return result_get_str_data(result, field, FIELD_TYPE_STRING, value);
}

int result_get_double(bing_result_t result, enum RESULT_FIELD field, double* value)
{
	return result_get_data(result, field, FIELD_TYPE_DOUBLE, value, sizeof(double));
}

int result_get_boolean(bing_result_t result, enum RESULT_FIELD field, int* value)
{
	return result_get_data(result, field, FIELD_TYPE_BOOLEAN, value, sizeof(int));
}

int result_get_array(bing_result_t result, enum RESULT_FIELD field, void* value)
{
	return result_get_str_data(result, field, FIELD_TYPE_ARRAY, value);
}

int result_custom_is_field_supported(bing_result_t result, const char* field)
{
	BOOL ret = FALSE;
	bing_result* res;
	if(result && field)
	{
		res = (bing_result*)result;
		ret = hashtable_key_exists(res->data, field) != -1;
	}
	return ret;
}

int result_custom_get_64bit_int(bing_result_t result, const char* field, long long* value)
{
	return hashtable_get_data_key(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(long long));
}

int result_custom_get_string(bing_result_t result, const char* field, char* value)
{
	return hashtable_get_string(result ? ((bing_result*)result)->data : NULL, field, value);
}

int result_custom_get_double(bing_result_t result, const char* field, double* value)
{
	return result_custom_get_64bit_int(result, field, (long long*)value);
}

int result_custom_get_boolean(bing_result_t result, const char* field, int* value)
{
	return hashtable_get_data_key(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(int));
}

int result_custom_get_array(bing_result_t result, const char* field, void* value)
{
	return result_custom_get_string(result, field, (char*)value);
}

int result_custom_set_64bit_int(bing_result_t result, const char* field, long long* value)
{
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(long long));
}

int result_custom_set_string(bing_result_t result, const char* field, const char* value)
{
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, value ? (strlen(value) + 1) : 0);
}

int result_custom_set_double(bing_result_t result, const char* field, double* value)
{
	return result_custom_set_64bit_int(result, field, (long long*)value);
}

int result_custom_set_boolean(bing_result_t result, const char* field, int* value)
{
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, sizeof(int));
}

int result_custom_set_array(bing_result_t result, const char* field, const void* value, size_t size)
{
	//This could be a safety hazard but we have no way of checking the size of the data passed in
	return hashtable_set_data(result ? ((bing_result*)result)->data : NULL, field, value, size);
}

void* result_custom_allocation(bing_result_t result, size_t size)
{
	void* ret = NULL;
	bing_result* res;
	if(result)
	{
		res = (bing_result*)result;
		if(res->type == BING_SOURCETYPE_CUSTOM && //We only want to allocate memory for custom types
				(size <= (5 * 1024) && size >= 1)) //We want it to be within a small range since multiple results can be allocated for the same response.
		{
			ret = allocateMemory(size, res->parent);
		}
	}
	return ret;
}

int result_register_result_creator(const char* name, int array, int common, result_creation_func creation_func, result_additional_result_func additional_func)
{
	BOOL ret = FALSE;
	BOOL cont = TRUE;
	unsigned int i;
	bing_result_creator* c;
	bing_result_creator_search* cr;
	char* nName;
	size_t size;
	if(name && creation_func)
	{
		//Check if the name is a standard supported name, if so return
		for(cr = result_def_creator; cr != NULL; cr = cr->next)
		{
			if(strcmp(name, cr->creator.name) == 0)
			{
				cont = FALSE;
				break;
			}
		}

		if(cont)
		{
			pthread_mutex_lock(&bingSystem.mutex);

			//Go through all registered creators and make sure it doesn't already exist
			i = 0;
			while(i < bingSystem.bingResultCreatorCount && cont)
			{
				if(strcmp(bingSystem.bingResultCreators[i++].name, name) == 0)
				{
					cont = FALSE;
				}
			}

			if(cont)
			{
				//Reproduce the name
				size = strlen(name) + 1;
				nName = BING_MALLOC(size);

				if(nName)
				{
					strlcpy(nName, name, size);

					//Create the new version of the name
					c = (bing_result_creator*)BING_REALLOC(bingSystem.bingResultCreators, sizeof(bing_result_creator) * (bingSystem.bingResultCreatorCount + 1));

					if(c)
					{
						bingSystem.bingResultCreators = c;

						c[bingSystem.bingResultCreatorCount].name = nName;
						c[bingSystem.bingResultCreatorCount].array = (BOOL)array;
						c[bingSystem.bingResultCreatorCount].common = (BOOL)common;
						c[bingSystem.bingResultCreatorCount].creation = creation_func;
						c[bingSystem.bingResultCreatorCount++].additionalResult = additional_func;

						ret = TRUE;
					}
					else
					{
						BING_FREE(nName);
					}
				}
			}

			pthread_mutex_unlock(&bingSystem.mutex);
		}
	}
	return ret;
}

int result_unregister_result_creator(const char* name)
{
	BOOL ret = FALSE;
	unsigned int i;
	bing_result_creator* c;
	if(name && bingSystem.bingResultCreatorCount > 0) //We don't want to run if there is nothing to run on
	{
		pthread_mutex_lock(&bingSystem.mutex);

		//Find the result
		i = 0;
		while(i < bingSystem.bingResultCreatorCount)
		{
			if(strcmp(bingSystem.bingResultCreators[i].name, name) == 0)
			{
				break;
			}
			i++;
		}

		if(i < bingSystem.bingResultCreatorCount)
		{
			//We don't want to reallocate because if we fail and the creator was not the last element, then we overwrote it
			c = (bing_result_creator*)BING_MALLOC(sizeof(bing_result_creator) * (bingSystem.bingResultCreatorCount - 1));

			if(c)
			{
				//If this is the last result then it's easy, we just free the data
				if(i != bingSystem.bingResultCreatorCount - 1)
				{
					memmove(bingSystem.bingResultCreators + i, bingSystem.bingResultCreators + (i + 1), (bingSystem.bingResultCreatorCount - i - 1) * sizeof(bing_result_creator));
				}
				memcpy(c, bingSystem.bingResultCreators, (--bingSystem.bingResultCreatorCount) * sizeof(bing_result_creator));
				BING_FREE(bingSystem.bingResultCreators);
				bingSystem.bingResultCreators = c;

				ret = TRUE;
			}
		}

		pthread_mutex_unlock(&bingSystem.mutex);
	}
	return ret;
}
