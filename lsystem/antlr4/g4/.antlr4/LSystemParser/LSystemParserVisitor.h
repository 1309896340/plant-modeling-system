
// Generated from LSystemParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "LSystemParserParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by LSystemParserParser.
 */
class  LSystemParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by LSystemParserParser.
   */
    virtual std::any visitLSystem(LSystemParserParser::LSystemContext *context) = 0;

    virtual std::any visitLProducion(LSystemParserParser::LProducionContext *context) = 0;

    virtual std::any visitSymSrc(LSystemParserParser::SymSrcContext *context) = 0;

    virtual std::any visitParams(LSystemParserParser::ParamsContext *context) = 0;

    virtual std::any visitSymDsts(LSystemParserParser::SymDstsContext *context) = 0;

    virtual std::any visitSymDst(LSystemParserParser::SymDstContext *context) = 0;

    virtual std::any visitParamMaps(LSystemParserParser::ParamMapsContext *context) = 0;

    virtual std::any visitMathExpr(LSystemParserParser::MathExprContext *context) = 0;

    virtual std::any visitMathItem(LSystemParserParser::MathItemContext *context) = 0;

    virtual std::any visitMathFactor(LSystemParserParser::MathFactorContext *context) = 0;

    virtual std::any visitSymName(LSystemParserParser::SymNameContext *context) = 0;

    virtual std::any visitSym(LSystemParserParser::SymContext *context) = 0;


};

