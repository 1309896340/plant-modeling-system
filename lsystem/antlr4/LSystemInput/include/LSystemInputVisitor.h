
// Generated from LSystemInput.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "LSystemInputParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by LSystemInputParser.
 */
class  LSystemInputVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by LSystemInputParser.
   */
    virtual std::any visitSymSeq(LSystemInputParser::SymSeqContext *context) = 0;

    virtual std::any visitSym(LSystemInputParser::SymContext *context) = 0;

    virtual std::any visitSymName(LSystemInputParser::SymNameContext *context) = 0;

    virtual std::any visitSymArgs(LSystemInputParser::SymArgsContext *context) = 0;

    virtual std::any visitSymArg(LSystemInputParser::SymArgContext *context) = 0;


};

