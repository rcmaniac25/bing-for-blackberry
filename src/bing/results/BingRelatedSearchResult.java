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
 * A related search for a search.
 */
public class BingRelatedSearchResult extends BingResult
{
	public BingRelatedSearchResult(Hashtable dict)
	{
		super(dict);
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
