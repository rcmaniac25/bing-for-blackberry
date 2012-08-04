/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.responses;

import java.util.Calendar;
import java.util.Vector;

import bing.Bing;
import bing.results.*;

/**
 * Base class for all Response objects.
 * 
 * Holds all general response parameters.
 */
public abstract class BingResponse
{
	protected long maxTotal, offset;
	protected Calendar updated;
	protected Vector results;
	protected String query, nextUrl;
	
	public synchronized long getMaxTotal()
	{
		return this.maxTotal;
	}
	
	public synchronized void setMaxTotal(long maxTotal)
	{
		this.maxTotal = maxTotal;
	}
	
	public synchronized long getOffset()
	{
		return this.offset;
	}
	
	public synchronized void setOffset(long offset)
	{
		this.offset = offset;
	}
	
	public synchronized Calendar getUpdated()
	{
		return this.updated;
	}
	
	public synchronized void setUpdated(Calendar updated)
	{
		this.updated = updated;
	}
	
	public synchronized boolean hasNextURL()
	{
		return this.nextUrl != null;
	}
	
	public synchronized void setNextURL(String nextUrl)
	{
		if(nextUrl == null || Bing.isValidURL(nextUrl))
		{
			this.nextUrl = nextUrl;
		}
	}
	
	public synchronized String getQuery()
	{
		//return new String(this.query); //Same reasons as Bing.getAppID()
		return this.query;
	}
	
	public synchronized void setQuery(String query)
	{
		this.query = query;
	}
	
	protected BingResponse()
	{
		results = new Vector();
	}
	
	/**
	 * Add a single result to this response.
	 * @param result The BingResult object to add.
	 */
	public void addResult(BingResult result)
	{
		this.results.addElement(result);
	}
	
	/**
	 * Get the results that this Response object holds.
	 * @return Array of Result objects. Empty if the search returned no results.
	 */
	public BingResult[] results()
	{
		BingResult[] res = new BingResult[this.results.size()];
		this.results.copyInto(res);
		return res;
	}
	
	public int hashCode()
	{
		return this.results.hashCode();
	}
}
