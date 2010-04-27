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
 * A deep link for a search item.
 */
public class DeepLink extends BingResult
{
	public DeepLink()
	{
		this(new Hashtable());
	}
	
	public DeepLink(Hashtable dict)
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
