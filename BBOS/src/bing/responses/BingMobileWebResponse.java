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
 * Represents the response retrieved from issuing a mobile web request.
 */
public class BingMobileWebResponse extends BingResponse
{
	public void handleElements(Hashtable table)
	{
		if(table.containsKey("Total"))
		{
			total = Long.parseLong((String)table.get("Total"));
		}
		if(table.containsKey("Offset"))
		{
			offset = Long.parseLong((String)table.get("Offset"));
		}
	}
}
