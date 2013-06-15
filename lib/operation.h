/** @file
	Defines the structure of an operation, which is composed
	of two integers and an operator.
*/

#ifndef OPERATION_H
#define OPERATION_H

typedef struct operation {
	/// The first operand, also used to store the result
	int num1;
	
	/// The operator, also used to pass the termination command
	char op;
	
	/// The second operand
	int num2;
} operation;

#endif
