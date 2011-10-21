/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.requests;

/**
 * Request object to be used when requesting translation results.
 */
public class BingTranslationRequest extends BingRequest
{
	public String sourceType()
	{
		return "Translation";
	}
	
	public String requestOptions()
	{
		StringBuffer options = new StringBuffer(super.requestOptions());
		
		if(super.attrDict.containsKey("sourceLanguage"))
		{
			options.append(bing.Bing.format("&Translation.SourceLanguage={0}", new Object[]{ super.attrDict.get("sourceLanguage") }));
		}
		
		if(super.attrDict.containsKey("targetLanguage"))
		{
			options.append(bing.Bing.format("&Translation.TargetLanguage={0}", new Object[]{ super.attrDict.get("targetLanguage") }));
		}
		
		return options.toString();
	}
	
	public void setSourceLanguage(String sourceLanguage)
	{
		super.attrDict.put("sourceLanguage", sourceLanguage);
	}
	
	public void setTargetLanguage(String targetLanguage)
	{
		super.attrDict.put("targetLanguage", targetLanguage);
	}
}
