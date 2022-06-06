/*
 * This file is a part of TTMath Bignum Library
 * and is distributed under the (new) BSD licence.
 * Author: Tomasz Sowa <t.sowa@ttmath.org>
 */

/* 
 * Copyright (c) 2006-2009, Tomasz Sowa
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



#ifndef headerfilettmathuint_x86
#define headerfilettmathuint_x86


#ifndef TTMATH_NOASM
#ifdef TTMATH_PLATFORM32


/*!
	\file ttmathuint_x86.h
    \brief template class UInt<uint> with assembler code for 32bit x86 processors

	this file is included at the end of ttmathuint.h
*/



/*!
    \brief a namespace for the TTMath library
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
		#ifndef __GNUC__
			static const char info[] = "asm_vc_32";
		#endif		

		#ifdef __GNUC__
			static const char info[] = "asm_gcc_32";
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
			LibTypeCode info = asm_vc_32;
		#endif		

		#ifdef __GNUC__
			LibTypeCode info = asm_gcc_32;
		#endif

	return info;
	}



	/*!
	*
	*	basic mathematic functions
	*
	*/


	/*!
		adding ss2 to the this and adding carry if it's defined
		(this = this + ss2 + c)

		c must be zero or one (might be a bigger value than 1)
		function returns carry (1) (if it has been)
	*/
	template<uint value_size>
	uint UInt<value_size>::Add(const UInt<value_size> & ss2, uint c)
	{
	uint b = value_size;
	uint * p1 = table;
	uint * p2 = const_cast<uint*>(ss2.table);

		// we don't have to use TTMATH_REFERENCE_ASSERT here
		// this algorithm doesn't require it

		#ifndef __GNUC__
			
			//	this part might be compiled with for example visual c

			__asm
			{
				push eax
				push ebx
				push ecx
				push edx
				push esi

				mov ecx,[b]
				
				mov ebx,[p1]
				mov esi,[p2]

				xor edx,edx          // edx=0
				mov eax,[c]
				neg eax              // CF=1 if rax!=0 , CF=0 if rax==0

			ttmath_loop:
				mov eax,[esi+edx*4]
				adc [ebx+edx*4],eax

				inc edx
				dec ecx
			jnz ttmath_loop

				adc ecx, ecx
				mov [c], ecx

				pop esi
				pop edx
				pop ecx
				pop ebx
				pop eax
			}



		#endif		
			

		#ifdef __GNUC__
		uint dummy, dummy2;
			//	this part should be compiled with gcc
			
			__asm__ __volatile__(

				"xorl %%edx, %%edx				\n"
				"negl %%eax						\n"  // CF=1 if rax!=0 , CF=0 if rax==0

			"1:									\n"
				"movl (%%esi,%%edx,4), %%eax	\n"
				"adcl %%eax, (%%ebx,%%edx,4)	\n"
			
				"incl %%edx						\n"
				"decl %%ecx						\n"
			"jnz 1b								\n"

				"adc %%ecx, %%ecx				\n"

				: "=c" (c), "=a" (dummy), "=d" (dummy2)
				: "0" (b),  "1" (c), "b" (p1), "S" (p2)
				: "cc", "memory" );
		#endif

		TTMATH_LOGC("UInt::Add", c)

	return c;
	}



	/*!
		adding one word (at a specific position)
		and returning a carry (if it has been)

		e.g.

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

			__asm
			{
				push eax
				push ebx
				push ecx
				push edx

				mov ecx, [b]
				sub ecx, [index]				

				mov edx, [index]
				mov ebx, [p1]

				mov eax, [value]

			ttmath_loop:
				add [ebx+edx*4], eax
			jnc ttmath_end

				mov eax, 1
				inc edx
				dec ecx
			jnz ttmath_loop

			ttmath_end:
				setc al
				movzx edx, al
				mov [c], edx

				pop edx
				pop ecx
				pop ebx
				pop eax
			}

		#endif		
			

		#ifdef __GNUC__
		uint dummy, dummy2;

			__asm__ __volatile__(
			
				"subl %%edx, %%ecx 				\n"

			"1:									\n"
				"addl %%eax, (%%ebx,%%edx,4)	\n"
			"jnc 2f								\n"
				
				"movl $1, %%eax					\n"
				"incl %%edx						\n"
				"decl %%ecx						\n"
			"jnz 1b								\n"

			"2:									\n"
				"setc %%al						\n"
				"movzx %%al, %%edx				\n"

				: "=d" (c),    "=a" (dummy), "=c" (dummy2)
				: "0" (index), "1" (value),  "2" (b), "b" (p1)
				: "cc", "memory" );

		#endif
	
		TTMATH_LOGC("UInt::AddInt", c)

	return c;
	}




	/*!
		adding only two unsigned words to the existing value
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
	uint b = value_size;
	uint * p1 = table;
	uint c;

		TTMATH_ASSERT( index < value_size - 1 )

		#ifndef __GNUC__
			__asm
			{
				push eax
				push ebx
				push ecx
				push edx

				mov ecx, [b]
				sub ecx, [index]				

				mov ebx, [p1]
				mov edx, [index]

				mov eax, [x1]
				add [ebx+edx*4], eax
				inc edx
				dec ecx

				mov eax, [x2]
			
			ttmath_loop:
				adc [ebx+edx*4], eax
			jnc ttmath_end

				mov eax, 0
				inc edx
				dec ecx
			jnz ttmath_loop

			ttmath_end:
				setc al
				movzx edx, al
				mov [c], edx
				
				pop edx
				pop ecx
				pop ebx
				pop eax

			}
		#endif		
			

		#ifdef __GNUC__
		uint dummy