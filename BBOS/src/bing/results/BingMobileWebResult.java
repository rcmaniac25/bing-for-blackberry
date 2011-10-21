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
 * Represents an individual mobile web result.
 */
public class BingMobileWebResult extends BingResult
{
	public BingMobileWebResult(Hashtable dict)
	{
		super(dict);
	}
	
	public String getDateTime()
	{
		return (String)super.attrDict.get("DateTime");
	}
	
	public String getDescription()
	{
		return (String)super.attrDict.get("Description");
	}
	
	public String getDisplayUrl()
	{
		return (String)super.attrDict.get("DisplayUrl");
	}
	
	public String getTitle()
	{
		return (String)super.attrDict.get("Title");
	}
	
	public String getUrl()
	{
		return (String)super.attrDict.get("Url");
	}
}
