
// Generated from LSystemParser.g4 by ANTLR 4.13.2


#include "LSystemParserLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct LSystemParserLexerStaticData final {
  LSystemParserLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  LSystemParserLexerStaticData(const LSystemParserLexerStaticData&) = delete;
  LSystemParserLexerStaticData(LSystemParserLexerStaticData&&) = delete;
  LSystemParserLexerStaticData& operator=(const LSystemParserLexerStaticData&) = delete;
  LSystemParserLexerStaticData& operator=(LSystemParserLexerStaticData&&) = delete;

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

::antlr4::internal::OnceFlag lsystemparserlexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<LSystemParserLexerStaticData> lsystemparserlexerLexerStaticData = nullptr;

void lsystemparserlexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (lsystemparserlexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(lsystemparserlexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<LSystemParserLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "Plus", "Minus", "Mul", "Div", "RightMap", 
      "Newline", "Punct", "String", "Number"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
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
  	4,0,12,74,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,1,0,1,0,1,1,1,1,1,2,1,2,
  	1,3,1,3,1,4,1,4,1,5,1,5,1,6,1,6,1,7,1,7,1,7,1,8,1,8,1,9,1,9,1,10,4,10,
  	48,8,10,11,10,12,10,49,1,10,5,10,53,8,10,10,10,12,10,56,9,10,1,11,4,11,
  	59,8,11,11,11,12,11,60,1,11,1,11,5,11,65,8,11,10,11,12,11,68,9,11,3,11,
  	70,8,11,1,11,3,11,73,8,11,0,0,12,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,
  	9,19,10,21,11,23,12,1,0,3,2,0,91,91,93,93,3,0,65,90,95,95,97,122,1,0,
  	48,57,79,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,
  	0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,
  	1,0,0,0,0,23,1,0,0,0,1,25,1,0,0,0,3,27,1,0,0,0,5,29,1,0,0,0,7,31,1,0,
  	0,0,9,33,1,0,0,0,11,35,1,0,0,0,13,37,1,0,0,0,15,39,1,0,0,0,17,42,1,0,
  	0,0,19,44,1,0,0,0,21,47,1,0,0,0,23,58,1,0,0,0,25,26,5,40,0,0,26,2,1,0,
  	0,0,27,28,5,41,0,0,28,4,1,0,0,0,29,30,5,44,0,0,30,6,1,0,0,0,31,32,5,43,
  	0,0,32,8,1,0,0,0,33,34,5,45,0,0,34,10,1,0,0,0,35,36,5,42,0,0,36,12,1,
  	0,0,0,37,38,5,47,0,0,38,14,1,0,0,0,39,40,5,45,0,0,40,41,5,62,0,0,41,16,
  	1,0,0,0,42,43,5,10,0,0,43,18,1,0,0,0,44,45,7,0,0,0,45,20,1,0,0,0,46,48,
  	7,1,0,0,47,46,1,0,0,0,48,49,1,0,0,0,49,47,1,0,0,0,49,50,1,0,0,0,50,54,
  	1,0,0,0,51,53,7,2,0,0,52,51,1,0,0,0,53,56,1,0,0,0,54,52,1,0,0,0,54,55,
  	1,0,0,0,55,22,1,0,0,0,56,54,1,0,0,0,57,59,7,2,0,0,58,57,1,0,0,0,59,60,
  	1,0,0,0,60,58,1,0,0,0,60,61,1,0,0,0,61,69,1,0,0,0,62,66,5,46,0,0,63,65,
  	7,2,0,0,64,63,1,0,0,0,65,68,1,0,0,0,66,64,1,0,0,0,66,67,1,0,0,0,67,70,
  	1,0,0,0,68,66,1,0,0,0,69,62,1,0,0,0,69,70,1,0,0,0,70,72,1,0,0,0,71,73,
  	5,102,0,0,72,71,1,0,0,0,72,73,1,0,0,0,73,24,1,0,0,0,7,0,49,54,60,66,69,
  	72,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  lsystemparserlexerLexerStaticData = std::move(staticData);
}

}

LSystemParserLexer::LSystemParserLexer(CharStream *input) : Lexer(input) {
  LSystemParserLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *lsystemparserlexerLexerStaticData->atn, lsystemparserlexerLexerStaticData->decisionToDFA, lsystemparserlexerLexerStaticData->sharedContextCache);
}

LSystemParserLexer::~LSystemParserLexer() {
  delete _interpreter;
}

std::string LSystemParserLexer::getGrammarFileName() const {
  return "LSystemParser.g4";
}

const std::vector<std::string>& LSystemParserLexer::getRuleNames() const {
  return lsystemparserlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& LSystemParserLexer::getChannelNames() const {
  return lsystemparserlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& LSystemParserLexer::getModeNames() const {
  return lsystemparserlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& LSystemParserLexer::getVocabulary() const {
  return lsystemparserlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView LSystemParserLexer::getSerializedATN() const {
  return lsystemparserlexerLexerStaticData->serializedATN;
}

const atn::ATN& LSystemParserLexer::getATN() const {
  return *lsystemparserlexerLexerStaticData->atn;
}




void LSystemParserLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  lsystemparserlexerLexerInitialize();
#else
  ::antlr4::internal::call_once(lsystemparserlexerLexerOnceFlag, lsystemparserlexerLexerInitialize);
#endif
}
