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
 * Represents an individual spell result.
 */
public class BingSpellResult extends BingResult
{
	public BingSpellResult(Hashtable dict)
	{
		super(dict);
	}
	
	public String getValue()
	{
		return (String)super.attrDict.get("Value");
	}
}
