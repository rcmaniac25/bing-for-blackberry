/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.responses;

import bing.results.BingResult;

/**
 * A bundle response object holds one or many other response objects.
 */
public class BingBundleResponse extends BingResponse
{
	/**
	 * Add a BingResponse to this bundle response
	 * @param response Response to be added to this bundle response
	 */
	public void addResponse(BingResponse response)
	{
		super.results.addElement(response);
	}
	
	/**
	 * Get the responses that this Response object holds.
	 * @return Array of Response objects. Empty if the search returned no results.
	 */
	public BingResponse[] responses()
	{
		BingResponse[] res = new BingResponse[this.results.size()];
		this.results.copyInto(res);
		return res;
	}
	
	/**
	 * Because this is a bundle response it does not return BingResults, it returns BingResponses. Use {@link #responses()}
	 * to get the bundled responses.
	 * @return Returns null.
	 */
	public BingResult[] results()
	{
		return null;
	}
}
