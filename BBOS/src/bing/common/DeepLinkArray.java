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
 * An array of DeepLinks. used internally when processing search request.
 */
public class DeepLinkArray extends BingResult implements Array
{
	public DeepLinkArray(Hashtable dict)
	{
		super(dict);
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof DeepLink)
		{
			if(super.attrDict.containsKey("DeepArray"))
			{
				((Vector)super.attrDict.get("DeepArray")).addElement(additions[0]);
			}
			else
			{
				Vector v = new Vector();
				v.addElement(additions[0]);
				super.attrDict.put("DeepArray", v);
			}
		}
	}
	
	public DeepLink[] getItems()
	{
		if(super.attrDict.containsKey("DeepArray"))
		{
			Vector v = (Vector)super.attrDict.get("DeepArray");
			DeepLink[] links = new DeepLink[v.size()];
			v.copyInto(links);
			return links;
		}
		return null;
	}
}
