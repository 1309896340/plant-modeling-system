
// Generated from LSystemRewriter.g4 by ANTLR 4.13.2


#include "LSystemRewriterVisitor.h"

#include "LSystemRewriterParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct LSystemRewriterParserStaticData final {
  LSystemRewriterParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  LSystemRewriterParserStaticData(const LSystemRewriterParserStaticData&) = delete;
  LSystemRewriterParserStaticData(LSystemRewriterParserStaticData&&) = delete;
  LSystemRewriterParserStaticData& operator=(const LSystemRewriterParserStaticData&) = delete;
  LSystemRewriterParserStaticData& operator=(LSystemRewriterParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag lsystemrewriterParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<LSystemRewriterParserStaticData> lsystemrewriterParserStaticData = nullptr;

void lsystemrewriterParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (lsystemrewriterParserStaticData != nullptr) {
    return;
  }
#else
  assert(lsystemrewriterParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<LSystemRewriterParserStaticData>(
    std::vector<std::string>{
      "symSeq", "sym", "symName", "symArgs", "symArg"
    },
    std::vector<std::string>{
      "", "'('", "')'", "','", "'\\n'"
    },
    std::vector<std::string>{
      "", "", "", "", "Newline", "Punct", "String", "Number"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,7,40,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,1,0,4,0,12,8,0,11,0,
  	12,0,13,1,0,1,0,1,1,1,1,1,1,3,1,21,8,1,1,1,1,1,1,1,3,1,26,8,1,1,2,1,2,
  	1,3,1,3,1,3,5,3,33,8,3,10,3,12,3,36,9,3,1,4,1,4,1,4,0,0,5,0,2,4,6,8,0,
  	0,38,0,11,1,0,0,0,2,25,1,0,0,0,4,27,1,0,0,0,6,29,1,0,0,0,8,37,1,0,0,0,
  	10,12,3,2,1,0,11,10,1,0,0,0,12,13,1,0,0,0,13,11,1,0,0,0,13,14,1,0,0,0,
  	14,15,1,0,0,0,15,16,5,0,0,1,16,1,1,0,0,0,17,18,3,4,2,0,18,20,5,1,0,0,
  	19,21,3,6,3,0,20,19,1,0,0,0,20,21,1,0,0,0,21,22,1,0,0,0,22,23,5,2,0,0,
  	23,26,1,0,0,0,24,26,5,5,0,0,25,17,1,0,0,0,25,24,1,0,0,0,26,3,1,0,0,0,
  	27,28,5,6,0,0,28,5,1,0,0,0,29,34,3,8,4,0,30,31,5,3,0,0,31,33,3,8,4,0,
  	32,30,1,0,0,0,33,36,1,0,0,0,34,32,1,0,0,0,34,35,1,0,0,0,35,7,1,0,0,0,
  	36,34,1,0,0,0,37,38,5,7,0,0,38,9,1,0,0,0,4,13,20,25,34
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  lsystemrewriterParserStaticData = std::move(staticData);
}

}

LSystemRewriterParser::LSystemRewriterParser(TokenStream *input) : LSystemRewriterParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

LSystemRewriterParser::LSystemRewriterParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  LSystemRewriterParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *lsystemrewriterParserStaticData->atn, lsystemrewriterParserStaticData->decisionToDFA, lsystemrewriterParserStaticData->sharedContextCache, options);
}

LSystemRewriterParser::~LSystemRewriterParser() {
  delete _interpreter;
}

const atn::ATN& LSystemRewriterParser::getATN() const {
  return *lsystemrewriterParserStaticData->atn;
}

std::string LSystemRewriterParser::getGrammarFileName() const {
  return "LSystemRewriter.g4";
}

const std::vector<std::string>& LSystemRewriterParser::getRuleNames() const {
  return lsystemrewriterParserStaticData->ruleNames;
}

const dfa::Vocabulary& LSystemRewriterParser::getVocabulary() const {
  return lsystemrewriterParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView LSystemRewriterParser::getSerializedATN() const {
  return lsystemrewriterParserStaticData->serializedATN;
}


//----------------- SymSeqContext ------------------------------------------------------------------

LSystemRewriterParser::SymSeqContext::SymSeqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemRewriterParser::SymSeqContext::EOF() {
  return getToken(LSystemRewriterParser::EOF, 0);
}

std::vector<LSystemRewriterParser::SymContext *> LSystemRewriterParser::SymSeqContext::sym() {
  return getRuleContexts<LSystemRewriterParser::SymContext>();
}

LSystemRewriterParser::SymContext* LSystemRewriterParser::SymSeqContext::sym(size_t i) {
  return getRuleContext<LSystemRewriterParser::SymContext>(i);
}


size_t LSystemRewriterParser::SymSeqContext::getRuleIndex() const {
  return LSystemRewriterParser::RuleSymSeq;
}


std::any LSystemRewriterParser::SymSeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemRewriterVisitor*>(visitor))
    return parserVisitor->visitSymSeq(this);
  else
    return visitor->visitChildren(this);
}

LSystemRewriterParser::SymSeqContext* LSystemRewriterParser::symSeq() {
  SymSeqContext *_localctx = _tracker.createInstance<SymSeqContext>(_ctx, getState());
  enterRule(_localctx, 0, LSystemRewriterParser::RuleSymSeq);
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
    setState(11); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(10);
      sym();
      setState(13); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == LSystemRewriterParser::Punct

    || _la == LSystemRewriterParser::String);
    setState(15);
    match(LSystemRewriterParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymContext ------------------------------------------------------------------

LSystemRewriterParser::SymContext::SymContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LSystemRewriterParser::SymNameContext* LSystemRewriterParser::SymContext::symName() {
  return getRuleContext<LSystemRewriterParser::SymNameContext>(0);
}

LSystemRewriterParser::SymArgsContext* LSystemRewriterParser::SymContext::symArgs() {
  return getRuleContext<LSystemRewriterParser::SymArgsContext>(0);
}

tree::TerminalNode* LSystemRewriterParser::SymContext::Punct() {
  return getToken(LSystemRewriterParser::Punct, 0);
}


size_t LSystemRewriterParser::SymContext::getRuleIndex() const {
  return LSystemRewriterParser::RuleSym;
}


std::any LSystemRewriterParser::SymContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemRewriterVisitor*>(visitor))
    return parserVisitor->visitSym(this);
  else
    return visitor->visitChildren(this);
}

LSystemRewriterParser::SymContext* LSystemRewriterParser::sym() {
  SymContext *_localctx = _tracker.createInstance<SymContext>(_ctx, getState());
  enterRule(_localctx, 2, LSystemRewriterParser::RuleSym);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(25);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LSystemRewriterParser::String: {
        enterOuterAlt(_localctx, 1);
        setState(17);
        symName();
        setState(18);
        match(LSystemRewriterParser::T__0);
        setState(20);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LSystemRewriterParser::Number) {
          setState(19);
          symArgs();
        }
        setState(22);
        match(LSystemRewriterParser::T__1);
        break;
      }

      case LSystemRewriterParser::Punct: {
        enterOuterAlt(_localctx, 2);
        setState(24);
        match(LSystemRewriterParser::Punct);
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

//----------------- SymNameContext ------------------------------------------------------------------

LSystemRewriterParser::SymNameContext::SymNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemRewriterParser::SymNameContext::String() {
  return getToken(LSystemRewriterParser::String, 0);
}


size_t LSystemRewriterParser::SymNameContext::getRuleIndex() const {
  return LSystemRewriterParser::RuleSymName;
}


std::any LSystemRewriterParser::SymNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemRewriterVisitor*>(visitor))
    return parserVisitor->visitSymName(this);
  else
    return visitor->visitChildren(this);
}

LSystemRewriterParser::SymNameContext* LSystemRewriterParser::symName() {
  SymNameContext *_localctx = _tracker.createInstance<SymNameContext>(_ctx, getState());
  enterRule(_localctx, 4, LSystemRewriterParser::RuleSymName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(27);
    match(LSystemRewriterParser::String);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymArgsContext ------------------------------------------------------------------

LSystemRewriterParser::SymArgsContext::SymArgsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LSystemRewriterParser::SymArgContext *> LSystemRewriterParser::SymArgsContext::symArg() {
  return getRuleContexts<LSystemRewriterParser::SymArgContext>();
}

LSystemRewriterParser::SymArgContext* LSystemRewriterParser::SymArgsContext::symArg(size_t i) {
  return getRuleContext<LSystemRewriterParser::SymArgContext>(i);
}


size_t LSystemRewriterParser::SymArgsContext::getRuleIndex() const {
  return LSystemRewriterParser::RuleSymArgs;
}


std::any LSystemRewriterParser::SymArgsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemRewriterVisitor*>(visitor))
    return parserVisitor->visitSymArgs(this);
  else
    return visitor->visitChildren(this);
}

LSystemRewriterParser::SymArgsContext* LSystemRewriterParser::symArgs() {
  SymArgsContext *_localctx = _tracker.createInstance<SymArgsContext>(_ctx, getState());
  enterRule(_localctx, 6, LSystemRewriterParser::RuleSymArgs);
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
    setState(29);
    symArg();
    setState(34);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LSystemRewriterParser::T__2) {
      setState(30);
      match(LSystemRewriterParser::T__2);
      setState(31);
      symArg();
      setState(36);
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

//----------------- SymArgContext ------------------------------------------------------------------

LSystemRewriterParser::SymArgContext::SymArgContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemRewriterParser::SymArgContext::Number() {
  return getToken(LSystemRewriterParser::Number, 0);
}


size_t LSystemRewriterParser::SymArgContext::getRuleIndex() const {
  return LSystemRewriterParser::RuleSymArg;
}


std::any LSystemRewriterParser::SymArgContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemRewriterVisitor*>(visitor))
    return parserVisitor->visitSymArg(this);
  else
    return visitor->visitChildren(this);
}

LSystemRewriterParser::SymArgContext* LSystemRewriterParser::symArg() {
  SymArgContext *_localctx = _tracker.createInstance<SymArgContext>(_ctx, getState());
  enterRule(_localctx, 8, LSystemRewriterParser::RuleSymArg);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(37);
    match(LSystemRewriterParser::Number);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void LSystemRewriterParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  lsystemrewriterParserInitialize();
#else
  ::antlr4::internal::call_once(lsystemrewriterParserOnceFlag, lsystemrewriterParserInitialize);
#endif
}
