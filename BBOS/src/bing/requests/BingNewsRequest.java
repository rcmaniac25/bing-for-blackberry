/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting news results.
 */
public class BingNewsRequest extends BingRequest
{
	public static String NEWS_SORT_OPTIONS_DATE = "Date";
	public static String NEWS_SORT_OPTIONS_RELEVANCE = "Relevance";
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		if(super.attrDict.containsKey("count"))
		{
			options.append(bing.Bing.format("&News.Count={0,number,integer}", new Object[]{ super.attrDict.get("count") }));
		}
		
		if(super.attrDict.containsKey("offset"))
		{
			options.append(bing.Bing.format("&News.Offset={0,number,integer}", new Object[]{ super.attrDict.get("offset") }));
		}
		
		if(super.attrDict.containsKey("category"))
		{
			options.append(bing.Bing.format("&News.Category={0}", new Object[]{ super.attrDict.get("category") }));
		}
		
		if(super.attrDict.containsKey("locationOverride"))
		{
			options.append(bing.Bing.format("&News.LocationOverride={0}", new Object[]{ super.attrDict.get("locationOverride") }));
		}
		
		if(super.attrDict.containsKey("sortby"))
		{
			options.append(bing.Bing.format("&News.Category={0}", new Object[]{ super.attrDict.get("sortby") }));
		}
		
		return options.toString();
	}
	
	public String sourceType()
	{
		return "news";
	}
	
	public void setCount(long count)
	{
		super.attrDict.put("count", new Long(count & 0xFFFFFFFFL));
	}
	
	public void setOffset(long offset)
	{
		super.attrDict.put("offset", new Long(offset & 0xFFFFFFFFL));
	}
	
	public void setCategory(String category)
	{
		super.attrDict.put("category", category);
	}
	
	public void setLocationOverride(String locationOverride)
	{
		super.attrDict.put("locationOverride", locationOverride);
	}
	
	/**
	 * @param sortBy One of the <code>NEWS_SORT_OPTIONS_<code> options.
	 */
	public void setSortBy(String sortBy)
	{
		super.attrDict.put("sortby", sortBy);
	}
}
