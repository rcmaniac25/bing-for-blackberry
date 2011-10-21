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
 * Base class for all Result objects.
 * 
 * Holds all general result parameters.
 */
public abstract class BingResult
{
	protected Hashtable attrDict;
	
	/**
	 * Create a result object from a {@link Hashtable}.
	 * @param dict {@link Hashtable} populated with result parameters
	 */
	protected BingResult(Hashtable dict)
	{
		this.attrDict = dict;
	}
	
	/**
	 * Certain, non-attribute style elements such as XML arrays may be added by the parser, they are added to the class here.
	 */
	public final void add(Object obj)
	{
		if(obj != null && obj instanceof BingResult[])
		{
			inAdd((BingResult[])obj);
		}
	}
	
	/**
	 * As stated in {@link #add(Object)} extra elements are added, if they pass a basic type check they will be returned here as an array of {@link BingResult}s.
	 * Though not supported as of right now, the array is to allow adding multiple elements at once. Right now only the first element will ever be set.
	 */
	protected void inAdd(BingResult[] additions)
	{
	}
	
	public int hashCode()
	{
		return this.attrDict.hashCode();
	}
}
