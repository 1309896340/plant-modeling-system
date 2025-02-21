
// Generated from LSystemInput.g4 by ANTLR 4.13.2


#include "LSystemInputVisitor.h"

#include "LSystemInputParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct LSystemInputParserStaticData final {
  LSystemInputParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  LSystemInputParserStaticData(const LSystemInputParserStaticData&) = delete;
  LSystemInputParserStaticData(LSystemInputParserStaticData&&) = delete;
  LSystemInputParserStaticData& operator=(const LSystemInputParserStaticData&) = delete;
  LSystemInputParserStaticData& operator=(LSystemInputParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag lsysteminputParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<LSystemInputParserStaticData> lsysteminputParserStaticData = nullptr;

void lsysteminputParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (lsysteminputParserStaticData != nullptr) {
    return;
  }
#else
  assert(lsysteminputParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<LSystemInputParserStaticData>(
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
  lsysteminputParserStaticData = std::move(staticData);
}

}

LSystemInputParser::LSystemInputParser(TokenStream *input) : LSystemInputParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

LSystemInputParser::LSystemInputParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  LSystemInputParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *lsysteminputParserStaticData->atn, lsysteminputParserStaticData->decisionToDFA, lsysteminputParserStaticData->sharedContextCache, options);
}

LSystemInputParser::~LSystemInputParser() {
  delete _interpreter;
}

const atn::ATN& LSystemInputParser::getATN() const {
  return *lsysteminputParserStaticData->atn;
}

std::string LSystemInputParser::getGrammarFileName() const {
  return "LSystemInput.g4";
}

const std::vector<std::string>& LSystemInputParser::getRuleNames() const {
  return lsysteminputParserStaticData->ruleNames;
}

const dfa::Vocabulary& LSystemInputParser::getVocabulary() const {
  return lsysteminputParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView LSystemInputParser::getSerializedATN() const {
  return lsysteminputParserStaticData->serializedATN;
}


//----------------- SymSeqContext ------------------------------------------------------------------

LSystemInputParser::SymSeqContext::SymSeqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemInputParser::SymSeqContext::EOF() {
  return getToken(LSystemInputParser::EOF, 0);
}

std::vector<LSystemInputParser::SymContext *> LSystemInputParser::SymSeqContext::sym() {
  return getRuleContexts<LSystemInputParser::SymContext>();
}

LSystemInputParser::SymContext* LSystemInputParser::SymSeqContext::sym(size_t i) {
  return getRuleContext<LSystemInputParser::SymContext>(i);
}


size_t LSystemInputParser::SymSeqContext::getRuleIndex() const {
  return LSystemInputParser::RuleSymSeq;
}


std::any LSystemInputParser::SymSeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemInputVisitor*>(visitor))
    return parserVisitor->visitSymSeq(this);
  else
    return visitor->visitChildren(this);
}

LSystemInputParser::SymSeqContext* LSystemInputParser::symSeq() {
  SymSeqContext *_localctx = _tracker.createInstance<SymSeqContext>(_ctx, getState());
  enterRule(_localctx, 0, LSystemInputParser::RuleSymSeq);
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
    } while (_la == LSystemInputParser::Punct

    || _la == LSystemInputParser::String);
    setState(15);
    match(LSystemInputParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymContext ------------------------------------------------------------------

LSystemInputParser::SymContext::SymContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LSystemInputParser::SymNameContext* LSystemInputParser::SymContext::symName() {
  return getRuleContext<LSystemInputParser::SymNameContext>(0);
}

LSystemInputParser::SymArgsContext* LSystemInputParser::SymContext::symArgs() {
  return getRuleContext<LSystemInputParser::SymArgsContext>(0);
}

tree::TerminalNode* LSystemInputParser::SymContext::Punct() {
  return getToken(LSystemInputParser::Punct, 0);
}


size_t LSystemInputParser::SymContext::getRuleIndex() const {
  return LSystemInputParser::RuleSym;
}


std::any LSystemInputParser::SymContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemInputVisitor*>(visitor))
    return parserVisitor->visitSym(this);
  else
    return visitor->visitChildren(this);
}

LSystemInputParser::SymContext* LSystemInputParser::sym() {
  SymContext *_localctx = _tracker.createInstance<SymContext>(_ctx, getState());
  enterRule(_localctx, 2, LSystemInputParser::RuleSym);
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
      case LSystemInputParser::String: {
        enterOuterAlt(_localctx, 1);
        setState(17);
        symName();
        setState(18);
        match(LSystemInputParser::T__0);
        setState(20);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LSystemInputParser::Number) {
          setState(19);
          symArgs();
        }
        setState(22);
        match(LSystemInputParser::T__1);
        break;
      }

      case LSystemInputParser::Punct: {
        enterOuterAlt(_localctx, 2);
        setState(24);
        match(LSystemInputParser::Punct);
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

LSystemInputParser::SymNameContext::SymNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemInputParser::SymNameContext::String() {
  return getToken(LSystemInputParser::String, 0);
}


size_t LSystemInputParser::SymNameContext::getRuleIndex() const {
  return LSystemInputParser::RuleSymName;
}


std::any LSystemInputParser::SymNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemInputVisitor*>(visitor))
    return parserVisitor->visitSymName(this);
  else
    return visitor->visitChildren(this);
}

LSystemInputParser::SymNameContext* LSystemInputParser::symName() {
  SymNameContext *_localctx = _tracker.createInstance<SymNameContext>(_ctx, getState());
  enterRule(_localctx, 4, LSystemInputParser::RuleSymName);

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
    match(LSystemInputParser::String);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymArgsContext ------------------------------------------------------------------

LSystemInputParser::SymArgsContext::SymArgsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LSystemInputParser::SymArgContext *> LSystemInputParser::SymArgsContext::symArg() {
  return getRuleContexts<LSystemInputParser::SymArgContext>();
}

LSystemInputParser::SymArgContext* LSystemInputParser::SymArgsContext::symArg(size_t i) {
  return getRuleContext<LSystemInputParser::SymArgContext>(i);
}


size_t LSystemInputParser::SymArgsContext::getRuleIndex() const {
  return LSystemInputParser::RuleSymArgs;
}


std::any LSystemInputParser::SymArgsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemInputVisitor*>(visitor))
    return parserVisitor->visitSymArgs(this);
  else
    return visitor->visitChildren(this);
}

LSystemInputParser::SymArgsContext* LSystemInputParser::symArgs() {
  SymArgsContext *_localctx = _tracker.createInstance<SymArgsContext>(_ctx, getState());
  enterRule(_localctx, 6, LSystemInputParser::RuleSymArgs);
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
    while (_la == LSystemInputParser::T__2) {
      setState(30);
      match(LSystemInputParser::T__2);
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

LSystemInputParser::SymArgContext::SymArgContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LSystemInputParser::SymArgContext::Number() {
  return getToken(LSystemInputParser::Number, 0);
}


size_t LSystemInputParser::SymArgContext::getRuleIndex() const {
  return LSystemInputParser::RuleSymArg;
}


std::any LSystemInputParser::SymArgContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LSystemInputVisitor*>(visitor))
    return parserVisitor->visitSymArg(this);
  else
    return visitor->visitChildren(this);
}

LSystemInputParser::SymArgContext* LSystemInputParser::symArg() {
  SymArgContext *_localctx = _tracker.createInstance<SymArgContext>(_ctx, getState());
  enterRule(_localctx, 8, LSystemInputParser::RuleSymArg);

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
    match(LSystemInputParser::Number);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void LSystemInputParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  lsysteminputParserInitialize();
#else
  ::antlr4::internal::call_once(lsysteminputParserOnceFlag, lsysteminputParserInitialize);
#endif
}
