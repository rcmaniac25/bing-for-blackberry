/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.responses;

import java.util.Hashtable;

/**
 * Represents the response retrieved from issuing an Ad request.
 */
public class BingAdResponse extends BingResponse
{
	private Hashtable attrDict;
	
	public void handleElements(Hashtable table)
	{
		this.attrDict = table;
		if(this.attrDict.containsKey("PageNumber"))
		{
			this.attrDict.put("PageNumber", new Long(Long.parseLong((String)this.attrDict.get("PageNumber"))));
		}
	}
	
	public String getAdAPIVersion()
	{
		return (String)this.attrDict.get("AdAPIVersion");
	}
	
	public long getPageNumber()
	{
		return ((Long)this.attrDict.get("PageNumber")).longValue();
	}
}
