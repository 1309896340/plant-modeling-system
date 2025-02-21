#include "LSystem.h"

#include "LSystemParserLexer.h"
#include "LSystemParserParser.h"
#include "LSystemParserVisitorImpl.h"

#include "LSystemRewriterLexer.h"
#include "LSystemRewriterParser.h"
#include "LSystemRewriterVisitorImpl.h"

#include "MathExprAst.h"

using namespace std;

LSystem::LSystem::LSystem(const string& axiom, const string& productions) : axiom(axiom), current(axiom) {
	antlr4::ANTLRInputStream input(productions);
	LSystemParserLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	LSystemParserParser parser(&tokens);

	antlr4::tree::ParseTree* tree = parser.lSystem();
	LSystemParserVisitorImpl visitor;
	auto res = visitor.visit(tree);
	shared_ptr<LSysConfig::LSystem> lsystem = any_cast<shared_ptr<LSysConfig::LSystem>>(res);

	this->lsystem = lsystem;
}
LSystem::LSystem::LSystem(const string& axiom, const vector<string>& productions) : axiom(axiom), current(axiom) {
	shared_ptr<LSysConfig::LSystem> lsystem = make_shared<LSysConfig::LSystem>();
	for (uint32_t i = 0; i < productions.size(); i++) {
		antlr4::ANTLRInputStream input(productions[i]);
		LSystemParserLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		LSystemParserParser parser(&tokens);
		antlr4::tree::ParseTree* tree = parser.lProducion();
		LSystemParserVisitorImpl visitor;
		shared_ptr<LSysConfig::LProduction> production = any_cast<shared_ptr<LSysConfig::LProduction>>(visitor.visit(tree));
		lsystem->productions.emplace_back(production);
	}

	this->lsystem = lsystem;
}

string LSystem::LSystem::iterate() {
	// 从productions中apply规则到current上，最后更新current

	// 首先要解析输入
	antlr4::ANTLRInputStream input(this->current);
	LSystemRewriterLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	LSystemRewriterParser parser(&tokens);
	antlr4::tree::ParseTree* tree = parser.symSeq();
	LSystemRewriterVisitorImpl visitor;
	shared_ptr<LSysConfig::SymSeq> symSeq = any_cast<shared_ptr<LSysConfig::SymSeq>>(visitor.visit(tree));

	stringstream strbuf;
	// 顺序遍历symSeq中每个Sym执行替换生成操作
	for (uint32_t i = 0; i < symSeq->syms.size(); i++) {
		shared_ptr<LSysConfig::Sym> sym = symSeq->syms[i];
		switch (sym->type) {
		case LSysConfig::Sym::NORMAL: {
			auto prod_iter = find_if(this->lsystem->productions.begin(), this->lsystem->productions.end(), [&sym](const shared_ptr<LSysConfig::LProduction>& production) {
				return production->src->name == sym->name;
				});
			if (prod_iter == this->lsystem->productions.end()) {
				// 找不到则原封不动放入
				strbuf << sym->name << "(";
				for (uint32_t kki = 0; kki < sym->args.size(); kki++) {
					strbuf << sym->args[kki];
					if (kki != sym->args.size() - 1)
						strbuf << ",";
				}
				strbuf << ")";
				continue;
			}
			// 使用指定产生式规则进行生成，为计算每个paramMap的结果准备运算环境，将实参作为环境变量绑定进去
			const shared_ptr<LSysConfig::LProduction>& production = *prod_iter;
			const shared_ptr<LSysConfig::SymSrc>& symSrc = production->src;
			assert(symSrc->paramNames.size() == sym->args.size() && "输入序列的符号实参数量与产生式规则形参数量不一致！");
			Expr::Environment env;
			for (uint32_t k = 0; k < production->dsts.size(); k++) {
				for (uint32_t j = 0; j < symSrc->paramNames.size(); j++)			// 形参与实参配对
					env.variables[symSrc->paramNames[j]] = sym->args[j];
				// 重写生成新字符串
				const shared_ptr<LSysConfig::SymDst>& symDst = production->dsts[k];
				switch (symDst->type) {
				case LSysConfig::SymDst::NORMAL:
					// 正常节点
					strbuf << symDst->name << "(";
					for (uint32_t j = 0; j < symDst->paramMaps.size(); j++) {
						float res = symDst->paramMaps[j]->evaluate(env);
						strbuf << res;
						if (j != symDst->paramMaps.size() - 1)
							strbuf << ",";
					}
					strbuf << ")";
					break;
				case LSysConfig::SymDst::PUNCT:
					strbuf << symDst->punct;
					break;
				default:
					cerr << "LSystem.cpp    iterate()    error!" << endl;
					exit(1);
				}
			}
			break;
		}
		case LSysConfig::Sym::PUNCT: {
			strbuf << sym->punct;
			break;
		}
		default:
			cerr << "LSystem.cpp    iterate()    error!  \"sym\"" << endl;
			exit(1);
		}
	}
	this->current = strbuf.str();
	return this->current;
}

string LSystem::LSystem::getCurrent()const {
	return this->current;
}

string LSystem::LSystem::iterate(uint32_t n) {
	for (uint32_t i = 0; i < n; i++)
		this->current = this->iterate();
	return this->current;
}


void LSystem::LSystem::reset() {
	this->current = this->axiom;
}