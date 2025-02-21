#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

namespace Expr {
	using expr = std::shared_ptr<struct expr_node>;
	struct Environment {
		//std::map<std::string, struct lsys_func> functions;
		std::map<std::string, float> variables;
	};

	struct expr_node {
		virtual float evaluate(Environment& env) = 0;
	};

	struct expr_unary : public expr_node {
		enum{
			NEG,
			POS
		}op{POS};
		expr left{nullptr};
		virtual float evaluate(Environment& env)override;
	};

	struct expr_binary : public expr_node {
		enum{
			ADD,
			SUB,
			MUL,
			DIV
		}op{ADD};
		expr left{ nullptr };
		expr right{ nullptr };
		virtual float evaluate(Environment& env) override;
	};
	//struct lsys_func : public expr_node {
	//	std::vector<float> params;
	//	virtual float evaluate(Environment& env) override;
	//};
	struct expr_var : public expr_node {
		std::string varname;
		virtual float evaluate(Environment& env) override;
	};
	struct expr_literal : public expr_node {
		float value{0.0f};
		virtual float evaluate(Environment& env) override;
	};


}



