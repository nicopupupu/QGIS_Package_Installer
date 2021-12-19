
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


	/*
		this namespace consists of auxiliary functions
		(something like 'private' in a class)
	*/
	namespace auxiliaryfunctions
	{

	/*!
		an auxiliary function for calculating the Sine
		(you don't have to call this function) 
	*/
	template<class ValueType>
	uint PrepareSin(ValueType & x, bool & change_sign)
	{
	ValueType temp;

		change_sign = false;
	
		if( x.IsSign() )
		{
			// we're using the formula 'sin(-x) = -sin(x)'
			change_sign = !change_sign;
			x.ChangeSign();
		}
	
		// we're reducing the period 2*PI
		// (for big values there'll always be zero)
		temp.Set2Pi();
		
		if( x.Mod(temp) )
			return 1;
		

		// we're setting 'x' as being in the range of <0, 0.5PI>

		temp.SetPi();

		if( x > temp )
		{
			// x is in (pi, 2*pi>
			x.Sub( temp );
			change_sign = !change_sign;
		}
		
		temp.Set05Pi();

		if( x > temp )
		{
			// x is in (0.5pi, pi>
			x.Sub( temp );
			x = temp - x;
		}

	return 0;
	}

	
	/*!
		an auxiliary function for calculating the Sine
		(you don't have to call this function) 

		it returns Sin(x) where 'x' is from <0, PI/2>
		we're calculating the Sin with using Taylor series in zero or PI/2
		(depending on which point of these two points is nearer to the 'x')

		Taylor series:
		sin(x) = sin(a) + cos(a)*(x-a)/(1!)
		         - sin(a)*((x-a)^2)/(2!) - cos(a)*((x-a)^3)/(3!)
				 + sin(a)*((x-a)^4)/(4!) + ...

		when a=0 it'll be:
		sin(x) = (x)/(1!) - (x^3)/(3!) + (x^5)/(5!) - (x^7)/(7!) + (x^9)/(9!) ...

		and when a=PI/2:
		sin(x) = 1 - ((x-PI/2)^2)/(2!) + ((x-PI/2)^4)/(4!) - ((x-PI/2)^6)/(6!) ...
	*/
	template<class ValueType>
	ValueType Sin0pi05(const ValueType & x)
	{
	ValueType result;
	ValueType numerator, denominator;
	ValueType d_numerator, d_denominator;
	ValueType one, temp, old_result;

		// temp = pi/4
		temp.Set05Pi();
		temp.exponent.SubOne();

		one.SetOne();

		if( x < temp ) 
		{	
			// we're using the Taylor series with a=0
			result    = x;
			numerator = x;
			denominator = one;

			// d_numerator = x^2
			d_numerator = x;
			d_numerator.Mul(x);

			d_denominator = 2;
		}
		else
		{
			// we're using the Taylor series with a=PI/2
			result = one;
			numerator = one;
			denominator = one;

			// d_numerator = (x-pi/2)^2
			ValueType pi05;
			pi05.Set05Pi();

			temp = x;
			temp.Sub( pi05 );
			d_numerator = temp;
			d_numerator.Mul( temp );

			d_denominator = one;
		}

		uint c = 0;
		bool addition = false;

		old_result = result;
		for(uint i=1 ; i<=TTMATH_ARITHMETIC_MAX_LOOP ; ++i)
		{
			// we're starting from a second part of the formula
			c += numerator.    Mul( d_numerator );
			c += denominator.  Mul( d_denominator );
			c += d_denominator.Add( one );
			c += denominator.  Mul( d_denominator );
			c += d_denominator.Add( one );
			temp = numerator;
			c += temp.Div(denominator);

			if( c )
				// Sin is from <-1,1> and cannot make an overflow
				// but the carry can be from the Taylor series
				// (then we only break our calculations)
				break;

			if( addition )
				result.Add( temp );
			else
				result.Sub( temp );


			addition = !addition;
	
			// we're testing whether the result has changed after adding
			// the next part of the Taylor formula, if not we end the loop
			// (it means 'x' is zero or 'x' is PI/2 or this part of the formula
			// is too small)
			if( result == old_result )
				break;

			old_result = result;
		}

	return result;
	}

	} // namespace auxiliaryfunctions



	/*!
		this function calculates the Sine
	*/
	template<class ValueType>
	ValueType Sin(ValueType x, ErrorCode * err = 0)
	{
	using namespace auxiliaryfunctions;

	ValueType one, result;
	bool change_sign;	
	
		if( x.IsNan() )
		{
			if( err )
				*err = err_improper_argument;

		return x;
		}

		if( err )
			*err = err_ok;

		if( PrepareSin( x, change_sign ) )
		{
			// x is too big, we cannnot reduce the 2*PI period
			// prior to version 0.8.5 the result was zero
			
			// result has NaN flag set by default

			if( err )
				*err = err_overflow; // maybe another error code? err_improper_argument?

		return result; // NaN is set by default
		}

		result = Sin0pi05( x );
	
		one.SetOne();

		// after calculations there can be small distortions in the result
		if( result > one )
			result = one;
		else
		if( result.IsSign() )
			// we've calculated the sin from <0, pi/2> and the result
			// should be positive
			result.SetZero();

		if( change_sign )
			result.ChangeSign();	
		
	return result;
	}

	
	/*!
		this function calulates the Cosine
		we're using the formula cos(x) = sin(x + PI/2)
	*/
	template<class ValueType>
	ValueType Cos(ValueType x, ErrorCode * err = 0)
	{
		if( x.IsNan() )
		{
			if( err )
				*err = err_improper_argument;

		return x; // NaN
		}

		ValueType pi05;
		pi05.Set05Pi();

		uint c = x.Add( pi05 );

		if( c )
		{
			if( err )
				*err = err_overflow;
	
		return ValueType(); // result is undefined (NaN is set by default)
		}

	return Sin(x, err);
	}
	

	/*!
		this function calulates the Tangent
		we're using the formula tan(x) = sin(x) / cos(x)

		it takes more time than calculating the Tan directly
		from for example Taylor series but should be a bit preciser
		because Tan receives its values from -infinity to +infinity
		and when we calculate it from any series then we can make
		a greater mistake than calculating 'sin/cos'
	*/
	template<class ValueType>
	ValueType Tan(const ValueType & x, ErrorCode * err = 0)
	{
		ValueType result = Cos(x, err);
		
		if( err && *err != err_ok )
			return result;

		if( result.IsZero() )
		{
			if( err )
				*err = err_improper_argument;

			result.SetNan();

		return result;
		}

	return Sin(x, err) / result;
	}


	/*!
		this function calulates the Tangent
		look at the description of Tan(...)

		(the abbreviation of Tangent can be 'tg' as well)
	*/
	template<class ValueType>
	ValueType Tg(const ValueType & x, ErrorCode * err = 0)
	{
		return Tan(x, err);
	}


	/*!
		this function calulates the Cotangent
		we're using the formula tan(x) = cos(x) / sin(x)

		(why do we make it in this way? 
		look at information in Tan() function)
	*/
	template<class ValueType>
	ValueType Cot(const ValueType & x, ErrorCode * err = 0)
	{
		ValueType result = Sin(x, err);

		if( err && *err != err_ok )
			return result;

		if( result.IsZero() )
		{
			if( err )
				*err = err_improper_argument;

			result.SetNan();

		return result;
		}
	
	return Cos(x, err) / result;
	}


	/*!