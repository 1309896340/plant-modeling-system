
// Generated from LSystemParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  LSystemParserParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, Plus = 4, Minus = 5, Mul = 6, Div = 7, 
    RightMap = 8, Newline = 9, Punct = 10, String = 11, Number = 12
  };

  enum {
    RuleLSystem = 0, RuleLProducion = 1, RuleSymSrc = 2, RuleParams = 3, 
    RuleSymDsts = 4, RuleSymDst = 5, RuleParamMaps = 6, RuleMathExpr = 7, 
    RuleMathItem = 8, RuleMathFactor = 9, RuleSymName = 10, RuleSym = 11
  };

  explicit LSystemParserParser(antlr4::TokenStream *input);

  LSystemParserParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~LSystemParserParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class LSystemContext;
  class LProducionContext;
  class SymSrcContext;
  class ParamsContext;
  class SymDstsContext;
  class SymDstContext;
  class ParamMapsContext;
  class MathExprContext;
  class MathItemContext;
  class MathFactorContext;
  class SymNameContext;
  class SymContext; 

  class  LSystemContext : public antlr4::ParserRuleContext {
  public:
    LSystemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<LProducionContext *> lProducion();
    LProducionContext* lProducion(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Newline();
    antlr4::tree::TerminalNode* Newline(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LSystemContext* lSystem();

  class  LProducionContext : public antlr4::ParserRuleContext {
  public:
    LProducionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SymSrcContext *symSrc();
    antlr4::tree::TerminalNode *RightMap();
    SymDstsContext *symDsts();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LProducionContext* lProducion();

  class  SymSrcContext : public antlr4::ParserRuleContext {
  public:
    SymSrcContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SymNameContext *symName();
    ParamsContext *params();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymSrcContext* symSrc();

  class  ParamsContext : public antlr4::ParserRuleContext {
  public:
    ParamsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<SymContext *> sym();
    SymContext* sym(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParamsContext* params();

  class  SymDstsContext : public antlr4::ParserRuleContext {
  public:
    SymDstsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<SymDstContext *> symDst();
    SymDstContext* symDst(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymDstsContext* symDsts();

  class  SymDstContext : public antlr4::ParserRuleContext {
  public:
    SymDstContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SymNameContext *symName();
    ParamMapsContext *paramMaps();
    antlr4::tree::TerminalNode *Punct();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymDstContext* symDst();

  class  ParamMapsContext : public antlr4::ParserRuleContext {
  public:
    ParamMapsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<MathExprContext *> mathExpr();
    MathExprContext* mathExpr(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParamMapsContext* paramMaps();

  class  MathExprContext : public antlr4::ParserRuleContext {
  public:
    LSystemParserParser::MathExprContext *left = nullptr;
    antlr4::Token *op = nullptr;
    LSystemParserParser::MathItemContext *right = nullptr;
    MathExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MathItemContext *mathItem();
    MathExprContext *mathExpr();
    antlr4::tree::TerminalNode *Plus();
    antlr4::tree::TerminalNode *Minus();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MathExprContext* mathExpr();
  MathExprContext* mathExpr(int precedence);
  class  MathItemContext : public antlr4::ParserRuleContext {
  public:
    LSystemParserParser::MathItemContext *left = nullptr;
    antlr4::Token *op = nullptr;
    LSystemParserParser::MathFactorContext *right = nullptr;
    MathItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MathFactorContext *mathFactor();
    MathItemContext *mathItem();
    antlr4::tree::TerminalNode *Mul();
    antlr4::tree::TerminalNode *Div();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MathItemContext* mathItem();
  MathItemContext* mathItem(int precedence);
  class  MathFactorContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *sign = nullptr;
    MathFactorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Number();
    antlr4::tree::TerminalNode *Plus();
    antlr4::tree::TerminalNode *Minus();
    SymContext *sym();
    MathExprContext *mathExpr();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MathFactorContext* mathFactor();

  class  SymNameContext : public antlr4::ParserRuleContext {
  public:
    SymNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *String();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymNameContext* symName();

  class  SymContext : public antlr4::ParserRuleContext {
  public:
    SymContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *String();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymContext* sym();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool mathExprSempred(MathExprContext *_localctx, size_t predicateIndex);
  bool mathItemSempred(MathItemContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

