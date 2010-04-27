/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

import java.util.*;

/**
 * A bundle request object has the ability to "bundle" together several different request types at once.
 * This allows a client application to send a single HTTP request for multiple, different request types.
 */
public class BingBundleRequest extends BingRequest
{
	private Vector requests;
	
	private BingBundleRequest()
	{
		requests = new Vector();
	}
	
	/**
	 * Initialize the bundle with a nil terminated list of requests
	 * @param requests A list of BingRequest objects
	 */
	public BingBundleRequest(BingRequest[] requests)
	{
		this();
		if((requests != null) && (requests.length > 0))
		{
			int len = requests.length;
			for(int i = 0; i < len; i++)
			{
				addRequest(requests[i]);
			}
		}
	}
	
	/**
	 * Add a BingRequest object to this bundle.
	 * The request will be retained.
	 * @param request A new BingRequest object to be added to the request
	 */
	public void addRequest(BingRequest request)
	{
		request.removeParentOptions();
		this.requests.addElement(request);
	}
	
	public String sourceType()
	{
		StringBuffer sources = new StringBuffer();
		
		boolean first = true;
		Enumeration en = super.attrDict.elements();
		while(en.hasMoreElements())
		{
			BingRequest request = (BingRequest)en.nextElement();
			if(first)
			{
				sources.append(request.sourceType());
				first = false;
			}
			else
			{
				sources.append(bing.Bing.format("+{0}", new Object[]{ request.sourceType() }));
			}
		}
		
		return sources.toString();
	}
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		Enumeration en = super.attrDict.elements();
		while(en.hasMoreElements())
		{
			BingRequest request = (BingRequest)en.nextElement();
			String requestOptions = request.requestOptions();
			options.append(requestOptions);
		}
		
		return options.toString();
	}
	
	public int hashCode()
	{
		return super.hashCode() + this.requests.hashCode();
	}
}
