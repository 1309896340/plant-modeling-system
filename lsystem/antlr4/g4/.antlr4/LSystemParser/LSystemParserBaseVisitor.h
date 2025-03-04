
// Generated from LSystemParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "LSystemParserVisitor.h"


/**
 * This class provides an empty implementation of LSystemParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  LSystemParserBaseVisitor : public LSystemParserVisitor {
public:

  virtual std::any visitLSystem(LSystemParserParser::LSystemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLProducion(LSystemParserParser::LProducionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymSrc(LSystemParserParser::SymSrcContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParams(LSystemParserParser::ParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymDsts(LSystemParserParser::SymDstsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymDst(LSystemParserParser::SymDstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParamMaps(LSystemParserParser::ParamMapsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMathExpr(LSystemParserParser::MathExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMathItem(LSystemParserParser::MathItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMathFactor(LSystemParserParser::MathFactorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSymName(LSystemParserParser::SymNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSym(LSystemParserParser::SymContext *ctx) override {
    return visitChildren(ctx);
  }


};

