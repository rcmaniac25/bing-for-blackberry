/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.responses;

import java.util.Vector;

import bing.results.*;

/**
 * Base class for all Response objects.
 * 
 * Holds all general response parameters.
 */
public abstract class BingResponse
{
	protected long total, offset;
	protected Vector results;
	protected String query, alteredQuery, alterationOverrideQuery;
	
	public synchronized long getTotal()
	{
		return this.total;
	}
	
	public synchronized void setTotal(long total)
	{
		this.total = total;
	}
	
	public synchronized long getOffset()
	{
		return this.offset;
	}
	
	public synchronized void setOffset(long offset)
	{
		this.offset = offset;
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
	
	public synchronized String getAlteredQuery()
	{
		//return new String(this.alteredQuery); //Same reasons as Bing.getAppID()
		return this.alteredQuery;
	}
	
	public synchronized void setAlteredQuery(String alteredQuery)
	{
		this.alteredQuery = alteredQuery;
	}
	
	public synchronized String getAlterationOverrideQuery()
	{
		//return new String(this.alterationOverrideQuery); //Same reasons as Bing.getAppID()
		return this.alterationOverrideQuery;
	}
	
	public synchronized void setAlterationOverrideQuery(String alterationOverrideQuery)
	{
		this.alterationOverrideQuery = alterationOverrideQuery;
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
	
	/**
	 * Some Bing responses can contain extra data then what is normally processed, process that data here if supported.
	 */
	public void handleElements(java.util.Hashtable table)
	{
	}
	
	public int hashCode()
	{
		return this.results.hashCode();
	}
}
