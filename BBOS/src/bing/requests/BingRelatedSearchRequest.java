/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting image results.
 */
public class BingRelatedSearchRequest extends BingRequest
{
	public String sourceType()
	{
		return "RelatedSearch";
	}
}
