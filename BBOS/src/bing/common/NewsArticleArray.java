/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.common;

import java.util.*;

import bing.results.*;

/**
 * An array of NewsArticles. used internally when processing search request.
 */
public class NewsArticleArray extends BingResult implements Array
{
	public NewsArticleArray(Hashtable dict)
	{
		super(dict);
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof BingNewsResult)
		{
			if(super.attrDict.containsKey("NewsArticles"))
			{
				((Vector)super.attrDict.get("NewsArticles")).addElement(additions[0]);
			}
			else
			{
				Vector v = new Vector();
				v.addElement(additions[0]);
				super.attrDict.put("NewsArticles", v);
			}
		}
	}
	
	public BingNewsResult[] getItems()
	{
		if(super.attrDict.containsKey("NewsArticles"))
		{
			Vector v = (Vector)super.attrDict.get("NewsArticles");
			BingNewsResult[] links = new BingNewsResult[v.size()];
			v.copyInto(links);
			return links;
		}
		return null;
	}
}
