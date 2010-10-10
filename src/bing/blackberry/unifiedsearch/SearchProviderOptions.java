//#preprocessor

/**
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Vincent Simonetti
 */
package bing.blackberry.unifiedsearch;

//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
import net.rim.device.api.system.Bitmap;
import net.rim.device.api.system.EncodedImage;
import net.rim.device.api.ui.image.Image;
import net.rim.device.api.ui.image.ImageFactory;
import net.rim.device.api.unifiedsearch.registry.RegistrationToken;
import net.rim.device.api.unifiedsearch.searchables.ExternalSearchProvider;
import bing.Bing;
import bing.requests.BingRequest;
//#endif

/**
 * Options for customized Unified Search integration.
 */
public class SearchProviderOptions
{
//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
	//Use the preprocessor so that code memory is saved if not supported.
	RegistrationToken register;
	SearchCallback callback;
//#endif
	
	/**
	 * Create a new Search Provider Options object.
	 * @param callback The SearchCallback that will handle searches and results.
	 */
	public SearchProviderOptions(SearchCallback callback)
	{
//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
		if(callback == null)
		{
			throw new NullPointerException();
		}
		this.callback = callback;
//#endif
	}
	
//#ifndef BlackBerrySDK4.5.0 | BlackBerrySDK4.6.0 | BlackBerrySDK4.6.1 | BlackBerrySDK4.7.0 | BlackBerrySDK5.0.0 | NO_SIGNING
	class ExternalSearch implements ExternalSearchProvider
	{
		private long id;
		
		public long getContentType()
		{
			return SearchProviderOptions.this.callback.getContentType();
		}
		
		public Image getProviderIcon()
		{
			//Not efficient but gets the job done in the easiest way possible for the developer.
			Object img = SearchProviderOptions.this.callback.getImage();
			if(img instanceof Image)
			{
				return (Image)img;
			}
			else if(img instanceof Bitmap)
			{
				return ImageFactory.createImage((Bitmap)img);
			}
			else if(img instanceof EncodedImage)
			{
				return ImageFactory.createImage((EncodedImage)img);
			}
			else if(img instanceof javax.microedition.lcdui.Image)
			{
				javax.microedition.lcdui.Image jImg = (javax.microedition.lcdui.Image)img;
				
				int w = jImg.getWidth();
				int h = jImg.getHeight();
				int[] rgbData = new int[w * h];
				jImg.getRGB(rgbData, 0, w, 0, 0, w, h);
				
				Bitmap map = new Bitmap(w, h);
				map.createAlpha(Bitmap.ALPHA_BITDEPTH_8BPP); //Not the most efficient but probably more efficient then going through entire image to check for alpha.
				map.setARGB(rgbData, 0, w, 0, 0, w, h);
				
				return ImageFactory.createImage(map);
			}
			return null;
		}
		
		public String getProviderName()
		{
			return SearchProviderOptions.this.callback.getName();
		}
		
		public long getRegistrationID()
		{
			return id;
		}
		
		public void search(String keywords)
		{
			SearchCallback callback = SearchProviderOptions.this.callback;
			if(callback != null)
			{
				if(callback.supportedSearch(keywords))
				{
					callback.searchStarted();
					Bing bing = callback.getBing();
					BingRequest request = callback.getRequest();
					bing.search(keywords, request, callback);
				}
			}
		}
		
		public void setRegistrationID(long id)
		{
			this.id = id;
		}
	}
//#endif
}
