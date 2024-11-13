#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "lexy/action/parse.hpp"
#include "lexy/action/parse_as_tree.hpp"
#include "lexy/callback.hpp"
#include "lexy/dsl.hpp"
#include "lexy/input/string_input.hpp"
#include "lexy_ext/report_error.hpp"

#include "Geometry.hpp"
#include "Transform.hpp"

#include "MathExprParser.hpp"

// 该文件用于符号到几何图形的解析
// 输入：字符串。输出：Geometry或者其子类

namespace GeometryGenerator {
namespace dsl = lexy::dsl;

struct GraphicsContext{
    // 用于LProdCallList的操作中记录几何体生成的中间状态
    Transform transform;
};

namespace config {
struct LProdCall {
  // 原先在LSystem中派生LControlSym和LCallSym的父类
  // 现在将两个派生类改名为 GraphicsControlSym 和 GraphicsCallSym，以此来区分这里的调用并不是LSystem的并行重写，而是几何体生成
  enum PType { CALL_SYM,
               CONTROL_SYM } ptype;
  LProdCall(PType ptype) : ptype(ptype) {}
  // 子类必须实现各自执行的几何体生成操作细节
  virtual void exec(shared_ptr<GraphicsContext> context) = 0;
};

struct GraphicsControlSym : public LProdCall {
  char ctrl_char;
  GraphicsControlSym(string cc) : ctrl_char(cc[0]), LProdCall(CONTROL_SYM) {}

  virtual void exec(shared_ptr<GraphicsContext> context) {
    // 根据不同的控制符，执行对应操作
    // 这里涉及到组装一个几何体的操作过程，这个正在被组装的几何体应当可被 LProdCallList 对象访问和操作
    switch (ctrl_char) {
    case '[': {
      // 入栈操作

      break;
    }
    case ']': {
      // 出栈操作

      break;
    }
    }
  }
};

struct GraphicsCallSym : public LProdCall {
  string name;
  vector<float> args;
  GraphicsCallSym(string name, vector<MathParser::ast::expr_ptr> args_expr) : name(LEXY_MOV(name)), args(), LProdCall(CALL_SYM) {
    // 构造函数中计算表达式值
    MathParser::ast::Environment env;
    for (auto &expr : args_expr)
      args.push_back(expr->evaluate(env)); // 计算出实参并入栈
  }

  virtual void exec(shared_ptr<GraphicsContext> context) {
    // 根据name执行对应的几何体生成操作
  }
};

struct GraphicsStructure{
    shared_ptr<GraphicsContext> context{nullptr};
    vector<shared_ptr<LProdCall>> call_list;
    GraphicsStructure(vector<shared_ptr<LProdCall>> call_list):context(make_shared<GraphicsContext>()),call_list(LEXY_MOV(call_list)){}

    void construct(){
        for(shared_ptr<LProdCall> cmd : this->call_list)
            cmd->exec(context);
    }
};

}; // namespace config

namespace grammar {
struct SymName {
  static constexpr auto rule =
      dsl::identifier(dsl::ascii::alpha, dsl::ascii::alpha_digit_underscore);
  static constexpr auto value = lexy::as_string<string>;
};

struct LControlSym {
  static constexpr auto rule = dsl::capture(dsl::ascii::punct);
  static constexpr auto value = lexy::as_string<string>;
};

struct LProdCall {
  struct ArgExpr {
    static constexpr auto rule = dsl::p<MathParser::grammar::MathExpr>;
    static constexpr auto value = lexy::construct<MathParser::ast::expr_ptr>;
  };
  struct ArgList {
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule =
        dsl::parenthesized.list(dsl::p<ArgExpr>, dsl::sep(dsl::comma));
    static constexpr auto value =
        lexy::as_list<vector<MathParser::ast::expr_ptr>>; // 在构造的过程中被计算
  };
  struct LCallSym {
    static constexpr auto rule = dsl::p<SymName> + dsl::p<ArgList>; // 这里暂时规定SymName后面即便没有参数也必须带括号
    static constexpr auto value = lexy::construct<config::GraphicsCallSym>;
  };
  static constexpr auto rule = [] {
    auto callsym = dsl::peek(dsl::p<SymName>) >> dsl::p<LCallSym>;
    auto ctrlsym = dsl::p<LControlSym>;
    return callsym | dsl::else_ >> ctrlsym;
  }();
  static constexpr auto value = lexy::callback<shared_ptr<config::LProdCall>>(
      lexy::new_<config::GraphicsCallSym, shared_ptr<config::LProdCall>>,
      lexy::new_<config::GraphicsControlSym, shared_ptr<config::LProdCall>>);
};

struct LProdCallList {
  static constexpr auto whitespace = dsl::ascii::blank;
  static constexpr auto rule =
      dsl::list((dsl::peek(dsl::p<SymName>) | dsl::peek(dsl::p<LControlSym>)) >>
                dsl::p<LProdCall>);
  static constexpr auto value =
      lexy::as_list<vector<shared_ptr<config::LProdCall>>>;
};

struct GraphicsStructure{
    static constexpr auto rule = dsl::p<LProdCallList>;
    static constexpr auto value = lexy::construct<config::GraphicsStructure>;
};

}; // namespace grammar
}; // namespace GeometryGenerator
