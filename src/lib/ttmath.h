
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



#ifndef headerfilettmathmathtt
#define headerfilettmathmathtt

/*!
	\file ttmath.h
    \brief Mathematics functions.
*/

#ifdef _MSC_VER
//warning C4127: conditional expression is constant
#pragma warning( disable: 4127 )
//warning C4702: unreachable code
#pragma warning( disable: 4702 )
//warning C4800: forcing value to bool 'true' or 'false' (performance warning)
#pragma warning( disable: 4800 )
#endif


#include "ttmathbig.h"
#include "ttmathobjects.h"


namespace ttmath
{
	/*
 	 *
	 *  functions defined here are used only with Big<> types
	 *
	 *
	 */


	/*
 	 *
	 *  functions for rounding
	 *
	 *
	 */


	/*!
		this function skips the fraction from x
		e.g  2.2  = 2
		     2.7  = 2
			 -2.2 = 2
			 -2.7 = 2
	*/
	template<class ValueType>
	ValueType SkipFraction(const ValueType & x)
	{
		ValueType result( x );
		result.SkipFraction();

	return result;
	}


	/*!
		this function rounds to the nearest integer value
		e.g  2.2  = 2
		     2.7  = 3
			 -2.2 = -2
			 -2.7 = -3
	*/
	template<class ValueType>
	ValueType Round(const ValueType & x, ErrorCode * err = 0)
	{
		if( x.IsNan() )
		{
			if( err )
				*err = err_improper_argument;

		return x; // NaN
		}

		ValueType result( x );
		uint c = result.Round();

		if( err )
			*err = c ? err_overflow : err_ok;

	return result;
	}



	/*!
		this function returns a value representing the smallest integer
		that is greater than or equal to x

		Ceil(-3.7) = -3
		Ceil(-3.1) = -3
		Ceil(-3.0) = -3
		Ceil(4.0)  = 4
		Ceil(4.2)  = 5
		Ceil(4.8)  = 5
	*/
	template<class ValueType>
	ValueType Ceil(const ValueType & x, ErrorCode * err = 0)
	{
		if( x.IsNan() )
		{
			if( err )
				*err = err_improper_argument;

		return x; // NaN
		}

		ValueType result(x);
		uint c = 0;

		result.SkipFraction();

		if( result != x )
		{
			// x is with fraction
			// if x is negative we don't have to do anything
			if( !x.IsSign() )
			{
				ValueType one;
				one.SetOne();

				c += result.Add(one);
			}
		}

		if( err )
			*err = c ? err_overflow : err_ok;

	return result;	
	}


	/*!
		this function returns a value representing the largest integer
		that is less than or equal to x

		Floor(-3.6) = -4
		Floor(-3.1) = -4
		Floor(-3)   = -3
		Floor(2)    = 2
		Floor(2.3)  = 2
		Floor(2.8)  = 2
	*/
	template<class ValueType>
	ValueType Floor(const ValueType & x, ErrorCode * err = 0)
	{
		if( x.IsNan())
		{
			if( err )
				*err = err_improper_argument;

		return x; // NaN
		}

		ValueType result(x);
		uint c = 0;

		result.SkipFraction();

		if( result != x )
		{
			// x is with fraction
			// if x is positive we don't have to do anything
			if( x.IsSign() )
			{
				ValueType one;
				one.SetOne();

				c += result.Sub(one);
			}
		}

		if( err )
			*err = c ? err_overflow : err_ok;

	return result;	
	}



	/*
 	 *
	 *  logarithms and the exponent
	 *
	 *
	 */

	
	/*!
		this function calculates the natural logarithm (logarithm with the base 'e')
	*/
	template<class ValueType>
	ValueType Ln(const ValueType & x, ErrorCode * err = 0)
	{
		if( x.IsNan() )
		{
			if( err )
				*err = err_improper_argument;

		return x; // NaN
		}

		ValueType result;
		uint state = result.Ln(x);

		if( err )
		{
			switch( state )
			{
			case 0:
				*err = err_ok;
				break;
			case 1:
				*err = err_overflow;
				break;
			case 2:
				*err = err_improper_argument;
				break;
			default:
				*err = err_internal_error;
				break;
			}
		}


	return result;
	}


	/*!
		this function calculates the logarithm
	*/
	template<class ValueType>
	ValueType Log(const ValueType & x, const ValueType & base, ErrorCode * err = 0)
	{
		if( x.IsNan() )
		{
			if( err ) *err = err_improper_argument;
			return x;
		}

		if( base.IsNan() )
		{
			if( err ) *err = err_improper_argument;
			return base;
		}

		ValueType result;
		uint state = result.Log(x, base);

		if( err )
		{
			switch( state )
			{
			case 0:
				*err = err_ok;
				break;
			case 1:
				*err = err_overflow;
				break;
			case 2:
			case 3:
				*err = err_improper_argument;
				break;
			default:
				*err = err_internal_error;
				break;
			}
		}

	return result;
	}


	/*!
		this function calculates the expression e^x
	*/
	template<class ValueType>
	ValueType Exp(const ValueType & x, ErrorCode * err = 0)
	{
		if( x.IsNan() )
		{
			if( err )
				*err = err_improper_argument;

		return x; // NaN
		}

		ValueType result;
		uint c = result.Exp(x);

		if( err )
			*err = c ? err_overflow : err_ok;

	return result;
	}


	/*!
	*
	*	trigonometric functions
	*
	*/
