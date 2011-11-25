/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting video results.
 */
public class BingVideoRequest extends BingRequest
{
	public static String VIDEO_SORT_OPTION_DATE = "Date";
	public static String VIDEO_SORT_OPTION_RELEVANCE = "Relevance";
	
	public static String VIDEO_FILTERS_SEPERATOR = BingRequest.OPTION_SEPERATOR;
	public static String VIDEO_FILTERS_DURATION_SHORT = "Duration:Short";
	public static String VIDEO_FILTERS_DURATION_MEDIUM = "Duration:Medium";
	public static String VIDEO_FILTERS_DURATION_LONG = "Duration:Long";
	public static String VIDEO_FILTERS_ASPECT_STANDARD = "Aspect:Standard";
	public static String VIDEO_FILTERS_ASPECT_WIDESCREEN = "Aspect:Widescreen";
	public static String VIDEO_FILTERS_RESOLUTION_LOW = "Resolution:Low";
	public static String VIDEO_FILTERS_RESOLUTION_MEDIUM = "Resolution:Medium";
	public static String VIDEO_FILTERS_RESOLUTION_HIGH = "Resolution:High";
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		if(super.attrDict.containsKey("count"))
		{
			options.append(bing.Bing.format("&Video.Count={0,number,integer}", new Object[]{ super.attrDict.get("count") }));
		}
		
		if(super.attrDict.containsKey("sortby"))
		{
			options.append(bing.Bing.format("&Video.FileType={0}", new Object[]{ super.attrDict.get("sortby") }));
		}
		
		if(super.attrDict.containsKey("offset"))
		{
			options.append(bing.Bing.format("&Video.Offset={0,number,integer}", new Object[]{ super.attrDict.get("offset") }));
		}
		
		if(super.attrDict.containsKey("filters"))
		{
			options.append(bing.Bing.format("&Video.Filters={0}", new Object[]{ super.attrDict.get("filters") }));
		}
		
		return options.toString();
	}
	
	public String sourceType()
	{
		return "video";
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
	 * @param filters One or more combinations of the <code>VIDEO_FILTERS_<code> options separated by <code>VIDEO_FILTERS_SEPERATOR<code>. You cannot include more than one 
	 * value for duration in the same request.
	 */
	public void setFilters(String filters)
	{
		super.attrDict.put("filters", filters);
	}
	
	/**
	 * @param adult One of the <code>VIDEO_SORT_OPTION_<code> options.
	 */
	public void setSortBy(String sortby)
	{
		super.attrDict.put("sortby", sortby);
	}
}
