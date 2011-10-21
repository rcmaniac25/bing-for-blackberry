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
 * A translation for a search.
 */
public class BingTranslationResult extends BingResult
{
	public BingTranslationResult(Hashtable dict)
	{
		super(dict);
	}
	
	public String getTranslatedTerm()
	{
		return (String)super.attrDict.get("TranslatedTerm");
	}
}
