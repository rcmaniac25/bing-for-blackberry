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
	
	/**
	 * Initialize the bundle with a list of requests.
	 */
	public BingBundleRequest()
	{
		requests = new Vector();
	}
	
	/**
	 * Initialize the bundle with a list of requests.
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
		request.lockStandardOptions();
		this.requests.addElement(request);
	}
	
	//TODO: Add ability to remove request
	
	public String sourceType()
	{
		StringBuffer sources = new StringBuffer("Composite?Sources=%27");
		
		boolean first = true;
		Enumeration en = this.requests.elements();
		while(en.hasMoreElements())
		{
			String source = ((BingRequest)en.nextElement()).sourceTypeComposite();
			if(source == null)
			{
				continue;
			}
			if(first)
			{
				sources.append(source);
				first = false;
			}
			else
			{
				sources.append(bing.Bing.format("%2b{0}", new Object[]{ source }));
			}
		}
		
		sources.append("%27&");
		
		return sources.toString();
	}
	
	public String sourceTypeComposite()
	{
		return null;
	}
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		Enumeration en = this.requests.elements();
		while(en.hasMoreElements())
		{
			BingRequest request = (BingRequest)en.nextElement();
			if(request.sourceTypeComposite() == null)
			{
				continue;
			}
			options.append(request.requestOptions());
		}
		
		return options.toString();
	}
	
	public int hashCode()
	{
		return super.hashCode() + this.requests.hashCode();
	}
}
