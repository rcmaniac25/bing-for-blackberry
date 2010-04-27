/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.results;

import java.util.Hashtable;

import bing.common.Thumbnail;

/**
 * Represents an individual video result.
 */
public class BingVideoResult extends BingResult
{
	public BingVideoResult(Hashtable dict)
	{
		super(dict);
	}
	
	public String getTitle()
	{
		return (String)super.attrDict.get("Title");
	}
	
	public String getSourceTitle()
	{
		return (String)super.attrDict.get("SourceTitle");
	}
	
	public String getRunTime()
	{
		return (String)super.attrDict.get("RunTime");
	}
	
	public String getPlayUrl()
	{
		return (String)super.attrDict.get("PlayUrl");
	}
	
	public String getClickThroughPageUrl()
	{
		return (String)super.attrDict.get("ClickThroughPageUrl");
	}
	
	public Thumbnail getStaticThumbnail()
	{
		return (Thumbnail)super.attrDict.get("StaticThumbnail");
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof Thumbnail)
		{
			super.attrDict.put("StaticThumbnail", additions[0]);
		}
	}
}
