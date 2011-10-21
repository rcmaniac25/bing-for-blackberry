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
 * Thumbnail for a static image.
 */
public class Thumbnail extends BingResult
{
	public Thumbnail()
	{
		this(new Hashtable());
	}
	
	public Thumbnail(Hashtable dict)
	{
		super(dict);
		if(super.attrDict.containsKey("Height"))
		{
			super.attrDict.put("Height", new Long(Long.parseLong((String)super.attrDict.get("Height"))));
		}
		if(super.attrDict.containsKey("Width"))
		{
			super.attrDict.put("Width", new Long(Long.parseLong((String)super.attrDict.get("Width"))));
		}
		if(super.attrDict.containsKey("FileSize"))
		{
			super.attrDict.put("FileSize", new Long(Long.parseLong((String)super.attrDict.get("FileSize"))));
		}
	}
	
	public String getUrl()
	{
		return (String)super.attrDict.get("Url");
	}
	
	public String getContentType()
	{
		return (String)super.attrDict.get("ContentType");
	}
	
	public long getHeight()
	{
		return ((Long)super.attrDict.get("Height")).longValue();
	}
	
	public long getWidth()
	{
		return ((Long)super.attrDict.get("Width")).longValue();
	}
	
	public long getFileSize()
	{
		return ((Long)super.attrDict.get("FileSize")).longValue();
	}
}
