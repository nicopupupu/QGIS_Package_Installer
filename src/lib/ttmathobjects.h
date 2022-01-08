
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