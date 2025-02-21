
// Generated from LSystemRewriter.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "LSystemRewriterParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by LSystemRewriterParser.
 */
class  LSystemRewriterVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by LSystemRewriterParser.
   */
    virtual std::any visitSymSeq(LSystemRewriterParser::SymSeqContext *context) = 0;

    virtual std::any visitSym(LSystemRewriterParser::SymContext *context) = 0;

    virtual std::any visitSymName(LSystemRewriterParser::SymNameContext *context) = 0;

    virtual std::any visitSymArgs(LSystemRewriterParser::SymArgsContext *context) = 0;

    virtual std::any visitSymArg(LSystemRewriterParser::SymArgContext *context) = 0;


};

