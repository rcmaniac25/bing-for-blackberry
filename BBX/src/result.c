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
		{{RESULT_FIELD_RANK,					FIELD_TYPE_LONG,	"Rank",					1,	{BING_SOURCETYPE_AD}},						&result_fields[1]},
		{{RESULT_FIELD_POSITION,				FIELD_TYPE_STRING,	"Position",				1,	{BING_SOURCETYPE_AD}},						&result_fields[2]},
		{{RESULT_FIELD_TITLE,					FIELD_TYPE_STRING,	"Title",				9,	{BING_SOURCETYPE_AD, BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER, BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_VIDEO, BING_SOURCETYPE_WEB}},												&result_fields[3]},
		{{RESULT_FIELD_DESCRIPTION,				FIELD_TYPE_STRING,	"Description",			3,	{BING_SOURCETYPE_AD,
				BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_WEB}},																			&result_fields[4]},
		{{RESULT_FIELD_DISPLAY_URL,				FIELD_TYPE_STRING,	"DisplayUrl",			5,	{BING_SOURCETYPE_AD, BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_PHONEBOOK, BING_SOURCETYPE_WEB}},												&result_fields[5]},
		{{RESULT_FIELD_ADLINK_URL,				FIELD_TYPE_STRING,	"AdlinkURL",			1,	{BING_SOURCETYPE_AD}},						&result_fields[6]},

		//Error
		{{RESULT_FIELD_CODE,					FIELD_TYPE_LONG,	"Code",					1,	{BING_RESULT_ERROR}},						&result_fields[7]},
		{{RESULT_FIELD_MESSAGE,					FIELD_TYPE_STRING,	"Message",				1,	{BING_RESULT_ERROR}},						&result_fields[8]},
		{{RESULT_FIELD_HELP_URL,				FIELD_TYPE_STRING,	"HelpUrl",				1,	{BING_RESULT_ERROR}},						&result_fields[9]},
		{{RESULT_FIELD_PARAMETER,				FIELD_TYPE_STRING,	"Parameter",			1,	{BING_RESULT_ERROR}},						&result_fields[10]},
		{{RESULT_FIELD_SOURCE_TYPE,				FIELD_TYPE_STRING,	"SourceType",			1,	{BING_RESULT_ERROR}},						&result_fields[11]},
		{{RESULT_FIELD_SOURCE_TYPE_ERROR_CODE,	FIELD_TYPE_LONG,	"SourceTypeErrorCode",	1,	{BING_RESULT_ERROR}},						&result_fields[12]},
		{{RESULT_FIELD_VALUE,					FIELD_TYPE_STRING,	"Value",				2,	{BING_RESULT_ERROR, BING_SOURCETYPE_SPELL}},&result_fields[13]},

		//Image
		{{RESULT_FIELD_HEIGHT,					FIELD_TYPE_LONG,	"Height",				1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[14]},
		{{RESULT_FIELD_WIDTH,					FIELD_TYPE_LONG,	"Width",				1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[15]},
		{{RESULT_FIELD_FILE_SIZE,				FIELD_TYPE_LONG,	"FileSize",				1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[16]},
		{{RESULT_FIELD_MEDIA_URL,				FIELD_TYPE_STRING,	"MediaUrl",				1,	{BING_SOURCETYPE_IMAGE}},					&result_fields[17]},
		{{RESULT_FIELD_URL,						FIELD_TYPE_STRING,	"Url",					7,	{BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER, BING_SOURCETYPE_MOBILE_WEB, BING_SOURCETYPE_NEWS, BING_SOURCETYPE_PHONEBOOK,
				BING_SOURCETYPE_RELATED_SEARCH, BING_SOURCETYPE_WEB}},																		&result_fields[18]},
		{{RESULT_FIELD_CONTENT_TYPE,			FIELD_TYPE_STRING,	"ContentType",			2,	{BING_SOURCETYPE_IMAGE,
				BING_SOURCETYPE_INSTANT_ANWSER}},																							&result_fields[19]},
		{{RESULT_FIELD_THUMBNAIL,				FIELD_TYPE_ARRAY,	"Thumbnail",			1,	{BING_SOURCETYPE_IMAGE}},					NULL},

		//InstantAnswer
		//TODO: Attribution
};
