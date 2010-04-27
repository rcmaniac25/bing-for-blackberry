//#preprocess

/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.common;

import org.xml.sax.*;
import org.xml.sax.helpers.*;

import java.util.*;

import bing.BingResource;
import bing.responses.*;
import bing.results.*;

/**
 * Used internally
 */
public final class BingXMLDelegate extends DefaultHandler
{
	private BingResponse response;
	private BingResponse current;
	private Stack lastResult;
	private Stack lastResultElement;
	private ClassDictionary classes;
	private String query, alteredQuery, alterationOverrideQuery;
	private net.rim.device.api.i18n.ResourceBundle _resources;
//#ifdef BING_DEBUG
	private boolean _errorRet;
//#endif
	
	//-------ONLY THING THAT SHOULD BE EDITED--------------------
	/* As of Component Pack 5.0.0.25 there is no reflection capability available on the BlackBerry platform. To get around this problem a fixed list that acts like
	 * reflection has been created.
	 * 
	 * 1. To add new SourceTypes create the source types (previously defined SourceTypes can be used as templates and examples).
	 * 		-Request types should go in bing.request.*
	 * 		-Response types should go in bing.responses.*
	 * 		-Result types should go in bing.response.*
	 * 		-All types that don't fit in the above 3 categories should go in bing.response.*. If this type can contain other types (so it is an array of elements, not
	 * 			an array of attributes) it should implement the bing.common.Array type. On the topic of elements, each element can contain a list of attributes, these
	 * 			attributes will be handed to all the above types in a Hashtable.
	 * 2. Increment the type counts for each specified type at #1.
	 * 3. Add the data type to #2. To keep with formatting look at the already implemented types. The XML element name should be used as the KEY and the data type's class
	 * 		should be used as the VALUE.
	 * 4. Finally in #3 the added RESULTS and NON-RESPONSE/REQUEST data types should be added, again look at the manner that they are implemented so that it can be easily
	 * 		updated and maintained. Some types that would have two or more XML element names might be the same object. If this is the case #2 should have the same class
	 * 		type but with a different XML element name and index, the indexes should then overlap in #3.
	 * 
	 * In the event that reflection is supported on a future version of the BlackBerry then the commented out PUSDO code should be used (PUSDO code replaced with real code)
	 * and #1, #3 (reflectiveLoad), and getClassIndex should be removed. The code should work exactly the same only using reflection instead of manual modification. This
	 * instruction set should be changed to only include step #2.
	 */
	
	//#1
	private static final int RESULT_INDEX = 0;
	private static final int RESPONSE_INDEX = RESULT_INDEX + 11; //Number of result types
	private static final int COMMON_INDEX = RESPONSE_INDEX + 11; //Number of response types
	
	public BingXMLDelegate(net.rim.device.api.i18n.ResourceBundle resource
//#ifdef BING_DEBUG
			, boolean returnError
//#endif
			)
	{
//#ifdef BING_DEBUG
		this._errorRet = returnError;
//#endif
		this._resources = resource;
		
		this.response = null;
		this.lastResult = new Stack();
		this.lastResultElement = new Stack();
		
		this.classes = new ClassDictionary();
		
		//#2
		//Results: RESULT_INDEX
		this.classes.put("web:WebResult", BingWebResult.class);						//0
		this.classes.put("pho:PhonebookResult", BingPhonebookResult.class);			//1
		this.classes.put("mms:VideoResult", BingVideoResult.class);					//2
		this.classes.put("mms:ImageResult", BingImageResult.class);					//3
		this.classes.put("news:NewsResult", BingNewsResult.class);					//4
		this.classes.put("ads:AdResult", BingAdResult.class);						//5
		this.classes.put("rs:RelatedSearchResult", BingRelatedSearchResult.class);	//6
		this.classes.put("tra:TranslationResult", BingTranslationResult.class);		//7
		this.classes.put("spl:SpellResult", BingSpellResult.class);					//8
		this.classes.put("mw:MobileWebResult", BingMobileWebResult.class);			//9
		this.classes.put("ia:InstantAnswerResult", BingInstantAnswerResult.class);	//10
		
		//Responses: RESPONSE_INDEX
		this.classes.put("web:Web", BingWebResponse.class);							//0
		this.classes.put("pho:Phonebook", BingPhonebookResponse.class);				//1
		this.classes.put("mms:Image", BingImageResponse.class);						//2
		this.classes.put("mms:Video", BingVideoResponse.class);						//3
		this.classes.put("news:News", BingNewsResponse.class);						//4
		this.classes.put("ads:Ad", BingAdResponse.class);							//5
		this.classes.put("rs:RelatedSearch", BingRelatedSearchResponse.class);		//6
		this.classes.put("tra:Translation", BingTranslationResponse.class);			//7
		this.classes.put("spl:Spell", BingSpellResponse.class);						//8
		this.classes.put("mw:MobileWeb", BingMobileWebResponse.class);				//9
		this.classes.put("ia:InstantAnswer", BingInstantAnswerResponse.class);		//10
		
		//Common types: COMMON_INDEX
		//-Web
		this.classes.put("web.DeepLinks", DeepLinkArray.class);						//0
		this.classes.put("web:DeepLink", DeepLink.class);							//1
		this.classes.put("web:SearchTags", SearchTagArray.class);					//2
		this.classes.put("web:WebSearchTag", SearchTag.class);						//3
		//-Image
		this.classes.put("mms:Thumbnail", Thumbnail.class);							//4
		//-Video
		this.classes.put("mms:StaticThumbnail", Thumbnail.class);					//5
		//-News
		this.classes.put("news:NewsCollections", NewsCollectionArray.class);		//6
		this.classes.put("news:NewsCollection", NewsCollection.class);				//7
		this.classes.put("news:NewsArticles", NewsArticleArray.class);				//8
		this.classes.put("news:NewsArticle", BingNewsResult.class);					//9
		this.classes.put("news:RelatedSearches", RelatedSearchArray.class);			//10
		this.classes.put("news:NewsRelatedSearch", BingRelatedSearchResult.class);	//11
	}
	
	//Though not well organized this is up here so changes can be made without looking through the entire class.
	private BingResult reflectiveLoad(String name, String namespace, Attributes attributeDict) throws Exception
	{
		//This (and getClassIndex) are not efficient but until reflection is supported there is not much that can be done about it.
		int index = getClassIndex(name, namespace);
		if(index > -1)
		{
			Hashtable aDict = loadDictionary(attributeDict);
			switch(index)
			{
				//#3
				case RESULT_INDEX + 0:
					return new BingWebResult(aDict);
				case RESULT_INDEX + 1:
					return new BingPhonebookResult(aDict);
				case RESULT_INDEX + 2:
					return new BingVideoResult(aDict);
				case RESULT_INDEX + 3:
					return new BingImageResult(aDict);
				case RESULT_INDEX + 4:
				case COMMON_INDEX + 9:
					return new BingNewsResult(aDict);
				case RESULT_INDEX + 5:
					return new BingAdResult(aDict);
				case RESULT_INDEX + 7:
					return new BingTranslationResult(aDict);
				case RESULT_INDEX + 8:
					return new BingSpellResult(aDict);
				case RESULT_INDEX + 9:
					return new BingMobileWebResult(aDict);
				case RESULT_INDEX + 10:
					return new BingInstantAnswerResult(aDict);
					
				case COMMON_INDEX + 0:
					return new DeepLinkArray(aDict);
				case COMMON_INDEX + 1:
					return new DeepLink(aDict);
				case COMMON_INDEX + 2:
					return new SearchTagArray(aDict);
				case COMMON_INDEX + 3:
					return new SearchTag(aDict);
				case COMMON_INDEX + 4:
				case COMMON_INDEX + 5:
					return new Thumbnail(aDict);
				case COMMON_INDEX + 6:
					return new NewsCollectionArray(aDict);
				case COMMON_INDEX + 7:
					return new NewsCollection(aDict);
				case COMMON_INDEX + 8:
					return new NewsArticleArray(aDict);
				case COMMON_INDEX + 10:
					return new RelatedSearchArray(aDict);
				case COMMON_INDEX + 11:
				case RESULT_INDEX + 6:
					return new BingRelatedSearchResult(aDict);
				default:
					throw new UnsupportedOperationException(bing.Bing.format(_resources.getString(BingResource.XML_REFLECT_CLASSNOTIMPLEMENTED), new Object[] { new Integer(index), name }));
			}
		}
		throw new InstantiationException(_resources.getString(BingResource.XML_REFLECT_NOCLASS));
	}
	
	//---------------END EDIT ZONE------------------------------
	
	public synchronized BingResponse getResponse()
	{
		return this.response;
	}
	
	public synchronized void setResponse(BingResponse response)
	{
		this.response = response;
	}
	
	public void startElement(String namespaceURI, String elementName, String qualifiedName, Attributes attributeDict) throws SAXException
	{
		try
		{
			if(elementName.endsWith("Result"))
			{
				if(current != null)
				{
					//Manual loaded types
					//Assume bing.results.Bing<ElementName>
					BingResult res = reflectiveLoad(elementName, "bing.results.Bing", attributeDict);
					this.lastResult.push(res);
					this.lastResultElement.push(elementName);
					current.addResult(res);
					
					//Reflection loaded types
					/*
					if(classes.containsKey(elementName))
					{
						Class clazz = classes.get(elementName);
						ReflectClass rClass = ReflectClass.getClass(clazz);
						BingResult res = rClass.newInstance(rClass.getConstructor(clazz.getName() + "(Hashtable)"), new Object[]{ loadDictionary(attributeDict) });
						this.lastResult.push(res);
						this.lastResultElement.push(elementName);
						current.addResult(res);
					}
					*/
				}
			}
			else if(elementName.equals("Query"))
			{
				query = attributeDict.getValue(namespaceURI, "SearchTerms");
				alteredQuery = attributeDict.getValue(namespaceURI, "AlteredQuery");
				alterationOverrideQuery = attributeDict.getValue(namespaceURI, "AlterationOverrideQuery");
			}
//#ifdef BING_DEBUG
			else if(elementName.equals("Error"))
			{
				BingError error = new BingError(loadDictionary(attributeDict));
				if(current != null && this._errorRet)
				{
					current.addResult(error);
				}
				System.out.println(bing.Bing.format(_resources.getString(BingResource.XML_ERROR_CODE), new Object[]{ new Long(error.getCode()) }));
				System.out.println(bing.Bing.format(_resources.getString(BingResource.XML_ERROR_MESSAGE), new Object[]{ error.getMessage() }));
				System.out.println(bing.Bing.format(_resources.getString(BingResource.XML_ERROR_PARAM), new Object[]{ error.getParameter() }));
			}
//#endif
			else
			{
				if(classes.containsKey(elementName))
				{
					if(getClassIndex(elementName, "bing.common.") >= COMMON_INDEX)
					{
						//Manual loaded types
						BingResult result = reflectiveLoad(elementName, "bing.common.", attributeDict);
						
						//Reflection loaded types
						/*
						if(classes.containsKey(elementName))
						{
							Class clazz = classes.get(elementName);
							ReflectClass rClass = ReflectClass.getClass(clazz);
							BingResult result = rClass.newInstance(rClass.getConstructor(clazz.getName() + "(Hashtable)"), new Object[]{ loadDictionary(attributeDict) });
						*/
						if(lastResult.size() > 0)
						{
							/* Say I have something like this
							 * <hello>
							 * 	<array>
							 * 		//This will work (the array has been added, no other mind numbing operations to do. Keep processing)
							 * 		<array>
							 * 			<item></item>
							 * 			<item></item>
							 * 		</array>
							 * 		//This will NOT work (This array will get added to the above array instead of the parent array. the only way to do this is to completely
							 * 		//rewrite the parser so it processes the FINAL result, that isn't hard but it destroys the progressive built process.)
							 * 		<array>
							 * 			<item></item>
							 * 			<item></item>
							 * 		</array>
							 * 	</array>
							 * </hello>
							 * 
							 * The idea is when endElement is called if the element name is the same as the last element added then it will pop off the last element
							 */
							((BingResult)this.lastResult.peek()).add(new BingResult[]{ result });
						}
						else
						{
							//Response item
							Hashtable tab = new Hashtable();
							tab.put(elementName.substring(elementName.indexOf(':') + 1), result);
							this.current.handleElements(tab);
						}
						if(result instanceof Array)
						{
							this.lastResult.push(result);
							this.lastResultElement.push(elementName);
						}
						//Reflection loaded types
						/*
						}
						 */
						return;
					}
					else
					{
						current = (BingResponse)classes.get(elementName).newInstance();
						current.setTotal(Long.parseLong(attributeDict.getValue(namespaceURI, "Total")));
						current.setOffset(Long.parseLong(attributeDict.getValue(namespaceURI, "Offset")));
						current.setQuery(query);
						current.setAlteredQuery(alteredQuery);
						current.setAlterationOverrideQuery(alterationOverrideQuery);
						current.handleElements(loadDictionary(attributeDict));
					}
				}
				else
				{
					return;
				}
				
				if (response == null && current != null)
				{
					response = current;
				}
				else if (BingBundleResponse.class.isInstance(response))
				{
					((BingBundleResponse)response).addResponse(current);
					response.setQuery(query);
				}
				else if (response != current)
				{ 
					BingResponse tmp = response;
					response = new BingBundleResponse();
					((BingBundleResponse)response).addResponse(tmp);
					((BingBundleResponse)response).addResponse(current);
				}
			}
		}
		catch (Exception e)
		{
			throw new SAXException(e);
		}
	}
	
	public void endElement(String namespaceURI, String elementName, String qualifiedName) throws SAXException
	{
		try
		{
			if(this.lastResultElement.size() > 0)
			{
				if(this.lastResultElement.peek().equals(elementName))
				{
					this.lastResultElement.pop();
					this.lastResult.pop();
				}
			}
		}
		catch (Exception e)
		{
			throw new SAXException(e);
		}
	}
	
	private static Hashtable loadDictionary(Attributes attributeDict)
	{
		if(attributeDict == null)
		{
			return new Hashtable();
		}
		int len;
		Hashtable aDict = new Hashtable(len = attributeDict.getLength());
		for(int i = 0; i < len; i++)
		{
			aDict.put(attributeDict.getLocalName(i), attributeDict.getValue(i));
		}
		return aDict;
	}
	
	private int getClassIndex(String name, String namespace)
	{
		int len = classes.size();
		Enumeration en = classes.elements();
		String className = name.substring(name.indexOf(':') + 1);
		try
		{
			Class clazz = Class.forName(namespace + className);
			Class lClazz = null;
			for(int i = -1; i < len; i++, lClazz = (Class)en.nextElement())
			{
				if(lClazz == null)
				{
					continue;
				}
				if(lClazz.equals(clazz))
				{
					return i;
				}
			}
		}
		catch(Exception e)
		{
			//See if a type exists under a different name, do it in the exception handler because the only time that this would pop up is if a type didn't exist
			en = classes.keys();
			String type = null;
			for(int i = -1; i < len; i++, type = (String)en.nextElement())
			{
				if(type == null)
				{
					continue;
				}
				if(type.equals(name))
				{
					return i;
				}
			}
		}
		return -1;
	}
}
