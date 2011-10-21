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
 * Represents the response retrieved from issuing a spell request.
 */
public class BingSpellResponse extends BingResponse
{
	private Hashtable attrDict;
	
	public void handleElements(Hashtable table)
	{
		this.attrDict = table;
		if(this.attrDict.containsKey("Total"))
		{
			this.attrDict.put("Total", new Long(Long.parseLong((String)this.attrDict.get("Total"))));
		}
	}
	
	public long getTotal()
	{
		return ((Long)this.attrDict.get("Total")).longValue();
	}
}
