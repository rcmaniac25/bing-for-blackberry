/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting web results.
 */
public class BingWebRequest extends BingRequest
{
	public static final String WEB_SEARCH_OPTIONS_SEPERATOR = BingRequest.OPTION_SEPERATOR;
	public static final String WEB_SEARCH_OPTIONS_DISABLE_HOST_COLLAPSING = "DisableHostCollapsing";
	public static final String WEB_SEARCH_OPTIONS_DISABLE_QUERY_ALTERATIONS = "DisableQueryAlterations";
	
	public String sourceType()
	{
		return "web";
	}
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		if(super.attrDict.containsKey("count"))
		{
			options.append(bing.Bing.format("&Web.Count={0,number,integer}", new Object[]{ super.attrDict.get("count") }));
		}
		
		if(super.attrDict.containsKey("fileType"))
		{
			options.append(bing.Bing.format("&Web.FileType={0}", new Object[]{ super.attrDict.get("fileType") }));
		}
		
		if(super.attrDict.containsKey("offset"))
		{
			options.append(bing.Bing.format("&Web.Offset={0,number,integer}", new Object[]{ super.attrDict.get("offset") }));
		}
		
		if(super.attrDict.containsKey("webOptions"))
		{
			options.append(bing.Bing.format("&Web.Options={0}", new Object[]{ super.attrDict.get("webOptions") }));
		}
		
		return options.toString();
	}
	
	public void setCount(long count)
	{
		super.attrDict.put("count", new Long(count & 0xFFFFFFFFL));
	}
	
	public void setFileType(String fileType)
	{
		super.attrDict.put("fileType", fileType);
	}
	
	public void setOffset(long offset)
	{
		super.attrDict.put("offset", new Long(offset & 0xFFFFFFFFL));
	}
	
	/**
	 * @param options One or more combinations of the <code>WEB_SEARCH_OPTIONS_<code> options separated by <code>WEB_SEARCH_OPTIONS_SEPERATOR<code>.
	 */
	public void setWebOptions(String options)
	{
		super.attrDict.put("webOptions", options);
	}
}
