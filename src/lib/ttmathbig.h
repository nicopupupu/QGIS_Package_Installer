
/*
 * This file is a part of TTMath Bignum Library
 * and is distributed under the (new) BSD licence.
 * Author: Tomasz Sowa <t.sowa@ttmath.org>
 */

/* 
 * Copyright (c) 2006-2012, Tomasz Sowa
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

#ifndef headerfilettmathbig
#define headerfilettmathbig

/*!
	\file ttmathbig.h
    \brief A Class for representing floating point numbers
*/

#include "ttmathint.h"
#include "ttmaththreads.h"

#include <iostream>

#ifdef TTMATH_MULTITHREADS
#include <signal.h>
#endif

namespace ttmath
{


/*!
	\brief Big implements the floating point numbers
*/
template <uint exp, uint man>
class Big
{

/*
	value = mantissa * 2^exponent	

	exponent - an integer value with a sign
	mantissa - an integer value without a sing

	mantissa must be pushed into the left side that is the highest bit from 
	mantissa must be one (of course if there's another value than zero) -- this job
	(pushing bits into the left side) making Standardizing() method

	for example:
	if we want to store value one (1) into our Big object we must:
		set mantissa to 1
		set exponent to 0
		set info to 0
		and call method Standardizing()
*/


public:

Int<exp>  exponent;
UInt<man> mantissa;
unsigned char info;


/*!
	Sign
	the mask of a bit from 'info' which means that there is a sign
	(when the bit is set)
*/
#define TTMATH_BIG_SIGN 128


/*!
	Not a number
	if this bit is set that there is not a valid number
*/
#define TTMATH_BIG_NAN  64


/*!
	Zero
	if this bit is set that there is value zero
	mantissa should be zero and exponent should be zero too
	(the Standardizing() method does this)
*/
#define TTMATH_BIG_ZERO  32


	/*!
		this method sets NaN if there was a carry (and returns 1 in such a case)

		c can be 0, 1 or other value different from zero
	*/
	uint CheckCarry(uint c)
	{
		if( c != 0 )
		{
			SetNan();
			return 1;
		}

	return 0;
	}

public:


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
	static const char * LibTypeStr()
	{
		return UInt<man>::LibTypeStr();
	}


	/*!
		returning the currect type of the library
	*/
	static LibTypeCode LibType()
	{
		return UInt<man>::LibType();
	}



	/*!
		this method moves all bits from mantissa into its left side
		(suitably changes the exponent) or if the mantissa is zero
		it sets the exponent to zero as well
		(and clears the sign bit and sets the zero bit)

		it can return a carry
		the carry will be when we don't have enough space in the exponent

		you don't have to use this method if you don't change the mantissa
		and exponent directly
	*/
	uint Standardizing()
	{
		if( mantissa.IsTheHighestBitSet() )
		{
			ClearInfoBit(TTMATH_BIG_ZERO);
			return 0;
		}

		if( CorrectZero() )
			return 0;

		uint comp = mantissa.CompensationToLeft();

	return exponent.Sub( comp );
	}


private:

	/*!
		if the mantissa is equal zero this method sets exponent to zero and
		info without the sign

		it returns true if there was the correction
	*/
	bool CorrectZero()
	{
		if( mantissa.IsZero() )
		{
			SetInfoBit(TTMATH_BIG_ZERO);
			ClearInfoBit(TTMATH_BIG_SIGN);
			exponent.SetZero();

			return true;
		}
		else
		{
			ClearInfoBit(TTMATH_BIG_ZERO);
		}

	return false;
	}


public:

	/*!
		this method clears a specific bit in the 'info' variable

		bit is one of: TTMATH_BIG_SIGN, TTMATH_BIG_NAN etc.
	*/
	void ClearInfoBit(unsigned char bit)
	{
		info = info & (~bit);
	}


	/*!
		this method sets a specific bit in the 'info' variable

		bit is one of: TTMATH_BIG_SIGN, TTMATH_BIG_NAN etc.

	*/
	void SetInfoBit(unsigned char bit)
	{
		info = info | bit;
	}


	/*!
		this method returns true if a specific bit in the 'info' variable is set

		bit is one of: TTMATH_BIG_SIGN, TTMATH_BIG_NAN etc.
	*/
	bool IsInfoBit(unsigned char bit) const
	{
		return (info & bit) != 0;
	}


	/*!
		this method sets zero
	*/
	void SetZero()
	{
		info = TTMATH_BIG_ZERO;
		exponent.SetZero();
		mantissa.SetZero();

		/*
			we don't have to compensate zero
		*/
	}

	
	/*!
		this method sets one
	*/
	void SetOne()
	{
		info = 0;
		mantissa.SetZero();
		mantissa.table[man-1] = TTMATH_UINT_HIGHEST_BIT;
		exponent = -sint(man * TTMATH_BITS_PER_UINT - 1);

		// don't have to Standardize() - the last bit from mantissa is set
	}


	/*!
		this method sets value 0.5
	*/
	void Set05()
	{
		SetOne();
		exponent.SubOne();
	}


	/*!
		this method sets NaN flag (Not a Number)
		when this flag is set that means there is no a valid number
	*/
	void SetNan()
	{
		SetInfoBit(TTMATH_BIG_NAN);
	}


	/*!
		this method sets NaN flag (Not a Number)
		also clears the mantissa and exponent (similarly as it would be a zero value)
	*/
	void SetZeroNan()
	{
		SetZero();
		SetNan();
	}


	/*!
		this method swappes this for an argument
	*/
	void Swap(Big<exp, man> & ss2)
	{
		unsigned char info_temp = info;
		info = ss2.info;
		ss2.info = info_temp;

		exponent.Swap(ss2.exponent);
		mantissa.Swap(ss2.mantissa);
	}


private:

	/*!
		this method sets the mantissa of the value of pi
	*/
	void SetMantissaPi()
	{
	// this is a static table which represents the value of Pi (mantissa of it)
	// (first is the highest word)
	// we must define this table as 'unsigned int' because 
	// both on 32bit and 64bit platforms this table is 32bit
	static const unsigned int temp_table[] = {
		0xc90fdaa2, 0x2168c234, 0xc4c6628b, 0x80dc1cd1, 0x29024e08, 0x8a67cc74, 0x020bbea6, 0x3b139b22, 
		0x514a0879, 0x8e3404dd, 0xef9519b3, 0xcd3a431b, 0x302b0a6d, 0xf25f1437, 0x4fe1356d, 0x6d51c245, 
		0xe485b576, 0x625e7ec6, 0xf44c42e9, 0xa637ed6b, 0x0bff5cb6, 0xf406b7ed, 0xee386bfb, 0x5a899fa5, 
		0xae9f2411, 0x7c4b1fe6, 0x49286651, 0xece45b3d, 0xc2007cb8, 0xa163bf05, 0x98da4836, 0x1c55d39a, 
		0x69163fa8, 0xfd24cf5f, 0x83655d23, 0xdca3ad96, 0x1c62f356, 0x208552bb, 0x9ed52907, 0x7096966d, 
		0x670c354e, 0x4abc9804, 0xf1746c08, 0xca18217c, 0x32905e46, 0x2e36ce3b, 0xe39e772c, 0x180e8603, 
		0x9b2783a2, 0xec07a28f, 0xb5c55df0, 0x6f4c52c9, 0xde2bcbf6, 0x95581718, 0x3995497c, 0xea956ae5, 
		0x15d22618, 0x98fa0510, 0x15728e5a, 0x8aaac42d, 0xad33170d, 0x04507a33, 0xa85521ab, 0xdf1cba64, 
		0xecfb8504, 0x58dbef0a, 0x8aea7157, 0x5d060c7d, 0xb3970f85, 0xa6e1e4c7, 0xabf5ae8c, 0xdb0933d7, 
		0x1e8c94e0, 0x4a25619d, 0xcee3d226, 0x1ad2ee6b, 0xf12ffa06, 0xd98a0864, 0xd8760273, 0x3ec86a64, 
		0x521f2b18, 0x177b200c, 0xbbe11757, 0x7a615d6c, 0x770988c0, 0xbad946e2, 0x08e24fa0, 0x74e5ab31, 
		0x43db5bfc, 0xe0fd108e, 0x4b82d120, 0xa9210801, 0x1a723c12, 0xa787e6d7, 0x88719a10, 0xbdba5b26, 
		0x99c32718, 0x6af4e23c, 0x1a946834, 0xb6150bda, 0x2583e9ca, 0x2ad44ce8, 0xdbbbc2db, 0x04de8ef9, 
		0x2e8efc14, 0x1fbecaa6, 0x287c5947, 0x4e6bc05d, 0x99b2964f, 0xa090c3a2, 0x233ba186, 0x515be7ed, 
		0x1f612970, 0xcee2d7af, 0xb81bdd76, 0x2170481c, 0xd0069127, 0xd5b05aa9, 0x93b4ea98, 0x8d8fddc1, 
		0x86ffb7dc, 0x90a6c08f, 0x4df435c9, 0x34028492, 0x36c3fab4, 0xd27c7026, 0xc1d4dcb2, 0x602646de, 
		0xc9751e76, 0x3dba37bd, 0xf8ff9406, 0xad9e530e, 0xe5db382f, 0x413001ae, 0xb06a53ed, 0x9027d831, 
		0x179727b0, 0x865a8918, 0xda3edbeb, 0xcf9b14ed, 0x44ce6cba, 0xced4bb1b, 0xdb7f1447, 0xe6cc254b, 
		0x33205151, 0x2bd7af42, 0x6fb8f401, 0x378cd2bf, 0x5983ca01, 0xc64b92ec, 0xf032ea15, 0xd1721d03, 
		0xf482d7ce, 0x6e74fef6, 0xd55e702f, 0x46980c82, 0xb5a84031, 0x900b1c9e, 0x59e7c97f, 0xbec7e8f3, 
		0x23a97a7e, 0x36cc88be, 0x0f1d45b7, 0xff585ac5, 0x4bd407b2, 0x2b4154aa, 0xcc8f6d7e, 0xbf48e1d8, 
		0x14cc5ed2, 0x0f8037e0, 0xa79715ee, 0xf29be328, 0x06a1d58b, 0xb7c5da76, 0xf550aa3d, 0x8a1fbff0, 
		0xeb19ccb1, 0xa313d55c, 0xda56c9ec, 0x2ef29632, 0x387fe8d7, 0x6e3c0468, 0x043e8f66, 0x3f4860ee, 
		0x12bf2d5b, 0x0b7474d6, 0xe694f91e, 0x6dbe1159, 0x74a3926f, 0x12fee5e4, 0x38777cb6, 0xa932df8c, 
		0xd8bec4d0, 0x73b931ba, 0x3bc832b6, 0x8d9dd300, 0x741fa7bf, 0x8afc47ed, 0x2576f693, 0x6ba42466, 
		0x3aab639c, 0x5ae4f568, 0x3423b474, 0x2bf1c978, 0x238f16cb, 0xe39d652d, 0xe3fdb8be, 0xfc848ad9, 
		0x22222e04, 0xa4037c07, 0x13eb57a8, 0x1a23f0c7, 0x3473fc64, 0x6cea306b, 0x4bcbc886, 0x2f8385dd, 
		0xfa9d4b7f, 0xa2c087e8, 0x79683303, 0xed5bdd3a, 0x062b3cf5, 0xb3a278a6, 0x6d2a13f8, 0x3f44f82d, 
		0xdf310ee0, 0x74ab6a36, 0x4597e899, 0xa0255dc1, 0x64f31cc5, 0x0846851d, 0xf9ab4819, 0x5ded7ea1, 
		0xb1d510bd, 0x7ee74d73, 0xfaf36bc3, 0x1ecfa268, 0x359046f4, 0xeb879f92, 0x4009438b, 0x481c6cd7, 
		0x889a002e, 0xd5ee382b, 0xc9190da6, 0xfc026e47, 0x9558e447, 0x5677e9aa, 0x9e3050e2, 0x765694df, 
		0xc81f56e8, 0x80b96e71, 0x60c980dd, 0x98a573ea, 0x4472065a, 0x139cd290, 0x6cd1cb72, 0x9ec52a53 // last one was: 0x9ec52a52
		//0x86d44014, ...
		// (the last word 0x9ec52a52 was rounded up because the next one is 0x86d44014 -- first bit is one 0x8..)
		// 256 32bit words for the mantissa -- about 2464 valid decimal digits
		};
		// the value of PI is comming from the website http://zenwerx.com/pi.php
		// 3101 digits were taken from this website
		//  (later the digits were compared with:
		//   http://www.eveandersson.com/pi/digits/1000000 and http://www.geom.uiuc.edu/~huberty/math5337/groupe/digits.html )
		// and they were set into Big<1,400> type (using operator=(const char*) on a 32bit platform)
		// and then the first 256 words were taken into this table
		// (TTMATH_BUILTIN_VARIABLES_SIZE on 32bit platform should have the value 256,
		// and on 64bit platform value 128 (256/2=128))
	
		mantissa.SetFromTable(temp_table, sizeof(temp_table) / sizeof(int));
	}

public:

