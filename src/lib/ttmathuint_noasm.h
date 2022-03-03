
/*
 * This file is a part of TTMath Bignum Library
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

#ifndef headerfilettmathuint_noasm
#define headerfilettmathuint_noasm


#ifdef TTMATH_NOASM

/*!
	\file ttmathuint_noasm.h
    \brief template class UInt<uint> with methods without any assembler code

	this file is included at the end of ttmathuint.h
*/


namespace ttmath
{

	/*!
		returning the string represents the currect type of the library
		we have following types:
		  asm_vc_32   - with asm code designed for Microsoft Visual C++ (32 bits)
		  asm_gcc_32  - with asm code designed for GCC (32 bits)
		  asm_vc_64   - with asm for VC (64 bit)
		  asm_gcc_64  - with asm for GCC (64 bit)
		  no_asm_32   - pure C++ version (32 bit) - without any asm code
		  no_asm_64   - pure C++ version (64 bit) - without any asm code
	*/
	template<uint value_size>
	const char * UInt<value_size>::LibTypeStr()
	{
		#ifdef TTMATH_PLATFORM32
			static const char info[] = "no_asm_32";
		#endif		

		#ifdef TTMATH_PLATFORM64
			static const char info[] = "no_asm_64";
		#endif

	return info;
	}

	
	/*!
		returning the currect type of the library
	*/
	template<uint value_size>
	LibTypeCode UInt<value_size>::LibType()
	{
		#ifdef TTMATH_PLATFORM32
			LibTypeCode info = no_asm_32;
		#endif		

		#ifdef TTMATH_PLATFORM64
			LibTypeCode info = no_asm_64;
		#endif

	return info;
	}


	/*!
		this method adds two words together
		returns carry

		this method is created only when TTMATH_NOASM macro is defined
	*/
	template<uint value_size>
	uint UInt<value_size>::AddTwoWords(uint a, uint b, uint carry, uint * result)
	{
	uint temp;

		if( carry == 0 )
		{
			temp = a + b;

			if( temp < a )
				carry = 1;
		}
		else
		{
			carry = 1;
			temp  = a + b + carry;

			if( temp > a ) // !(temp<=a)
				carry = 0;
		}

		*result = temp;

	return carry;
	}



	/*!
		this method adding ss2 to the this and adding carry if it's defined
		(this = this + ss2 + c)

		c must be zero or one (might be a bigger value than 1)
		function returns carry (1) (if it was)
	*/
	
	template<uint value_size>
	uint UInt<value_size>::Add(const UInt<value_size> & ss2, uint c)
	{
	uint i;

		for(i=0 ; i<value_size ; ++i)
			c = AddTwoWords(table[i], ss2.table[i], c, &table[i]);

		TTMATH_LOGC("UInt::Add", c)
	
	return c;
	}


	/*!
		this method adds one word (at a specific position)
		and returns a carry (if it was)

		if we've got (value_size=3):
			table[0] = 10;
			table[1] = 30;
			table[2] = 5;	
		and we call:
			AddInt(2,1)
		then it'll be:
			table[0] = 10;
			table[1] = 30 + 2;
			table[2] = 5;

		of course if there was a carry from table[2] it would be returned
	*/
	template<uint value_size>
	uint UInt<value_size>::AddInt(uint value, uint index)
	{
	uint i, c;

		TTMATH_ASSERT( index < value_size )


		c = AddTwoWords(table[index], value, 0, &table[index]);

		for(i=index+1 ; i<value_size && c ; ++i)
			c = AddTwoWords(table[i], 0, c, &table[i]);

		TTMATH_LOGC("UInt::AddInt", c)
	
	return c;
	}





	/*!
		this method adds only two unsigned words to the existing value
		and these words begin on the 'index' position
		(it's used in the multiplication algorithm 2)

		index should be equal or smaller than value_size-2 (index <= value_size-2)
		x1 - lower word, x2 - higher word

		for example if we've got value_size equal 4 and:
			table[0] = 3
			table[1] = 4
			table[2] = 5
			table[3] = 6
		then let
			x1 = 10
			x2 = 20
		and
			index = 1

		the result of this method will be:
			table[0] = 3
			table[1] = 4 + x1 = 14
			table[2] = 5 + x2 = 25
			table[3] = 6
		
		and no carry at the end of table[3]

		(of course if there was a carry in table[2](5+20) then 
		this carry would be passed to the table[3] etc.)
	*/
	template<uint value_size>
	uint UInt<value_size>::AddTwoInts(uint x2, uint x1, uint index)
	{
	uint i, c;

		TTMATH_ASSERT( index < value_size - 1 )


		c = AddTwoWords(table[index],   x1, 0, &table[index]);
		c = AddTwoWords(table[index+1], x2, c, &table[index+1]);

		for(i=index+2 ; i<value_size && c ; ++i)
			c = AddTwoWords(table[i], 0, c, &table[i]);

		TTMATH_LOGC("UInt::AddTwoInts", c)
	
	return c;
	}



	/*!
		this static method addes one vector to the other
		'ss1' is larger in size or equal to 'ss2'

		ss1 points to the first (larger) vector
		ss2 points to the second vector
		ss1_size - size of the ss1 (and size of the result too)
		ss2_size - size of the ss2
		result - is the result vector (which has size the same as ss1: ss1_size)

		Example:  ss1_size is 5, ss2_size is 3
		ss1:      ss2:   result (output):
		  5        1         5+1
		  4        3         4+3
		  2        7         2+7
		  6                  6
		  9                  9
	  of course the carry is propagated and will be returned from the last item
	  (this method is used by the Karatsuba multiplication algorithm)
	*/
	template<uint value_size>
	uint UInt<value_size>::AddVector(const uint * ss1, const uint * ss2, uint ss1_size, uint ss2_size, uint * result)
	{
	uint i, c = 0;

		TTMATH_ASSERT( ss1_size >= ss2_size )
		
		for(i=0 ; i<ss2_size ; ++i)
			c = AddTwoWords(ss1[i], ss2[i], c, &result[i]);

		for( ; i<ss1_size ; ++i)
			c = AddTwoWords(ss1[i], 0, c, &result[i]);

		TTMATH_VECTOR_LOGC("UInt::AddVector", c, result, ss1_size)

	return c;
	}




	/*!
		this method subtractes one word from the other
		returns carry

		this method is created only when TTMATH_NOASM macro is defined
	*/
	template<uint value_size>
	uint UInt<value_size>::SubTwoWords(uint a, uint b, uint carry, uint * result)
	{
		if( carry == 0 )
		{
			*result = a - b;

			if( a < b )
				carry = 1;
		}
		else
		{
			carry   = 1;
			*result = a - b - carry;

			if( a > b ) // !(a <= b )
				carry = 0;
		}

	return carry;
	}




	/*!
		this method's subtracting ss2 from the 'this' and subtracting
		carry if it has been defined
		(this = this - ss2 - c)

		c must be zero or one (might be a bigger value than 1)
		function returns carry (1) (if it was)
	*/
	template<uint value_size>
	uint UInt<value_size>::Sub(const UInt<value_size> & ss2, uint c)
	{
	uint i;

		for(i=0 ; i<value_size ; ++i)
			c = SubTwoWords(table[i], ss2.table[i], c, &table[i]);

		TTMATH_LOGC("UInt::Sub", c)

	return c;
	}




	/*!
		this method subtracts one word (at a specific position)
		and returns a carry (if it was)

		if we've got (value_size=3):
			table[0] = 10;
			table[1] = 30;
			table[2] = 5;	
		and we call:
			SubInt(2,1)
		then it'll be:
			table[0] = 10;
			table[1] = 30 - 2;
			table[2] = 5;

		of course if there was a carry from table[2] it would be returned
	*/
	template<uint value_size>
	uint UInt<value_size>::SubInt(uint value, uint index)
	{
	uint i, c;

		TTMATH_ASSERT( index < value_size )


		c = SubTwoWords(table[index], value, 0, &table[index]);

		for(i=index+1 ; i<value_size && c ; ++i)
			c = SubTwoWords(table[i], 0, c, &table[i]);

		TTMATH_LOGC("UInt::SubInt", c)
	
	return c;
	}


	/*!
		this static method subtractes one vector from the other
		'ss1' is larger in size or equal to 'ss2'

		ss1 points to the first (larger) vector
		ss2 points to the second vector
		ss1_size - size of the ss1 (and size of the result too)
		ss2_size - size of the ss2
		result - is the result vector (which has size the same as ss1: ss1_size)

		Example:  ss1_size is 5, ss2_size is 3
		ss1:      ss2:   result (output):
		  5        1         5-1
		  4        3         4-3
		  2        7         2-7
		  6                  6-1  (the borrow from previous item)
		  9                  9
		                 return (carry): 0
	  of course the carry (borrow) is propagated and will be returned from the last item
	  (this method is used by the Karatsuba multiplication algorithm)
	*/
	template<uint value_size>
	uint UInt<value_size>::SubVector(const uint * ss1, const uint * ss2, uint ss1_size, uint ss2_size, uint * result)
	{
	uint i, c = 0;

		TTMATH_ASSERT( ss1_size >= ss2_size )
		
		for(i=0 ; i<ss2_size ; ++i)
			c = SubTwoWords(ss1[i], ss2[i], c, &result[i]);

		for( ; i<ss1_size ; ++i)
			c = SubTwoWords(ss1[i], 0, c, &result[i]);

		TTMATH_VECTOR_LOGC("UInt::SubVector", c, result, ss1_size)

	return c;
	}



	/*!
		this method moves all bits into the left hand side
		return value <- this <- c

		the lowest *bit* will be held the 'c' and
		the state of one additional bit (on the left hand side)
		will be returned

		for example:
		let this is 001010000
		after Rcl2_one(1) there'll be 010100001 and Rcl2_one returns 0
	*/
	template<uint value_size>
	uint UInt<value_size>::Rcl2_one(uint c)
	{
	uint i, new_c;

		if( c != 0 )
			c = 1;

		for(i=0 ; i<value_size ; ++i)
		{
			new_c    = (table[i] & TTMATH_UINT_HIGHEST_BIT) ? 1 : 0;
			table[i] = (table[i] << 1) | c;
			c        = new_c;
		}

		TTMATH_LOGC("UInt::Rcl2_one", c)

	return c;
	}







	/*!
		this method moves all bits into the right hand side
		c -> this -> return value
