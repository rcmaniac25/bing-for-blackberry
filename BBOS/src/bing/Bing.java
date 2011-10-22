//#preprocessor

/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing;

import java.io.*;

import javax.microedition.io.*;

import javax.xml.parsers.*;

import bing.common.*;
import bing.requests.*;
import bing.responses.*;

/**
 * A service object used to send, receive, and process Bing API requests and responses.
 */
public class Bing
{
	private static String BING_URL = "http://api.bing.net/xml.aspx?";
	
	private net.rim.device.api.i18n.ResourceBundle _resources;
	
	private String appId;
//#ifdef BING_DEBUG
	private boolean errorRet;
//#endif
	
	/**
	 * Initialize a Bing service object. To obtain a Bing Application ID visit http://www.bing.com/developers
	 * @param application_ID A valid Bing Application ID
	 */
	public Bing(final String application_ID)
	{
		this.appId = application_ID;
		this._resources = net.rim.device.api.i18n.ResourceBundle.getBundle(BingResource.BUNDLE_ID, BingResource.BUNDLE_NAME);
	}
	
//#ifdef BING_DEBUG
	/**
	 * Set if an error occurs that is server-side, should a BingError be returned in the result.
	 * @param error <code>true</code> if an error should be returned, <code>false</code> if otherwise.
	 */
	public void setErrorReturn(boolean error)
	{
		this.errorRet = error;
	}
	
	/**
	 * Get if an error occurs that is server-side, should a BingError be returned in the result.
	 * @return <code>true</code> if an error should be returned, <code>false</code> if otherwise.
	 */
	public boolean getErrorReturn()
	{
		return this.errorRet;
	}
//#endif
	
	/**
	 * Get the Bing Application ID
	 * @return The Bing Application ID
	 */
	public synchronized String getAppID()
	{
		//return new String(this.appId); //Don't use, it's not recommended, somewhat poor code, and since String is immutable it is pointless to copy the String if it can't be changed anyway.
		return this.appId;
	}
	
	/**
	 * Set the Bing Application ID. To obtain a Bing Application ID visit http://www.bing.com/developers
	 * @param appId The Bing Application ID to use.
	 */
	public synchronized void setAppID(String appId)
	{
		this.appId = appId;
	}
	
	/**
	 * Perform a synchronous search on the Bing API.
	 * @param query A string specifying the query to perform
	 * @param request A BingRequest object
	 * @return The response from the Bing API
	 */
	public synchronized BingResponse search(final String query, BingRequest request)
	{
		String requestURL = this.requestUrl(query, request);
		
//#ifdef BING_DEBUG
		System.out.println(format(_resources.getString(BingResource.SEARCH_SYNC), new Object[]{ requestURL }));
//#endif
		
		BingXMLDelegate xmlDelegate = null;
		InputStream in = null;
		try
		{
			in = Connector.openInputStream(requestURL);
			
			SAXParserFactory factory = SAXParserFactory.newInstance();
			SAXParser builder = factory.newSAXParser();
			
			xmlDelegate = new BingXMLDelegate(this._resources
//#ifdef BING_DEBUG
					, this.errorRet
//#endif
					);
			builder.parse(in, xmlDelegate);
		}
		catch (Exception e)
		{
			xmlDelegate = null;
//#ifdef BING_DEBUG
			System.out.println(_resources.getString(BingResource.SEARCH_ERROR));
//#endif
		}
		finally
		{
			if(in != null)
			{
				try
				{
					in.close();
				}
				catch(Exception e)
				{
				}
			}
		}
		return xmlDelegate != null ? xmlDelegate.getResponse() : null;
	}
	
	/**
	 * Perform an asynchronous search on the Bing API.
	 * @param query A string specifying the query to perform
	 * @param request A {@link BingRequest} object
	 * @param delegate An object implementing the receiveBingResponse selector
	 */
	public synchronized void search(final String query, BingRequest request, BingAsyncRequestNotification delegate)
	{
		String requestURL = this.requestUrl(query, request);
		try
		{
			HttpConnection urlRequest = (HttpConnection)Connector.open(requestURL, Connector.READ_WRITE);
			BingAsyncRequest asyncDelegate = new BingAsyncRequest(delegate, this._resources
//#ifdef BING_DEBUG
					, this.errorRet
//#endif
					);
			
//#ifdef BING_DEBUG
			System.out.println(format(_resources.getString(BingResource.SEARCH_ASYNC), new Object[]{ requestURL }));
//#endif
			
			requestDataAsync(urlRequest, asyncDelegate);
		}
		catch(Exception e)
		{
//#ifdef BING_DEBUG
			System.out.println(_resources.getString(BingResource.SEARCH_ERROR));
//#endif
		}
	}
	
	private void requestDataAsync(HttpConnection con, BingAsyncRequest async)
	{
		Thread asyncDown = new Thread(async);
		async.setCon(con);
		
		asyncDown.start();
	}
	
	/**
	 * Generates the URL to use in order to query the Bing API with.
	 * @param query A string specifying the query to perform
	 * @param request A BingRequest object
	 * @return A string representing the URL to use in querying the Bing API
	 */
	public synchronized String requestUrl(final String query, BingRequest request)
	{
		StringBuffer requestURL = new StringBuffer(format("{0}xmltype=attributebased&AppId={1}&Query={2}&Sources={3}", 
				new Object[]{ BING_URL, this.appId, encodeUrl(query), request.sourceType() }));
		
		//replace(query, " ", "%20")
		
		String requestOptions = request.requestOptions();
		if (requestOptions != null)
		{
			requestURL.append(requestOptions);
		}
		return requestURL.toString();
	}
	
	private static String URL_UNRESERVED = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ" +
		"abcdefghijklmnopqrstuvwxyz" +
		"0123456789-_.~";
	private static final char[] HEX = "0123456789ABCDEF".toCharArray();
	
	//requestUrl formats the URL, this encodes it so it is formatted in a manner that can be interpreted properly
	private String encodeUrl(String url)
	{
		byte[] bytes = null;
		try
		{
			ByteArrayOutputStream bos = new ByteArrayOutputStream();
			DataOutputStream dos = new DataOutputStream(bos);
			dos.writeUTF(url);
			bytes = bos.toByteArray();
		}
		catch (IOException e)
		{
			// ignore
		}
		if(bytes != null)
		{
			StringBuffer buf = new StringBuffer();
			int len = bytes.length;
			for(int i = 2; i < len; i++)
			{
				byte b = bytes[i];
				/*
				if(b == '%')
				{
					buf.append((char)b);
					buf.append((char)bytes[i++]);
					buf.append((char)bytes[i++]);
				}
				else*/if(URL_UNRESERVED.indexOf(b) >= 0)
				{
					buf.append((char)b);
				}
				else
				{
					buf.append('%').append(HEX[(b >> 4) & 0x0f]).append(HEX[b & 0x0f]);
				}
			}
			return buf.toString();
		}
		return "";
	}
	
	/*
	private static String replace(String source, String oldValue, String newValue)
	{
		StringBuffer buffer = new StringBuffer();
		
		int len = source.length();
		for(int i = 0; i < len; i++)
		{
			char c = source.charAt(i);
			if(c == oldValue.charAt(0))
			{
				if(source.startsWith(oldValue, i))
				{
					buffer.append(newValue);
					i += oldValue.length() - 1;
					continue;
				}
			}
			buffer.append(c);
		}
		
		return buffer.toString();
	}
	*/
	
	public int hashCode()
	{
		return this.appId.hashCode();
	}
	
	public boolean equals(Object obj)
	{
		if((obj != null) && (obj instanceof Bing))
		{
			return this.appId.equals(((Bing)obj).appId);
		}
		return false;
	}
	
	/**
	 * Internal use only, but for those who want to know it is a manual implemented java.text.MessageFormat.
	 */
	public static String format(String format, Object[] args)
	{
		//Until net.rim.device.api.i18n.MessageFormat supports the "number" property this has to be done manually.
		int len = format.length();
		StringBuffer buf = new StringBuffer();
		boolean onFormat = false;
		for(int i = 0; i < len; i++)
		{
			char c;
			if(onFormat)
			{
				StringBuffer index = new StringBuffer();
				boolean hasIndex = false;
				String elementFormat = null;
				String styleFormat = null;
				for(int k = i; k < len; k++, i++)
				{
					c = format.charAt(k);
					if(c == '}')
					{
						onFormat = false;
						break;
					}
					else if(Character.isDigit(c) && !hasIndex)
					{
						index.append(c);
					}
					else
					{
						if(c != '}')
						{
							String sub = format.substring(k + 1, format.indexOf('}', k));
							int l = sub.length();
							k += l;
							i += l;
							if(sub.indexOf(',') != -1)
							{
								elementFormat = sub.substring(0, sub.indexOf(','));
								styleFormat = sub.substring(sub.indexOf(',') + 1);
							}
							else
							{
								elementFormat = sub;
							}
						}
						hasIndex = true;
					}
				}
				buf.append(inFormat(args[Integer.parseInt(index.toString())], elementFormat, styleFormat));
			}
			else
			{
				c = format.charAt(i);
				if(c == '{')
				{
					onFormat = true;
				}
				else
				{
					buf.append(c);
				}
			}
		}
		return buf.toString();
	}
	
	private static String inFormat(Object obj, String element, String style)
	{
		if(element != null)
		{
			if(element.equals("number"))
			{
				//Numbers have to be supported manually
				return numFormat(obj, style);
			}
			else if(element.equals("time") || element.equals("date"))
			{
				//Time and date is supported by BlackBerry so use the built in system.
				String format = "{0," + element;
				if(style != null)
				{
					format += "," + style;
				}
				format += "}";
				return net.rim.device.api.i18n.MessageFormat.format(format, new Object[]{ obj });
			}
		}
		return obj.toString();
	}
	
	private static String numFormat(Object obj, String style)
	{
		javax.microedition.global.Formatter formatter = new javax.microedition.global.Formatter();
		//Numbers are the only supported element type right now
		if(obj instanceof Double || obj instanceof Float)
		{
			double dob = 0.0;
			if(obj instanceof Double)
			{
				dob = ((Double)obj).doubleValue();
			}
			else
			{
				dob = ((Float)obj).doubleValue();
			}
			if(style != null)
			{
				return numStyleFormat(formatter, dob, style);
			}
			return formatter.formatNumber(dob);
		}
		else if(obj instanceof Integer || obj instanceof Long || obj instanceof Short || obj instanceof Byte)
		{
			long l = 0;
			if(obj instanceof Integer)
			{
				l = ((Integer)obj).longValue();
			}
			else if(obj instanceof Long)
			{
				l = ((Long)obj).longValue();
			}
			else if(obj instanceof Short)
			{
				l = ((Short)obj).shortValue();
			}
			else
			{
				l = ((Byte)obj).byteValue();
			}
			if(style != null)
			{
				return numStyleFormat(formatter, l, style);
			}
			return formatter.formatNumber(l);
		}
		else
		{
			return obj.toString();
		}
	}
	
	private static String numStyleFormat(javax.microedition.global.Formatter formatter, long l, String style)
	{
		if(style.equals("currency"))
		{
			return formatter.formatCurrency(l);
		}
		else if(style.equals("percent"))
		{
			return formatter.formatPercentage((float)l, 4);
		}
		else
		{
			return Long.toString(l);
		}
	}
	
	private static String numStyleFormat(javax.microedition.global.Formatter formatter, double dob, String style)
	{
		if(style.equals("currency"))
		{
			return formatter.formatCurrency(dob);
		}
		else if(style.equals("percent"))
		{
			return formatter.formatPercentage((float)dob, 4);
		}
		else if(style.equals("integer"))
		{
			return Long.toString((long)dob);
		}
		else
		{
			//Formatting such as ##.### is not supported because it would be a pain in the butt to implement, I already went above and beyond for actually writing a format function
			return Double.toString(dob);
		}
	}
}
