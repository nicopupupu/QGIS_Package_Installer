
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


#ifndef headerfilettmathuint_x86_64
#define headerfilettmathuint_x86_64


#ifndef TTMATH_NOASM
#ifdef TTMATH_PLATFORM64


/*!
	\file ttmathuint_x86_64.h
    \brief template class UInt<uint> with assembler code for 64bit x86_64 processors

	this file is included at the end of ttmathuint.h
*/

#ifndef __GNUC__
#include <intrin.h>
#endif


namespace ttmath
{

	#ifndef __GNUC__

		extern "C"
			{
			uint __fastcall ttmath_adc_x64(uint* p1, const uint* p2, uint nSize, uint c);
			uint __fastcall ttmath_addindexed_x64(uint* p1, uint nSize, uint nPos, uint nValue);
			uint __fastcall ttmath_addindexed2_x64(uint* p1, uint nSize, uint nPos, uint nValue1, uint nValue2);
			uint __fastcall ttmath_addvector_x64(const uint * ss1, const uint * ss2, uint ss1_size, uint ss2_size, uint * result);
			uint __fastcall ttmath_sbb_x64(uint* p1, const uint* p2, uint nSize, uint c);
			uint __fastcall ttmath_subindexed_x64(uint* p1, uint nSize, uint nPos, uint nValue);
			uint __fastcall ttmath_subvector_x64(const uint * ss1, const uint * ss2, uint ss1_size, uint ss2_size, uint * result);
			uint __fastcall ttmath_rcl_x64(uint* p1, uint nSize, uint nLowestBit);
			uint __fastcall ttmath_rcr_x64(uint* p1, uint nSize, uint nLowestBit);
			uint __fastcall ttmath_div_x64(uint* pnValHi, uint* pnValLo, uint nDiv);
			uint __fastcall ttmath_rcl2_x64(uint* p1, uint nSize, uint nBits, uint c);
			uint __fastcall ttmath_rcr2_x64(uint* p1, uint nSize, uint nBits, uint c);
			};
	#endif


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
		#ifndef __GNUC__
			static const char info[] = "asm_vc_64";
		#endif		

		#ifdef __GNUC__
			static const char info[] = "asm_gcc_64";
		#endif

	return info;
	}


	/*!
		returning the currect type of the library
	*/
	template<uint value_size>
	LibTypeCode UInt<value_size>::LibType()
	{
		#ifndef __GNUC__
			LibTypeCode info = asm_vc_64;
		#endif		

		#ifdef __GNUC__
			LibTypeCode info = asm_gcc_64;
		#endif

	return info;
	}


	/*!
	*
	*	basic mathematic functions
	*
	*/



	/*!
		this method adding ss2 to the this and adding carry if it's defined
		(this = this + ss2 + c)

		***this method is created only on a 64bit platform***

		c must be zero or one (might be a bigger value than 1)
		function returns carry (1) (if it was)
	*/
	template<uint value_size>
	uint UInt<value_size>::Add(const UInt<value_size> & ss2, uint c)
	{
	uint b = value_size;
	uint * p1 = table;
	const uint * p2 = ss2.table;

		// we don't have to use TTMATH_REFERENCE_ASSERT here
		// this algorithm doesn't require it

		#ifndef __GNUC__
			c = ttmath_adc_x64(p1,p2,b,c);
		#endif

		#ifdef __GNUC__
		uint dummy, dummy2;

			/*
				this part should be compiled with gcc
			*/
			__asm__ __volatile__(
	
				"xorq %%rdx, %%rdx				\n"
				"negq %%rax						\n"     // CF=1 if rax!=0 , CF=0 if rax==0

			"1:									\n"
				"movq (%%rsi,%%rdx,8), %%rax	\n"
				"adcq %%rax, (%%rbx,%%rdx,8)	\n"
			
				"incq %%rdx						\n"
				"decq %%rcx						\n"
			"jnz 1b								\n"

				"adcq %%rcx, %%rcx				\n"

				: "=c" (c), "=a" (dummy), "=d" (dummy2)
				: "0" (b),  "1" (c), "b" (p1), "S" (p2)
				: "cc", "memory" );

		#endif

		TTMATH_LOGC("UInt::Add", c)
	
	return c;
	}



	/*!
		this method adds one word (at a specific position)
		and returns a carry (if it was)

		***this method is created only on a 64bit platform***


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
	uint b = value_size;
	uint * p1 = table;
	uint c;

		TTMATH_ASSERT( index < value_size )

		#ifndef __GNUC__
			c = ttmath_addindexed_x64(p1,b,index,value);
		#endif


		#ifdef __GNUC__
		uint dummy, dummy2;
	
			__asm__ __volatile__(

				"subq %%rdx, %%rcx 				\n"

			"1:									\n"
				"addq %%rax, (%%rbx,%%rdx,8)	\n"
			"jnc 2f								\n"
				
				"movq $1, %%rax					\n"
				"incq %%rdx						\n"
				"decq %%rcx						\n"
			"jnz 1b								\n"

			"2:									\n"
				"setc %%al						\n"
				"movzx %%al, %%rdx				\n"

				: "=d" (c),    "=a" (dummy), "=c" (dummy2)
				: "0" (index), "1" (value),  "2" (b), "b" (p1)
				: "cc", "memory" );

		#endif

		TTMATH_LOGC("UInt::AddInt", c)
	
	return c;
	}



	/*!
		this method adds only two unsigned words to the existing value
		and these words begin on the 'index' position
		(it's used in the multiplication algorithm 2)

		***this method is created only on a 64bit platform***

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
	uint b = value_size;
	uint * p1 = table;
	uint c;

		TTMATH_ASSERT( index < value_size - 1 )

		#ifndef __GNUC__