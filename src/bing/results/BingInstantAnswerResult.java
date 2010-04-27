/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.results;

import java.util.Hashtable;

/**
 * Represents an individual instant answer result.
 */
public class BingInstantAnswerResult extends BingResult
{
	public BingInstantAnswerResult(Hashtable dict)
	{
		super(dict);
	}
	
	public String getUrl()
	{
		return (String)super.attrDict.get("Url");
	}
	
	public String getAttribution()
	{
		return (String)super.attrDict.get("Attribution");
	}
	
	/**
	 * The section labeled "Content Types" on the bottom of http://msdn.microsoft.com/en-us/library/bceeb75a-69ee-4a2e-98db-9cd560830b13 should be consulted to determine
	 * type content types are available for use.
	 */
	public String getContentType()
	{
		return (String)super.attrDict.get("ContentType");
	}
	
	public String getTitle()
	{
		return (String)super.attrDict.get("Title");
	}
	
	/**
	 * As the property name indicates, this is content specific data. To see a table of content type specific data results check: http://msdn.microsoft.com/en-us/library/1f42bcb1-f2fb-4cd1-ae31-38ae39d2adda
	 */
	public String getInstantAnswerSpecificData()
	{
		return (String)super.attrDict.get("InstantAnswerSpecificData");
	}
}
