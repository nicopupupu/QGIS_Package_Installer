
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



#ifndef headerfilettmathuint
#define headerfilettmathuint


/*!
	\file ttmathuint.h
    \brief template class UInt<uint>
*/

#include <iostream>
#include <iomanip>


#include "ttmathtypes.h"
#include "ttmathmisc.h"



/*!
    \brief a namespace for the TTMath library
*/
namespace ttmath
{

/*! 
	\brief UInt implements a big integer value without a sign

	value_size - how many bytes specify our value
		on 32bit platforms: value_size=1 -> 4 bytes -> 32 bits
		on 64bit platforms: value_size=1 -> 8 bytes -> 64 bits
	value_size = 1,2,3,4,5,6....
*/
template<uint value_size>
class UInt
{
public:

	/*!
		buffer for the integer value
		  table[0] - the lowest word of the value
	*/
	uint table[value_size];



	/*!
		some methods used for debugging purposes
	*/


	/*!
		this method is only for debugging purposes or when we want to make
		a table of a variable (constant) in ttmathbig.h

		it prints the table in a nice form of several columns
	*/
	template<class ostream_type>
	void PrintTable(ostream_type & output) const
	{
		// how many columns there'll be
		const int columns = 8;

		int c = 1;
		for(int i=value_size-1 ; i>=0 ; --i)
		{
			output << "0x" << std::setfill('0');
			
			#ifdef TTMATH_PLATFORM32
				output << std::setw(8);
			#else
				output << std::setw(16);
			#endif
				
			output << std::hex << table[i];
			
			if( i>0 )
			{
				output << ", ";		
			
				if( ++c > columns )
				{
					output << std::endl;
					c = 1;
				}
			}
		}
		
		output << std::dec << std::endl;
	}


	/*!
		this method is used when macro TTMATH_DEBUG_LOG is defined
	*/
	template<class char_type, class ostream_type>
	static void PrintVectorLog(const char_type * msg, ostream_type & output, const uint * vector, uint vector_len)
	{
		output << msg << std::endl;

		for(uint i=0 ; i<vector_len ; ++i)
			output << " table[" << i << "]: " << vector[i] << std::endl;
	}


	/*!
		this method is used when macro TTMATH_DEBUG_LOG is defined
	*/
	template<class char_type, class ostream_type>
	static void PrintVectorLog(const char_type * msg, uint carry, ostream_type & output, const uint * vector, uint vector_len)
	{
		PrintVectorLog(msg, output, vector, vector_len);
		output << " carry: " << carry << std::endl;
	}


	/*!
		this method is used when macro TTMATH_DEBUG_LOG is defined
	*/
	template<class char_type, class ostream_type>
	void PrintLog(const char_type * msg, ostream_type & output) const
	{
		PrintVectorLog(msg, output, table, value_size);
	}


	/*!
		this method is used when macro TTMATH_DEBUG_LOG is defined
	*/
	template<class char_type, class ostream_type>
	void PrintLog(const char_type * msg, uint carry, ostream_type & output) const
	{
		PrintVectorLog(msg, output, table, value_size);
		output << " carry: " << carry << std::endl;
	}


	/*!
		this method returns the size of the table
	*/
	uint Size() const
	{
		return value_size;
	}


	/*!
		this method sets zero
	*/
	void SetZero()
	{
		// in the future here can be 'memset'

		for(uint i=0 ; i<value_size ; ++i)
			table[i] = 0;

		TTMATH_LOG("UInt::SetZero")
	}


	/*!
		this method sets one
	*/
	void SetOne()
	{
		SetZero();
		table[0] = 1;

		TTMATH_LOG("UInt::SetOne")
	}


	/*!
		this method sets the max value which this class can hold
		(all bits will be one)
	*/
	void SetMax()
	{
		for(uint i=0 ; i<value_size ; ++i)
			table[i] = TTMATH_UINT_MAX_VALUE;

		TTMATH_LOG("UInt::SetMax")
	}


	/*!
		this method sets the min value which this class can hold
		(for an unsigned integer value the zero is the smallest value)
	*/
	void SetMin()
	{
		SetZero();

		TTMATH_LOG("UInt::SetMin")
	}


	/*!
		this method swappes this for an argument
	*/
	void Swap(UInt<value_size> & ss2)
	{
		for(uint i=0 ; i<value_size ; ++i)
		{
			uint temp = table[i];
			table[i] = ss2.table[i];
			ss2.table[i] = temp;
		}
	}


#ifdef TTMATH_PLATFORM32

	/*!
		this method copies the value stored in an another table
		(warning: first values in temp_table are the highest words -- it's different
		from our table)

		we copy as many words as it is possible
		
		if temp_table_len is bigger than value_size we'll try to round 
		the lowest word from table depending on the last not used bit in temp_table
		(this rounding isn't a perfect rounding -- look at the description below)

		and if temp_table_len is smaller than value_size we'll clear the rest words
		in the table
	*/
	void SetFromTable(const uint * temp_table, uint temp_table_len)
	{
		uint temp_table_index = 0;
		sint i; // 'i' with a sign

		for(i=value_size-1 ; i>=0 && temp_table_index<temp_table_len; --i, ++temp_table_index)
			table[i] = temp_table[ temp_table_index ];


		// rounding mantissa
		if( temp_table_index < temp_table_len )
		{
			if( (temp_table[temp_table_index] & TTMATH_UINT_HIGHEST_BIT) != 0 )
			{
				/*
					very simply rounding
					if the bit from not used last word from temp_table is set to one
					we're rouding the lowest word in the table

					in fact there should be a normal addition but
					we don't use Add() or AddTwoInts() because these methods 
					can set a carry and then there'll be a small problem
					for optimization
				*/
				if( table[0] != TTMATH_UINT_MAX_VALUE )
					++table[0];
			}
		}

		// cleaning the rest of the mantissa
		for( ; i>=0 ; --i)
			table[i] = 0;


		TTMATH_LOG("UInt::SetFromTable")
	}

#endif


#ifdef TTMATH_PLATFORM64
	/*!
		this method copies the value stored in an another table
		(warning: first values in temp_table are the highest words -- it's different
		from our table)

		***this method is created only on a 64bit platform***

		we copy as many words as it is possible
		
		if temp_table_len is bigger than value_size we'll try to round 
		the lowest word from table depending on the last not used bit in temp_table
		(this rounding isn't a perfect rounding -- look at the description below)

		and if temp_table_len is smaller than value_size we'll clear the rest words
		in the table

		warning: we're using 'temp_table' as a pointer at 32bit words
	*/
	void SetFromTable(const unsigned int * temp_table, uint temp_table_len)
	{
		uint temp_table_index = 0;
		sint i; // 'i' with a sign

		for(i=value_size-1 ; i>=0 && temp_table_index<temp_table_len; --i, ++temp_table_index)
		{
			table[i] = uint(temp_table[ temp_table_index ]) << 32;

			++temp_table_index;

			if( temp_table_index<temp_table_len )
				table[i] |= temp_table[ temp_table_index ];
		}


		// rounding mantissa
		if( temp_table_index < temp_table_len )
		{
			if( (temp_table[temp_table_index] & TTMATH_UINT_HIGHEST_BIT) != 0 )
			{
				/*
					very simply rounding
					if the bit from not used last word from temp_table is set to one
					we're rouding the lowest word in the table

					in fact there should be a normal addition but
					we don't use Add() or AddTwoInts() because these methods 
					can set a carry and then there'll be a small problem
					for optimization
				*/
				if( table[0] != TTMATH_UINT_MAX_VALUE )
					++table[0];
			}
		}

		// cleaning the rest of the mantissa
		for( ; i >= 0 ; --i)
			table[i] = 0;

		TTMATH_LOG("UInt::SetFromTable")
	}

#endif





	/*!
	*
	*	basic mathematic functions
	*
	*/




	/*!
		this method adds one to the existing value
	*/
	uint AddOne()
	{
		return AddInt(1);
	}


	/*!
		this method subtracts one from the existing value
	*/
	uint SubOne()
	{
		return SubInt(1);
	}


private:


	/*!    
		an auxiliary method for moving bits into the left hand side

		this method moves only words
	*/
	void RclMoveAllWords(uint & rest_bits, uint & last_c, uint bits, uint c)
	{
		rest_bits      = bits % TTMATH_BITS_PER_UINT;
		uint all_words = bits / TTMATH_BITS_PER_UINT;
		uint mask      = ( c ) ? TTMATH_UINT_MAX_VALUE : 0;


		if( all_words >= value_size )
		{
			if( all_words == value_size && rest_bits == 0 )
				last_c = table[0] & 1;
			// else: last_c is default set to 0

			// clearing
			for(uint i = 0 ; i<value_size ; ++i)
				table[i] = mask;

			rest_bits = 0;
		}
		else
		if( all_words > 0 )  
		{
			// 0 < all_words < value_size
	
			sint first, second;
			last_c = table[value_size - all_words] & 1; // all_words is greater than 0

			// copying the first part of the value
			for(first = value_size-1, second=first-all_words ; second>=0 ; --first, --second)
				table[first] = table[second];

			// setting the rest to 'c'
			for( ; first>=0 ; --first )
				table[first] = mask;
		}

		TTMATH_LOG("UInt::RclMoveAllWords")
	}
	
public:

	/*!
		moving all bits into the left side 'bits' times
		return value <- this <- C

		bits is from a range of <0, man * TTMATH_BITS_PER_UINT>
		or it can be even bigger then all bits will be set to 'c'

		the value c will be set into the lowest bits
		and the method returns state of the last moved bit
	*/
	uint Rcl(uint bits, uint c=0)
	{
	uint last_c    = 0;
	uint rest_bits = bits;

		if( bits == 0 )
			return 0;

		if( bits >= TTMATH_BITS_PER_UINT )
			RclMoveAllWords(rest_bits, last_c, bits, c);

		if( rest_bits == 0 )
		{
			TTMATH_LOG("UInt::Rcl")
			return last_c;
		}

		// rest_bits is from 1 to TTMATH_BITS_PER_UINT-1 now
		if( rest_bits == 1 )
		{
			last_c = Rcl2_one(c);
		}
		else if( rest_bits == 2 )
		{
			// performance tests showed that for rest_bits==2 it's better to use Rcl2_one twice instead of Rcl2(2,c)
			Rcl2_one(c);
			last_c = Rcl2_one(c);
		}
		else
		{
			last_c = Rcl2(rest_bits, c);
		}

		TTMATH_LOGC("UInt::Rcl", last_c)

	return last_c;
	}

private:

	/*!    
		an auxiliary method for moving bits into the right hand side

		this method moves only words
	*/
	void RcrMoveAllWords(uint & rest_bits, uint & last_c, uint bits, uint c)
	{
		rest_bits      = bits % TTMATH_BITS_PER_UINT;
		uint all_words = bits / TTMATH_BITS_PER_UINT;
		uint mask      = ( c ) ? TTMATH_UINT_MAX_VALUE : 0;


		if( all_words >= value_size )
		{
			if( all_words == value_size && rest_bits == 0 )
				last_c = (table[value_size-1] & TTMATH_UINT_HIGHEST_BIT) ? 1 : 0;
			// else: last_c is default set to 0

			// clearing
			for(uint i = 0 ; i<value_size ; ++i)
				table[i] = mask;

			rest_bits = 0;
		}
		else if( all_words > 0 )
		{
			// 0 < all_words < value_size

			uint first, second;
			last_c = (table[all_words - 1] & TTMATH_UINT_HIGHEST_BIT) ? 1 : 0; // all_words is > 0

			// copying the first part of the value
			for(first=0, second=all_words ; second<value_size ; ++first, ++second)
				table[first] = table[second];

			// setting the rest to 'c'
			for( ; first<value_size ; ++first )
				table[first] = mask;
		}

		TTMATH_LOG("UInt::RcrMoveAllWords")
	}

public:

	/*!
		moving all bits into the right side 'bits' times
		c -> this -> return value

		bits is from a range of <0, man * TTMATH_BITS_PER_UINT>
		or it can be even bigger then all bits will be set to 'c'

		the value c will be set into the highest bits
		and the method returns state of the last moved bit
	*/
	uint Rcr(uint bits, uint c=0)
	{
	uint last_c    = 0;
	uint rest_bits = bits;
	
		if( bits == 0 )
			return 0;

		if( bits >= TTMATH_BITS_PER_UINT )
			RcrMoveAllWords(rest_bits, last_c, bits, c);

		if( rest_bits == 0 )
		{
			TTMATH_LOG("UInt::Rcr")
			return last_c;
		}

		// rest_bits is from 1 to TTMATH_BITS_PER_UINT-1 now
		if( rest_bits == 1 )
		{
			last_c = Rcr2_one(c);
		}
		else if( rest_bits == 2 )
		{
			// performance tests showed that for rest_bits==2 it's better to use Rcr2_one twice instead of Rcr2(2,c)
			Rcr2_one(c);
			last_c = Rcr2_one(c);
		}
		else
		{
			last_c = Rcr2(rest_bits, c);
		}

		TTMATH_LOGC("UInt::Rcr", last_c)

	return last_c;
	}


	/*!
		this method moves all bits into the left side
		(it returns value how many bits have been moved)
	*/
	uint CompensationToLeft()
	{
		uint moving = 0;

		// a - index a last word which is different from zero
		sint a;
		for(a=value_size-1 ; a>=0 && table[a]==0 ; --a);

		if( a < 0 )
			return moving; // all words in table have zero

		if( a != value_size-1 )
		{
			moving += ( value_size-1 - a ) * TTMATH_BITS_PER_UINT;

			// moving all words
			sint i;
			for(i=value_size-1 ; a>=0 ; --i, --a)
				table[i] = table[a];

			// setting the rest word to zero
			for(; i>=0 ; --i)
				table[i] = 0;
		}

		uint moving2 = FindLeadingBitInWord( table[value_size-1] );
		// moving2 is different from -1 because the value table[value_size-1]
		// is not zero

		moving2 = TTMATH_BITS_PER_UINT - moving2 - 1;
		Rcl(moving2);

		TTMATH_LOG("UInt::CompensationToLeft")

	return moving + moving2;
	}


	/*!
		this method looks for the highest set bit
		
		result:
			if 'this' is not zero:
				return value - true
				'table_id'   - the index of a word <0..value_size-1>
				'index'      - the index of this set bit in the word <0..TTMATH_BITS_PER_UINT)

			if 'this' is zero: 
				return value - false
				both 'table_id' and 'index' are zero
	*/
	bool FindLeadingBit(uint & table_id, uint & index) const
	{
		for(table_id=value_size-1 ; table_id!=0 && table[table_id]==0 ; --table_id);

		if( table_id==0 && table[table_id]==0 )
		{
			// is zero
			index = 0;

		return false;
		}
		
		// table[table_id] is different from 0
		index = FindLeadingBitInWord( table[table_id] );

	return true;
	}


	/*!
		this method looks for the smallest set bit
		
		result:
			if 'this' is not zero:
				return value - true
				'table_id'   - the index of a word <0..value_size-1>
				'index'      - the index of this set bit in the word <0..TTMATH_BITS_PER_UINT)

			if 'this' is zero: 
				return value - false
				both 'table_id' and 'index' are zero
	*/
	bool FindLowestBit(uint & table_id, uint & index) const
	{
		for(table_id=0 ; table_id<value_size && table[table_id]==0 ; ++table_id);

		if( table_id >= value_size )
		{
			// is zero
			index    = 0;
			table_id = 0;

		return false;
		}
		
		// table[table_id] is different from 0
		index = FindLowestBitInWord( table[table_id] );

	return true;
	}


	/*!
		getting the 'bit_index' bit

		bit_index bigger or equal zero
	*/
	uint GetBit(uint bit_index) const
	{
		TTMATH_ASSERT( bit_index < value_size * TTMATH_BITS_PER_UINT )

		uint index = bit_index / TTMATH_BITS_PER_UINT;
		uint bit   = bit_index % TTMATH_BITS_PER_UINT;

		uint temp = table[index];
		uint res  = SetBitInWord(temp, bit);

	return res;
	}


	/*!
		setting the 'bit_index' bit
		and returning the last state of the bit

		bit_index bigger or equal zero
	*/
	uint SetBit(uint bit_index)
	{
		TTMATH_ASSERT( bit_index < value_size * TTMATH_BITS_PER_UINT )

		uint index = bit_index / TTMATH_BITS_PER_UINT;
		uint bit   = bit_index % TTMATH_BITS_PER_UINT;
		uint res   = SetBitInWord(table[index], bit);

		TTMATH_LOG("UInt::SetBit")

	return res;
	}


	/*!
		this method performs a bitwise operation AND 
	*/
	void BitAnd(const UInt<value_size> & ss2)
	{
		for(uint x=0 ; x<value_size ; ++x)
			table[x] &= ss2.table[x];

		TTMATH_LOG("UInt::BitAnd")
	}


	/*!
		this method performs a bitwise operation OR 
	*/
	void BitOr(const UInt<value_size> & ss2)
	{
		for(uint x=0 ; x<value_size ; ++x)
			table[x] |= ss2.table[x];

		TTMATH_LOG("UInt::BitOr")
	}


	/*!
		this method performs a bitwise operation XOR 
	*/
	void BitXor(const UInt<value_size> & ss2)
	{
		for(uint x=0 ; x<value_size ; ++x)
			table[x] ^= ss2.table[x];

		TTMATH_LOG("UInt::BitXor")
	}
