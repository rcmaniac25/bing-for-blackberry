/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.responses;

import java.util.Hashtable;

import bing.common.*;
import bing.results.BingRelatedSearchResult;

/**
 * Represents the response retrieved from issuing a news request.
 */
public class BingNewsResponse extends BingResponse
{
	private Hashtable attrDict;
	
	public void handleElements(Hashtable table)
	{
		this.attrDict = table;
	}
	
	public BingRelatedSearchResult[] getRelatedSearches()
	{
		return ((RelatedSearchArray)this.attrDict.get("RelatedSearches")).getItems();
	}
}
