/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.responses;

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
}
