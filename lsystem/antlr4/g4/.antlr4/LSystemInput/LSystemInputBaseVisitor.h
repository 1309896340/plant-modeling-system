
// Generated from LSystemInput.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "LSystemInputVisitor.h"


/**
 * This class provides an empty implementation of LSystemInputVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  LSystemInputBaseVisitor : public LSystemInputVisitor {
public:

  virtual std::any visitSymSeq(LSystemInputParser::SymSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSym(LSystemInputParser::SymContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymName(LSystemInputParser::SymNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymArgs(LSystemInputParser::SymArgsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymArg(LSystemInputParser::SymArgContext *ctx) override {
    return visitChildren(ctx);
  }


};

