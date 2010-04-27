/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.common;

import java.util.*;

import bing.results.BingNewsResult;
import bing.results.BingResult;

/**
 * A NewsCollection, has an array of NewsArticles in it. used internally when processing search request.
 */
public class NewsCollection extends BingResult implements Array
{
	public NewsCollection(Hashtable dict)
	{
		super(dict);
	}
	
	public String getName()
	{
		return (String)super.attrDict.get("Name");
	}
	
	public BingNewsResult[] getNewsArticles()
	{
		return ((NewsArticleArray)super.attrDict.get("NewsArticles")).getItems();
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof NewsArticleArray)
		{
			super.attrDict.put("NewsArticles", additions[0]);
		}
	}
}
