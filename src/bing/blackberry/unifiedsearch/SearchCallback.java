/**
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Vincent Simonetti
 */
package bing.blackberry.unifiedsearch;

import net.rim.device.api.ui.image.Image;

import bing.Bing;
import bing.BingAsyncRequestNotification;
import bing.requests.BingRequest;

/**
 * Callback for Unified Search.
 */
public interface SearchCallback extends BingAsyncRequestNotification
{
	/**
	 * Search operation has been started, this is the point where operations such as UI display should occur.
	 */
	void searchStarted();
	
	/**
	 * Is the specified keywords supported search parameters.
	 * @param keywords The keywords that will be used for searching.
	 * @return <code>true</code> if the keywords are supported, <code>false</code> if not supported.
	 */
	boolean supportedSearch(String keywords);
	
	/**
	 * Get the requests that should be searched for. This is called if {@link #supportedSearch(String)} returns <code>true</code> which will get the
	 * keywords passed into it. If the request is determined by the search keywords they should be setup in {@link #supportedSearch(String)}.
	 * @return The BingRequest that will be used to perform the search operation.
	 * @see #supportedSearch(String)
	 */
	BingRequest getRequest();
	
	/**
	 * Get the Bing search object.
	 * @return The Bing object that will be used to search.
	 */
	Bing getBing();
	
	/**
	 * Get the search content type using SearchableContentTypeConstants.
	 * @return The search content type.
	 */
	long getContentType();
	
	/**
	 * Get the search icon. This will appear when the user does a search.
	 * @return The icon that represents the search.
	 */
	Image getImage();
	
	/**
	 * The search name. This will appear when the user does a search. Format is "Search <search callback name>".
	 * @return The search name of the search.
	 */
	String getName();
	
	/**
	 * If the image/icon or name have changed.
	 * @return <code>true</code> if the image/icon or name have changed, <code>false</code> if otherwise.
	 */
	boolean updated();
}
