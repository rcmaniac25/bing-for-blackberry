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
	public static final String DEFAULT_SEARCH_MARKET = "en-US";
	public static final String DEFAULT_API_VERSION = "2.2";
	
	public static final String OPTION_SEPERATOR = "+";
	
	public static final String ADULT_OPTIONS_OFF = "Off";
	public static final String ADULT_OPTIONS_MODERATE = "Moderate";
	public static final String ADULT_OPTIONS_STRICT = "Strict";
	
	public static final String SEARCH_OPTIONS_SEPERATOR = OPTION_SEPERATOR;
	public static final String SEARCH_OPTIONS_DISABLE_LOCATION_DETECTION = "DisableLocationDetection";
	public static final String SEARCH_OPTIONS_ENABLE_HIGHLIGHTING = "EnableHighlighting";
	
	private static final String DOUBLE_FORMAT = ",number";
	
	protected Hashtable attrDict;
	
	protected BingRequest()
	{
		this.attrDict = new Hashtable();
		//Set up default values
		this.attrDict.put("version", DEFAULT_API_VERSION);
		this.attrDict.put("market", DEFAULT_SEARCH_MARKET);
	}
	
	/**
	 * Specifies the source type of the current request object.
	 * @return A string representing the source type of the current object.
	 */
	public abstract String sourceType();
	
	/**
	 * Specifies request specific options to be passed to the API
	 * @return A string representing the various set API parameters
	 */
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer();
		
		if(this.attrDict.containsKey("version"))
		{
			options.append(bing.Bing.format("&Version={0}", new Object[]{this.attrDict.get("version")}));
		}
		
		if(this.attrDict.containsKey("market"))
		{
			options.append(bing.Bing.format("&Market={0}", new Object[]{this.attrDict.get("market")}));
		}
		
		if(this.attrDict.containsKey("adult"))
		{
			options.append(bing.Bing.format("&Adult={0}", new Object[]{this.attrDict.get("adult")}));
		}
		
		if(this.attrDict.containsKey("options"))
		{
			options.append(bing.Bing.format("&Options={0}", new Object[]{this.attrDict.get("options")}));
		}
		
		if(this.attrDict.containsKey("latitude"))
		{
			options.append(bing.Bing.format("&Latitude={0" + DOUBLE_FORMAT + "}", new Object[]{this.attrDict.get("latitude")}));
		}
		
		if(this.attrDict.containsKey("longitude"))
		{
			options.append(bing.Bing.format("&Longitude={0" + DOUBLE_FORMAT + "}", new Object[]{this.attrDict.get("longitude")}));
		}
		
		if(this.attrDict.containsKey("language"))
		{
			options.append(bing.Bing.format("&UILanguage={0}", new Object[]{this.attrDict.get("language")}));
		}
		
		if(this.attrDict.containsKey("radius"))
		{
			options.append(bing.Bing.format("&Radius={0" + DOUBLE_FORMAT + "}", new Object[]{this.attrDict.get("radius")}));
		}
		
		return options.toString();
	}
	
	public void setMarket(String market)
	{
		this.attrDict.put("market", market);
	}
	
	public void setVersion(String version)
	{
		this.attrDict.put("version", version);
	}
	
	/**
	 * @param adult One of the <code>ADULT_OPTIONS_<code> options.
	 */
	public void setAdult(String adult)
	{
		this.attrDict.put("adult", adult);
	}
	
	/**
	 * @param options One or more combinations of the <code>SEARCH_OPTIONS_<code> options separated by <code>SEARCH_OPTIONS_SEPERATOR<code>.
	 */
	public void setOptions(String options)
	{
		this.attrDict.put("options", options);
	}
	
	public void setLatitude(double latitude)
	{
		this.attrDict.put("latitude", new Double(latitude));
	}
	
	public void setLongitude(double longitude)
	{
		this.attrDict.put("longitude", new Double(longitude));
	}
	
	public void setLanguage(String language)
	{
		this.attrDict.put("language", language);
	}
	
	public void setRadius(double radius)
	{
		this.attrDict.put("radius", new Double(radius));
	}
	
	public void removeParentOptions()
	{
		this.attrDict.remove("market");
		this.attrDict.remove("version");
		this.attrDict.remove("adult");
		this.attrDict.remove("options");
		this.attrDict.remove("latitude");
		this.attrDict.remove("longitude");
		this.attrDict.remove("language");
		this.attrDict.remove("radius");
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
