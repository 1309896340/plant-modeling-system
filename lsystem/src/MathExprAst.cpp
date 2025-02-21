#include "MathExprAst.h"
#include <stdexcept>
#include <iostream>

using namespace std;
namespace Expr {
	float expr_literal::evaluate(Environment& env) {
		return value;
	}
	float expr_var::evaluate(Environment& env) {
		auto it = env.variables.find(varname);
		if (it == env.variables.end()) {
			cerr << "Variable not found: " << varname << endl;
			throw std::runtime_error("Variable not found");
		}
		return it->second;
	}

	float expr_unary::evaluate(Environment& env) {
		float value = left->evaluate(env);
		switch (op) {
		case NEG:
			return -value;
		case POS:
			return value;
		default:
			cerr << "Unknown unary operator" << endl;
			throw std::runtime_error("Unknown unary operator");
		}
	}
	float expr_binary::evaluate(Environment& env) {
		float lhs = left->evaluate(env);
		float rhs = right->evaluate(env);
		switch (op) {
		case ADD:
			return lhs + rhs;
		case SUB:
			return lhs - rhs;
		case MUL:
			return lhs * rhs;
		case DIV:
			return lhs / rhs;
		default:
			cerr << "Unknown binary operator" << endl;
			throw std::runtime_error("Unknown binary operator");
		}
	}
	//float lsys_func::evaluate(Environment& env) {
	//	cerr << "Function call not implemented" << endl;
	//	throw std::runtime_error("Function call not implemented");
};
