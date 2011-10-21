/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing;

import bing.responses.*;

/**
 * Informal interface which Bing Asynchronous Search delegates should implement
 */
public interface BingAsyncRequestNotification
{
	/**
	 * Executed when a BingResponse is received from the API.
	 * @param response The BingResponse object for the received API response.
	 */
	public void receiveBingResponse(BingResponse response);
}
