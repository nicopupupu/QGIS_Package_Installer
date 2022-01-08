
/*
 * This file is a part of TTMath Bignum Library
 * and is distributed under the (new) BSD licence.
 * Author: Tomasz Sowa <t.sowa@ttmath.org>
 */

/* 
 * Copyright (c) 2006-2011, Tomasz Sowa
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



#ifndef headerfilettmathint
#define headerfilettmathint

/*!
	\file ttmathint.h
    \brief template class Int<uint>
*/

#include "ttmathuint.h"

namespace ttmath
{


/*!
	\brief Int implements a big integer value with a sign

	value_size - how many bytes specify our value
		on 32bit platforms: value_size=1 -> 4 bytes -> 32 bits
		on 64bit platforms: value_size=1 -> 8 bytes -> 64 bits
	value_size = 1,2,3,4,5,6....
*/
template<uint value_size>
class Int : public UInt<value_size>
{
public:

	/*!
		this method sets the max value which this class can hold
		(all bits will be one besides the last one)
	*/
	void SetMax()
	{
		UInt<value_size>::SetMax();
		UInt<value_size>::table[value_size-1] = ~ TTMATH_UINT_HIGHEST_BIT;
	}


	/*!
		this method sets the min value which this class can hold
		(all bits will be zero besides the last one which is one)
	*/
	void SetMin()
	{
		UInt<value_size>::SetZero();
		UInt<value_size>::table[value_size-1] = TTMATH_UINT_HIGHEST_BIT;
	}


	/*!
		this method sets -1 as the value
		(-1 is equal the max value in an unsigned type)
	*/
	void SetSignOne()
	{
		UInt<value_size>::SetMax();
	}


	/*!
		we change the sign of the value

		if it isn't possible to change the sign this method returns 1
		else return 0 and changing the sign
	*/
	uint ChangeSign()
	{
		/*
			if the value is equal that one which has been returned from SetMin
			(only the highest bit is set) that means we can't change sign
			because the value is too big (bigger about one)

			e.g. when value_size = 1 and value is -2147483648 we can't change it to the
			2147483648 because the max value which can be held is 2147483647

			we don't change the value and we're using this fact somewhere in some methods
			(if we look on our value without the sign we get the correct value 
			eg. -2147483648 in Int<1> will be 2147483648 on the UInt<1> type)
		*/
		if( UInt<value_size>::IsOnlyTheHighestBitSet() )
			return 1;

		UInt<value_size> temp(*this);
		UInt<value_size>::SetZero();
		UInt<value_size>::Sub(temp);

	return 0;
	}



	/*!	
		this method sets the sign

		e.g. 1  -> -1
		     -2 -> -2
		
		from a positive value we make a negative value,
		if the value is negative we do nothing
	*/
	void SetSign()
	{
		if( IsSign() )
			return;

		ChangeSign();
	}



	/*!
		this method returns true if there's the sign

		(the highest bit will be converted to the bool)
	*/
	bool IsSign() const
	{
		return UInt<value_size>::IsTheHighestBitSet();
	}



	/*!
		it sets an absolute value

		it can return carry (1) (look on ChangeSign() for details)
	*/
	uint Abs()
	{
		if( !IsSign() )
			return 0;

	return ChangeSign();
	}




	/*!
	*
	*	basic mathematic functions
	*
	*/

private:

	uint CorrectCarryAfterAdding(bool p1_is_sign, bool p2_is_sign)
	{
		if( !p1_is_sign && !p2_is_sign )
		{
			if( UInt<value_size>::IsTheHighestBitSet() )
				return 1;
		}

		if( p1_is_sign && p2_is_sign )
		{	