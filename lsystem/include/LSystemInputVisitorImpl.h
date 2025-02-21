#pragma once

#include "LSystemInputBaseVisitor.h"

class LSystemInputVisitorImpl : public LSystemInputBaseVisitor {
public:
	virtual std::any visitSymSeq(LSystemInputParser::SymSeqContext* ctx) override;
	virtual std::any visitSym(LSystemInputParser::SymContext* ctx) override;
	virtual std::any visitSymName(LSystemInputParser::SymNameContext* ctx) override;
	virtual std::any visitSymArgs(LSystemInputParser::SymArgsContext* ctx) override;
	virtual std::any visitSymArg(LSystemInputParser::SymArgContext* ctx) override;
};

