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
 * An array of NewsCollection. used internally when processing search request.
 */
public class NewsCollectionArray extends BingResult implements Array
{
	public NewsCollectionArray(Hashtable dict)
	{
		super(dict);
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof NewsCollection)
		{
			if(super.attrDict.containsKey("NewsCollection"))
			{
				((Vector)super.attrDict.get("NewsCollection")).addElement(additions[0]);
			}
			else
			{
				Vector v = new Vector();
				v.addElement(additions[0]);
				super.attrDict.put("NewsCollection", v);
			}
		}
	}
	
	public NewsCollection[] getItems()
	{
		if(super.attrDict.containsKey("NewsCollection"))
		{
			Vector v = (Vector)super.attrDict.get("NewsCollection");
			NewsCollection[] links = new NewsCollection[v.size()];
			v.copyInto(links);
			return links;
		}
		return null;
	}
}
