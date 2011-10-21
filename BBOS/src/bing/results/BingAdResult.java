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
 * Represents an individual ad result.
 */
public class BingAdResult extends BingResult
{
	public BingAdResult(Hashtable dict)
	{
		super(dict);
		if(super.attrDict.containsKey("Rank"))
		{
			super.attrDict.put("Rank", new Long(Long.parseLong((String)super.attrDict.get("Rank"))));
		}
	}
	
	public long getRank()
	{
		return ((Long)super.attrDict.get("Rank")).longValue();
	}
	
	public String getPosition()
	{
		return (String)super.attrDict.get("Position");
	}
	
	public String getTitle()
	{
		return (String)super.attrDict.get("Title");
	}
	
	public String getDescription()
	{
		return (String)super.attrDict.get("Description");
	}
	
	public String getDisplayURL()
	{
		return (String)super.attrDict.get("DisplayURL");
	}
	
	public String getAdlinkURL()
	{
		return (String)super.attrDict.get("AdlinkURL");
	}
}
