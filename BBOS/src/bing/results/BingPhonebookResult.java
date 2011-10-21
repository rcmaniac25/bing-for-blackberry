/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.results;

import java.util.Hashtable;

/**
 * Represents an individual phonebook result.
 */
public class BingPhonebookResult extends BingResult
{
	public BingPhonebookResult(Hashtable dict)
	{
		super(dict);
		if(super.attrDict.containsKey("Latitude"))
		{
			super.attrDict.put("Latitude", Double.valueOf((String)super.attrDict.get("Latitude")));
		}
		if(super.attrDict.containsKey("Longitude"))
		{
			super.attrDict.put("Longitude", Double.valueOf((String)super.attrDict.get("Longitude")));
		}
		if(super.attrDict.containsKey("UserRating"))
		{
			super.attrDict.put("UserRating", Double.valueOf((String)super.attrDict.get("UserRating")));
		}
		if(super.attrDict.containsKey("ReviewCount"))
		{
			super.attrDict.put("ReviewCount", new Long(Long.parseLong((String)super.attrDict.get("ReviewCount"))));
		}
	}
	
	public double getLatitude()
	{
		return ((Double)super.attrDict.get("Latitude")).doubleValue();
	}
	
	public double getLongitude()
	{
		return ((Double)super.attrDict.get("Longitude")).doubleValue();
	}
	
	public double getUserRating()
	{
		return ((Double)super.attrDict.get("UserRating")).doubleValue();
	}
	
	public long getReviewCount()
	{
		return ((Long)super.attrDict.get("ReviewCount")).longValue();
	}
	
	public String getTitle()
	{
		return (String)super.attrDict.get("Title");
	}
	
	public String getUrl()
	{
		return (String)super.attrDict.get("Url");
	}
	
	public String getBusinessUrl()
	{
		return (String)super.attrDict.get("BusinessUrl");
	}
	
	public String getCity()
	{
		return (String)super.attrDict.get("City");
	}
	
	public String getCountryOrRegion()
	{
		return (String)super.attrDict.get("CountryOrRegion");
	}
	
	public String getDisplayUrl()
	{
		return (String)super.attrDict.get("DisplayUrl");
	}
	
	public String getPhoneNumber()
	{
		return (String)super.attrDict.get("PhoneNumber");
	}
	
	public String getPostalCode()
	{
		return (String)super.attrDict.get("PostalCode");
	}
	
	public String getStateOrProvince()
	{
		return (String)super.attrDict.get("StateOrProvince");
	}
	
	public String getUniqueId()
	{
		return (String)super.attrDict.get("UniqueId");
	}
	
	public String getBusiness()
	{
		return (String)super.attrDict.get("Business");
	}
	
	public String getAddress()
	{
		return (String)super.attrDict.get("Address");
	}
}
