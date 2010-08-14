//#preprocessor

/**
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.blackberry.unifiedsearch;

import bing.Bing;

/**
 * Options for customized unified search integration.
 */
public class SearchProviderOptions
{
//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
	//Use the preprocessor so that code memory is saved if not supported.
	private Bing bingObj;
	Object register; //TODO: Replace with token type or something.
//#endif
	
	public SearchProviderOptions(String application_ID)
	{
//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
		this.bingObj = new Bing(application_ID);
//#endif
	}
	
	//XXX Other additions: "CLICK-TO-SEARCH", callbacks for search, filters
}
