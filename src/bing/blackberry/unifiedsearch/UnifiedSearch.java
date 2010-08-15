//#preprocessor

/**
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Vincent Simonetti
 */
package bing.blackberry.unifiedsearch;

//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
import net.rim.device.api.unifiedsearch.registry.SearchRegistry;
//#endif

/**
 * Utility class for Unified Search. Allows simple setup and cleanup of a external search provider that uses Bing do do the actual searching.
 */
public final class UnifiedSearch
{
	private UnifiedSearch(){}
	
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
	
	/**
	 * Register a search provider that will be used to do a search. No management of already registered search providers is done except checking 
	 * if it's already registered. If the Search Provider's name or icon have changed and it states that it has been updated then it can be 
	 * reregistered with the new name and icon.
	 * @param options The search provider to register and that will be used for searching.
	 * @return <code>true</code> if the search provider was registered successfully, <code>false</code> if otherwise.
	 */
	public static boolean registerSearchProvider(SearchProviderOptions options)
	{
//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
		if(unifiedSearchVersion > 0 && options != null)
		{
			if(options.register != null)
			{
				if(options.callback.updated())
				{
					//Icon or name have been updated, need to reregister
					if(!SearchRegistry.getInstance().deregister(options.register))
					{
						return false; //Something went wrong
					}
				}
				else
				{
					return true;
				}
			}
			options.register = SearchRegistry.getInstance().register(options.new ExternalSearch());
			return options.register.isValid();
		}
//#endif
		return false;
	}
	
	/**
	 * Unregister a search provider. No management of search providers is done except checking if it was registered.
	 * @param options The search provider to unregister;
	 * @return <code>true</code> if the search provider was unregistered successfully, <code>false</code> if otherwise.
	 */
	public static boolean unregisterSearchProvider(SearchProviderOptions options)
	{
//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
		if(unifiedSearchVersion > 0 && options != null && options.register != null)
		{
			if(SearchRegistry.getInstance().deregister(options.register))
			{
				options.register = null;
				return true;
			}
		}
//#endif
		return false;
	}
}
