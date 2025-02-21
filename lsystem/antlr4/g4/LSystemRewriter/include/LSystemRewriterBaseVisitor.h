
// Generated from LSystemRewriter.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "LSystemRewriterVisitor.h"


/**
 * This class provides an empty implementation of LSystemRewriterVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  LSystemRewriterBaseVisitor : public LSystemRewriterVisitor {
public:

  virtual std::any visitSymSeq(LSystemRewriterParser::SymSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSym(LSystemRewriterParser::SymContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymName(LSystemRewriterParser::SymNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymArgs(LSystemRewriterParser::SymArgsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymArg(LSystemRewriterParser::SymArgContext *ctx) override {
    return visitChildren(ctx);
  }


};

