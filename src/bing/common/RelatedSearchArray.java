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
 * An array of RelatedSearch. used internally when processing search request.
 */
public class RelatedSearchArray extends BingResult implements Array
{
	public RelatedSearchArray(Hashtable dict)
	{
		super(dict);
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof BingRelatedSearchResult)
		{
			if(super.attrDict.containsKey("RelatedSearch"))
			{
				((Vector)super.attrDict.get("RelatedSearch")).addElement(additions[0]);
			}
			else
			{
				Vector v = new Vector();
				v.addElement(additions[0]);
				super.attrDict.put("RelatedSearch", v);
			}
		}
	}
	
	public BingRelatedSearchResult[] getItems()
	{
		if(super.attrDict.containsKey("RelatedSearch"))
		{
			Vector v = (Vector)super.attrDict.get("RelatedSearch");
			BingRelatedSearchResult[] links = new BingRelatedSearchResult[v.size()];
			v.copyInto(links);
			return links;
		}
		return null;
	}
}
