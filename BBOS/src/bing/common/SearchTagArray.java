/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.common;

import java.util.*;

import bing.results.BingResult;

/**
 * An array of SearchTags. used internally when processing search request.
 */
public class SearchTagArray extends BingResult implements Array
{
	public SearchTagArray(Hashtable dict)
	{
		super(dict);
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof DeepLink)
		{
			if(super.attrDict.containsKey("SearchTag"))
			{
				((Vector)super.attrDict.get("SearchTag")).addElement(additions[0]);
			}
			else
			{
				Vector v = new Vector();
				v.addElement(additions[0]);
				super.attrDict.put("SearchTag", v);
			}
		}
	}
	
	public SearchTag[] getItems()
	{
		if(super.attrDict.containsKey("SearchTag"))
		{
			Vector v = (Vector)super.attrDict.get("SearchTag");
			SearchTag[] links = new SearchTag[v.size()];
			v.copyInto(links);
			return links;
		}
		return null;
	}
}
