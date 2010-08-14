//#preprocessor

/**
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Vincent Simonetti
 */
package bing.blackberry.unifiedsearch;

/**
 * Utility class for Unified Search.
 */
public final class UnifiedSearch
{
	//Do this so that if future updates to the unified search system the updates can be determined and kept track of.
//#ifdef BlackBerrySDK6.0.0
	static final int unifiedSearchVersion = 1;
//#else
	static final int unifiedSearchVersion = 0;
//#endif
	
	/**
	 * Is Unified Search supported?
	 * @return <code>true</code> if unified search is supported, <code>false</code> if otherwise.
	 */
	public static boolean isSupported()
	{
//#ifndef NO_SIGNING
		return unifiedSearchVersion > 0;
//#else
		return false;
//#endif
	}
	
	public static boolean registerSearchProvider(SearchProviderOptions options)
	{
//#ifndef NO_SIGNING
		if(unifiedSearchVersion > 0)
		{
			if(options != null && options.register != null)
			{
				//TODO: Check to see if the options have changed, if so then unregister and reregister (if necessary), else return true since already registered.
			}
			//TODO: Register the options
		}
//#endif
		return false;
	}
	
	public static boolean unregisterSearchProvider(SearchProviderOptions options)
	{
//#ifndef NO_SIGNING
		if(options != null)
		{
			if(unifiedSearchVersion > 0 && options.register != null)
			{
				//TODO: Unregister the options
			}
		}
//#endif
		return false;
	}
}

//ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0