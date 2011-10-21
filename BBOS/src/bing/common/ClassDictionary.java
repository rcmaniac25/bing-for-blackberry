/**
 * (c) 2009 Microsoft corp.
 * This software is distributed under Microsoft Public License (MSPL)
 * see http://opensource.org/licenses/ms-pl.html
 * 
 * @author Microsoft, Vincent Simonetti
 */
package bing.common;

import java.util.*;

/**
 * Basic dictionary class
 */
class ClassDictionary
{
	Vector keys;
	Vector values;
	
	public ClassDictionary()
	{
		keys = new Vector();
		values = new Vector();
	}
	
	public boolean containsKey(String key)
	{
		return this.keys.contains(key);
	}
	
	public Enumeration elements()
	{
		return new ClassEnumeration(values);
	}
	
	public Enumeration keys()
	{
		return new ClassEnumeration(keys);
	}
	
	public int size()
	{
		return this.keys.size();
	}
	
	public Class get(String key)
	{
		int index = this.keys.indexOf(key);
		return index == -1 ? null : (Class)this.values.elementAt(index);
	}
	
	public Class put(String key, Class value)
	{
		int index = this.keys.indexOf(key);
		if(index == -1)
		{
			this.keys.addElement(key);
			this.values.addElement(value);
			return null;
		}
		Class old = (Class)this.values.elementAt(index);
		this.values.setElementAt(value, index);
		return old;
	}
	
	private final class ClassEnumeration implements Enumeration
	{
		private Vector values;
		private int index;
		
		public ClassEnumeration(Vector values)
		{
			index = 0;
			this.values = values;
		}

		public boolean hasMoreElements()
		{
			return index < this.values.size();
		}

		public Object nextElement()
		{
			if(index < this.values.size())
			{
				return this.values.elementAt(index++);
			}
			if(index > this.values.size())
			{
				throw new NoSuchElementException();
			}
			index++;
			return null;
		}
	}
}
