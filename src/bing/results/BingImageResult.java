/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.results;

import java.util.Hashtable;

import bing.common.*;

/**
 * Represents an individual image result.
 */
public class BingImageResult extends BingResult
{
	public BingImageResult(Hashtable dict)
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
	
	public String getTitle()
	{
		return (String)super.attrDict.get("Title");
	}
	
	public String getMediaUrl()
	{
		return (String)super.attrDict.get("MediaUrl");
	}
	
	public String getUrl()
	{
		return (String)super.attrDict.get("Url");
	}
	
	public String getDisplayUrl()
	{
		return (String)super.attrDict.get("DisplayUrl");
	}
	
	public String getContentType()
	{
		return (String)super.attrDict.get("ContentType");
	}
	
	public Thumbnail getThumbnail()
	{
		return (Thumbnail)super.attrDict.get("Thumbnail");
	}
	
	protected void inAdd(BingResult[] additions)
	{
		if(additions[0] instanceof Thumbnail)
		{
			super.attrDict.put("Thumbnail", additions[0]);
		}
	}
}
