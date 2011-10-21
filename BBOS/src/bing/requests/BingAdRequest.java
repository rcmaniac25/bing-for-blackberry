/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting Ad results.
 */
public class BingAdRequest extends BingRequest
{
	public BingAdRequest(long pageNumber, long adUnitId, long propertyId)
	{
		super.attrDict.put("pageNumber", new Long(pageNumber & 0xFFFFFFFFL));
		super.attrDict.put("adUnitId", new Long(adUnitId & 0xFFFFFFFFL));
		super.attrDict.put("propertyId", new Long(propertyId & 0xFFFFFFFFL));
	}
	
	public String sourceType()
	{
		return "Ad";
	}
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		if(super.attrDict.containsKey("pageNumber"))
		{
			options.append(bing.Bing.format("&Ad.PageNumber={0,number,integer}", new Object[]{ super.attrDict.get("pageNumber") }));
		}
		
		if(super.attrDict.containsKey("adUnitId"))
		{
			options.append(bing.Bing.format("&Ad.AdUnitId={0,number,integer}", new Object[]{ super.attrDict.get("adUnitId") }));
		}
		
		if(super.attrDict.containsKey("propertyId"))
		{
			options.append(bing.Bing.format("&Ad.PropertyId={0,number,integer}", new Object[]{ super.attrDict.get("propertyId") }));
		}
		
		if(super.attrDict.containsKey("channelId"))
		{
			options.append(bing.Bing.format("&Ad.ChannelId={0}", new Object[]{ super.attrDict.get("channelId") }));
		}
		
		if(super.attrDict.containsKey("mlAdcount"))
		{
			options.append(bing.Bing.format("&Ad.MlAdcount={0,number,integer}", new Object[]{ super.attrDict.get("mlAdcount") }));
		}
		
		if(super.attrDict.containsKey("sbAdCount"))
		{
			options.append(bing.Bing.format("&Ad.SbAdCount={0,number,integer}", new Object[]{ super.attrDict.get("sbAdCount") }));
		}
		
		return options.toString();
	}
	
	public void setPageNumber(long pageNumber)
	{
		super.attrDict.put("pageNumber", new Long(pageNumber & 0xFFFFFFFFL));
	}
	
	public void setAdUnitId(long adUnitId)
	{
		super.attrDict.put("adUnitId", new Long(adUnitId & 0xFFFFFFFFL));
	}
	
	public void setPropertyId(long propertyId)
	{
		super.attrDict.put("propertyId", new Long(propertyId & 0xFFFFFFFFL));
	}
	
	public void setChannelId(String channelId)
	{
		super.attrDict.put("channelId", channelId);
	}
	
	public void setMlAdcount(long mlAdcount)
	{
		super.attrDict.put("mlAdcount", new Long(mlAdcount & 0xFFFFFFFFL));
	}
	
	public void setSbAdCount(long sbAdCount)
	{
		super.attrDict.put("sbAdCount", new Long(sbAdCount & 0xFFFFFFFFL));
	}
}
