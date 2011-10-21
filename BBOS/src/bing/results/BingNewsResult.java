/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.results;

import java.util.Hashtable;

import bing.common.NewsCollectionArray;
import bing.common.NewsCollection;

/**
 * Represents an individual news result.
 */
public class BingNewsResult extends BingResult
{
	public BingNewsResult(Hashtable dict)
	{
		super(dict);
		if(super.attrDict.containsKey("BreakingNews"))
		{
			super.attrDict.put("BreakingNews", new Boolean(((String)super.attrDict.get("BreakingNews")).equals("1")));
		}
	}
	
	public boolean getBreakingNews()
	{
		return ((Boolean)super.attrDict.get("BreakingNews")).booleanValue();
	}
	
	public String getDate()
	{
		return (String)super.attrDict.get("Date");
	}
	
	public String getSnippet()
	{
		return (String)super.attrDict.get("Snipper");
	}
	
	public String getSource()
	{
		return (String)super.attrDict.get("Source");
	}
	
	public String getTitle()
	{
		return (String)super.attrDict.get("Title");
	}
	
	public String getUrl()
	{
		return (String)super.attrDict.get("Url");
	}
	
	public NewsCollection[] getNewsCollection()
	{
		return ((NewsCollectionArray)super.attrDict.get("NewsCollection")).getItems();
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof NewsCollectionArray)
		{
			super.attrDict.put("NewsCollection", additions[0]);
		}
	}
}
