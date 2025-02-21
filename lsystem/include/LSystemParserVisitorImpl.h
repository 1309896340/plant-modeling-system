#pragma once

#include "LSystemParserBaseVisitor.h"

class LSystemParserVisitorImpl : public LSystemParserBaseVisitor {
public:
	virtual std::any visitLSystem(LSystemParserParser::LSystemContext* ctx) override;
	virtual std::any visitLProducion(LSystemParserParser::LProducionContext* ctx) override;
	virtual std::any visitSymSrc(LSystemParserParser::SymSrcContext* ctx) override;
	virtual std::any visitParams(LSystemParserParser::ParamsContext* ctx) override;
	virtual std::any visitSymDsts(LSystemParserParser::SymDstsContext* ctx) override;
	virtual std::any visitSymDst(LSystemParserParser::SymDstContext* ctx) override;
	virtual std::any visitParamMaps(LSystemParserParser::ParamMapsContext* ctx) override;
	virtual std::any visitMathExpr(LSystemParserParser::MathExprContext* ctx) override;
	virtual std::any visitSym(LSystemParserParser::SymContext* ctx) override;

	virtual std::any visitMathItem(LSystemParserParser::MathItemContext* ctx) override;
	virtual std::any visitMathFactor(LSystemParserParser::MathFactorContext* ctx) override;
};


