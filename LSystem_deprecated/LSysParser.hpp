#include "LSystem.h"
#include "lexy/dsl/ascii.hpp"
#include "lexy/dsl/brackets.hpp"
#include "lexy/dsl/option.hpp"

#define DEBUG

#ifndef __WIND_LSYSEXPR
#    define __WIND_LSYSEXPR

#    include "MathExprParser.hpp"

namespace LSysParser {
using namespace std;

namespace exception {
class Unknown_SymDstType : public std::exception {
    string message;

public:
    Unknown_SymDstType() { this->message = "Unkown SymDst Type"; }
    virtual const char* what() const noexcept override {
        return this->message.c_str();
    }
};

class Unknown_LProdCallType : public std::exception {
    string message;

public:
    Unknown_LProdCallType() { this->message = "Unkown LProdCall Type"; }
    virtual const char* what() const noexcept override {
        return this->message.c_str();
    }
};

};   // namespace exception

namespace config {
struct SymSrc {
    string         name;
    vector<string> params;
    SymSrc() = default;
    SymSrc(string name, vector<string> paramNames)
        : name(LEXY_MOV(name))
        , params(LEXY_MOV(paramNames)) {}
};

// struct SymDst {
//     string name;
//     vector<MathParser::ast::expr_ptr> paramMaps;
//     SymDst() = default;
// };

// struct SymDstMiddle {
//     // middle layer for distinguishing real SymDst and ControlSym
//     enum {
//         SYMDST,
//         CONTROLSYM
//     } stype;
//     SymDst symdst;
//     string ctrlsym;
//     SymDstMiddle(SymDst sdst)
//         : stype(SYMDST), symdst(sdst), ctrlsym() {}
//     SymDstMiddle(string cs)
//         : stype(CONTROLSYM), symdst(), ctrlsym(cs) {}
// };

struct SymDst {
    enum SType { MAP_SYM,
                 CONTROL_SYM } stype;
    SymDst(SType st)
        : stype(st) {}
};

// MapSym和原本的SymDst结构保持一致
struct MapSym : public SymDst {
    string                                      name;
    optional<vector<MathParser::ast::expr_ptr>> paramMaps;   // 可以允许SymDst不带参数
    MapSym()
        : SymDst(MAP_SYM) {}
    MapSym(string name, optional<vector<MathParser::ast::expr_ptr>> paramMaps)
        : name(LEXY_MOV(name))
        , paramMaps(LEXY_MOV(paramMaps))
        , SymDst(MAP_SYM) {}
};

// ControlSym则只保存一个控制字符
struct ControlSym : public SymDst {
    char ctrlChar;
    ControlSym()
        : SymDst(CONTROL_SYM) {}
    ControlSym(string cc)
        : SymDst(CONTROL_SYM)
        , ctrlChar(cc[0]) {}
};

struct LProduction {
    SymSrc                     sSrc;
    vector<shared_ptr<SymDst>> sDsts;
    LProduction() = default;
    LProduction(SymSrc sSrc, vector<shared_ptr<SymDst>> sDsts)
        : sSrc(LEXY_MOV(sSrc))
        , sDsts(LEXY_MOV(sDsts)) {}
};

struct LSystem {
    map<string, LProduction> prods;
    LSystem() = default;
    LSystem(map<string, LProduction> prods)
        : prods(LEXY_MOV(prods)) {}
    LSystem(vector<LProduction> prods)
        : prods() {
        for (const LProduction& prod : prods)
            this->prods[prod.sSrc.name] = prod;
    }
};

struct LProdCall {
    enum PType { CALL_SYM,
                 CONTROL_SYM } ptype;
    LProdCall(PType ptype)
        : ptype(ptype) {}
};

struct LControlSym : public LProdCall {
    char ctrlChar;
    LControlSym(string cc)
        : ctrlChar(cc[0])
        , LProdCall(CONTROL_SYM) {}
};

struct LCallSym : public LProdCall {
    string        name;
    vector<float> args;   // 在构造时进行计算
    LCallSym(string name, vector<MathParser::ast::expr_ptr> args_expr)
        : name(LEXY_MOV(name))
        , args()
        , LProdCall(CALL_SYM) {

        // 初始化中计算表达式值
        MathParser::ast::Environment env;
        for (auto& expr : args_expr)
            args.push_back(expr->evaluate(env));   // 计算出实参并入栈
    }

    string apply(const config::LSystem& lsys) const {
        stringstream ss;

        auto iter = lsys.prods.find(this->name);
        if (iter == lsys.prods.end()) {
            ss << this->name << '(';
            for (float arg : this->args)
                ss << arg << ',';
            ss.seekp(-1, ss.cur);
            ss << ')';

            return ss.str();
        }
        // 找到对应的LProduction
        const config::LProduction& lprod = iter->second;
        // 找到形参名列表
        const vector<string>& paramNames = lprod.sSrc.params;
        // 检查形参与实参的数量
        const int &argNum = this->args.size(), &paramNum = paramNames.size();
        // 创建ParamMap的局部作用域MathParser::ast::Environment，并初始化传入参数。这里只考虑值传递，不实现引用传递
        MathParser::ast::Environment env;
        auto                         p_iter = paramNames.begin(), p_end = paramNames.end();
        auto                         a_iter = this->args.begin(), a_end = this->args.end();
        while (p_iter != p_end) {
            if (a_iter == a_end) {
                // 填充arg不足param的部分
                env.vars[*p_iter] = 0.0f;
            }
            else {
                // 相同数量按顺序传递
                env.vars[*p_iter] = *a_iter;
                a_iter++;
            }
            p_iter++;
            // 忽略arg超出param的部分
        }

        for (const shared_ptr<config::SymDst>& sDst : lprod.sDsts) {
            switch (sDst->stype) {
            case config::SymDst::MAP_SYM: {
                const config::MapSym* sd =
                    static_cast<config::MapSym*>(sDst.get());
                ss << sd->name;
                if (sd->paramMaps.has_value()) {
                    ss << '(';
                    for (const MathParser::ast::expr_ptr& paramExpr : sd->paramMaps.value())
                        ss << paramExpr->evaluate(env) << ',';
                    ss.seekp(-1, ss.cur);
                    ss << ')';
                }
                break;
            }
            case config::SymDst::CONTROL_SYM: {
                const config::ControlSym* sd =
                    static_cast<config::ControlSym*>(sDst.get());
                const char& ctrlSym = sd->ctrlChar;
                ss << ctrlSym;
                break;
            }
            default:
                throw exception::Unknown_SymDstType();
            }
        }

        return ss.str();
    }
};

struct LSysCall {
    vector<shared_ptr<LProdCall>> calls;
    LSysCall(vector<shared_ptr<LProdCall>> calls)
        : calls(LEXY_MOV(calls)) {}
    string apply(const LSystem& lsys) const {
        stringstream ss;
        for (const shared_ptr<config::LProdCall>& call : calls) {
            switch (call->ptype) {
            case config::LProdCall::CALL_SYM: {
                const LCallSym* callsym = static_cast<LCallSym*>(call.get());
                ss << callsym->apply(lsys);
                break;
            }
            case config::LProdCall::CONTROL_SYM: {
                const LControlSym* ctrlsym = static_cast<LControlSym*>(call.get());
                ss << ctrlsym->ctrlChar;
                break;
            }
            default:
                throw exception::Unknown_LProdCallType();
            }
        }
        return ss.str();
    }
};

};   // namespace config

namespace grammar {
namespace dsl = lexy::dsl;

struct SymName {
    static constexpr auto rule =
        dsl::identifier(dsl::ascii::alpha, dsl::ascii::alpha_digit_underscore);
    static constexpr auto value = lexy::as_string<string>;
};

struct ArgExpr {
    static constexpr auto rule  = dsl::p<MathParser::grammar::MathExpr>;
    static constexpr auto value = lexy::construct<MathParser::ast::expr_ptr>;
};

struct ArgList {
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule =
        dsl::parenthesized.list(dsl::p<ArgExpr>, dsl::sep(dsl::comma));
    static constexpr auto value =
        lexy::as_list<vector<MathParser::ast::expr_ptr>>;   // 在构造的过程中被计算
};

struct LCallSym {
    static constexpr auto rule  = dsl::p<SymName> + dsl::p<ArgList>;   // 这里暂时规定SymName后面即便没有参数也必须带括号
    static constexpr auto value = lexy::construct<config::LCallSym>;
};

struct LControlSym {
    static constexpr auto rule  = dsl::capture(dsl::ascii::punct);
    static constexpr auto value = lexy::as_string<string>;
};

struct LProdCall {
    static constexpr auto rule = [] {
        auto callsym = dsl::peek(dsl::p<SymName>) >> dsl::p<LCallSym>;
        auto ctrlsym = dsl::p<LControlSym>;
        return callsym | dsl::else_ >> ctrlsym;
    }();
    static constexpr auto value = lexy::callback<shared_ptr<config::LProdCall>>(
        lexy::new_<config::LCallSym, shared_ptr<config::LProdCall>>,
        lexy::new_<config::LControlSym, shared_ptr<config::LProdCall>>);
};

struct LProdCallList {
    // static constexpr auto whitespace = dsl::ascii::blank;
    static constexpr auto rule =
        dsl::list((dsl::peek(dsl::p<SymName>) | dsl::peek(dsl::p<LControlSym>)) >>
                  dsl::p<LProdCall>);
    static constexpr auto value =
        lexy::as_list<vector<shared_ptr<config::LProdCall>>>;
};

struct LSysCall {
    // static constexpr auto whitespace = dsl::ascii::blank;
    static constexpr auto rule       = dsl::p<LProdCallList>;
    static constexpr auto value      = lexy::construct<config::LSysCall>;
};

struct Param {
    static constexpr auto rule =
        dsl::identifier(dsl::ascii::alpha, dsl::ascii::alpha_digit_underscore);
    static constexpr auto value = lexy::as_string<string>;
};

struct ParamList {
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule =
        dsl::parenthesized.list(dsl::p<Param>, dsl::sep(dsl::comma));
    static constexpr auto value = lexy::as_list<vector<string>>;
};

struct SymSrc {
    static constexpr auto rule  = dsl::p<SymName> + dsl::p<ParamList>;
    static constexpr auto value = lexy::construct<config::SymSrc>;
};

struct ParamMap {
    static constexpr auto rule  = dsl::p<MathParser::grammar::MathExpr>;
    static constexpr auto value = lexy::forward<MathParser::ast::expr_ptr>;
};

struct ParamMapList {
    static constexpr auto whitespace = dsl::ascii::blank;
    static constexpr auto rule =
        dsl::parenthesized.list(dsl::p<ParamMap>, dsl::sep(dsl::comma));
    static constexpr auto value =
        lexy::as_list<vector<MathParser::ast::expr_ptr>>;
};

struct MapSym {
    static constexpr auto rule  = dsl::p<SymName> + dsl::opt(dsl::peek(dsl::lit_c<'('>) >> dsl::p<ParamMapList>);
    static constexpr auto value = lexy::construct<config::MapSym>;
};

struct ControlSym {
    static constexpr auto rule  = dsl::capture(dsl::ascii::punct);
    static constexpr auto value = lexy::as_string<string>;
};

struct SymDst {
    static constexpr auto rule = [] {
        auto mapSym  = dsl::peek(dsl::p<SymName>) >> dsl::p<MapSym>;
        auto ctrlSym = dsl::p<ControlSym>;
        return mapSym | dsl::else_ >> ctrlSym;
    }();
    static constexpr auto value = lexy::callback<shared_ptr<config::SymDst>>(
        lexy::new_<config::MapSym, shared_ptr<config::SymDst>>,
        lexy::new_<config::ControlSym, shared_ptr<config::SymDst>>);
};

struct SymDstList {
    // static constexpr auto whitespace = dsl::ascii::newline;
    static constexpr auto rule =
        dsl::list((dsl::peek(dsl::p<SymName>) | dsl::peek(dsl::p<ControlSym>)) >>
                  dsl::p<SymDst>);
    static constexpr auto value =
        lexy::as_list<vector<shared_ptr<config::SymDst>>>;
};

struct LProduction {
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule =
        dsl::p<SymSrc> + LEXY_LIT("->") + dsl::p<SymDstList>;
    static constexpr auto value = lexy::construct<config::LProduction>;
};

struct LSystem {
    // static constexpr auto whitespace = dsl::ascii::blank;
    static constexpr auto rule =
        dsl::list(dsl::p<LProduction>,
                  dsl::sep(dsl::newline));   // 通过换行来分割LProduction字符串
    static constexpr auto value = lexy::as_list<vector<config::LProduction>>;
};

};   // namespace grammar

};   // namespace LSysParser

#endif
