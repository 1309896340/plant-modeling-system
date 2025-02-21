#pragma once
#include "MathExprAst.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace LSysConfig {
	// 定义部分
	struct SymDst {
		enum {
			NORMAL,PUNCT
		}type{NORMAL};
		// 第1种情况，是一个正常SymDst节点
		std::string name;
		std::vector<Expr::expr> paramMaps;
		// 第2种情况，是一个punct符号
		std::string punct;
	};

	struct SymSrc {
		std::string name;
		std::vector<std::string> paramNames;
	};

	struct LProduction {
		std::shared_ptr<SymSrc> src;								// 映射源
		std::vector<std::shared_ptr<SymDst>> dsts;		// 映射目标
	};

	struct LSystem {
		std::vector<std::shared_ptr<LProduction>> productions;
	};

	// 调用部分
	struct Sym {
		enum {
			NORMAL, PUNCT
		}type{ NORMAL };
		// 第1种情况，是一个正常Sym节点
		std::string name;
		std::vector<float> args;
		// 第2种情况，是一个punct符号
		std::string punct;
	};
	struct SymSeq {
		std::vector<std::shared_ptr<Sym>> syms;
	};

};   // namespace LSystem