#include "LSystemParserVisitorImpl.h"
#include "MathExprAst.h"
#include "LSysConfig.h"
#include <iostream>
#include <string>

//#define SHOW_NODE
using namespace std;

any LSystemParserVisitorImpl::visitLSystem(LSystemParserParser::LSystemContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitLSystem: " << ctx->getText() << endl;
#endif

	auto prods = ctx->lProducion();
	vector<shared_ptr<LSysConfig::LProduction>> productions;
	for (uint32_t i = 0; i < prods.size(); i++)
		productions.emplace_back(any_cast<shared_ptr<LSysConfig::LProduction>>(visit(prods[i])));

	shared_ptr<LSysConfig::LSystem> lsystem = make_shared<LSysConfig::LSystem>();
	lsystem->productions = productions;

	return lsystem;
}

any LSystemParserVisitorImpl::visitSymDsts(LSystemParserParser::SymDstsContext* ctx) {
	auto syms = ctx->symDst();
	vector<shared_ptr<LSysConfig::SymDst>> symDsts;
	for (uint32_t i = 0; i < syms.size(); i++)
		symDsts.emplace_back(any_cast<shared_ptr<LSysConfig::SymDst>>(visit(syms[i])));
	return symDsts;
}

any LSystemParserVisitorImpl::visitParams(LSystemParserParser::ParamsContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitParams: " << ctx->getText() << endl;
#endif

	auto params = ctx->sym();
	vector<string> paramNames;
	for (uint32_t i = 0; i < params.size(); i++)
		paramNames.emplace_back(params[i]->getText());
	return paramNames;
}

any LSystemParserVisitorImpl::visitSymSrc(LSystemParserParser::SymSrcContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitSymSrc: " << ctx->getText() << endl;
#endif 

	string symName = ctx->symName()->getText();
	vector<string> paramNames = any_cast<vector<string>>(visit(ctx->params()));

	shared_ptr<LSysConfig::SymSrc> symSrc = make_shared<LSysConfig::SymSrc>();
	symSrc->name = symName;
	symSrc->paramNames = paramNames;

	return symSrc;
}

any LSystemParserVisitorImpl::visitLProducion(LSystemParserParser::LProducionContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitLProduction: " << ctx->getText() << endl;
#endif

	shared_ptr<LSysConfig::SymSrc> symSrc = any_cast<shared_ptr<LSysConfig::SymSrc>>(visit(ctx->symSrc()));
	vector<shared_ptr<LSysConfig::SymDst>> symDstList = any_cast<vector<shared_ptr<LSysConfig::SymDst>>>(visit(ctx->symDsts()));

	shared_ptr<LSysConfig::LProduction> production = make_shared<LSysConfig::LProduction>();
	production->src = symSrc;
	production->dsts = symDstList;

	return production;
}

any LSystemParserVisitorImpl::visitSymDst(LSystemParserParser::SymDstContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitSymDst: " << ctx->getText() << endl;
#endif

	shared_ptr<LSysConfig::SymDst> symDst = make_shared<LSysConfig::SymDst>();
	if (ctx->symName()) {
		symDst->name = ctx->symName()->getText();
		symDst->paramMaps = any_cast<vector<Expr::expr>>(visit(ctx->paramMaps()));
	}
	else if (ctx->Punct()) {
		symDst->type = LSysConfig::SymDst::PUNCT;
		symDst->punct = ctx->Punct()->getText();
	}
	else {
		cerr << "SymDst 未知类型，既不是NORMAL也不是PUNCT" << endl;
		throw runtime_error("LSystemParserVisitorImpl.cpp: SymDst error!");
	}

	return symDst;
}

any LSystemParserVisitorImpl::visitParamMaps(LSystemParserParser::ParamMapsContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitParamMaps: " << ctx->getText() << endl;
#endif

	auto exprs = ctx->mathExpr();
	vector<Expr::expr> nodes;
	for (uint32_t i = 0; i < exprs.size(); i++)
		nodes.emplace_back(any_cast<Expr::expr>(visit(exprs[i])));
	return nodes;
}

any LSystemParserVisitorImpl::visitMathExpr(LSystemParserParser::MathExprContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitMathExpr: " << ctx->getText() << endl;
#endif
	Expr::expr ret_node = nullptr;
	if (ctx->left == nullptr && ctx->right == nullptr) {
		// 匹配一个mathItem
		ret_node = any_cast<Expr::expr>(visit(ctx->mathItem()));
	}
	else {
		// 匹配 mathExpr +/- mathItem
		shared_ptr<Expr::expr_binary> node = make_shared<Expr::expr_binary>();
		node->left = any_cast<Expr::expr>(visit(ctx->left));
		node->right = any_cast<Expr::expr>(visit(ctx->right));
		switch (ctx->op->getType()) {
		case LSystemParserParser::Plus:
			node->op = Expr::expr_binary::ADD;
			break;
		case LSystemParserParser::Minus:
			node->op = Expr::expr_binary::SUB;
			break;
		default:
			cerr << "Unknown operator: " << ctx->op->getText() << endl;
			return nullptr;
		}
		ret_node = node;
	}
	return ret_node;
	
}

any LSystemParserVisitorImpl::visitSym(LSystemParserParser::SymContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitSym: " << ctx->getText() << endl;
#endif 
	// 从ctx中获取符号名
	shared_ptr<Expr::expr_var> var = make_shared<Expr::expr_var>();
	var->varname = ctx->String()->getText();
	return static_cast<Expr::expr>(var);
}


any LSystemParserVisitorImpl::visitMathItem(LSystemParserParser::MathItemContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitMathItem: " << ctx->getText() << endl;
#endif 
	Expr::expr ret_node = nullptr;
	if (ctx->left == nullptr && ctx->right == nullptr) {
		ret_node = any_cast<Expr::expr>(visit(ctx->mathFactor()));
	}
	else {
		shared_ptr<Expr::expr_binary> node = make_shared<Expr::expr_binary>();
		node->left = any_cast<Expr::expr>(visit(ctx->left));
		node->right = any_cast<Expr::expr>(visit(ctx->right));
		switch (ctx->op->getType()) {
		case LSystemParserParser::Mul:
			node->op = Expr::expr_binary::MUL;
			break;
		case LSystemParserParser::Div:
			node->op = Expr::expr_binary::DIV;
			break;
		default:
			cerr << "Unknown operator: " << ctx->op->getText() << endl;
			return nullptr;
		}
		ret_node = node;
	}
	return ret_node;
}
any LSystemParserVisitorImpl::visitMathFactor(LSystemParserParser::MathFactorContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitMathFactor: " << ctx->getText() << endl;
#endif 

	Expr::expr ret_node = nullptr;
	if (ctx->Number()) {
		// 字面量(可选符号)
		shared_ptr<Expr::expr_literal> node = make_shared<Expr::expr_literal>();
		float literal = stof(ctx->Number()->getText());
		node->value = literal;
		if (ctx->sign && ctx->sign->getType() == LSystemParserParser::Minus)
			node->value = -literal;
		ret_node = node;
	}
	if (ctx->sym()) {
		// 变量(可选符号)
		shared_ptr<Expr::expr_unary> node = make_shared<Expr::expr_unary>();
		node->op = Expr::expr_unary::POS;
		if (ctx->sign && ctx->sign->getType() == LSystemParserParser::Minus)
			node->op = Expr::expr_unary::NEG;
		node->left = any_cast<Expr::expr>(visit(ctx->sym()));
		ret_node = node;
	}
	if (ctx->mathExpr()) 
		ret_node = any_cast<Expr::expr>(visit(ctx->mathExpr()));
	
	return ret_node;
}
