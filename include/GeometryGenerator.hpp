#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Geometry.hpp"
#include "Skeleton.hpp"
#include "Transform.hpp"

// #include "MathExprParser.hpp"

// 该文件用于符号到几何图形的解析
// 输入：字符串。输出：Geometry或者其子类

namespace GeometryGenerator {
// namespace dsl = lexy::dsl;

struct GraphicsContext {
  // 用于LProdCallList的操作中的上下文
  Transform transform;
  SkNode *cur_node{nullptr};
  // shared_ptr<Geometry> cur_geometry{nullptr};  // 目前不涉及跨指令影响的Primitive生成，因此注释掉

  // 用以实现栈操作的上下文保存
  vector<Transform> backup_transforms;
  vector<SkNode *> backup_nodes;
  // Transform backup_transform;
  // SkNode *backup_node{nullptr};

  shared_ptr<Skeleton> skeleton{nullptr};

  GraphicsContext() {
    // skeleton伴随着上下文创建而创建，最终会被作为parse的结果传递出去，由引用计数自动释放
    this->skeleton = make_shared<Skeleton>();
    this->cur_node = this->skeleton->root; // 初始指向root节点
  }
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
      // 入栈操作(保存当前状态)
      context->backup_transforms.emplace_back(context->transform);
      context->backup_nodes.emplace_back(context->cur_node);
      break;
    }
    case ']': {
      // 出栈操作(恢复先前状态)
      if (context->backup_nodes.size() <= 0)
        printf("WARNNING: context backup_node is not nullptr! Maybe the brackets are incomplete.");

      context->transform = context->backup_transforms.back();
      context->backup_transforms.pop_back();
      context->cur_node = context->backup_nodes.back();
      context->backup_nodes.pop_back();
      break;
    }
    default:
      printf("unknown control symbol : \'%c\'\n", ctrl_char);
    }
  }
};

struct GraphicsCallSym : public LProdCall {
  string name;
  vector<float> args;
  // GraphicsCallSym(string name, vector<MathParser::ast::expr_ptr> args_expr) :
  // name(LEXY_MOV(name)), args(), LProdCall(CALL_SYM) {
  //   // 构造函数中计算表达式值
  //   MathParser::ast::Environment env;
  //   for (auto &expr : args_expr)
  //     args.push_back(expr->evaluate(env)); // 计算出实参并入栈
  // }

  virtual void exec(shared_ptr<GraphicsContext> context) {
    // 根据name执行对应的几何体生成操作
    // 暂时先用if else进行粗暴的实现

    if (name == "S") {
      // 构造SkNode
      SkNode *node = new SkNode();
      node->parent = context->cur_node;
      context->cur_node->addChild(node);
      context->cur_node = node;
      // 创建几何体
      shared_ptr<Sphere> geometry = make_shared<Sphere>(args[0]);
      context->cur_node->obj = static_pointer_cast<Geometry>(geometry);
      // 更新偏移量（球没有位置偏移）
      context->cur_node->setPosition(context->transform.getPosition());
      context->cur_node->setAttitude(context->transform.getAttitude());
      // 为新节点重置上下文的transform
      context->transform = Transform();
    } else if (name == "C") {
      // 构造新节点
      SkNode *node = new SkNode();
      node->parent = context->cur_node;
      context->cur_node->addChild(node);
      context->cur_node = node;
      // 创建几何体
      shared_ptr<Cylinder> geometry = make_shared<Cylinder>(args[0], args[1]);
      context->cur_node->obj = static_pointer_cast<Geometry>(geometry);
      // 更新偏移量
      context->cur_node->setAttitude(context->transform.getAttitude());
      context->cur_node->setPosition(context->transform.getPosition());
      // 为新节点重置上下文的transform
      context->transform = Transform();
      // 当前几何体的位置偏移将更新到下一个节点中
      vec3 offset = std::get<float>(geometry->parameters["height"]) * _up;
      context->transform.translate(offset);
    } else if (name == "F") { // 不构造几何体，也不生成节点
      vec3 offset = _up;
      if (args.size() > 0) // 无参数则前进1个单位
        offset *= args[0];
      context->transform.translate_relative(offset); // 以当前姿态下的_up为默认前进方向
      context->cur_node->setPosition(context->transform.getPosition());
    } else if (name == "RX") {
      context->transform.rotate(glm::radians(args[0]), _right);
      // cout << "绕+x旋转" << args[0] << "度" << endl;
    } else if (name == "RY") {
      context->transform.rotate(glm::radians(args[0]), _up);
      // cout << "绕+y旋转" << args[0] << "度" << endl;
    } else if (name == "RZ") {
      context->transform.rotate(glm::radians(args[0]), _front);
      // cout << "绕-z旋转" << args[0] << "度" << endl;
    } else {
      printf("unknown graphics symbol : \"%s\"\n", name.c_str());
      fflush(stdout);
      return;
    }
  }
};

struct GraphicsStructure {
  shared_ptr<GraphicsContext> context{nullptr};
  vector<shared_ptr<LProdCall>> call_list;

  // GraphicsStructure(vector<shared_ptr<LProdCall>> call_list) : context(make_shared<GraphicsContext>()), call_list(LEXY_MOV(call_list)) {}

  shared_ptr<Skeleton> construct() const {
    for (shared_ptr<LProdCall> cmd : this->call_list) {
      cmd->exec(context);
    }
    context->skeleton->update(); // 根据每个节点的相对位置姿态计算绝对位置姿态
    // 符号序列执行完毕，最后一步将生成的Skeleton返回
    return context->skeleton;
  }
};

}; // namespace config

// namespace grammar {
// struct SymName {
//   static constexpr auto rule =
//       dsl::identifier(dsl::ascii::alpha, dsl::ascii::alpha_digit_underscore);
//   static constexpr auto value = lexy::as_string<string>;
// };

// struct LControlSym {
//   static constexpr auto rule = dsl::capture(dsl::ascii::punct);
//   static constexpr auto value = lexy::as_string<string>;
// };

// struct LProdCall {
//   struct ArgExpr {
//     static constexpr auto rule = dsl::p<MathParser::grammar::MathExpr>;
//     static constexpr auto value = lexy::construct<MathParser::ast::expr_ptr>;
//   };
//   struct ArgList {
//     static constexpr auto whitespace = dsl::ascii::space;
//     static constexpr auto rule =
//         dsl::parenthesized.list(dsl::p<ArgExpr>, dsl::sep(dsl::comma));
//     static constexpr auto value =
//         lexy::as_list<vector<MathParser::ast::expr_ptr>>; // 在构造的过程中被计算
//   };
//   struct LCallSym {
//     static constexpr auto rule = dsl::p<SymName> + dsl::p<ArgList>; // 这里暂时规定SymName后面即便没有参数也必须带括号
//     static constexpr auto value = lexy::construct<config::GraphicsCallSym>;
//   };
//   static constexpr auto rule = [] {
//     auto callsym = dsl::peek(dsl::p<SymName>) >> dsl::p<LCallSym>;
//     auto ctrlsym = dsl::p<LControlSym>;
//     return callsym | dsl::else_ >> ctrlsym;
//   }();
//   static constexpr auto value = lexy::callback<shared_ptr<config::LProdCall>>(
//       lexy::new_<config::GraphicsCallSym, shared_ptr<config::LProdCall>>,
//       lexy::new_<config::GraphicsControlSym, shared_ptr<config::LProdCall>>);
// };

// struct LProdCallList {
//   static constexpr auto whitespace = dsl::ascii::blank;
//   static constexpr auto rule =
//       dsl::list((dsl::peek(dsl::p<SymName>) | dsl::peek(dsl::p<LControlSym>)) >>
//                 dsl::p<LProdCall>);
//   static constexpr auto value =
//       lexy::as_list<vector<shared_ptr<config::LProdCall>>>;
// };

// struct GraphicsStructure {
//   static constexpr auto rule = dsl::p<LProdCallList>;
//   static constexpr auto value = lexy::construct<config::GraphicsStructure>;
// };

// }; // namespace grammar
}; // namespace GeometryGenerator
