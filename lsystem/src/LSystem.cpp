#include "LSystem.h"

#include "LSysConfig.h"
#include "LSystemParserLexer.h"
#include "LSystemParserParser.h"
#include "LSystemParserVisitorImpl.h"

#include "LSystemInputLexer.h"
#include "LSystemInputParser.h"
#include "LSystemInputVisitorImpl.h"

#include "MathExprAst.h"
#include <any>
#include <exception>
#include <memory>
#include <stdexcept>

using namespace std;

LSystem::LSystem::LSystem(const string& axiom, const string& productions)
  : axiom(axiom)
  , current(axiom) {
  antlr4::ANTLRInputStream        input(productions);
  LSystemParserLexer              lexer(&input);
  antlr4::CommonTokenStream       tokens(&lexer);
  LSystemParserParser             parser(&tokens);
  antlr4::tree::ParseTree*        tree = parser.lSystem();
  LSystemParserVisitorImpl        visitor;
  auto                            res = visitor.visit(tree);
  shared_ptr<LSysConfig::LSystem> lsystem =
    any_cast<shared_ptr<LSysConfig::LSystem>>(res);

  this->lsystem = lsystem;
}
LSystem::LSystem::LSystem(const string&         axiom,
                          const vector<string>& productions)
  : axiom(axiom)
  , current(axiom) {
  this->updateProduction(productions);
}

bool LSystem::LSystem::updateAxiom(const string& new_axiom) {
  this->axiom = new_axiom;
  this->reset();
  return true;
}

bool LSystem::LSystem::updateProduction(const vector<string>& new_productions) {
  shared_ptr<LSysConfig::LSystem> lsystem = make_shared<LSysConfig::LSystem>();
  for (uint32_t i = 0; i < new_productions.size(); i++) {
    try {
      antlr4::ANTLRInputStream            input(new_productions[i]);
      LSystemParserLexer                  lexer(&input);
      antlr4::CommonTokenStream           tokens(&lexer);
      LSystemParserParser                 parser(&tokens);
      antlr4::tree::ParseTree*            tree = parser.lProducion();
      LSystemParserVisitorImpl            visitor;
      shared_ptr<LSysConfig::LProduction> production =
        any_cast<shared_ptr<LSysConfig::LProduction>>(visitor.visit(tree));
      lsystem->productions.emplace_back(production);
    }
    catch (const exception& e) {
      cerr << e.what() << endl;
      return false;
    }
  }
  this->lsystem = lsystem;
  return true;

  // vector<LSysConfig::LProduction> prods;
  //     bool                                    is_success = true;
  //     for (const string& production : productions) {
  //         auto lsys_prod_string = lexy::string_input(production);
  //         auto lsys_prod_res    =
  //         lexy::parse<LSysParser::grammar::LProduction>(lsys_prod_string,
  //         lexy_ext::report_error); if (!lsys_prod_res.is_success()) {
  //             is_success = false;
  //             break;
  //         }
  //         prods.push_back(lsys_prod_res.value());
  //     }
  //     if (is_success)
  //         this->lsystem = LSysParser::config::LSystem(prods);
  //     return true;
}

shared_ptr<LSysConfig::SymSeq>
LSystem::LSystem::parseInput(const string& sinput) {
  antlr4::ANTLRInputStream       input(sinput);
  LSystemInputLexer              lexer(&input);
  antlr4::CommonTokenStream      tokens(&lexer);
  LSystemInputParser             parser(&tokens);
  antlr4::tree::ParseTree*       tree = parser.symSeq();
  LSystemInputVisitorImpl        visitor;
  shared_ptr<LSysConfig::SymSeq> symSeq = nullptr;
  try {
    symSeq = any_cast<shared_ptr<LSysConfig::SymSeq>>(visitor.visit(tree));
  }
  catch (const bad_any_cast& e) {
    cerr << e.what() << endl;
  }
  return symSeq;
}

string LSystem::LSystem::iterate() {
  shared_ptr<LSysConfig::SymSeq> symSeq = parseInput(this->current);

  stringstream strbuf;
  // 顺序遍历symSeq中每个Sym执行替换生成操作
  for (uint32_t i = 0; i < symSeq->syms.size(); i++) {
    shared_ptr<LSysConfig::Sym> sym = symSeq->syms[i];
    switch (sym->type) {
    case LSysConfig::Sym::NORMAL: {
      auto prod_iter = find_if(
        this->lsystem->productions.begin(),
        this->lsystem->productions.end(),
        [&sym](const shared_ptr<LSysConfig::LProduction>& production) {
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
      const shared_ptr<LSysConfig::SymSrc>&      symSrc     = production->src;
      assert(symSrc->paramNames.size() == sym->args.size() &&
             "输入序列的符号实参数量与产生式规则形参数量不一致！");
      Expr::Environment env;
      for (uint32_t k = 0; k < production->dsts.size(); k++) {
        for (uint32_t j = 0; j < symSrc->paramNames.size();
             j++)   // 形参与实参配对
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

string LSystem::LSystem::getCurrent() const {
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