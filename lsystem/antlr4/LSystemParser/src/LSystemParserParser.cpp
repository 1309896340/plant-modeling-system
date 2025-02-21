
// Generated from LSystemParser.g4 by ANTLR 4.13.2


#include "LSystemParserVisitor.h"

#include "LSystemParserParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct LSystemParserParserStaticData final {
  LSystemParserParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  LSystemParserParserStaticData(const LSystemParserParserStaticData&) = delete;
  LSystemParserParserStaticData(LSystemParserParserStaticData&&) = delete;
  LSystemParserParserStaticData& operator=(const LSystemParserParserStaticData&) = delete;
  LSystemParserParserStaticData& operator=(LSystemParserParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag lsystemparserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<LSystemParserParserStaticData> lsystemparserParserStaticData = nullptr;

void lsystemparserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (lsystemparserParserStaticData != nullptr) {
    return;
  }
#else
  assert(lsystemparserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<LSystemParserParserStaticData>(
    std::vector<std::string>{
      "lSystem", "lProducion", "symSrc", "params", "symDsts", "symDst", 
      "paramMaps", "mathExpr", "mathItem", "mathFactor", "symName", "sym"
    },
    std::vector<std::string>{
      "", "'('", "')'", "','", "'+'", "'-'", "'*'", "'/'", "'->'", "'\\n'"
    },
    std::vector<std::string>{
      "", "", "", "", "Plus", "Minus", "Mul", "Div", "RightMap", "Newline", 
      "Punct", "String", "Number"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,12,117,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,1,0,1,0,5,0,27,8,0,10,0,12,
  	0,30,9,0,5,0,32,8,0,10,0,12,0,35,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,
  	1,2,1,2,1,2,1,3,1,3,1,3,5,3,51,8,3,10,3,12,3,54,9,3,1,4,4,4,57,8,4,11,
  	4,12,4,58,1,5,1,5,1,5,1,5,1,5,1,5,3,5,67,8,5,1,6,1,6,1,6,5,6,72,8,6,10,
  	6,12,6,75,9,6,1,7,1,7,1,7,1,7,1,7,1,7,5,7,83,8,7,10,7,12,7,86,9,7,1,8,
  	1,8,1,8,1,8,1,8,1,8,5,8,94,8,8,10,8,12,8,97,9,8,1,9,3,9,100,8,9,1,9,1,
  	9,3,9,104,8,9,1,9,1,9,1,9,1,9,1,9,3,9,111,8,9,1,10,1,10,1,11,1,11,1,11,
  	0,2,14,16,12,0,2,4,6,8,10,12,14,16,18,20,22,0,2,1,0,4,5,1,0,6,7,116,0,
  	33,1,0,0,0,2,38,1,0,0,0,4,42,1,0,0,0,6,47,1,0,0,0,8,56,1,0,0,0,10,66,
  	1,0,0,0,12,68,1,0,0,0,14,76,1,0,0,0,16,87,1,0,0,0,18,110,1,0,0,0,20,112,
  	1,0,0,0,22,114,1,0,0,0,24,28,3,2,1,0,25,27,5,9,0,0,26,25,1,0,0,0,27,30,
  	1,0,0,0,28,26,1,0,0,0,28,29,1,0,0,0,29,32,1,0,0,0,30,28,1,0,0,0,31,24,
  	1,0,0,0,32,35,1,0,0,0,33,31,1,0,0,0,33,34,1,0,0,0,34,36,1,0,0,0,35,33,
  	1,0,0,0,36,37,5,0,0,1,37,1,1,0,0,0,38,39,3,4,2,0,39,40,5,8,0,0,40,41,
  	3,8,4,0,41,3,1,0,0,0,42,43,3,20,10,0,43,44,5,1,0,0,44,45,3,6,3,0,45,46,
  	5,2,0,0,46,5,1,0,0,0,47,52,3,22,11,0,48,49,5,3,0,0,49,51,3,22,11,0,50,
  	48,1,0,0,0,51,54,1,0,0,0,52,50,1,0,0,0,52,53,1,0,0,0,53,7,1,0,0,0,54,
  	52,1,0,0,0,55,57,3,10,5,0,56,55,1,0,0,0,57,58,1,0,0,0,58,56,1,0,0,0,58,
  	59,1,0,0,0,59,9,1,0,0,0,60,61,3,20,10,0,61,62,5,1,0,0,62,63,3,12,6,0,
  	63,64,5,2,0,0,64,67,1,0,0,0,65,67,5,10,0,0,66,60,1,0,0,0,66,65,1,0,0,
  	0,67,11,1,0,0,0,68,73,3,14,7,0,69,70,5,3,0,0,70,72,3,14,7,0,71,69,1,0,
  	0,0,72,75,1,0,0,0,73,71,1,0,0,0,73,74,1,0,0,0,74,13,1,0,0,0,75,73,1,0,
  	0,0,76,77,6,7,-1,0,77,78,3,16,8,0,78,84,1,0,0,0,79,80,10,2,0,0,80,81,
  	7,0,0,0,81,83,3,16,8,0,82,79,1,0,0,0,83,86,1,0,0,0,84,82,1,0,0,0,84,85,
  	1,0,0,0,85,15,1,0,0,0,86,84,1,0,0,0,87,88,6,8,-1,0,88,89,3,18,9,0,89,
  	95,1,0,0,0,90,91,10,2,0,0,91,92,7,1,0,0,92,94,3,18,9,0,93,90,1,0,0,0,
  	94,97,1,0,0,0,95,93,1,0,0,0,95,96,1,0,0,0,96,17,1,0,0,0,97,95,1,0,0,0,
  	98,100,7,0,0,0,99,98,1,0,0,0,99,100,1,0,0,0,100,101,1,0,0,0,101,111,5,
  	12,0,0,102,104,7,0,0,0,103,102,1,0,0,0,103,104,1,0,0,0,104,105,1,0,0,
  	0,105,111,3,22,11,0,106,107,5,1,0,0,107,108,3,14,7,0,108,109,5,2,0,0,
  	109,111,1,0,0,0,110,99,1,0,0,0,110,103,1,0,0,0,110,106,1,0,0,0,111,19,
  	1,0,0,0,112,113,5,11,0,0,113,21,1,0,0,0,114,115,5,11,0,0,115,23,1,0,0,
  	0,11,28,33,52,58,66,73,84,95,99,103,110
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  lsystemparserParserStaticData = std::move(staticData);
}

}

LSystemParserParser::LSystemParserParser(TokenStream *input) : LSystemParserParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

LSystemParserParser::LSystemParserParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  LSystemParserParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *lsystemparserParserStaticData->atn, lsystemparserParserStaticData->decisionToDFA, lsystemparserParserStaticData->sharedContextCache, options);
}

LSystemParserParser::~LSystemParserParser() {
  delete _interpreter;
}

const atn::ATN& LSystemParserParser::getATN() const {
  return *lsystemparserParserStaticData->atn;
}

std::string LSystemParserParser::getGrammarFileName() const {
  return "LSystemParser.g4";
}

const std::vector<std::string>& LSystemParserParser::getRuleNames() const {
  return lsystemparserParserStaticData->ruleNames;
}

const dfa::Vocabulary& LSystemParserParser::getVocabulary() const {
  return lsystemparserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView LSystemParserParser::getSerializedATN() const {
  return lsystemparserParserStaticData->serializedATN;
}


//----------------- LSystemContext ------------------------------------------------------------------

LSystemParserParser::LSystemContext::LSystemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemParserParser::LSystemContext::EOF() {
  return getToken(LSystemParserParser::EOF, 0);
}

std::vector<LSystemParserParser::LProducionContext *> LSystemParserParser::LSystemContext::lProducion() {
  return getRuleContexts<LSystemParserParser::LProducionContext>();
}

LSystemParserParser::LProducionContext* LSystemParserParser::LSystemContext::lProducion(size_t i) {
  return getRuleContext<LSystemParserParser::LProducionContext>(i);
}

std::vector<tree::TerminalNode *> LSystemParserParser::LSystemContext::Newline() {
  return getTokens(LSystemParserParser::Newline);
}

tree::TerminalNode* LSystemParserParser::LSystemContext::Newline(size_t i) {
  return getToken(LSystemParserParser::Newline, i);
}


size_t LSystemParserParser::LSystemContext::getRuleIndex() const {
  return LSystemParserParser::RuleLSystem;
}


std::any LSystemParserParser::LSystemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitLSystem(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::LSystemContext* LSystemParserParser::lSystem() {
  LSystemContext *_localctx = _tracker.createInstance<LSystemContext>(_ctx, getState());
  enterRule(_localctx, 0, LSystemParserParser::RuleLSystem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(33);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LSystemParserParser::String) {
      setState(24);
      lProducion();
      setState(28);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == LSystemParserParser::Newline) {
        setState(25);
        match(LSystemParserParser::Newline);
        setState(30);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(35);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(36);
    match(LSystemParserParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LProducionContext ------------------------------------------------------------------

LSystemParserParser::LProducionContext::LProducionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LSystemParserParser::SymSrcContext* LSystemParserParser::LProducionContext::symSrc() {
  return getRuleContext<LSystemParserParser::SymSrcContext>(0);
}

tree::TerminalNode* LSystemParserParser::LProducionContext::RightMap() {
  return getToken(LSystemParserParser::RightMap, 0);
}

LSystemParserParser::SymDstsContext* LSystemParserParser::LProducionContext::symDsts() {
  return getRuleContext<LSystemParserParser::SymDstsContext>(0);
}


size_t LSystemParserParser::LProducionContext::getRuleIndex() const {
  return LSystemParserParser::RuleLProducion;
}


std::any LSystemParserParser::LProducionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitLProducion(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::LProducionContext* LSystemParserParser::lProducion() {
  LProducionContext *_localctx = _tracker.createInstance<LProducionContext>(_ctx, getState());
  enterRule(_localctx, 2, LSystemParserParser::RuleLProducion);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(38);
    symSrc();
    setState(39);
    match(LSystemParserParser::RightMap);
    setState(40);
    symDsts();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymSrcContext ------------------------------------------------------------------

LSystemParserParser::SymSrcContext::SymSrcContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LSystemParserParser::SymNameContext* LSystemParserParser::SymSrcContext::symName() {
  return getRuleContext<LSystemParserParser::SymNameContext>(0);
}

LSystemParserParser::ParamsContext* LSystemParserParser::SymSrcContext::params() {
  return getRuleContext<LSystemParserParser::ParamsContext>(0);
}


size_t LSystemParserParser::SymSrcContext::getRuleIndex() const {
  return LSystemParserParser::RuleSymSrc;
}


std::any LSystemParserParser::SymSrcContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitSymSrc(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::SymSrcContext* LSystemParserParser::symSrc() {
  SymSrcContext *_localctx = _tracker.createInstance<SymSrcContext>(_ctx, getState());
  enterRule(_localctx, 4, LSystemParserParser::RuleSymSrc);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(42);
    symName();
    setState(43);
    match(LSystemParserParser::T__0);
    setState(44);
    params();
    setState(45);
    match(LSystemParserParser::T__1);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParamsContext ------------------------------------------------------------------

LSystemParserParser::ParamsContext::ParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LSystemParserParser::SymContext *> LSystemParserParser::ParamsContext::sym() {
  return getRuleContexts<LSystemParserParser::SymContext>();
}

LSystemParserParser::SymContext* LSystemParserParser::ParamsContext::sym(size_t i) {
  return getRuleContext<LSystemParserParser::SymContext>(i);
}


size_t LSystemParserParser::ParamsContext::getRuleIndex() const {
  return LSystemParserParser::RuleParams;
}


std::any LSystemParserParser::ParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitParams(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::ParamsContext* LSystemParserParser::params() {
  ParamsContext *_localctx = _tracker.createInstance<ParamsContext>(_ctx, getState());
  enterRule(_localctx, 6, LSystemParserParser::RuleParams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(47);
    sym();
    setState(52);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LSystemParserParser::T__2) {
      setState(48);
      match(LSystemParserParser::T__2);
      setState(49);
      sym();
      setState(54);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymDstsContext ------------------------------------------------------------------

LSystemParserParser::SymDstsContext::SymDstsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LSystemParserParser::SymDstContext *> LSystemParserParser::SymDstsContext::symDst() {
  return getRuleContexts<LSystemParserParser::SymDstContext>();
}

LSystemParserParser::SymDstContext* LSystemParserParser::SymDstsContext::symDst(size_t i) {
  return getRuleContext<LSystemParserParser::SymDstContext>(i);
}


size_t LSystemParserParser::SymDstsContext::getRuleIndex() const {
  return LSystemParserParser::RuleSymDsts;
}


std::any LSystemParserParser::SymDstsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitSymDsts(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::SymDstsContext* LSystemParserParser::symDsts() {
  SymDstsContext *_localctx = _tracker.createInstance<SymDstsContext>(_ctx, getState());
  enterRule(_localctx, 8, LSystemParserParser::RuleSymDsts);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(56); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(55);
              symDst();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(58); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymDstContext ------------------------------------------------------------------

LSystemParserParser::SymDstContext::SymDstContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LSystemParserParser::SymNameContext* LSystemParserParser::SymDstContext::symName() {
  return getRuleContext<LSystemParserParser::SymNameContext>(0);
}

LSystemParserParser::ParamMapsContext* LSystemParserParser::SymDstContext::paramMaps() {
  return getRuleContext<LSystemParserParser::ParamMapsContext>(0);
}

tree::TerminalNode* LSystemParserParser::SymDstContext::Punct() {
  return getToken(LSystemParserParser::Punct, 0);
}


size_t LSystemParserParser::SymDstContext::getRuleIndex() const {
  return LSystemParserParser::RuleSymDst;
}


std::any LSystemParserParser::SymDstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitSymDst(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::SymDstContext* LSystemParserParser::symDst() {
  SymDstContext *_localctx = _tracker.createInstance<SymDstContext>(_ctx, getState());
  enterRule(_localctx, 10, LSystemParserParser::RuleSymDst);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(66);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LSystemParserParser::String: {
        enterOuterAlt(_localctx, 1);
        setState(60);
        symName();
        setState(61);
        match(LSystemParserParser::T__0);
        setState(62);
        paramMaps();
        setState(63);
        match(LSystemParserParser::T__1);
        break;
      }

      case LSystemParserParser::Punct: {
        enterOuterAlt(_localctx, 2);
        setState(65);
        match(LSystemParserParser::Punct);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParamMapsContext ------------------------------------------------------------------

LSystemParserParser::ParamMapsContext::ParamMapsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LSystemParserParser::MathExprContext *> LSystemParserParser::ParamMapsContext::mathExpr() {
  return getRuleContexts<LSystemParserParser::MathExprContext>();
}

LSystemParserParser::MathExprContext* LSystemParserParser::ParamMapsContext::mathExpr(size_t i) {
  return getRuleContext<LSystemParserParser::MathExprContext>(i);
}


size_t LSystemParserParser::ParamMapsContext::getRuleIndex() const {
  return LSystemParserParser::RuleParamMaps;
}


std::any LSystemParserParser::ParamMapsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitParamMaps(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::ParamMapsContext* LSystemParserParser::paramMaps() {
  ParamMapsContext *_localctx = _tracker.createInstance<ParamMapsContext>(_ctx, getState());
  enterRule(_localctx, 12, LSystemParserParser::RuleParamMaps);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(68);
    mathExpr(0);
    setState(73);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LSystemParserParser::T__2) {
      setState(69);
      match(LSystemParserParser::T__2);
      setState(70);
      mathExpr(0);
      setState(75);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MathExprContext ------------------------------------------------------------------

LSystemParserParser::MathExprContext::MathExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LSystemParserParser::MathItemContext* LSystemParserParser::MathExprContext::mathItem() {
  return getRuleContext<LSystemParserParser::MathItemContext>(0);
}

LSystemParserParser::MathExprContext* LSystemParserParser::MathExprContext::mathExpr() {
  return getRuleContext<LSystemParserParser::MathExprContext>(0);
}

tree::TerminalNode* LSystemParserParser::MathExprContext::Plus() {
  return getToken(LSystemParserParser::Plus, 0);
}

tree::TerminalNode* LSystemParserParser::MathExprContext::Minus() {
  return getToken(LSystemParserParser::Minus, 0);
}


size_t LSystemParserParser::MathExprContext::getRuleIndex() const {
  return LSystemParserParser::RuleMathExpr;
}


std::any LSystemParserParser::MathExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitMathExpr(this);
  else
    return visitor->visitChildren(this);
}


LSystemParserParser::MathExprContext* LSystemParserParser::mathExpr() {
   return mathExpr(0);
}

LSystemParserParser::MathExprContext* LSystemParserParser::mathExpr(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  LSystemParserParser::MathExprContext *_localctx = _tracker.createInstance<MathExprContext>(_ctx, parentState);
  LSystemParserParser::MathExprContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 14;
  enterRecursionRule(_localctx, 14, LSystemParserParser::RuleMathExpr, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(77);
    mathItem(0);
    _ctx->stop = _input->LT(-1);
    setState(84);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<MathExprContext>(parentContext, parentState);
        _localctx->left = previousContext;
        pushNewRecursionContext(_localctx, startState, RuleMathExpr);
        setState(79);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(80);
        antlrcpp::downCast<MathExprContext *>(_localctx)->op = _input->LT(1);
        _la = _input->LA(1);
        if (!(_la == LSystemParserParser::Plus

        || _la == LSystemParserParser::Minus)) {
          antlrcpp::downCast<MathExprContext *>(_localctx)->op = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(81);
        antlrcpp::downCast<MathExprContext *>(_localctx)->right = mathItem(0); 
      }
      setState(86);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- MathItemContext ------------------------------------------------------------------

LSystemParserParser::MathItemContext::MathItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LSystemParserParser::MathFactorContext* LSystemParserParser::MathItemContext::mathFactor() {
  return getRuleContext<LSystemParserParser::MathFactorContext>(0);
}

LSystemParserParser::MathItemContext* LSystemParserParser::MathItemContext::mathItem() {
  return getRuleContext<LSystemParserParser::MathItemContext>(0);
}

tree::TerminalNode* LSystemParserParser::MathItemContext::Mul() {
  return getToken(LSystemParserParser::Mul, 0);
}

tree::TerminalNode* LSystemParserParser::MathItemContext::Div() {
  return getToken(LSystemParserParser::Div, 0);
}


size_t LSystemParserParser::MathItemContext::getRuleIndex() const {
  return LSystemParserParser::RuleMathItem;
}


std::any LSystemParserParser::MathItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitMathItem(this);
  else
    return visitor->visitChildren(this);
}


LSystemParserParser::MathItemContext* LSystemParserParser::mathItem() {
   return mathItem(0);
}

LSystemParserParser::MathItemContext* LSystemParserParser::mathItem(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  LSystemParserParser::MathItemContext *_localctx = _tracker.createInstance<MathItemContext>(_ctx, parentState);
  LSystemParserParser::MathItemContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 16;
  enterRecursionRule(_localctx, 16, LSystemParserParser::RuleMathItem, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(88);
    mathFactor();
    _ctx->stop = _input->LT(-1);
    setState(95);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<MathItemContext>(parentContext, parentState);
        _localctx->left = previousContext;
        pushNewRecursionContext(_localctx, startState, RuleMathItem);
        setState(90);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(91);
        antlrcpp::downCast<MathItemContext *>(_localctx)->op = _input->LT(1);
        _la = _input->LA(1);
        if (!(_la == LSystemParserParser::Mul

        || _la == LSystemParserParser::Div)) {
          antlrcpp::downCast<MathItemContext *>(_localctx)->op = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(92);
        antlrcpp::downCast<MathItemContext *>(_localctx)->right = mathFactor(); 
      }
      setState(97);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- MathFactorContext ------------------------------------------------------------------

LSystemParserParser::MathFactorContext::MathFactorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemParserParser::MathFactorContext::Number() {
  return getToken(LSystemParserParser::Number, 0);
}

tree::TerminalNode* LSystemParserParser::MathFactorContext::Plus() {
  return getToken(LSystemParserParser::Plus, 0);
}

tree::TerminalNode* LSystemParserParser::MathFactorContext::Minus() {
  return getToken(LSystemParserParser::Minus, 0);
}

LSystemParserParser::SymContext* LSystemParserParser::MathFactorContext::sym() {
  return getRuleContext<LSystemParserParser::SymContext>(0);
}

LSystemParserParser::MathExprContext* LSystemParserParser::MathFactorContext::mathExpr() {
  return getRuleContext<LSystemParserParser::MathExprContext>(0);
}


size_t LSystemParserParser::MathFactorContext::getRuleIndex() const {
  return LSystemParserParser::RuleMathFactor;
}


std::any LSystemParserParser::MathFactorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitMathFactor(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::MathFactorContext* LSystemParserParser::mathFactor() {
  MathFactorContext *_localctx = _tracker.createInstance<MathFactorContext>(_ctx, getState());
  enterRule(_localctx, 18, LSystemParserParser::RuleMathFactor);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(110);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(99);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LSystemParserParser::Plus

      || _la == LSystemParserParser::Minus) {
        setState(98);
        antlrcpp::downCast<MathFactorContext *>(_localctx)->sign = _input->LT(1);
        _la = _input->LA(1);
        if (!(_la == LSystemParserParser::Plus

        || _la == LSystemParserParser::Minus)) {
          antlrcpp::downCast<MathFactorContext *>(_localctx)->sign = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
      }
      setState(101);
      match(LSystemParserParser::Number);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(103);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LSystemParserParser::Plus

      || _la == LSystemParserParser::Minus) {
        setState(102);
        antlrcpp::downCast<MathFactorContext *>(_localctx)->sign = _input->LT(1);
        _la = _input->LA(1);
        if (!(_la == LSystemParserParser::Plus

        || _la == LSystemParserParser::Minus)) {
          antlrcpp::downCast<MathFactorContext *>(_localctx)->sign = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
      }
      setState(105);
      sym();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(106);
      match(LSystemParserParser::T__0);
      setState(107);
      mathExpr(0);
      setState(108);
      match(LSystemParserParser::T__1);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymNameContext ------------------------------------------------------------------

LSystemParserParser::SymNameContext::SymNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemParserParser::SymNameContext::String() {
  return getToken(LSystemParserParser::String, 0);
}


size_t LSystemParserParser::SymNameContext::getRuleIndex() const {
  return LSystemParserParser::RuleSymName;
}


std::any LSystemParserParser::SymNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitSymName(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::SymNameContext* LSystemParserParser::symName() {
  SymNameContext *_localctx = _tracker.createInstance<SymNameContext>(_ctx, getState());
  enterRule(_localctx, 20, LSystemParserParser::RuleSymName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(112);
    match(LSystemParserParser::String);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymContext ------------------------------------------------------------------

LSystemParserParser::SymContext::SymContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemParserParser::SymContext::String() {
  return getToken(LSystemParserParser::String, 0);
}


size_t LSystemParserParser::SymContext::getRuleIndex() const {
  return LSystemParserParser::RuleSym;
}


std::any LSystemParserParser::SymContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemParserVisitor*>(visitor))
    return parserVisitor->visitSym(this);
  else
    return visitor->visitChildren(this);
}

LSystemParserParser::SymContext* LSystemParserParser::sym() {
  SymContext *_localctx = _tracker.createInstance<SymContext>(_ctx, getState());
  enterRule(_localctx, 22, LSystemParserParser::RuleSym);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(114);
    match(LSystemParserParser::String);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool LSystemParserParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 7: return mathExprSempred(antlrcpp::downCast<MathExprContext *>(context), predicateIndex);
    case 8: return mathItemSempred(antlrcpp::downCast<MathItemContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool LSystemParserParser::mathExprSempred(MathExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 2);

  default:
    break;
  }
  return true;
}

bool LSystemParserParser::mathItemSempred(MathItemContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 1: return precpred(_ctx, 2);

  default:
    break;
  }
  return true;
}

void LSystemParserParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  lsystemparserParserInitialize();
#else
  ::antlr4::internal::call_once(lsystemparserParserOnceFlag, lsystemparserParserInitialize);
#endif
}
