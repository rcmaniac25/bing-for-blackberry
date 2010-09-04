//#preprocessor

/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.common;

import javax.microedition.io.*;

import javax.xml.parsers.*;

import bing.BingResource;

import java.io.*;

/**
 * Used internally
 */
public class BingAsyncRequest implements Runnable
{
	private bing.BingAsyncRequestNotification delegate;
	private ByteArrayOutputStream responseData;
	private HttpConnection con;
	private net.rim.device.api.i18n.ResourceBundle _resources;
//#ifdef BING_DEBUG
	private boolean errorRet;
//#endif
	
	/**
	 * Initialize the async request with a delegate
	 * @param theDelegate A delegate which will get called with a BingResponse when the request completes.
	 */
	public BingAsyncRequest(bing.BingAsyncRequestNotification theDelegate, net.rim.device.api.i18n.ResourceBundle resource
//#ifdef BING_DEBUG
			, boolean returnError
//#endif
			)
	{
//#ifdef BING_DEBUG
		this.errorRet = returnError;
//#endif
		this.delegate = theDelegate;
		this._resources = resource;
	}
	
	public void setCon(HttpConnection con)
	{
		this.con = con;
	}

	public void run()
	{
		int count = 0;
		InputStream in = null;
		int dataBufLen;
		byte[] dataBuf = new byte[dataBufLen = 1024];
		byte[] read;
		while(con != null)
		{
			try
			{
				switch(count)
				{
					case 0:
						//Wait for response
						int code = this.con.getResponseCode();
						if(code == HttpConnection.HTTP_OK)
						{
							//If a connection has been made then get the data length and start the ball rolling
							code = (int)this.con.getLength();
							onResponse(code);
							count++;
						}
						break;
					case 1:
						//Read the data
						if(in == null)
						{
							//If no input connection exists then open one, this will get closed then con.close is called.
							in = this.con.openInputStream();
						}
						//Get the number of currently available
						int av = in.available();
						//New data exists
						if(av > dataBufLen)
						{
							//The new data is larger then the buffer
							dataBuf = new byte[dataBufLen = av];
						}
						//Read in the data
						av = in.read(dataBuf);
						if(av > 0)
						{
							//Data has been read
							read = new byte[av];
							System.arraycopy(dataBuf, 0, read, 0, av);
							onData(read);
						}
						else if(av < 0)
						{
							//No more data exists, next step
							count++;
						}
						break;
					case 2:
						//Finish and parse
						onFinish();
						break;
				}
			}
			catch(Exception e)
			{
				//Error
				onError(e);
			}
		}
	}
	
	public synchronized void onResponse(int dataSize)
	{
//#ifdef BING_DEBUG
		System.out.println(_resources.getString(BingResource.ASYNC_RESPONSE));
//#endif
		dataSize = dataSize <= 0 ? 100 : dataSize; //Just in case the server doesn't support size.
		if(this.responseData != null)
		{
			try
			{
				this.responseData.close();
			}
			catch(Exception e)
			{
			}
			this.responseData = null;
		}
		this.responseData = new ByteArrayOutputStream(dataSize);
	}
	
	public synchronized void onData(byte[] data)
	{
		try
		{
			this.responseData.write(data);
		}
		catch (Exception e)
		{
		}
	}
	
	public synchronized void onError(Exception e)
	{
		this.delegate.receiveBingResponse(null);
		try
		{
			this.con.close();
			this.con = null;
		}
		catch(Exception ee)
		{
		}
	}
	
	public synchronized void onFinish()
	{
		BingXMLDelegate xmlDelegate = null;
		InputStream in = null;
		try
		{
			SAXParserFactory factory = SAXParserFactory.newInstance();
			SAXParser builder = factory.newSAXParser();
			
			in = new ByteArrayInputStream(this.responseData.toByteArray());
			
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
		
//#ifdef BING_DEBUG
		System.out.println(_resources.getString(BingResource.ASYNC_DELEGATE));
//#endif
		this.delegate.receiveBingResponse(xmlDelegate != null ? xmlDelegate.getResponse() : null);
		
		try
		{
			this.con.close();
			this.con = null;
			this.responseData.close();
			this.responseData = null;
		}
		catch(Exception ee)
		{
		}
	}
}
