
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


#ifndef headerfilettmathtypes
#define headerfilettmathtypes

/*!
	\file ttmathtypes.h
    \brief constants used in the library
    
    As our library is written in header files (templates) we cannot use
	constants like 'const int' etc. because we should have some source files
	*.cpp to define this variables. Only what we can have are constants
	defined by #define preprocessor macros.

	All macros are preceded by TTMATH_ prefix
*/


#include <stdexcept>
#include <sstream>
#include <vector>

#ifndef _MSC_VER
#include <stdint.h>
// for uint64_t and int64_t on a 32 bit platform
#endif



/*!
	the version of the library

	TTMATH_PRERELEASE_VER is either zero or one
	zero means that this is the release version of the library
	(one means something like beta)
*/
#define TTMATH_MAJOR_VER		0
#define TTMATH_MINOR_VER		9
#define TTMATH_REVISION_VER		3

#define TTMATH_PRERELEASE_VER	0



/*!
	you can define a platform explicitly by defining either
	TTMATH_PLATFORM32 or TTMATH_PLATFORM64 macro
*/
#if !defined TTMATH_PLATFORM32 && !defined TTMATH_PLATFORM64

	#if !defined _M_X64 && !defined __x86_64__

		/*
			other platforms than x86 and amd64 are not recognized at the moment
			so you should set TTMATH_PLATFORMxx manually
		*/

		// we're using a 32bit platform