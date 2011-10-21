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
 * Represents error that was returned by the Bing service.
 */
public class BingError extends BingResult
{
	public BingError(Hashtable dict)
	{
		super(dict);
		if(super.attrDict.containsKey("Code"))
		{
			super.attrDict.put("Code", new Long(Long.parseLong((String)super.attrDict.get("Code"))));
		}
		if(super.attrDict.containsKey("SourceTypeErrorCode"))
		{
			super.attrDict.put("SourceTypeErrorCode", new Long(Long.parseLong((String)super.attrDict.get("SourceTypeErrorCode"))));
		}
	}
	
	public long getCode()
	{
		return ((Long)super.attrDict.get("Code")).longValue();
	}
	
	public String getMessage()
	{
		return (String)super.attrDict.get("Message");
	}
	
	public String getHelpUrl()
	{
		return (String)super.attrDict.get("HelpUrl");
	}
	
	public String getParameter()
	{
		return (String)super.attrDict.get("Parameter");
	}
	
	public String getSourceType()
	{
		return (String)super.attrDict.get("SourceType");
	}
	
	public long getSourceTypeErrorCode()
	{
		return ((Long)super.attrDict.get("SourceTypeErrorCode")).longValue();
	}
	
	public String getValue()
	{
		return (String)super.attrDict.get("Value");
	}
}
