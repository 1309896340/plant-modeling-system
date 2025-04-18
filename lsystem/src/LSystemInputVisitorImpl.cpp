#include "LSystemInputVisitorImpl.h"
// #include "MathExprAst.h"
#include "LSysConfig.h"
#include <iostream>
#include <string>
using namespace std;

any LSystemInputVisitorImpl::visitSymSeq(LSystemInputParser::SymSeqContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitSymSeq: " << ctx->getText() << endl;
#endif

	auto syms = ctx->sym();
	vector<shared_ptr<LSysConfig::Sym>> symList;
	for (uint32_t i = 0; i < syms.size(); i++)
		symList.emplace_back(any_cast<shared_ptr<LSysConfig::Sym>>(visit(syms[i])));
	shared_ptr<LSysConfig::SymSeq>  symSeq = make_shared<LSysConfig::SymSeq>();
	symSeq->syms = symList;

	return symSeq;
}
any LSystemInputVisitorImpl::visitSym(LSystemInputParser::SymContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitSym: " << ctx->getText() << endl;
#endif

	shared_ptr<LSysConfig::Sym> sym = make_shared<LSysConfig::Sym>();
	if(ctx->symName()){
		string symName = ctx->symName()->getText();
		vector<float> symArgs = {};
		if (ctx->symArgs())
			symArgs = any_cast<vector<float>>(visit(ctx->symArgs()));
		sym->name = symName;
		sym->args = symArgs;
	}
	else if (ctx->Punct()) {
		sym->type = LSysConfig::Sym::PUNCT;
		sym->punct = ctx->Punct()->getText()[0];
	}
	else {
		cerr << "LSystemInputVisitorImpl.cpp    visitSym()    error!";
		exit(1);
	}

	return sym;
}
any LSystemInputVisitorImpl::visitSymName(LSystemInputParser::SymNameContext* ctx) {
	return ctx->String()->getText();
}
any LSystemInputVisitorImpl::visitSymArgs(LSystemInputParser::SymArgsContext* ctx) {
#ifdef SHOW_NODE
	cout << "visitSymArgs: " << ctx->getText() << endl;
#endif

	auto argList = ctx->symArg();
	vector<float> args;
	for (uint32_t i = 0; i < argList.size(); i++)
		args.emplace_back(any_cast<float>(visit(argList[i])));
	return args;
}
any LSystemInputVisitorImpl::visitSymArg(LSystemInputParser::SymArgContext* ctx) {
	return stof(ctx->Number()->getText());
}

