/*
 * request.c
 *
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 *
 * Author: Vincent Simonetti
 */

#include "bing_internal.h"

static bing_field_search request_fields[] =
{
		//Universal
		{{REQUEST_FIELD_VERSION,			FIELD_TYPE_STRING,	"version",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[1]},
		{{REQUEST_FIELD_MARKET,				FIELD_TYPE_STRING,	"market",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[2]},
		{{REQUEST_FIELD_ADULT,				FIELD_TYPE_STRING,	"adult",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[3]},
		{{REQUEST_FIELD_OPTIONS,			FIELD_TYPE_STRING,	"options",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[4]},
		{{REQUEST_FIELD_LATITUDE,			FIELD_TYPE_DOUBLE,	"latitude",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[5]},
		{{REQUEST_FIELD_LONGITUDE,			FIELD_TYPE_DOUBLE,	"longitude",		BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[6]},
		{{REQUEST_FIELD_LANGUAGE,			FIELD_TYPE_STRING,	"language",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[7]},
		{{REQUEST_FIELD_RADIUS,				FIELD_TYPE_DOUBLE,	"radius",			BING_FIELD_SUPPORT_ALL_FIELDS,	{}},					&request_fields[8]},

		//Ad
		{{REQUEST_FIELD_PAGE_NUMBER,		FIELD_TYPE_LONG,	"pageNumber",		1,	{BING_SOURCETYPE_AD}},								&request_fields[9]},
		{{REQUEST_FIELD_AD_UNIT_ID,			FIELD_TYPE_LONG,	"adUnitId",			1,	{BING_SOURCETYPE_AD}},								&request_fields[10]},
		{{REQUEST_FIELD_PROPERTY_ID,		FIELD_TYPE_LONG,	"propertyId",		1,	{BING_SOURCETYPE_AD}},								&request_fields[11]},
		{{REQUEST_FIELD_CHANNEL_ID,			FIELD_TYPE_LONG,	"channelId",		1,	{BING_SOURCETYPE_AD}},								&request_fields[12]},
		{{REQUEST_FIELD_MAINLINE_AD_COUNT,	FIELD_TYPE_LONG,	"mlAdcount",		1,	{BING_SOURCETYPE_AD}},								&request_fields[13]},
		{{REQUEST_FIELD_SIDEBAR_AD_COUNT,	FIELD_TYPE_LONG,	"sbAdCount",		1,	{BING_SOURCETYPE_AD}},								&request_fields[14]},

		//MobileWeb
		{{REQUEST_FIELD_MOBILE_WEB_OPTIONS,	FIELD_TYPE_STRING,	"mobileWebOptions",	1,	{BING_SOURCETYPE_MOBILE_WEB}},						&request_fields[15]},

		//News
		{{REQUEST_FIELD_CATEGORY,			FIELD_TYPE_STRING,	"category",			1,	{BING_SOURCETYPE_NEWS}},							&request_fields[16]},
		{{REQUEST_FIELD_LOCATION_OVERRIDE,	FIELD_TYPE_STRING,	"locationOverride",	1,	{BING_SOURCETYPE_NEWS}},							&request_fields[17]},

		//Phonebook
		{{REQUEST_FIELD_LOC_ID,				FIELD_TYPE_STRING,	"locId",			1,	{BING_SOURCETYPE_PHONEBOOK}},						&request_fields[18]},

		//Translation
		{{REQUEST_FIELD_SOURCE_LANGUAGE,	FIELD_TYPE_STRING,	"sourceLanguage",	1,	{BING_SOURCETYPE_TRANSLATION}},						&request_fields[19]},
		{{REQUEST_FIELD_TARGET_LANGUAGE,	FIELD_TYPE_STRING,	"targetLanguage",	1,	{BING_SOURCETYPE_TRANSLATION}},						&request_fields[20]},

		//Web
		{{REQUEST_FIELD_WEB_OPTIONS,		FIELD_TYPE_STRING,	"webOptions",		1,	{BING_SOURCETYPE_WEB}},								&request_fields[21]},

		//Multi (less obvious breaks in original source type)
		{{REQUEST_FIELD_COUNT,				FIELD_TYPE_LONG,	"count",			6,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},								&request_fields[22]},
		{{REQUEST_FIELD_OFFSET,				FIELD_TYPE_LONG,	"offset",			6,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_MOBILE_WEB,
				BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},								&request_fields[23]},
		{{REQUEST_FIELD_FILTERS,			FIELD_TYPE_STRING,	"filters",			2,	{BING_SOURCETYPE_IMAGE, BING_SOURCETYPE_VIDEO}},	&request_fields[24]},
		{{REQUEST_FIELD_SORT_BY,			FIELD_TYPE_STRING,	"sortby",			3,	{BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_VIDEO}},																									&request_fields[25]},
		{{REQUEST_FIELD_FILE_TYPE,			FIELD_TYPE_STRING,	"filetype",			2,	{BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_WEB}},	NULL}
};

//TODO
