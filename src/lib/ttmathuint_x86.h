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
		  no_asm_64   - pure C++