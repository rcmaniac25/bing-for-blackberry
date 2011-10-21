/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.results;

import java.util.Hashtable;

import bing.common.*;

/**
 * Represents an individual web result.
 */
public class BingWebResult extends BingResult
{
	public BingWebResult(Hashtable dict)
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
	
	public String getCacheUrl()
	{
		return (String)super.attrDict.get("CacheUrl");
	}
	
	public DeepLink[] getDeepLinks()
	{
		return ((DeepLinkArray)super.attrDict.get("DeepLink")).getItems();
	}
	
	public SearchTag[] getSearchTags()
	{
		return ((SearchTagArray)super.attrDict.get("SearchTag")).getItems();
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof DeepLinkArray)
		{
			super.attrDict.put("DeepLink", additions[0]);
		}
		else if(additions[0] instanceof SearchTagArray)
		{
			super.attrDict.put("SearchTag", additions[0]);
		}
	}
}
