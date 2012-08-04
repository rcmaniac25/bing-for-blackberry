/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

import java.util.Hashtable;

/**
 * Base class for all request objects. 
 * 
 * All general search options are set in this class.
 *
 * Each subclass of BingRequest has options that can be set on the request such as count, offset, or other source type specific options.
 * For a list of options, please refer to: http://msdn.microsoft.com/en-us/library/dd250847.aspx
 * Each option on a specific source type can be set by using the related setter method (ex: Count can be set through setCount).
 */ 
public abstract class BingRequest
{
	public static String OPTION_SEPERATOR = "+";
	
	public static String ADULT_OPTIONS_OFF = "Off";
	public static String ADULT_OPTIONS_MODERATE = "Moderate";
	public static String ADULT_OPTIONS_STRICT = "Strict";
	
	public static String SEARCH_OPTIONS_SEPERATOR = OPTION_SEPERATOR;
	public static String SEARCH_OPTIONS_DISABLE_LOCATION_DETECTION = "DisableLocationDetection";
	public static String SEARCH_OPTIONS_ENABLE_HIGHLIGHTING = "EnableHighlighting";
	
	private static String NUMBER_FORMAT = ",number";
	
	private int compositeCounter;
	
	protected Hashtable attrDict;
	
	protected BingRequest()
	{
		this.attrDict = new Hashtable();
		
		this.compositeCounter = 0;
	}
	
	/**
	 * Specifies the source type of the current request object.
	 * @return A string representing the source type of the current object.
	 */
	public abstract String sourceType();
	
	/**
	 * Specifies the source type of the current request object when used for a composite type.
	 * @return A string representing the source type of the current object for a composite type.
	 */
	public abstract String sourceTypeComposite();
	
	/**
	 * Specifies request specific options to be passed to the API
	 * @return A string representing the various set API parameters
	 */
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer();
		
		if(this.compositeCounter == 0)
		{
			if(this.attrDict.containsKey("maxTotal"))
			{
				options.append(bing.Bing.format("&$top={0" + NUMBER_FORMAT + "}", new Object[]{this.attrDict.get("maxTotal")}));
			}
			
			if(this.attrDict.containsKey("offset"))
			{
				options.append(bing.Bing.format("&$skip={0" + NUMBER_FORMAT + "}", new Object[]{this.attrDict.get("offset")}));
			}
			
			if(this.attrDict.containsKey("market"))
			{
				options.append(bing.Bing.format("&Market=%27{0}%27", new Object[]{this.attrDict.get("market")}));
			}
			
			if(this.attrDict.containsKey("adult"))
			{
				options.append(bing.Bing.format("&Adult=%27{0}%27", new Object[]{this.attrDict.get("adult")}));
			}
			
			if(this.attrDict.containsKey("options"))
			{
				options.append(bing.Bing.format("&Options=%27{0}%27", new Object[]{this.attrDict.get("options")}));
			}
			
			if(this.attrDict.containsKey("latitude"))
			{
				options.append(bing.Bing.format("&Latitude={0" + NUMBER_FORMAT + "}", new Object[]{this.attrDict.get("latitude")}));
			}
			
			if(this.attrDict.containsKey("longitude"))
			{
				options.append(bing.Bing.format("&Longitude={0" + NUMBER_FORMAT + "}", new Object[]{this.attrDict.get("longitude")}));
			}
		}
		
		return options.toString();
	}
	
	public void setMarket(String market)
	{
		this.attrDict.put("market", market);
	}
	
	public String getMarket()
	{
		if(this.attrDict.containsKey("market"))
		{
			return (String)this.attrDict.get("market");
		}
		return null;
	}
	
	/**
	 * @param adult One of the <code>ADULT_OPTIONS_<code> options.
	 */
	public void setAdult(String adult)
	{
		this.attrDict.put("adult", adult);
	}
	
	public String getAdult()
	{
		if(this.attrDict.containsKey("adult"))
		{
			return (String)this.attrDict.get("adult");
		}
		return null;
	}
	
	/**
	 * @param options One or more combinations of the <code>SEARCH_OPTIONS_<code> options separated by <code>SEARCH_OPTIONS_SEPERATOR<code>.
	 */
	public void setOptions(String options)
	{
		this.attrDict.put("options", options);
	}
	
	public String getOptions()
	{
		if(this.attrDict.containsKey("options"))
		{
			return (String)this.attrDict.get("options");
		}
		return null;
	}
	
	public void setLatitude(double latitude)
	{
		this.attrDict.put("latitude", new Double(latitude));
	}
	
	public double getLatitude()
	{
		if(this.attrDict.containsKey("latitude"))
		{
			return ((Double)this.attrDict.get("latitude")).doubleValue();
		}
		return Double.NaN;
	}
	
	public void setLongitude(double longitude)
	{
		this.attrDict.put("longitude", new Double(longitude));
	}
	
	public double getLongitude()
	{
		if(this.attrDict.containsKey("longitude"))
		{
			return ((Double)this.attrDict.get("longitude")).doubleValue();
		}
		return Double.NaN;
	}
	
	public void setMaxTotal(long maxTotal)
	{
		this.attrDict.put("maxTotal", new Long(maxTotal));
	}
	
	public long getMaxTotal()
	{
		if(this.attrDict.containsKey("maxTotal"))
		{
			return ((Long)this.attrDict.get("maxTotal")).longValue();
		}
		return -1;
	}
	
	public void setOffset(long offset)
	{
		this.attrDict.put("offset", new Long(offset));
	}
	
	public long getOffset()
	{
		if(this.attrDict.containsKey("offset"))
		{
			return ((Long)this.attrDict.get("offset")).longValue();
		}
		return -1;
	}
	
	/**
	 * Prevent the request from using it's own "standard" options.
	 */
	public void lockStandardOptions()
	{
		if(compositeCounter < 0x7FFFFFFF)
		{
			compositeCounter++;
		}
	}
	
	/**
	 * Allow the request to use it's own "standard" options.
	 * 
	 * @return A boolean stating if it's options will be used, otherwise it is still locked by some composite request.S
	 */
	public boolean unlockStandardOptions()
	{
		if(compositeCounter == 0)
		{
			//It's unlocked
			return true;
		}
		return compositeCounter-- == 1;
	}
	
	public String toString()
	{
		return this.sourceType();
	}
	
	public int hashCode()
	{
		return this.attrDict.hashCode();
	}
}
