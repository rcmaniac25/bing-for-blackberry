/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting mobile web results.
 */
public class BingMobileWebRequest extends BingRequest
{
	public static String MOBILE_WEB_SEARCH_OPTIONS_SEPERATOR = BingRequest.OPTION_SEPERATOR;
	public static String MOBILE_WEB_SEARCH_OPTIONS_DISABLE_HOST_COLLAPSING = "DisableHostCollapsing";
	public static String MOBILE_WEB_SEARCH_OPTIONS_DISABLE_QUERY_ALTERATIONS = "DisableQueryAlterations";
	
	public String sourceType()
	{
		return "MobileWeb";
	}
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		if(super.attrDict.containsKey("count"))
		{
			options.append(bing.Bing.format("&MobileWeb.Count={0,number,integer}", new Object[]{ super.attrDict.get("count") }));
		}
		
		if(super.attrDict.containsKey("offset"))
		{
			options.append(bing.Bing.format("&MobileWeb.Offset={0,number,integer}", new Object[]{ super.attrDict.get("offset") }));
		}
		
		if(super.attrDict.containsKey("mobileWebOptions"))
		{
			options.append(bing.Bing.format("&MobileWeb.Options={0}", new Object[]{ super.attrDict.get("webOptions") }));
		}
		
		return options.toString();
	}
	
	public void setCount(long count)
	{
		super.attrDict.put("count", new Long(count & 0xFFFFFFFFL));
	}
	
	public void setOffset(long offset)
	{
		super.attrDict.put("offset", new Long(offset & 0xFFFFFFFFL));
	}
	
	/**
	 * @param options One or more combinations of the <code>MOBILE_WEB_SEARCH_OPTIONS_<code> options separated by <code>MOBILE_WEB_SEARCH_OPTIONS_SEPERATOR<code>.
	 */
	public void setMobileWebOptions(String options)
	{
		super.attrDict.put("mobileWebOptions", options);
	}
}
