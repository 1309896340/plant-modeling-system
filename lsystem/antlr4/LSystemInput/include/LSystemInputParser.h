
// Generated from LSystemInput.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  LSystemInputParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, Newline = 4, Punct = 5, String = 6, Number = 7
  };

  enum {
    RuleSymSeq = 0, RuleSym = 1, RuleSymName = 2, RuleSymArgs = 3, RuleSymArg = 4
  };

  explicit LSystemInputParser(antlr4::TokenStream *input);

  LSystemInputParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~LSystemInputParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class SymSeqContext;
  class SymContext;
  class SymNameContext;
  class SymArgsContext;
  class SymArgContext; 

  class  SymSeqContext : public antlr4::ParserRuleContext {
  public:
    SymSeqContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<SymContext *> sym();
    SymContext* sym(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymSeqContext* symSeq();

  class  SymContext : public antlr4::ParserRuleContext {
  public:
    SymContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SymNameContext *symName();
    SymArgsContext *symArgs();
    antlr4::tree::TerminalNode *Punct();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymContext* sym();

  class  SymNameContext : public antlr4::ParserRuleContext {
  public:
    SymNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *String();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymNameContext* symName();

  class  SymArgsContext : public antlr4::ParserRuleContext {
  public:
    SymArgsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<SymArgContext *> symArg();
    SymArgContext* symArg(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymArgsContext* symArgs();

  class  SymArgContext : public antlr4::ParserRuleContext {
  public:
    SymArgContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Number();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymArgContext* symArg();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

