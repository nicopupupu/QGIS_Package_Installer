
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



#ifndef headerfilettmathparser
#define headerfilettmathparser

/*!
	\file ttmathparser.h
    \brief A mathematical parser
*/

#include <cstdio>
#include <vector>
#include <map>
#include <set>

#include "ttmath.h"
#include "ttmathobjects.h"
#include "ttmathmisc.h"



namespace ttmath
{

/*! 
	\brief Mathematical parser

	let x will be an input string meaning an expression for converting:
	
	x = [+|-]Value[operator[+|-]Value][operator[+|-]Value]...
	where:
		an operator can be:
			^ (pow)   (the heighest priority)

			* (mul)   (or multiplication without an operator -- short mul)
			/ (div)   (* and / have the same priority)

			+ (add)
			- (sub)   (+ and - have the same priority)

			< (lower than)
			> (greater than)
			<= (lower or equal than)
			>= (greater or equal than)
			== (equal)
			!= (not equal)   (all above logical operators have the same priority)
			
			&& (logical and)

			|| (logical or) (the lowest priority)

		short mul:
 		 if the second Value (Var below) is either a variable or function there might not be 
		 an operator between them, e.g.
	        "[+|-]Value Var" is treated as "[+|-]Value * Var" and the multiplication
	        has the same priority as a normal multiplication:
			4x       = 4 * x
			2^3m     = (2^3)* m
			6h^3     = 6 * (h^3)
	        2sin(pi) = 2 * sin(pi)
			etc.

		Value can be:
			constant e.g. 100, can be preceded by operators for changing the base (radix): [#|&]
			                   # - hex
							   & - bin
							   sample: #10  = 16
							           &10  = 2
			variable e.g. pi
			another expression between brackets e.g (x)
			function e.g. sin(x)

	for example a correct input string can be:
		"1"
		"2.1234"
		"2,1234"    (they are the same, by default we can either use a comma or a dot)
		"1 + 2"
		"(1 + 2) * 3"
		"pi"
		"sin(pi)"
		"(1+2)*(2+3)"
		"log(2;1234)"    there's a semicolon here (not a comma), we use it in functions
		                 for separating parameters
	    "1 < 2"  (the result will be: 1)
	    "4 < 3"  (the result will be: 0)
		"2+x"    (of course if the variable 'x' is defined)
		"4x+10"
		"#20+10"     = 32 + 10 = 42
		"10 ^ -&101" = 10 ^ -5 = 0.00001
		"8 * -&10"   = 8 * -2  = -16
		etc.

	we can also use a semicolon for separating any 'x' input strings
	for example:
		"1+2;4+5"
	the result will be on the stack as follows:
		stack[0].value=3
		stack[1].value=9
*/
template<class ValueType>
class Parser
{
private:

/*!
	there are 5 mathematical operators as follows (with their standard priorities):
		add (+)
		sub (-)
		mul (*)
		div (/)
		pow (^)
		and 'shortmul' used when there is no any operators between
		a first parameter and a variable or function
		(the 'shortmul' has the same priority as the normal multiplication )
*/
	class MatOperator
	{
	public:

		enum Type
		{
			none,add,sub,mul,div,pow,lt,gt,let,get,eq,neq,lor,land,shortmul
		};

		enum Assoc
		{
			right,		// right-associative
			non_right	// associative or left-associative
		};

		Type  GetType()     const { return type; }
		int   GetPriority() const { return priority; }
		Assoc GetAssoc()    const { return assoc; }

		void SetType(Type t)
		{
			type  = t;
			assoc = non_right;

			switch( type )
			{		
			case lor:
				priority = 4;
				break;

			case land:
				priority = 5;
				break;

			case eq:
			case neq:
			case lt:
			case gt:
			case let:
			case get:
				priority = 7;
				break;

			case add:
			case sub:
				priority = 10;
				break;

			case mul:
			case shortmul:
			case div:
				priority = 12;
				break;

			case pow:
				priority = 14;
				assoc    = right;
				break;

			default:
				Error( err_internal_error );
				break;
			}
		}

		MatOperator(): type(none), priority(0), assoc(non_right)
		{
		}

	private:

		Type  type;
		int   priority;
		Assoc assoc;
	}; // end of MatOperator class



public:



	/*!
		Objects of type 'Item' we are keeping on our stack
	*/
	struct Item
	{
		enum Type
		{
			none, numerical_value, mat_operator, first_bracket,
			last_bracket, variable, semicolon
		};

		// The kind of type which we're keeping
		Type type;

		// if type == numerical_value
		ValueType value;

		// if type == mat_operator
		MatOperator moperator;

		/*
			if type == first_bracket

			if 'function' is set to true it means that the first recognized bracket
			was the bracket from function in other words we must call a function when
			we'll find the 'last' bracket
		*/
		bool function;

		// if function is true
		std::string function_name;

		/*
			the sign of value

			it can be for type==numerical_value or type==first_bracket
			when it's true it means e.g. that value is equal -value
		*/
		bool sign;

		Item(): type(none), function(false), sign(false)
		{
		}

	}; // end of Item struct


/*!
	stack on which we're keeping the Items

	at the end of parsing we'll have the result here
	the result don't have to be one value, it can be
	more than one if we have used a semicolon in the global space
	e.g. such input string "1+2;3+4" will generate a result:
	 stack[0].value=3
	 stack[1].value=7

	you should check if the stack is not empty, because if there was
	a syntax error in the input string then we do not have any results
	on the stack 
*/
std::vector<Item> stack;


private:


/*!
	size of the stack when we're starting parsing of the string

	if it's to small while parsing the stack will be automatically resized
*/
const int default_stack_size;



/*!
	index of an object in our stack
	it's pointing on the place behind the last element
	for example at the beginning of parsing its value is zero
*/
unsigned int stack_index;


/*!
	code of the last error
*/
ErrorCode error;


/*!
	pointer to the currently reading char
	when an error has occured it may be used to count the index of the wrong character
*/
const char * pstring;


/*!
	the base (radix) of the mathematic system (for example it may be '10')
*/
int base;


/*!
	the unit of angles used in: sin,cos,tan,cot,asin,acos,atan,acot
	0 - deg
	1 - rad (default)
	2 - grad
*/
int deg_rad_grad;



/*!
	a pointer to an object which tell us whether we should stop calculating or not
*/
const volatile StopCalculating * pstop_calculating;



/*!
	a pointer to the user-defined variables' table
*/
const Objects * puser_variables;

/*!
	a pointer to the user-defined functions' table
*/
const Objects * puser_functions;


typedef std::map<std::string, ValueType> FunctionLocalVariables;

/*!
	a pointer to the local variables of a function
*/
const FunctionLocalVariables * pfunction_local_variables;


/*!
	a temporary set using during parsing user defined variables
*/
std::set<std::string> visited_variables;


/*!
	a temporary set using during parsing user defined functions
*/
std::set<std::string> visited_functions;




/*!
	pfunction is the type of pointer to a mathematic function

	these mathematic functions are private members of this class,
	they are the wrappers for standard mathematics function

	'pstack' is the pointer to the first argument on our stack
	'amount_of_arg' tell us how many argument there are in our stack
	'result' is the reference for result of function 
*/
typedef void (Parser<ValueType>::*pfunction)(int pstack, int amount_of_arg, ValueType & result);


/*!
	pfunction is the type of pointer to a method which returns value of variable
*/
typedef void (ValueType::*pfunction_var)();


/*!
	table of mathematic functions

	this map consists of:
		std::string - function's name
		pfunction - pointer to specific function
*/
typedef std::map<std::string, pfunction> FunctionsTable;
FunctionsTable functions_table;


/*!
	table of mathematic operators

	this map consists of:
		std::string - operators's name
		MatOperator::Type - type of the operator
*/
typedef std::map<std::string, typename MatOperator::Type> OperatorsTable;
OperatorsTable operators_table;


/*!
	table of mathematic variables

	this map consists of:
		std::string     - variable's name
		pfunction_var - pointer to specific function which returns value of variable
*/
typedef std::map<std::string, pfunction_var> VariablesTable;
VariablesTable variables_table;


/*!
	some coefficients used when calculating the gamma (or factorial) function
*/
CGamma<ValueType> cgamma;


/*!
	temporary object for a whole string when Parse(std::wstring) is used
*/
std::string wide_to_ansi;


/*!
	group character (used when parsing)
	default zero (not used)
*/
int group;


/*!
	characters used as a comma
	default: '.' and ','
	comma2 can be zero (it means it is not used)
*/
int comma, comma2;


/*!
	an additional character used as a separator between function parameters
	(semicolon is used always)
*/
int param_sep;


/*!
	true if something was calculated (at least one mathematical operator was used or a function or a variable)
*/
bool calculated;



/*!
	we're using this method for reporting an error
*/
static void Error(ErrorCode code)
{
	throw code;
}


/*!
	this method skips the white character from the string

	it's moving the 'pstring' to the first no-white character
*/
void SkipWhiteCharacters()
{
	while( (*pstring==' ' ) || (*pstring=='\t') )
		++pstring;
}


/*!
	an auxiliary method for RecurrenceParsingVariablesOrFunction(...)
*/
void RecurrenceParsingVariablesOrFunction_CheckStopCondition(bool variable, const std::string & name)
{
	if( variable )
	{
		if( visited_variables.find(name) != visited_variables.end() )
			Error( err_variable_loop );
	}
	else
	{
		if( visited_functions.find(name) != visited_functions.end() )
			Error( err_functions_loop );
	}
}


/*!
	an auxiliary method for RecurrenceParsingVariablesOrFunction(...)
*/
void RecurrenceParsingVariablesOrFunction_AddName(bool variable, const std::string & name)
{
	if( variable )
		visited_variables.insert( name );
	else
		visited_functions.insert( name );
}


/*!
	an auxiliary method for RecurrenceParsingVariablesOrFunction(...)
*/
void RecurrenceParsingVariablesOrFunction_DeleteName(bool variable, const std::string & name)
{
	if( variable )
		visited_variables.erase( name );
	else
		visited_functions.erase( name );
}


/*!
	this method returns the value of a variable or function
	by creating a new instance of the mathematical parser 
	and making the standard parsing algorithm on the given string

	this method is used only during parsing user defined variables or functions

	(there can be a recurrence here therefore we're using 'visited_variables'
	and 'visited_functions' sets to make a stop condition)
*/
ValueType RecurrenceParsingVariablesOrFunction(bool variable, const std::string & name, const char * new_string,
											   FunctionLocalVariables * local_variables = 0)
{
	RecurrenceParsingVariablesOrFunction_CheckStopCondition(variable, name);
	RecurrenceParsingVariablesOrFunction_AddName(variable, name);

	Parser<ValueType> NewParser(*this);
	ErrorCode err;

	NewParser.pfunction_local_variables = local_variables;

	try
	{
		err = NewParser.Parse(new_string);
	}
	catch(...)
	{
		RecurrenceParsingVariablesOrFunction_DeleteName(variable, name);

	throw;
	}

	RecurrenceParsingVariablesOrFunction_DeleteName(variable, name);

	if( err != err_ok )
		Error( err );

	if( NewParser.stack.size() != 1 )
		Error( err_must_be_only_one_value );

	if( NewParser.stack[0].type != Item::numerical_value )
		// I think there shouldn't be this error here
		Error( err_incorrect_value );

return NewParser.stack[0].value;
}


public:


/*!
	this method returns the user-defined value of a variable
*/
bool GetValueOfUserDefinedVariable(const std::string & variable_name,ValueType & result)
{
	if( !puser_variables )
		return false;

	const char * string_value;

	if( puser_variables->GetValue(variable_name, &string_value) != err_ok )
		return false;

	result = RecurrenceParsingVariablesOrFunction(true, variable_name, string_value);
	calculated = true;

return true;
}


/*!
	this method returns the value of a local variable of a function
*/
bool GetValueOfFunctionLocalVariable(const std::string & variable_name, ValueType & result)
{
	if( !pfunction_local_variables )
		return false;

	typename FunctionLocalVariables::const_iterator i = pfunction_local_variables->find(variable_name);

	if( i == pfunction_local_variables->end() )
		return false;

	result = i->second;

return true;
}


/*!
	this method returns the value of a variable from variables' table

	we make an object of type ValueType then call a method which 
	sets the correct value in it and finally we'll return the object
*/
ValueType GetValueOfVariable(const std::string & variable_name)
{
ValueType result;

	if( GetValueOfFunctionLocalVariable(variable_name, result) )
		return result;

	if( GetValueOfUserDefinedVariable(variable_name, result) )
		return result;


	typename std::map<std::string, pfunction_var>::iterator i =
													variables_table.find(variable_name);

	if( i == variables_table.end() )
		Error( err_unknown_variable );

	(result.*(i->second))();
	calculated = true;

return result;
}


private:

/*!
	wrappers for mathematic functions

	'sindex' is pointing on the first argument on our stack 
			 (the second argument has 'sindex+2'
			 because 'sindex+1' is guaranted for the 'semicolon' operator)
			 the third artument has of course 'sindex+4' etc.

	'result' will be the result of the function

	(we're using exceptions here for example when function gets an improper argument)
*/


/*!
	used by: sin,cos,tan,cot
*/
ValueType ConvertAngleToRad(const ValueType & input)
{
	if( deg_rad_grad == 1 ) // rad
		return input;

	ValueType result;
	ErrorCode err;

	if( deg_rad_grad == 0 ) // deg
		result = ttmath::DegToRad(input, &err);
	else // grad
		result = ttmath::GradToRad(input, &err);

	if( err != err_ok )
		Error( err );

return result;
}


/*!
	used by: asin,acos,atan,acot
*/
ValueType ConvertRadToAngle(const ValueType & input)
{
	if( deg_rad_grad == 1 ) // rad
		return input;

	ValueType result;
	ErrorCode err;

	if( deg_rad_grad == 0 ) // deg
		result = ttmath::RadToDeg(input, &err);
	else // grad
		result = ttmath::RadToGrad(input, &err);

	if( err != err_ok )
		Error( err );

return result;
}


void Gamma(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	
	result = ttmath::Gamma(stack[sindex].value, cgamma, &err, pstop_calculating);

	if(err != err_ok)
		Error( err );
}


/*!
	factorial
	result = 1 * 2 * 3 * 4 * .... * x
*/
void Factorial(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;

	result = ttmath::Factorial(stack[sindex].value, cgamma, &err, pstop_calculating);

	if(err != err_ok)
		Error( err );
}


void Abs(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	result = ttmath::Abs(stack[sindex].value);
}

void Sin(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Sin( ConvertAngleToRad(stack[sindex].value), &err );

	if(err != err_ok)
		Error( err );
}

void Cos(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Cos( ConvertAngleToRad(stack[sindex].value), &err );

	if(err != err_ok)
		Error( err );
}

void Tan(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Tan(ConvertAngleToRad(stack[sindex].value), &err);

	if(err != err_ok)
		Error( err );
}

void Cot(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Cot(ConvertAngleToRad(stack[sindex].value), &err);

	if(err != err_ok)
		Error( err );
}

void Int(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	result = ttmath::SkipFraction(stack[sindex].value);
}


void Round(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	result = stack[sindex].value;

	if( result.Round() )
		Error( err_overflow );
}


void Ln(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Ln(stack[sindex].value, &err);

	if(err != err_ok)
		Error( err );
}

void Log(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 2 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Log(stack[sindex].value, stack[sindex+2].value, &err);

	if(err != err_ok)
		Error( err );
}

void Exp(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Exp(stack[sindex].value, &err);

	if(err != err_ok)
		Error( err );
}


void Max(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args == 0 )
	{
		result.SetMax();

	return;
	}

	result = stack[sindex].value;

	for(int i=1 ; i<amount_of_args ; ++i)
	{
		if( result < stack[sindex + i*2].value )
			result = stack[sindex + i*2].value;
	}
}


void Min(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args == 0 )
	{
		result.SetMin();

	return;
	}

	result = stack[sindex].value;

	for(int i=1 ; i<amount_of_args ; ++i)
	{
		if( result > stack[sindex + i*2].value )
			result = stack[sindex + i*2].value;
	}
}


void ASin(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	ValueType temp = ttmath::ASin(stack[sindex].value, &err);

	if(err != err_ok)
		Error( err );

	result = ConvertRadToAngle(temp);
}


void ACos(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	ValueType temp = ttmath::ACos(stack[sindex].value, &err);

	if(err != err_ok)
		Error( err );

	result = ConvertRadToAngle(temp);
}


void ATan(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	result = ConvertRadToAngle(ttmath::ATan(stack[sindex].value));
}


void ACot(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	result = ConvertRadToAngle(ttmath::ACot(stack[sindex].value));
}


void Sgn(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	result = ttmath::Sgn(stack[sindex].value);
}


void Mod(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 2 )
		Error( err_improper_amount_of_arguments );

	if( stack[sindex+2].value.IsZero() )
		Error( err_improper_argument );

	result = stack[sindex].value;
	uint c = result.Mod(stack[sindex+2].value);

	if( c )
		Error( err_overflow );
}


void If(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 3 )
		Error( err_improper_amount_of_arguments );


	if( !stack[sindex].value.IsZero() )
		result = stack[sindex+2].value;
	else
		result = stack[sindex+4].value;
}


void Or(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args < 2 )
		Error( err_improper_amount_of_arguments );

	for(int i=0 ; i<amount_of_args ; ++i)
	{
		if( !stack[sindex+i*2].value.IsZero() )
		{
			result.SetOne();
			return;
		}
	}

	result.SetZero();
}


void And(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args < 2 )
		Error( err_improper_amount_of_arguments );

	for(int i=0 ; i<amount_of_args ; ++i)
	{
		if( stack[sindex+i*2].value.IsZero() )
		{
			result.SetZero();
			return;
		}
	}

	result.SetOne();
}


void Not(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );


	if( stack[sindex].value.IsZero() )
		result.SetOne();
	else
		result.SetZero();
}


void DegToRad(int sindex, int amount_of_args, ValueType & result)
{
	ErrorCode err = err_ok;

	if( amount_of_args == 1 )
	{
		result = ttmath::DegToRad(stack[sindex].value, &err);
	}
	else
	if( amount_of_args == 3 )
	{
		result = ttmath::DegToRad(	stack[sindex].value, stack[sindex+2].value,
									stack[sindex+4].value, &err);
	}
	else
		Error( err_improper_amount_of_arguments );


	if( err != err_ok )
		Error( err );
}


void RadToDeg(int sindex, int amount_of_args, ValueType & result)
{
	ErrorCode err;

	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );
	
	result = ttmath::RadToDeg(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void DegToDeg(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 3 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::DegToDeg(	stack[sindex].value, stack[sindex+2].value,
								stack[sindex+4].value, &err);

	if( err != err_ok )
		Error( err );
}


void GradToRad(int sindex, int amount_of_args, ValueType & result)
{
	ErrorCode err;

	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );
	
	result = ttmath::GradToRad(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void RadToGrad(int sindex, int amount_of_args, ValueType & result)
{
	ErrorCode err;

	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );
	
	result = ttmath::RadToGrad(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void DegToGrad(int sindex, int amount_of_args, ValueType & result)
{
	ErrorCode err = err_ok;

	if( amount_of_args == 1 )
	{
		result = ttmath::DegToGrad(stack[sindex].value, &err);
	}
	else
	if( amount_of_args == 3 )
	{
		result = ttmath::DegToGrad(	stack[sindex].value, stack[sindex+2].value,
									stack[sindex+4].value, &err);
	}
	else
		Error( err_improper_amount_of_arguments );


	if( err != err_ok )
		Error( err );
}


void GradToDeg(int sindex, int amount_of_args, ValueType & result)
{
	ErrorCode err;

	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );
	
	result = ttmath::GradToDeg(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void Ceil(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Ceil(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void Floor(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Floor(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}

void Sqrt(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Sqrt(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void Sinh(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Sinh(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void Cosh(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Cosh(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void Tanh(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Tanh(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void Coth(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Coth(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void Root(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 2 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::Root(stack[sindex].value, stack[sindex+2].value, &err);

	if( err != err_ok )
		Error( err );
}



void ASinh(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::ASinh(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void ACosh(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::ACosh(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void ATanh(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::ATanh(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void ACoth(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	ErrorCode err;
	result = ttmath::ACoth(stack[sindex].value, &err);

	if( err != err_ok )
		Error( err );
}


void BitAnd(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 2 )
		Error( err_improper_amount_of_arguments );

	uint err;
	result = stack[sindex].value;
	err = result.BitAnd(stack[sindex+2].value);

	switch(err)
	{
	case 1:
		Error( err_overflow );
		break;
	case 2:
		Error( err_improper_argument );
		break;
	}
}

void BitOr(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 2 )
		Error( err_improper_amount_of_arguments );

	uint err;
	result = stack[sindex].value;
	err = result.BitOr(stack[sindex+2].value);

	switch(err)
	{
	case 1:
		Error( err_overflow );
		break;
	case 2:
		Error( err_improper_argument );
		break;
	}
}


void BitXor(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 2 )
		Error( err_improper_amount_of_arguments );

	uint err;
	result = stack[sindex].value;
	err = result.BitXor(stack[sindex+2].value);

	switch(err)
	{
	case 1:
		Error( err_overflow );
		break;
	case 2:
		Error( err_improper_argument );
		break;
	}
}


void Sum(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args == 0 )
		Error( err_improper_amount_of_arguments );

	result = stack[sindex].value;

	for(int i=1 ; i<amount_of_args ; ++i )
		if( result.Add( stack[ sindex + i*2 ].value ) )
			Error( err_overflow );
}	

void Avg(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args == 0 )
		Error( err_improper_amount_of_arguments );

	result = stack[sindex].value;

	for(int i=1 ; i<amount_of_args ; ++i )
		if( result.Add( stack[ sindex + i*2 ].value ) )
			Error( err_overflow );

	if( result.Div( amount_of_args ) )
		Error( err_overflow );
}	


void Frac(int sindex, int amount_of_args, ValueType & result)
{
	if( amount_of_args != 1 )
		Error( err_improper_amount_of_arguments );

	result = stack[sindex].value;
	result.RemainFraction();
}




/*!
	we use such a method because 'wvsprintf' is not everywhere defined
*/
void Sprintf(char * buffer, int par)
{
char buf[30]; // char, not wchar_t
int i;

	#ifdef _MSC_VER
	#pragma warning( disable: 4996 )
	//warning C4996: 'sprintf': This function or variable may be unsafe.
	#endif

	sprintf(buf, "%d", par);
	for(i=0 ; buf[i] != 0 ; ++i)
		buffer[i] = buf[i];

	buffer[i] = 0;

	#ifdef _MSC_VER
	#pragma warning( default: 4996 )
	#endif
}




/*!
	this method returns the value from a user-defined function

	(look at the description in 'CallFunction(...)')
*/
bool GetValueOfUserDefinedFunction(const std::string & function_name, int amount_of_args, int sindex)
{
	if( !puser_functions )
		return false;

	const char * string_value;
	int param;

	if( puser_functions->GetValueAndParam(function_name, &string_value, &param) != err_ok )
		return false;

	if( param != amount_of_args )
		Error( err_improper_amount_of_arguments );


	FunctionLocalVariables local_variables;

	if( amount_of_args > 0 )
	{
		char buffer[30];

		// x = x1
		buffer[0] = 'x';
		buffer[1] = 0;
		local_variables.insert( std::make_pair(buffer, stack[sindex].value) );

		for(int i=0 ; i<amount_of_args ; ++i)
		{
			buffer[0] = 'x';
			Sprintf(buffer+1, i+1);
			local_variables.insert( std::make_pair(buffer, stack[sindex + i*2].value) );
		}
	}

	stack[sindex-1].value = RecurrenceParsingVariablesOrFunction(false, function_name, string_value, &local_variables);
	calculated = true;

return true;
}


/*
	we're calling a specific function

	function_name  - name of the function
	amount_of_args - how many arguments there are on our stack
					 (function must check whether this is a correct value or not)
	sindex         - index of the first argument on the stack (sindex is greater than zero)
  					 if there aren't any arguments on the stack 'sindex' pointing on
					 a non existend element (after the first bracket)

	result will be stored in 'stack[sindex-1].value'
	(we don't have to set the correct type of this element, it'll be set later)
*/
void CallFunction(const std::string & function_name, int amount_of_args, int sindex)
{
	if( GetValueOfUserDefinedFunction(function_name, amount_of_args, sindex) )
		return;

	typename FunctionsTable::iterator i = functions_table.find( function_name );

	if( i == functions_table.end() )
		Error( err_unknown_function );

	/*
		calling the specify function
	*/
	(this->*(i->second))(sindex, amount_of_args, stack[sindex-1].value);
	calculated = true;
}





/*!
	inserting a function to the functions' table

	function_name - name of the function
	pf - pointer to the function (to the wrapper)
*/
void InsertFunctionToTable(const char * function_name, pfunction pf)
{
	std::string str;
	Misc::AssignString(str, function_name);

	functions_table.insert( std::make_pair(str, pf) );
}



/*!
	inserting a function to the variables' table
	(this function returns value of variable)

	variable_name - name of the function
	pf - pointer to the function
*/
void InsertVariableToTable(const char * variable_name, pfunction_var pf)
{
	std::string str;
	Misc::AssignString(str, variable_name);

	variables_table.insert( std::make_pair(str, pf) );
}


/*!
	this method creates the table of functions
*/
void CreateFunctionsTable()
{
	InsertFunctionToTable("gamma",		&Parser<ValueType>::Gamma);
	InsertFunctionToTable("factorial",	&Parser<ValueType>::Factorial);
	InsertFunctionToTable("abs",   		&Parser<ValueType>::Abs);
	InsertFunctionToTable("sin",   		&Parser<ValueType>::Sin);
	InsertFunctionToTable("cos",   		&Parser<ValueType>::Cos);
	InsertFunctionToTable("tan",   		&Parser<ValueType>::Tan);
	InsertFunctionToTable("tg",			&Parser<ValueType>::Tan);
	InsertFunctionToTable("cot",  		&Parser<ValueType>::Cot);
	InsertFunctionToTable("ctg",  		&Parser<ValueType>::Cot);
	InsertFunctionToTable("int",	   	&Parser<ValueType>::Int);
	InsertFunctionToTable("round",	 	&Parser<ValueType>::Round);
	InsertFunctionToTable("ln",			&Parser<ValueType>::Ln);
	InsertFunctionToTable("log",	   	&Parser<ValueType>::Log);
	InsertFunctionToTable("exp",	   	&Parser<ValueType>::Exp);
	InsertFunctionToTable("max",	   	&Parser<ValueType>::Max);
	InsertFunctionToTable("min",	   	&Parser<ValueType>::Min);
	InsertFunctionToTable("asin",   	&Parser<ValueType>::ASin);
	InsertFunctionToTable("acos",   	&Parser<ValueType>::ACos);
	InsertFunctionToTable("atan",   	&Parser<ValueType>::ATan);
	InsertFunctionToTable("atg",	   	&Parser<ValueType>::ATan);
	InsertFunctionToTable("acot",   	&Parser<ValueType>::ACot);
	InsertFunctionToTable("actg",   	&Parser<ValueType>::ACot);
	InsertFunctionToTable("sgn",   		&Parser<ValueType>::Sgn);
	InsertFunctionToTable("mod",   		&Parser<ValueType>::Mod);
	InsertFunctionToTable("if",   		&Parser<ValueType>::If);
	InsertFunctionToTable("or",   		&Parser<ValueType>::Or);
	InsertFunctionToTable("and",  		&Parser<ValueType>::And);
	InsertFunctionToTable("not",  		&Parser<ValueType>::Not);
	InsertFunctionToTable("degtorad",	&Parser<ValueType>::DegToRad);
	InsertFunctionToTable("radtodeg",	&Parser<ValueType>::RadToDeg);
	InsertFunctionToTable("degtodeg",	&Parser<ValueType>::DegToDeg);
	InsertFunctionToTable("gradtorad",	&Parser<ValueType>::GradToRad);
	InsertFunctionToTable("radtograd",	&Parser<ValueType>::RadToGrad);
	InsertFunctionToTable("degtograd",	&Parser<ValueType>::DegToGrad);
	InsertFunctionToTable("gradtodeg",	&Parser<ValueType>::GradToDeg);
	InsertFunctionToTable("ceil",		&Parser<ValueType>::Ceil);
	InsertFunctionToTable("floor",		&Parser<ValueType>::Floor);
	InsertFunctionToTable("sqrt",		&Parser<ValueType>::Sqrt);
	InsertFunctionToTable("sinh",		&Parser<ValueType>::Sinh);
	InsertFunctionToTable("cosh",		&Parser<ValueType>::Cosh);
	InsertFunctionToTable("tanh",		&Parser<ValueType>::Tanh);
	InsertFunctionToTable("tgh",		&Parser<ValueType>::Tanh);
	InsertFunctionToTable("coth",		&Parser<ValueType>::Coth);
	InsertFunctionToTable("ctgh",		&Parser<ValueType>::Coth);
	InsertFunctionToTable("root",		&Parser<ValueType>::Root);
	InsertFunctionToTable("asinh",		&Parser<ValueType>::ASinh);
	InsertFunctionToTable("acosh",		&Parser<ValueType>::ACosh);
	InsertFunctionToTable("atanh",		&Parser<ValueType>::ATanh);
	InsertFunctionToTable("atgh",		&Parser<ValueType>::ATanh);
	InsertFunctionToTable("acoth",		&Parser<ValueType>::ACoth);
	InsertFunctionToTable("actgh",		&Parser<ValueType>::ACoth);
	InsertFunctionToTable("bitand",		&Parser<ValueType>::BitAnd);
	InsertFunctionToTable("bitor",		&Parser<ValueType>::BitOr);
	InsertFunctionToTable("bitxor",		&Parser<ValueType>::BitXor);
	InsertFunctionToTable("band",		&Parser<ValueType>::BitAnd);
	InsertFunctionToTable("bor",		&Parser<ValueType>::BitOr);
	InsertFunctionToTable("bxor",		&Parser<ValueType>::BitXor);
	InsertFunctionToTable("sum",		&Parser<ValueType>::Sum);
	InsertFunctionToTable("avg",		&Parser<ValueType>::Avg);
	InsertFunctionToTable("frac",		&Parser<ValueType>::Frac);
}


/*!
	this method creates the table of variables
*/
void CreateVariablesTable()
{
	InsertVariableToTable("pi", &ValueType::SetPi);
	InsertVariableToTable("e",  &ValueType::SetE);
}


/*!
	converting from a big letter to a small one
*/
int ToLowerCase(int c)
{
	if( c>='A' && c<='Z' )
		return c - 'A' + 'a';

return c;
}


/*!
	this method read the name of a variable or a function
	
		'result' will be the name of a variable or a function
		function return 'false' if this name is the name of a variable
		or function return 'true' if this name is the name of a function

	what should be returned is tested just by a '(' character that means if there's
	a '(' character after a name that function returns 'true'
*/
bool ReadName(std::string & result)
{
int character;


	result.erase();
	character = *pstring;

	/*
		the first letter must be from range 'a' - 'z' or 'A' - 'Z'
	*/
	if( ! (( character>='a' && character<='z' ) || ( character>='A' && character<='Z' )) )
		Error( err_unknown_character );


	do
	{
		result   += static_cast<char>( character );
		character = * ++pstring;
	}
	while(	(character>='a' && character<='z') ||
			(character>='A' && character<='Z') ||
			(character>='0' && character<='9') ||
			character=='_' );
	

	SkipWhiteCharacters();
	

	/*
		if there's a character '(' that means this name is a name of a function
	*/
	if( *pstring == '(' )
	{
		++pstring;
		return true;
	}
	
	
return false;
}


/*!
	we're checking whether the first character is '-' or '+'
	if it is we'll return 'true' and if it is equally '-' we'll set the 'sign' member of 'result'
*/
bool TestSign(Item & result)
{
	SkipWhiteCharacters();
	result.sign = false;

	if( *pstring == '-' || *pstring == '+' )
	{
		if( *pstring == '-' )
			result.sign = true;

		++pstring;

	return true;
	}

return false;
}


/*!
	we're reading the name of a variable or a function
	if is there a function we'll return 'true'
*/
bool ReadVariableOrFunction(Item & result)
{
std::string name;
bool is_it_name_of_function = ReadName(name);

	if( is_it_name_of_function )
	{
		/*
			we've read the name of a function
		*/
		result.function_name = name;
		result.type     = Item::first_bracket;
		result.function = true;
	}
	else
	{
		/*
			we've read the name of a variable and we're getting its value now
		*/
		result.value = GetValueOfVariable( name );
	}

return is_it_name_of_function;
}




/*!
	we're reading a numerical value directly from the string
*/
void ReadValue(Item & result, int reading_base)
{
const char * new_stack_pointer;
bool value_read;
Conv conv;

	conv.base   = reading_base;
	conv.comma  = comma;
	conv.comma2 = comma2;
	conv.group  = group;

	uint carry = result.value.FromString(pstring, conv, &new_stack_pointer, &value_read);
	pstring    = new_stack_pointer;

	if( carry )
		Error( err_overflow );

	if( !value_read )
		Error( err_unknown_character );
}


/*!
	this method returns true if 'character' is a proper first digit for the value (or a comma -- can be first too)
*/
bool ValueStarts(int character, int base)
{
	if( character == comma )
		return true;

	if( comma2!=0 && character==comma2 )
		return true;

	if( Misc::CharToDigit(character, base) != -1 )
		return true;

return false;
}


/*!
	we're reading the item
  
	return values:
		0 - all ok, the item is successfully read
		1 - the end of the string (the item is not read)
		2 - the final bracket ')'
*/
int ReadValueVariableOrFunction(Item & result)
{
bool it_was_sign = false;
int  character;


	if( TestSign(result) )
		// 'result.sign' was set as well
		it_was_sign = true;

	SkipWhiteCharacters();
	character = ToLowerCase( *pstring );


	if( character == 0 )
	{
		if( it_was_sign )
			// at the end of the string a character like '-' or '+' has left
			Error( err_unexpected_end );

		// there's the end of the string here
		return 1;
	}
	else
	if( character == '(' )
	{
		// we've got a normal bracket (not a function)
		result.type = Item::first_bracket;
		result.function = false;
		++pstring;

	return 0;
	}
	else
	if( character == ')' )
	{
		// we've got a final bracket
		// (in this place we can find a final bracket only when there are empty brackets
		// without any values inside or with a sign '-' or '+' inside)

		if( it_was_sign )
			Error( err_unexpected_final_bracket );

		result.type = Item::last_bracket;

		// we don't increment 'pstring', this final bracket will be read next by the 
		// 'ReadOperatorAndCheckFinalBracket(...)' method

	return 2;
	}
	else
	if( character == '#' )
	{
		++pstring;
		SkipWhiteCharacters();

		// after '#' character we do not allow '-' or '+' (can be white characters)
		if(	ValueStarts(*pstring, 16) )
			ReadValue( result, 16 );
		else
			Error( err_unknown_character );
	}
	else
	if( character == '&' )
	{
		++pstring;
		SkipWhiteCharacters();

		// after '&' character we do not allow '-' or '+' (can be white characters)
		if(	ValueStarts(*pstring, 2) )
			ReadValue( result, 2 );
		else
			Error( err_unknown_character );
	}
	else
	if(	ValueStarts(character, base) )
	{
		ReadValue( result, base );
	}
	else
	if( character>='a' && character<='z' )
	{
		if( ReadVariableOrFunction(result) )
			// we've read the name of a function
			return 0;
	}
	else
		Error( err_unknown_character );



	/*
		we've got a value in the 'result'
		this value is from a variable or directly from the string
	*/
	result.type = Item::numerical_value;
	
	if( result.sign )
	{
		result.value.ChangeSign();
		result.sign = false;
	}
	

return 0;
}


void InsertOperatorToTable(const char * name, typename MatOperator::Type type)
{
	operators_table.insert( std::make_pair(std::string(name), type) );
}


/*!
	this method creates the table of operators
*/
void CreateMathematicalOperatorsTable()
{
	InsertOperatorToTable("||", MatOperator::lor);
	InsertOperatorToTable("&&", MatOperator::land);
	InsertOperatorToTable("!=", MatOperator::neq);
	InsertOperatorToTable("==", MatOperator::eq);
	InsertOperatorToTable(">=", MatOperator::get);
	InsertOperatorToTable("<=", MatOperator::let);
	InsertOperatorToTable(">",  MatOperator::gt);
	InsertOperatorToTable("<",  MatOperator::lt);
	InsertOperatorToTable("-",  MatOperator::sub);
	InsertOperatorToTable("+",  MatOperator::add);
	InsertOperatorToTable("/",  MatOperator::div);
	InsertOperatorToTable("*",  MatOperator::mul);
	InsertOperatorToTable("^",  MatOperator::pow);
}


/*!
	returns true if 'str2' is the substring of str1

	e.g.
	true when str1="test" and str2="te"
*/
bool IsSubstring(const std::string & str1, const std::string & str2)
{
	if( str2.length() > str1.length() )
		return false;

	for(typename std::string::size_type i=0 ; i<str2.length() ; ++i)
		if( str1[i] != str2[i] )
			return false;

return true;
}


/*!
	this method reads a mathematical (or logical) operator
*/
void ReadMathematicalOperator(Item & result)
{
std::string oper;
typename OperatorsTable::iterator iter_old, iter_new;

	iter_old = operators_table.end();

	for( ; true ; ++pstring )
	{
		oper += *pstring;
		iter_new = operators_table.lower_bound(oper);
		
		if( iter_new == operators_table.end() || !IsSubstring(iter_new->first, oper) )
		{
			oper.erase( --oper.end() ); // we've got mininum one element

			if( iter_old != operators_table.end() && iter_old->first == oper )
			{
				result.type = Item::mat_operator;
				result.moperator.SetType( iter_old->second );
				break;
			}
			
			Error( err_unknown_operator );
		}
	
		iter_old = iter_new;
	}
}


/*!
	this method makes a calculation for the percentage operator
	e.g.
	1000-50% = 1000-(1000*0,5) = 500
*/
void OperatorPercentage()
{
	if( stack_index < 3										||
		stack[stack_index-1].type != Item::numerical_value	||
		stack[stack_index-2].type != Item::mat_operator		||
		stack[stack_index-3].type != Item::numerical_value	)
		Error(err_percent_from);

	++pstring;
	SkipWhiteCharacters();

	uint c = 0;
	c += stack[stack_index-1].value.Div(100);
	c += stack[stack_index-1].value.Mul(stack[stack_index-3].value);

	if( c )
		Error(err_overflow);
}


/*!
	this method reads a mathematic operators
	or the final bracket or the semicolon operator

	return values:
		0 - ok
		1 - the string is finished
*/
int ReadOperator(Item & result)
{
	SkipWhiteCharacters();

	if( *pstring == '%' )
		OperatorPercentage();


	if( *pstring == 0 )
		return 1;
	else
	if( *pstring == ')' )
	{
		result.type = Item::last_bracket;
		++pstring;
	}
	else
	if( *pstring == ';' || (param_sep!=0 && *pstring==param_sep) )
	{
		result.type = Item::semicolon;
		++pstring;
	}
	else
	if( (*pstring>='a' && *pstring<='z') || (*pstring>='A' && *pstring<='Z') )
	{
		// short mul (without any operators)

		result.type = Item::mat_operator;
		result.moperator.SetType( MatOperator::shortmul );
	}
	else
		ReadMathematicalOperator(result);

return 0;
}



/*!
	this method is making the standard mathematic operation like '-' '+' '*' '/' and '^'

	the operation is made between 'value1' and 'value2'
	the result of this operation is stored in the 'value1'
*/
void MakeStandardMathematicOperation(ValueType & value1, typename MatOperator::Type mat_operator,
									const ValueType & value2)
{
uint res;

	calculated = true;

	switch( mat_operator )
	{
	case MatOperator::land:
		(!value1.IsZero() && !value2.IsZero()) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::lor:
		(!value1.IsZero() || !value2.IsZero()) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::eq:
		(value1 == value2) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::neq:
		(value1 != value2) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::lt:
		(value1 < value2) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::gt:
		(value1 > value2) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::let:
		(value1 <= value2) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::get:
		(value1 >= value2) ? value1.SetOne() : value1.SetZero();
		break;

	case MatOperator::sub: