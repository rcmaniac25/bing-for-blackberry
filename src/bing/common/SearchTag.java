/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.common;

import java.util.Hashtable;

import bing.results.*;

/**
 * A search tag for a search item.
 */
public class SearchTag extends BingResult
{
	public SearchTag()
	{
		this(new Hashtable());
	}
	
	public SearchTag(Hashtable dict)
	{
		super(dict);
	}
	
	public String getName()
	{
		return (String)super.attrDict.get("Name");
	}
	
	public String getValue()
	{
		return (String)super.attrDict.get("Value");
	}
}
