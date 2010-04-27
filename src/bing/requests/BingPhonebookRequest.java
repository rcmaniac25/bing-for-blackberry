/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting phonebook/local results.
 */
public class BingPhonebookRequest extends BingRequest
{
	public static final String PHONEBOOK_SORT_OPTION_DEFAULT = "Default";
	public static final String PHONEBOOK_SORT_OPTION_DISTANCE = "Distance";
	public static final String PHONEBOOK_SORT_OPTION_RELEVANCE = "Relevance";
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		if(super.attrDict.containsKey("count"))
		{
			options.append(bing.Bing.format("&Phonebook.Count={0,number,integer}", new Object[]{ super.attrDict.get("count") }));
		}
		
		if(super.attrDict.containsKey("offset"))
		{
			options.append(bing.Bing.format("&Phonebook.Offset={0,number,integer}", new Object[]{ super.attrDict.get("offset") }));
		}
		
		if(super.attrDict.containsKey("filetype"))
		{
			options.append(bing.Bing.format("&Phonebook.FileType={0}", new Object[]{ super.attrDict.get("filetype") }));
		}
		
		if(super.attrDict.containsKey("locId"))
		{
			options.append(bing.Bing.format("&Phonebook.LocId={0}", new Object[]{ super.attrDict.get("locId") }));
		}
		
		if(super.attrDict.containsKey("sortby"))
		{
			options.append(bing.Bing.format("&Phonebook.SortBy={0}", new Object[]{ super.attrDict.get("sortby") }));
		}
		
		return options.toString();
	}
	
	public String sourceType()
	{
		return "phonebook";
	}
	
	public void setCount(long count)
	{
		super.attrDict.put("count", new Long(count & 0xFFFFFFFFL));
	}
	
	public void setOffset(long offset)
	{
		super.attrDict.put("offset", new Long(offset & 0xFFFFFFFFL));
	}
	
	public void setFileType(String filetype)
	{
		super.attrDict.put("filetype", filetype);
	}
	
	public void setLocId(String locId)
	{
		super.attrDict.put("locId", locId);
	}
	
	/**
	 * @param sortby One of the <code>PHONEBOOK_SORT_OPTION_<code> options.
	 */
	public void setSortBy(String sortby)
	{
		super.attrDict.put("sortby", sortby);
	}
}
