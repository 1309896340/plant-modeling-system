#pragma once

#include "LSystemRewriterBaseVisitor.h"

class LSystemRewriterVisitorImpl : public LSystemRewriterBaseVisitor {
public:
	virtual std::any visitSymSeq(LSystemRewriterParser::SymSeqContext* ctx) override;
	virtual std::any visitSym(LSystemRewriterParser::SymContext* ctx) override;
	virtual std::any visitSymName(LSystemRewriterParser::SymNameContext* ctx) override;
	virtual std::any visitSymArgs(LSystemRewriterParser::SymArgsContext* ctx) override;
	virtual std::any visitSymArg(LSystemRewriterParser::SymArgContext* ctx) override;
};

