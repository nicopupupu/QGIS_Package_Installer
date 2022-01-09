
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