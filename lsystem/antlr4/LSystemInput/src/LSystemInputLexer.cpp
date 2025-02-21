
// Generated from LSystemInput.g4 by ANTLR 4.13.2


#include "LSystemInputLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct LSystemInputLexerStaticData final {
  LSystemInputLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  LSystemInputLexerStaticData(const LSystemInputLexerStaticData&) = delete;
  LSystemInputLexerStaticData(LSystemInputLexerStaticData&&) = delete;
  LSystemInputLexerStaticData& operator=(const LSystemInputLexerStaticData&) = delete;
  LSystemInputLexerStaticData& operator=(LSystemInputLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag lsysteminputlexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<LSystemInputLexerStaticData> lsysteminputlexerLexerStaticData = nullptr;

void lsysteminputlexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (lsysteminputlexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(lsysteminputlexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<LSystemInputLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "Newline", "Punct", "String", "Number"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'('", "')'", "','", "'\\n'"
    },
    std::vector<std::string>{
      "", "", "", "", "Newline", "Punct", "String", "Number"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,7,56,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,
  	1,0,1,0,1,1,1,1,1,2,1,2,1,3,1,3,1,4,1,4,1,5,4,5,27,8,5,11,5,12,5,28,1,
  	5,5,5,32,8,5,10,5,12,5,35,9,5,1,6,3,6,38,8,6,1,6,4,6,41,8,6,11,6,12,6,
  	42,1,6,1,6,5,6,47,8,6,10,6,12,6,50,9,6,3,6,52,8,6,1,6,3,6,55,8,6,0,0,
  	7,1,1,3,2,5,3,7,4,9,5,11,6,13,7,1,0,4,2,0,91,91,93,93,3,0,65,90,95,95,
  	97,122,1,0,48,57,2,0,43,43,45,45,62,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,
  	0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,1,15,1,0,0,0,3,17,
  	1,0,0,0,5,19,1,0,0,0,7,21,1,0,0,0,9,23,1,0,0,0,11,26,1,0,0,0,13,37,1,
  	0,0,0,15,16,5,40,0,0,16,2,1,0,0,0,17,18,5,41,0,0,18,4,1,0,0,0,19,20,5,
  	44,0,0,20,6,1,0,0,0,21,22,5,10,0,0,22,8,1,0,0,0,23,24,7,0,0,0,24,10,1,
  	0,0,0,25,27,7,1,0,0,26,25,1,0,0,0,27,28,1,0,0,0,28,26,1,0,0,0,28,29,1,
  	0,0,0,29,33,1,0,0,0,30,32,7,2,0,0,31,30,1,0,0,0,32,35,1,0,0,0,33,31,1,
  	0,0,0,33,34,1,0,0,0,34,12,1,0,0,0,35,33,1,0,0,0,36,38,7,3,0,0,37,36,1,
  	0,0,0,37,38,1,0,0,0,38,40,1,0,0,0,39,41,7,2,0,0,40,39,1,0,0,0,41,42,1,
  	0,0,0,42,40,1,0,0,0,42,43,1,0,0,0,43,51,1,0,0,0,44,48,5,46,0,0,45,47,
  	7,2,0,0,46,45,1,0,0,0,47,50,1,0,0,0,48,46,1,0,0,0,48,49,1,0,0,0,49,52,
  	1,0,0,0,50,48,1,0,0,0,51,44,1,0,0,0,51,52,1,0,0,0,52,54,1,0,0,0,53,55,
  	5,102,0,0,54,53,1,0,0,0,54,55,1,0,0,0,55,14,1,0,0,0,8,0,28,33,37,42,48,
  	51,54,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  lsysteminputlexerLexerStaticData = std::move(staticData);
}

}

LSystemInputLexer::LSystemInputLexer(CharStream *input) : Lexer(input) {
  LSystemInputLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *lsysteminputlexerLexerStaticData->atn, lsysteminputlexerLexerStaticData->decisionToDFA, lsysteminputlexerLexerStaticData->sharedContextCache);
}

LSystemInputLexer::~LSystemInputLexer() {
  delete _interpreter;
}

std::string LSystemInputLexer::getGrammarFileName() const {
  return "LSystemInput.g4";
}

const std::vector<std::string>& LSystemInputLexer::getRuleNames() const {
  return lsysteminputlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& LSystemInputLexer::getChannelNames() const {
  return lsysteminputlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& LSystemInputLexer::getModeNames() const {
  return lsysteminputlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& LSystemInputLexer::getVocabulary() const {
  return lsysteminputlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView LSystemInputLexer::getSerializedATN() const {
  return lsysteminputlexerLexerStaticData->serializedATN;
}

const atn::ATN& LSystemInputLexer::getATN() const {
  return *lsysteminputlexerLexerStaticData->atn;
}




void LSystemInputLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  lsysteminputlexerLexerInitialize();
#else
  ::antlr4::internal::call_once(lsysteminputlexerLexerOnceFlag, lsysteminputlexerLexerInitialize);
#endif
}
