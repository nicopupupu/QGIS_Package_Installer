
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
		#define TTMATH_PLATFORM32

	#else

		//	we're using a 64bit platform
		#define TTMATH_PLATFORM64

	#endif

#endif


/*!
	asm version of the library is available by default only for:
	x86 and amd64 platforms and for Microsoft Visual and GCC compilers

	but you can force using asm version (the same asm as for Microsoft Visual)
	by defining TTMATH_FORCEASM macro
	you have to be sure that your compiler accept such an asm format
*/
#ifndef TTMATH_FORCEASM

	#if !defined __i386__  && !defined _X86_ && !defined  _M_IX86 && !defined __x86_64__  && !defined _M_X64
		/*!
			x86 architecture:
			__i386__    defined by GNU C
			_X86_  	    defined by MinGW32
			_M_IX86     defined by Visual Studio, Intel C/C++, Digital Mars and Watcom C/C++

			amd64 architecture:
			__x86_64__  defined by GNU C, CLANG (LLVM) and Sun Studio
			_M_X64  	defined by Visual Studio

			asm version is available only for x86 or amd64 platforms
		*/
		#define TTMATH_NOASM
	#endif



	#if !defined _MSC_VER && !defined __GNUC__
		/*!
			another compilers than MS VC or GCC or CLANG (LLVM) by default use no asm version
			(CLANG defines __GNUC__ too)
		*/
		#define TTMATH_NOASM
	#endif

#endif


namespace ttmath
{


#ifdef TTMATH_PLATFORM32

	/*!
		on 32bit platforms one word (uint, sint) will be equal 32bits
	*/
	typedef unsigned int uint;
	typedef signed   int sint;

	/*!
		on 32 bit platform ulint and slint will be equal 64 bits
	*/
	#ifdef _MSC_VER
		// long long on MS Windows (Visual and GCC mingw compilers) have 64 bits
		// stdint.h is not available on Visual Studio prior to VS 2010 version
		typedef unsigned long long int ulint;
		typedef signed   long long int slint;
	#else
		// we do not use 'long' here because there is a difference in unix and windows
		// environments: in unix 'long' has 64 bits but in windows it has only 32 bits
		typedef uint64_t ulint;
		typedef int64_t  slint;
	#endif

	/*!
		how many bits there are in the uint type
	*/
	#define TTMATH_BITS_PER_UINT 32u

	/*!
		the mask for the highest bit in the unsigned 32bit word (2^31)
	*/
	#define TTMATH_UINT_HIGHEST_BIT 2147483648u

	/*!
		the max value of the unsigned 32bit word (2^32 - 1)
		(all bits equal one)
	*/
	#define TTMATH_UINT_MAX_VALUE 4294967295u

	/*!
		the number of words (32bit words on 32bit platform)
		which are kept in built-in variables for a Big<> type
		(these variables are defined in ttmathbig.h)
	*/
	#define TTMATH_BUILTIN_VARIABLES_SIZE 256u

	/*!
		this macro returns the number of machine words 
		capable to hold min_bits bits
		e.g. TTMATH_BITS(128) returns 4
	*/
	#define TTMATH_BITS(min_bits) ((min_bits-1)/32 + 1)

#else

	/*!
		on 64bit platforms one word (uint, sint) will be equal 64bits
	*/
	#ifdef _MSC_VER
		/* in VC 'long' type has 32 bits, __int64 is VC extension */
		typedef unsigned __int64 uint;
		typedef signed   __int64 sint;
	#else
		typedef unsigned long uint;
		typedef signed   long sint;
	#endif 

	/*!
		on 64bit platforms we do not define ulint and slint
	*/

	/*!
		how many bits there are in the uint type
	*/
	#define TTMATH_BITS_PER_UINT 64ul

	/*!
		the mask for the highest bit in the unsigned 64bit word (2^63)
	*/
	#define TTMATH_UINT_HIGHEST_BIT 9223372036854775808ul

	/*!
		the max value of the unsigned 64bit word (2^64 - 1)
		(all bits equal one)
	*/
	#define TTMATH_UINT_MAX_VALUE 18446744073709551615ul

	/*!
		the number of words (64bit words on 64bit platforms)
		which are kept in built-in variables for a Big<> type
		(these variables are defined in ttmathbig.h)
	*/
	#define TTMATH_BUILTIN_VARIABLES_SIZE 128ul

	/*!
		this macro returns the number of machine words 
		capable to hold min_bits bits
		e.g. TTMATH_BITS(128) returns 2
	*/
	#define TTMATH_BITS(min_bits) ((min_bits-1)/64 + 1)

#endif
}


#if defined(TTMATH_MULTITHREADS) && !defined(TTMATH_MULTITHREADS_NOSYNC)
	#if !defined(TTMATH_POSIX_THREADS) && !defined(TTMATH_WIN32_THREADS)

		#if defined(_WIN32)
			#define TTMATH_WIN32_THREADS
		#elif defined(unix) || defined(__unix__) || defined(__unix)
			#define TTMATH_POSIX_THREADS
		#endif

	#endif
#endif



/*!
	this variable defines how many iterations are performed
	during some kind of calculating when we're making any long formulas
	(for example Taylor series)

	it's used in ExpSurrounding0(...), LnSurrounding1(...), Sin0pi05(...), etc.

	note! there'll not be so many iterations, iterations are stopped when
	there is no sense to continue calculating (for example when the result
	still remains unchanged after adding next series and we know that the next
	series are smaller than previous ones)
*/
#define TTMATH_ARITHMETIC_MAX_LOOP 10000



/*!
	this is a limit when calculating Karatsuba multiplication
	if the size of a vector is smaller than TTMATH_USE_KARATSUBA_MULTIPLICATION_FROM_SIZE
	the Karatsuba algorithm will use standard schoolbook multiplication
*/
#ifdef TTMATH_DEBUG_LOG
	// if TTMATH_DEBUG_LOG is defined then we should use the same size regardless of the compiler
	#define TTMATH_USE_KARATSUBA_MULTIPLICATION_FROM_SIZE 3
#else
	#ifdef __GNUC__
		#define TTMATH_USE_KARATSUBA_MULTIPLICATION_FROM_SIZE 3
	#else
		#define TTMATH_USE_KARATSUBA_MULTIPLICATION_FROM_SIZE 5
	#endif
#endif


/*!
	this is a special value used when calculating the Gamma(x) function
	if x is greater than this value then the Gamma(x) will be calculated using
	some kind of series

	don't use smaller values than about 100
*/
#define TTMATH_GAMMA_BOUNDARY 2000





namespace ttmath
{

	/*!
		lib type codes:
		  asm_vc_32   - with asm code designed for Microsoft Visual C++ (32 bits)
		  asm_gcc_32  - with asm code designed for GCC (32 bits)
		  asm_vc_64   - with asm for VC (64 bit)
		  asm_gcc_64  - with asm for GCC (64 bit)
		  no_asm_32   - pure C++ version (32 bit) - without any asm code
		  no_asm_64   - pure C++ version (64 bit) - without any asm code
	*/
	enum LibTypeCode
	{
	  asm_vc_32 = 0,
	  asm_gcc_32,
	  asm_vc_64,
	  asm_gcc_64,
	  no_asm_32,
	  no_asm_64
	};


	/*!
		error codes
	*/
	enum ErrorCode
	{
		err_ok = 0,
		err_nothing_has_read,
		err_unknown_character,
		err_unexpected_final_bracket,
		err_stack_not_clear,
		err_unknown_variable,
		err_division_by_zero,
		err_interrupt,
		err_overflow,
		err_unknown_function,
		err_unknown_operator,
		err_unexpected_semicolon_operator,
		err_improper_amount_of_arguments,
		err_improper_argument,
		err_unexpected_end,
		err_internal_error,
		err_incorrect_name,
		err_incorrect_value,
		err_variable_exists,
		err_variable_loop,
		err_functions_loop,
		err_must_be_only_one_value,
		err_object_exists,
		err_unknown_object,
		err_still_calculating,
		err_in_short_form_used_function,
		err_percent_from
	};


	/*!
		this struct is used when converting to/from a string
		/temporarily only in Big::ToString() and Big::FromString()/
	*/
	struct Conv
	{
		/*!
			base (radix) on which the value will be shown (or read)
			default: 10
		*/
		uint base;


		/*!
			used only in Big::ToString()
			if true the value will be always shown in the scientific mode, e.g: 123e+30
			default: false
		*/
		bool scient;


		/*!
			used only in Big::ToString()
			if scient is false then the value will be printed in the scientific mode
			only if the exponent is greater than scien_from
			default: 15
		*/
		sint scient_from;


		/*!
			if 'base_round' is true and 'base' is different from 2, 4, 8, or 16
			and the result value is not an integer then we make an additional rounding
			(after converting the last digit from the result is skipped)
			default: true

			e.g.
			Conv c;
			c.base_round = false;
			Big<1, 1> a = "0.1";                       // decimal input
			std::cout << a.ToString(c) << std::endl;   // the result is: 0.099999999
		*/
		bool base_round;


		/*!
			used only in Big::ToString()
			tells how many digits after comma are possible
			default: -1 which means all digits are printed

			set it to zero if you want integer value only

			for example when the value is: