/*
 * result.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

static bing_field_search result_fields[] =
{
		//Ad
		{{RESULT_FIELD_RANK,							FIELD_TYPE_LONG,	"Rank",							1,	{BING_SOURCETYPE_AD}},						&result_fields[1]},
		{{RESULT_FIELD_POSITION,						FIELD_TYPE_STRING,	"Position",						1,	{BING_SOURCETYPE_AD}},						&result_fields[2]},
		{{RESULT_FIELD_TITLE,							FIELD_TYPE_STRING,	"Title",						9,	{BING_SOURCETYPE_AD, BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER, BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},																&result_fields[3]},
		{{RESULT_FIELD_DESCRIPTION,						FIELD_TYPE_STRING,	"Description",					3,	{BING_SOURCETYPE_AD,
				BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_WEB}},																							&result_fields[4]},
		{{RESULT_FIELD_DISPLAY_URL,						FIELD_TYPE_STRING,	"DisplayUrl",					5,	{BING_SOURCETYPE_AD, BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_WEB}},																&result_fields[5]},
		{{RESULT_FIELD_ADLINK_URL,						FIELD_TYPE_STRING,	"AdlinkURL",					1,	{BING_SOURCETYPE_AD}},						&result_fields[6]},

		//Error
		{{RESULT_FIELD_CODE,							FIELD_TYPE_LONG,	"Code",							1,	{BING_RESULT_ERROR}},						&result_fields[7]},
		{{RESULT_FIELD_MESSAGE,							FIELD_TYPE_STRING,	"Message",						1,	{BING_RESULT_ERROR}},						&result_fields[8]},
		{{RESULT_FIELD_HELP_URL,						FIELD_TYPE_STRING,	"HelpUrl",						1,	{BING_RESULT_ERROR}},						&result_fields[9]},
		{{RESULT_FIELD_PARAMETER,						FIELD_TYPE_STRING,	"Parameter",					1,	{BING_RESULT_ERROR}},						&result_fields[10]},
		{{RESULT_FIELD_SOURCE_TYPE,						FIELD_TYPE_STRING,	"SourceType",					1,	{BING_RESULT_ERROR}},						&result_fields[11]},
		{{RESULT_FIELD_SOURCE_TYPE_ERROR_CODE,			FIELD_TYPE_LONG,	"SourceTypeErrorCode",			1,	{BING_RESULT_ERROR}},						&result_fields[12]},
		{{RESULT_FIELD_VALUE,							FIELD_TYPE_STRING,	"Value",						2,	{BING_RESULT_ERROR, BING_SOURCETYPE_SPELL}},&result_fields[13]},

		//Image
		{{RESULT_FIELD_HEIGHT,							FIELD_TYPE_LONG,	"Height",						1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[14]},
		{{RESULT_FIELD_WIDTH,							FIELD_TYPE_LONG,	"Width",						1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[15]},
		{{RESULT_FIELD_FILE_SIZE,						FIELD_TYPE_LONG,	"FileSize",						1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[16]},
		{{RESULT_FIELD_MEDIA_URL,						FIELD_TYPE_STRING,	"MediaUrl",						1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[17]},
		{{RESULT_FIELD_URL,								FIELD_TYPE_STRING,	"Url",							7,	{BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER, BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_WEB}},																						&result_fields[18]},
		{{RESULT_FIELD_CONTENT_TYPE,					FIELD_TYPE_STRING,	"ContentType",					2,	{BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER}},																											&result_fields[19]},
		{{RESULT_FIELD_THUMBNAIL,						FIELD_TYPE_ARRAY,	"Thumbnail",					1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[20]},

		//InstantAnswer
		{{RESULT_FIELD_ATTRIBUTION,						FIELD_TYPE_STRING,	"Attribution",					1,	{BING_SOURCETYPE_INSTANT_ANWSER}},			&result_fields[21]},
		{{RESULT_FIELD_INSTANT_ANWSER_SPECIFIC_DATA,	FIELD_TYPE_STRING,	"InstantAnswerSpecificData",	1,	{BING_SOURCETYPE_INSTANT_ANWSER}},			&result_fields[22]},

		//MobileWeb
		{{RESULT_FIELD_DATE_TIME,						FIELD_TYPE_STRING,	"DateTime",						2,	{BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_WEB}},																														&result_fields[23]},

		//News
		{{RESULT_FIELD_BREAKING_NEWS,					FIELD_TYPE_BOOLEAN,	"BreakingNews",					1,	{BING_SOURCETYPE_NEWS}},					&result_fields[24]},
		{{RESULT_FIELD_DATE,							FIELD_TYPE_STRING,	"Date",							1,	{BING_SOURCETYPE_NEWS}},					&result_fields[25]},
		{{RESULT_FIELD_SNIPPET,							FIELD_TYPE_STRING,	"Snippet",						1,	{BING_SOURCETYPE_NEWS}},					&result_fields[26]},
		{{RESULT_FIELD_SOURCE,							FIELD_TYPE_STRING,	"Source",						1,	{BING_SOURCETYPE_NEWS}},					&result_fields[27]},
		{{RESULT_FIELD_NEWSCOLLECTION,					FIELD_TYPE_ARRAY,	"NewsCollection",				1,	{BING_SOURCETYPE_NEWS}},					&result_fields[28]},

		//PhoneBook
		{{RESULT_FIELD_LATITUDE,						FIELD_TYPE_DOUBLE,	"Latitude",						1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[29]},
		{{RESULT_FIELD_LONGITUDE,						FIELD_TYPE_DOUBLE,	"Longitude",					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[30]},
		{{RESULT_FIELD_USER_RATING,						FIELD_TYPE_DOUBLE,	"UserRating",					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[31]},
		{{RESULT_FIELD_REVIEW_COUNT,					FIELD_TYPE_LONG,	"ReviewCount",					1,	{BING_SOURCETYPE_PHONEBOOK}},				&result_fields[32]},
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
		{{RESULT_FIELD_STATIC_THUMBNAIL,				FIELD_TYPE_ARRAY,	"StaticThumbnail",				1,	{BING_SOURCETYPE_VIDEO}},					&result_fields[47]},

		//Web
		{{RESULT_FIELD_CACHE_URL,						FIELD_TYPE_STRING,	"CacheUrl",						1,	{BING_SOURCETYPE_WEB}},						&result_fields[48]},
		{{RESULT_FIELD_DEEP_LINKS,						FIELD_TYPE_ARRAY,	"DeepLink",						1,	{BING_SOURCETYPE_WEB}},						&result_fields[49]},
		{{RESULT_FIELD_SEARCH_TAGS,						FIELD_TYPE_ARRAY,	"SearchTag",					1,	{BING_SOURCETYPE_WEB}},						NULL}
};

//TODO
