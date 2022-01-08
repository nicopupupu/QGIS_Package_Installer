
/*
 * This file is a part of TTMath Mathematical Library
 * and is distributed under the (new) BSD licence.
 * Author: Tomasz Sowa <t.sowa@ttmath.org>
 */

/* 
 * Copyright (c) 2006-2010, Tomasz Sowa
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *    
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *    
 *  * Neither the name Tomasz Sowa nor the names of contributors to this
 *    project may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef headerfilettmathobject
#define headerfilettmathobject

/*!
	\file ttmathobjects.h
    \brief Mathematic functions.
*/

#include <string>
#include <vector>
#include <list>
#include <map>

#include "ttmathtypes.h"
#include "ttmathmisc.h"


namespace ttmath
{

/*!
	objects of this class are used with the mathematical parser
	they hold variables or functions defined by a user

	each object has its own table in which we're keeping variables or functions
*/
class Objects
{
public:


	/*!
		one item (variable or function)
		'items' will be on the table
	*/
	struct Item
	{
		// name of a variable of a function
		// internally we store variables and funcions as std::string (not std::wstring even when wide characters are used)
		std::string value;

		// number of parameters required by the function
		// (if there's a variable this 'param' is ignored)
		int param;

		Item() {}
		Item(const std::string & v, int p) : value(v), param(p) {}
	};

	// 'Table' is the type of our table
	typedef std::map<std::string, Item> Table;
	typedef	Table::iterator Iterator;
	typedef	Table::const_iterator CIterator;



	/*!
		this method returns true if a character 'c' is a character
		which can be in a name
		
		if 'can_be_digit' is true that means when the 'c' is a digit this 
		method returns true otherwise it returns false
	*/
	static bool CorrectCharacter(int c, bool can_be_digit)
	{
		if( (c>='a' && c<='z') || (c>='A' && c<='Z') )
			return true;

		if( can_be_digit && ((c>='0' && c<='9') || c=='_') )
			return true;

	return false;
	}


	/*!
		this method returns true if the name can be as a name of an object
	*/
	template<class string_type>
	static bool IsNameCorrect(const string_type & name)
	{
		if( name.empty() )
			return false;

		if( !CorrectCharacter(name[0], false) )
			return false;

		typename string_type::const_iterator i = name.begin();

		for(++i ; i!=name.end() ; ++i)
			if( !CorrectCharacter(*i, true) )
				return false;
		
	return true;
	}


	/*!
		this method returns true if such an object is defined (name exists)
	*/
	bool IsDefined(const std::string & name)
	{
		Iterator i = table.find(name);

		if( i != table.end() )
			// we have this object in our table
			return true;

	return false;
	}



#ifndef TTMATH_DONT_USE_WCHAR

	/*!
		this method returns true if such an object is defined (name exists)
	*/
	bool IsDefined(const std::wstring & name)
	{
		// we should check whether the name (in wide characters) are correct
		// before calling AssignString() function
		if( !IsNameCorrect(name) )
			return false;

		Misc::AssignString(str_tmp1, name);

	return IsDefined(str_tmp1);
	}

#endif


	/*!
		this method adds one object (variable of function) into the table
	*/
	ErrorCode Add(const std::string & name, const std::string & value, int param = 0)
	{
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Iterator i = table.find(name);

		if( i != table.end() )
			// we have this object in our table
			return err_object_exists;

		table.insert( std::make_pair(name, Item(value, param)) );

	return err_ok;
	}


#ifndef TTMATH_DONT_USE_WCHAR

	/*!
		this method adds one object (variable of function) into the table
	*/
	ErrorCode Add(const std::wstring & name, const std::wstring & value, int param = 0)
	{
		// we should check whether the name (in wide characters) are correct
		// before calling AssignString() function
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Misc::AssignString(str_tmp1, name);
		Misc::AssignString(str_tmp2, value);
		
	return Add(str_tmp1, str_tmp2, param);
	}

#endif


	/*!
		this method returns 'true' if the table is empty
	*/
	bool Empty() const
	{
		return table.empty();
	}


	/*!
		this method clears the table
	*/
	void Clear()
	{
		return table.clear();
	}


	/*!
		this method returns 'const_iterator' on the first item on the table
	*/
	CIterator Begin() const
	{
		return table.begin();
	}


	/*!
		this method returns 'const_iterator' pointing at the space after last item
		(returns table.end())
	*/
	CIterator End() const
	{
		return table.end();
	}


	/*!
		this method changes the value and the number of parameters for a specific object
	*/
	ErrorCode EditValue(const std::string & name, const std::string & value, int param = 0)
	{
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Iterator i = table.find(name);

		if( i == table.end() )
			return err_unknown_object;
	
		i->second.value = value;
		i->second.param = param;

	return err_ok;
	}


#ifndef TTMATH_DONT_USE_WCHAR


	/*!
		this method changes the value and the number of parameters for a specific object
	*/
	ErrorCode EditValue(const std::wstring & name, const std::wstring & value, int param = 0)
	{
		// we should check whether the name (in wide characters) are correct
		// before calling AssignString() function
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Misc::AssignString(str_tmp1, name);
		Misc::AssignString(str_tmp2, value);
		
	return EditValue(str_tmp1, str_tmp2, param);
	}

#endif


	/*!
		this method changes the name of a specific object
	*/
	ErrorCode EditName(const std::string & old_name, const std::string & new_name)
	{
		if( !IsNameCorrect(old_name) || !IsNameCorrect(new_name) )
			return err_incorrect_name;

		Iterator old_i = table.find(old_name);
		if( old_i == table.end() )
			return err_unknown_object;
		
		if( old_name == new_name )
			// the new name is the same as the old one
			// we treat it as a normal situation
			return err_ok;

		ErrorCode err = Add(new_name, old_i->second.value, old_i->second.param);
		
		if( err == err_ok ) 
		{
			old_i = table.find(old_name);
			TTMATH_ASSERT( old_i != table.end() )

			table.erase(old_i);
		}

	return err;
	}



#ifndef TTMATH_DONT_USE_WCHAR


	/*!
		this method changes the name of a specific object
	*/
	ErrorCode EditName(const std::wstring & old_name, const std::wstring & new_name)
	{
		// we should check whether the name (in wide characters) are correct
		// before calling AssignString() function
		if( !IsNameCorrect(old_name) || !IsNameCorrect(new_name) )
			return err_incorrect_name;

		Misc::AssignString(str_tmp1, old_name);
		Misc::AssignString(str_tmp2, new_name);

	return EditName(str_tmp1, str_tmp2);
	}

#endif


	/*!
		this method deletes an object
	*/
	ErrorCode Delete(const std::string & name)
	{
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Iterator i = table.find(name);

		if( i == table.end() )
			return err_unknown_object;

		table.erase( i );

	return err_ok;
	}


#ifndef TTMATH_DONT_USE_WCHAR


	/*!
		this method deletes an object
	*/
	ErrorCode Delete(const std::wstring & name)
	{
		// we should check whether the name (in wide characters) are correct
		// before calling AssignString() function
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Misc::AssignString(str_tmp1, name);

	return Delete(str_tmp1);
	}	
		
#endif


	/*!
		this method gets the value of a specific object
	*/
	ErrorCode GetValue(const std::string & name, std::string & value) const
	{
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		CIterator i = table.find(name);

		if( i == table.end() )
		{
			value.clear();
			return err_unknown_object;
		}

		value = i->second.value;

	return err_ok;
	}


#ifndef TTMATH_DONT_USE_WCHAR

	/*!
		this method gets the value of a specific object
	*/
	ErrorCode GetValue(const std::wstring & name, std::wstring & value)
	{
		// we should check whether the name (in wide characters) are correct
		// before calling AssignString() function
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Misc::AssignString(str_tmp1, name);
		ErrorCode err = GetValue(str_tmp1, str_tmp2);
		Misc::AssignString(value, str_tmp2);

	return err;
	}

#endif


	/*!
		this method gets the value of a specific object
		(this version is used for not copying the whole string)
	*/
	ErrorCode GetValue(const std::string & name, const char ** value) const
	{
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		CIterator i = table.find(name);

		if( i == table.end() )
		{
			*value = 0;
			return err_unknown_object;
		}

		*value = i->second.value.c_str();

	return err_ok;
	}


#ifndef TTMATH_DONT_USE_WCHAR

	/*!
		this method gets the value of a specific object
		(this version is used for not copying the whole string)
	*/
	ErrorCode GetValue(const std::wstring & name, const char ** value)
	{
		// we should check whether the name (in wide characters) are correct
		// before calling AssignString() function
		if( !IsNameCorrect(name) )
			return err_incorrect_name;

		Misc::AssignString(str_tmp1, name);

	return GetValue(str_tmp1, value);
	}

#endif


	/*!
		this method gets the value and the number of parameters
		of a specific object
	*/
	ErrorCode GetValueAndParam(const std::string & name, std::string & value, int * param) const