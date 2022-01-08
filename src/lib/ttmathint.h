
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
			if( ! UInt<value_size>::IsTheHighestBitSet() )
				return 1;
		}

	return 0;
	}


public:

	/*!
		this method adds two value with a sign and returns a carry

		we're using methods from the base class because values are stored with U2
		we must only make the carry correction

		this = p1(=this) + p2

		when p1>=0 i p2>=0 carry is set when the highest bit of value is set
		when p1<0  i p2<0  carry is set when the highest bit of value is clear
		when p1>=0 i p2<0  carry will never be set
		when p1<0  i p2>=0 carry will never be set
	*/
	uint Add(const Int<value_size> & ss2)
	{
		bool p1_is_sign = IsSign();
		bool p2_is_sign = ss2.IsSign();

		UInt<value_size>::Add(ss2);		

	return CorrectCarryAfterAdding(p1_is_sign, p2_is_sign);
	}


	/*!
		this method adds one *unsigned* word (at a specific position)
		and returns a carry (if it was)

		look at a description in UInt<>::AddInt(...)
	*/
	uint AddInt(uint value, uint index = 0)
	{
		bool p1_is_sign = IsSign();

		UInt<value_size>::AddInt(value, index);		

	return CorrectCarryAfterAdding(p1_is_sign, false);
	}


	/*!
		this method adds two *unsigned* words to the existing value
		and these words begin on the 'index' position

		index should be equal or smaller than value_size-2 (index <= value_size-2)
		x1 - lower word, x2 - higher word

		look at a description in UInt<>::AddTwoInts(...)
	*/
	uint AddTwoInts(uint x2, uint x1, uint index)
	{
		bool p1_is_sign = IsSign();

		UInt<value_size>::AddTwoInts(x2, x1, index);		

	return CorrectCarryAfterAdding(p1_is_sign, false);
	}

private:

	uint CorrectCarryAfterSubtracting(bool p1_is_sign, bool p2_is_sign)
	{
		if( !p1_is_sign && p2_is_sign )
		{
			if( UInt<value_size>::IsTheHighestBitSet() )
				return 1;
		}

		if( p1_is_sign && !p2_is_sign )
		{	
			if( ! UInt<value_size>::IsTheHighestBitSet() )
				return 1;
		}

	return 0;
	}

public:

	/*!	
		this method subtracts two values with a sign

		we don't use the previous Add because the method ChangeSign can
		sometimes return carry 

		this = p1(=this) - p2

		when p1>=0 i p2>=0 carry will never be set
		when p1<0  i p2<0  carry will never be set
		when p1>=0 i p2<0  carry is set when the highest bit of value is set
		when p1<0  i p2>=0 carry is set when the highest bit of value is clear
	*/
	uint Sub(const Int<value_size> & ss2)
	{
		bool p1_is_sign = IsSign();
		bool p2_is_sign = ss2.IsSign();

		UInt<value_size>::Sub(ss2);		

	return CorrectCarryAfterSubtracting(p1_is_sign, p2_is_sign);
	}


	/*!
		this method subtracts one *unsigned* word (at a specific position)
		and returns a carry (if it was)
	*/
	uint SubInt(uint value, uint index = 0)
	{
		bool p1_is_sign = IsSign();

		UInt<value_size>::SubInt(value, index);		

	return CorrectCarryAfterSubtracting(p1_is_sign, false);
	}


	/*!
		this method adds one to the value and returns carry
	*/
	uint AddOne()
	{
		bool p1_is_sign = IsSign();

		UInt<value_size>::AddOne();		

	return CorrectCarryAfterAdding(p1_is_sign, false);
	}


	/*!
		this method subtracts one from the value and returns carry
	*/
	uint SubOne()
	{
		bool p1_is_sign = IsSign();

		UInt<value_size>::SubOne();		

	return CorrectCarryAfterSubtracting(p1_is_sign, false);
	}


private:


	uint CheckMinCarry(bool ss1_is_sign, bool ss2_is_sign)
	{
		/*
			we have to examine the sign of the result now
			but if the result is with the sign then:
				1. if the signs were the same that means the result is too big
				(the result must be without a sign)
				2. if the signs were different that means if the result
				is different from that one which has been returned from SetMin()
				that is carry (result too big) but if the result is equal SetMin()
				there'll be ok (and the next SetSign will has no effect because
				the value is actually negative -- look at description of that case
				in ChangeSign())
		*/
		if( IsSign() )
		{
			if( ss1_is_sign != ss2_is_sign )
			{
				/*
					there can be one case where signs are different and
					the result will be equal the value from SetMin() (only the highest bit is set)
					(this situation is ok)
				*/
				if( !UInt<value_size>::IsOnlyTheHighestBitSet() )
					return 1;
			}
			else
			{
				// signs were the same
				return 1;
			}
		}

	return 0;
	}


public:


	/*!
		multiplication: this = this * ss2

		it can return a carry
	*/
	uint MulInt(sint ss2)
	{
	bool ss1_is_sign, ss2_is_sign;
	uint c;

		ss1_is_sign = IsSign();

		/*
			we don't have to check the carry from Abs (values will be correct
			because next we're using the method MulInt from the base class UInt
			which is without a sign)
		*/
		Abs();

		if( ss2 < 0 )
		{
			ss2 = -ss2;
			ss2_is_sign = true;
		}
		else
		{
			ss2_is_sign = false;
		}

		c  = UInt<value_size>::MulInt((uint)ss2);
		c += CheckMinCarry(ss1_is_sign, ss2_is_sign);

		if( ss1_is_sign != ss2_is_sign )
			SetSign();

	return c;
	}



	/*!
		multiplication this = this * ss2

		it returns carry if the result is too big
		(we're using the method from the base class but we have to make
		one correction in account of signs)
	*/
	uint Mul(Int<value_size> ss2)
	{
	bool ss1_is_sign, ss2_is_sign;
	uint c;

		ss1_is_sign = IsSign();
		ss2_is_sign = ss2.IsSign();

		/*
			we don't have to check the carry from Abs (values will be correct
			because next we're using the method Mul from the base class UInt
			which is without a sign)
		*/
		Abs();
		ss2.Abs();

		c  = UInt<value_size>::Mul(ss2);
		c += CheckMinCarry(ss1_is_sign, ss2_is_sign);

		if( ss1_is_sign != ss2_is_sign )
			SetSign();

	return c;
	}


	/*!
		division this = this / ss2
		returned values:
			0 - ok
			1 - division by zero

		for example: (result means 'this')
			 20 /  3 --> result:  6   remainder:  2
			-20 /  3 --> result: -6   remainder: -2
			 20 / -3 --> result: -6   remainder:  2
			-20 / -3 --> result:  6   remainder: -2

		in other words: this(old) = ss2 * this(new)(result) + remainder
	*/
	uint Div(Int<value_size> ss2, Int<value_size> * remainder = 0)
	{
	bool ss1_is_sign, ss2_is_sign;

		ss1_is_sign = IsSign();
		ss2_is_sign = ss2.IsSign();

		/*
			we don't have to test the carry from Abs as well as in Mul
		*/
		Abs();
		ss2.Abs();

		uint c = UInt<value_size>::Div(ss2, remainder);

		if( ss1_is_sign != ss2_is_sign )
			SetSign();

		if( ss1_is_sign && remainder )
			remainder->SetSign();

	return c;
	}
	
	uint Div(const Int<value_size> & ss2, Int<value_size> & remainder)
	{
		return Div(ss2, &remainder);
	}


	/*!
		division this = this / ss2  (ss2 is int)
		returned values:
			0 - ok
			1 - division by zero

		for example: (result means 'this')
			 20 /  3 --> result:  6   remainder:  2
			-20 /  3 --> result: -6   remainder: -2
			 20 / -3 --> result: -6   remainder:  2
			-20 / -3 --> result:  6   remainder: -2

		in other words: this(old) = ss2 * this(new)(result) + remainder
	*/
	uint DivInt(sint ss2, sint * remainder = 0)
	{
	bool ss1_is_sign, ss2_is_sign;

		ss1_is_sign = IsSign();

		/*
			we don't have to test the carry from Abs as well as in Mul
		*/
		Abs();

		if( ss2 < 0 )
		{
			ss2 = -ss2;
			ss2_is_sign = true;
		}
		else
		{
			ss2_is_sign = false;
		}

		uint rem;
		uint c = UInt<value_size>::DivInt((uint)ss2, &rem);

		if( ss1_is_sign != ss2_is_sign )
			SetSign();

		if( remainder )
		{
			if( ss1_is_sign )
				*remainder = -sint(rem);
			else
				*remainder = sint(rem);
		}

	return c;
	}


	uint DivInt(sint ss2, sint & remainder)
	{
		return DivInt(ss2, &remainder);
	}


private:


	/*!
		power this = this ^ pow
		this can be negative
		pow is >= 0
	*/
	uint Pow2(const Int<value_size> & pow)
	{
		bool was_sign = IsSign();
		uint c = 0;

		if( was_sign )
			c += Abs();

		uint c_temp = UInt<value_size>::Pow(pow);
		if( c_temp > 0 )
			return c_temp; // c_temp can be: 0, 1 or 2
		
		if( was_sign && (pow.table[0] & 1) == 1 )
			// negative value to the power of odd number is negative
			c += ChangeSign();

	return (c==0)? 0 : 1;
	}


public:


	/*!
		power this = this ^ pow

		return values:
		0 - ok
		1 - carry
		2 - incorrect arguments 0^0 or 0^(-something)
	*/
	uint Pow(Int<value_size> pow)
	{
		if( !pow.IsSign() )
			return Pow2(pow);

		if( UInt<value_size>::IsZero() )
			// if 'pow' is negative then
			// 'this' must be different from zero
			return 2;

		if( pow.ChangeSign() )
			return 1;

		Int<value_size> t(*this);
		uint c_temp = t.Pow2(pow);
		if( c_temp > 0 )
			return c_temp;

		UInt<value_size>::SetOne();
		if( Div(t) )
			return 1;

	return 0;
	}



	/*!
	*
	*	convertion methods
	*
	*/
private:


	/*!
		an auxiliary method for converting both from UInt and Int
	*/
	template<uint argument_size>
	uint FromUIntOrInt(const UInt<argument_size> & p, bool UInt_type)
	{
		uint min_size = (value_size < argument_size)? value_size : argument_size;
		uint i;

		for(i=0 ; i<min_size ; ++i)
			UInt<value_size>::table[i] = p.table[i];


		if( value_size > argument_size )
		{	
			uint fill;

			if( UInt_type )
				fill = 0;
			else
				fill = (p.table[argument_size-1] & TTMATH_UINT_HIGHEST_BIT)?
														TTMATH_UINT_MAX_VALUE : 0;

			// 'this' is longer than 'p'
			for( ; i<value_size ; ++i)
				UInt<value_size>::table[i] = fill;
		}
		else
		{
			uint test = (UInt<value_size>::table[value_size-1] & TTMATH_UINT_HIGHEST_BIT)?
																TTMATH_UINT_MAX_VALUE : 0;

			if( UInt_type && test!=0 )
				return 1;

			for( ; i<argument_size ; ++i)
				if( p.table[i] != test )
					return 1;
		}

	return 0;
	}

public:

	/*!
		this method converts an Int<another_size> type into this class

		this operation has mainly sense if the value from p
		can be held in this type

		it returns a carry if the value 'p' is too big
	*/
	template<uint argument_size>
	uint FromInt(const Int<argument_size> & p)
	{
		return FromUIntOrInt(p, false);
	}


	/*!
		this method converts the sint type into this class
	*/
	uint FromInt(sint value)
	{
	uint fill = ( value<0 ) ? TTMATH_UINT_MAX_VALUE : 0;

		for(uint i=1 ; i<value_size ; ++i)
			UInt<value_size>::table[i] = fill;

		UInt<value_size>::table[0] = uint(value);
	
		// there'll never be a carry here
	return 0;
	}


	/*!
		this method converts UInt<another_size> into this class
	*/
	template<uint argument_size>
	uint FromUInt(const UInt<argument_size> & p)
	{
		return FromUIntOrInt(p, true);
	}


	/*!
		this method converts UInt<another_size> into this class
	*/
	template<uint argument_size>
	uint FromInt(const UInt<argument_size> & p)
	{
		return FromUIntOrInt(p, true);
	}


	/*!
		this method converts the uint type into this class
	*/
	uint FromUInt(uint value)
	{
		for(uint i=1 ; i<value_size ; ++i)
			UInt<value_size>::table[i] = 0;

		UInt<value_size>::table[0] = value;

		// there can be a carry here when the size of this value is equal one word
		// and the 'value' has the highest bit set
		if( value_size==1 && (value & TTMATH_UINT_HIGHEST_BIT)!=0 )
			return 1;

	return 0;
	}


	/*!
		this method converts the uint type into this class
	*/
	uint FromInt(uint value)
	{
		return FromUInt(value);
	}


	/*!
		the default assignment operator
	*/
	Int<value_size> & operator=(const Int<value_size> & p)
	{
		FromInt(p);

	return *this;
	}


	/*!
		this operator converts an Int<another_size> type to this class

		it doesn't return a carry
	*/
	template<uint argument_size>
	Int<value_size> & operator=(const Int<argument_size> & p)
	{
		FromInt(p);

	return *this;
	}


	/*!
		this method converts the sint type to this class
	*/
	Int<value_size> & operator=(sint i)
	{
		FromInt(i);

	return *this;
	}


	/*!
		a constructor for converting the uint to this class
	*/
	Int(sint i)
	{
		FromInt(i);
	}


	/*!
		a copy constructor
	*/
	Int(const Int<value_size> & u)
	{
		FromInt(u);
	}


	/*!
		a constructor for copying from another types
	*/
	template<uint argument_size>
	Int(const Int<argument_size> & u)
	{
		// look that 'size' we still set as 'value_size' and not as u.value_size
		FromInt(u);
	}



	/*!
		this operator converts an UInt<another_size> type to this class

		it doesn't return a carry
	*/
	template<uint argument_size>
	Int<value_size> & operator=(const UInt<argument_size> & p)
	{
		FromUInt(p);

	return *this;
	}


	/*!
		this method converts the Uint type to this class
	*/
	Int<value_size> & operator=(uint i)
	{
		FromUInt(i);

	return *this;
	}


	/*!
		a constructor for converting the uint to this class
	*/
	Int(uint i)
	{
		FromUInt(i);
	}


	/*!
		a constructor for copying from another types
	*/
	template<uint argument_size>
	Int(const UInt<argument_size> & u)
	{
		// look that 'size' we still set as 'value_size' and not as u.value_size
		FromUInt(u);
	}
 


#ifdef TTMATH_PLATFORM32


	/*!
		this method converts unsigned 64 bit int type to this class
		***this method is created only on a 32bit platform***
	*/
	uint FromUInt(ulint n)
	{
		uint c = UInt<value_size>::FromUInt(n);

		if( c )
			return 1;

		if( value_size == 1 )
			return ((UInt<value_size>::table[0] & TTMATH_UINT_HIGHEST_BIT) == 0) ? 0 : 1;
		
		if( value_size == 2 )
			return ((UInt<value_size>::table[1] & TTMATH_UINT_HIGHEST_BIT) == 0) ? 0 : 1;

	return 0;
	}


	/*!
		this method converts unsigned 64 bit int type to this class
		***this method is created only on a 32bit platform***
	*/
	uint FromInt(ulint n)
	{
		return FromUInt(n);
	}

		
	/*!