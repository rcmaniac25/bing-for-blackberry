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
 * Represents the response retrieved from issuing a phonebook request.
 */
public class BingPhonebookResponse extends BingResponse
{
	private Hashtable attrDict;
	
	public void handleElements(Hashtable table)
	{
		this.attrDict = table;
	}
	
	public String getTitle()
	{
		return (String)this.attrDict.get("Title");
	}
	
	public String getLocalSerpUrl()
	{
		return (String)this.attrDict.get("LocalSerpUrl");
	}
}
