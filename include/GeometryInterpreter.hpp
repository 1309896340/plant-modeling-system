#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "LSysConfig.h"

#include "Geometry.h"
#include "Skeleton.hpp"
#include "Transform.h"

namespace GeometryInterpreter {
struct GraphicsContext {
  Transform transform;
  SkNode*   cur_node{nullptr};
  // shared_ptr<Geometry> cur_geometry{nullptr};  // 目前不涉及跨指令影响的Primitive生成，因此注释掉

  // 用以实现栈操作的上下文保存
  vector<Transform>    backup_transforms;
  vector<SkNode*>      backup_nodes;
  shared_ptr<Skeleton> skeleton{nullptr};

  GraphicsContext() {
    // skeleton伴随着上下文创建而创建，最终会被作为parse的结果传递出去，由引用计数自动释放
    this->skeleton = make_shared<Skeleton>();
    this->cur_node = this->skeleton->root;   // 初始指向root节点
  }
};


class GeometryCmd {
  protected:
  enum cmd_type { GEOMETRY,
                  CONTROL } type{GEOMETRY};

  public:
  GeometryCmd(cmd_type type)
    : type(type) {}
  virtual void exec(shared_ptr<GraphicsContext> context) = 0;
};

class GeometryController : public GeometryCmd {
  private:
  char ctrl_sym;   // 约定punct是单字符

  public:
  GeometryController(char ctrl_sym)
    : GeometryCmd{CONTROL}
    , ctrl_sym{ctrl_sym} {}
  virtual void exec(shared_ptr<GraphicsContext> context) {
    switch (ctrl_sym) {
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
      printf("unknown control symbol : \'%c\'\n", ctrl_sym);
    }
  }
};

class GeometryGenerater : public GeometryCmd {
  private:
  string        name;
  vector<float> args;

  public:
  GeometryGenerater(const string& name, const vector<float>& args)
    : GeometryCmd{GEOMETRY}
    , name{name}
    , args{args} {}
  virtual void exec(shared_ptr<GraphicsContext> context) {
    // 暂时将Primitive生成逻辑封装在这里
    if (name == "S") {
      // 构造SkNode
      SkNode* node = new SkNode();
      node->parent = context->cur_node;
      context->cur_node->addChild(node);
      context->cur_node = node;
      // 创建几何体
      auto geometry = Mesh::Sphere(args[0], 36, 18);
      // shared_ptr<Sphere> geometry = make_shared<Sphere>(args[0]);
      context->cur_node->obj = static_pointer_cast<Geometry>(geometry);
      // 更新偏移量（球没有位置偏移）
      context->cur_node->setPosition(context->transform.getPosition());
      context->cur_node->setAttitude(context->transform.getAttitude());
      // 为新节点重置上下文的transform
      context->transform = Transform();
    }
    else if (name == "C") {
      // 构造新节点
      SkNode* node = new SkNode();
      node->parent = context->cur_node;
      context->cur_node->addChild(node);
      context->cur_node = node;
      // 创建几何体
      auto geometry = Composition::Cylinder(args[0], args[1], 36, 36, 20);
      // shared_ptr<Cylinder> geometry = make_shared<Cylinder>(args[0], args[1]);
      context->cur_node->obj = static_pointer_cast<Geometry>(geometry);
      // 更新偏移量
      context->cur_node->setAttitude(context->transform.getAttitude());
      context->cur_node->setPosition(context->transform.getPosition());
      // 为新节点重置上下文的transform
      context->transform = Transform();
      // 当前几何体的位置偏移将更新到下一个节点中
      vec3 offset = std::get<float>(geometry->parameters["height"]->getProp()) * _up;
      context->transform.translate(offset);
    }
    else if (name == "F") {   // 不构造几何体，也不生成节点
      vec3 offset = _up;
      if (args.size() > 0)   // 无参数则前进1个单位
        offset *= args[0];
      context->transform.translate_relative(offset);   // 以当前姿态下的_up为默认前进方向
      context->cur_node->setPosition(context->transform.getPosition());
    }
    else if (name == "RX") {
      context->transform.rotate(glm::radians(args[0]), _right);
      // cout << "绕+x旋转" << args[0] << "度" << endl;
    }
    else if (name == "RY") {
      context->transform.rotate(glm::radians(args[0]), _up);
      // cout << "绕+y旋转" << args[0] << "度" << endl;
    }
    else if (name == "RZ") {
      context->transform.rotate(glm::radians(args[0]), _front);
      // cout << "绕-z旋转" << args[0] << "度" << endl;
    }
    else {
      printf("unknown graphics symbol : \"%s\"\n", name.c_str());
      fflush(stdout);
      return;
    }
  }
};


class GraphicsStructure {
  private:
  shared_ptr<GraphicsContext>     context{nullptr};
  vector<shared_ptr<GeometryCmd>> cmds;

  public:
  GraphicsStructure(const shared_ptr<LSysConfig::SymSeq>& symSeq) {
    this->context = make_shared<GraphicsContext>();
    for (uint32_t i = 0; i < symSeq->syms.size(); i++) {
      shared_ptr<LSysConfig::Sym> sym = symSeq->syms[i];
      shared_ptr<GeometryCmd>     new_cmd{nullptr};
      switch (sym->type) {
      case LSysConfig::Sym::NORMAL:
        new_cmd = make_shared<GeometryGenerater>(sym->name, sym->args);
        break;
      case LSysConfig::Sym::PUNCT:
        new_cmd = make_shared<GeometryController>(sym->punct);
        break;
      default:
        cerr << "GraphicsStructure: unknown sym->type" << endl;
      }
      this->cmds.emplace_back(new_cmd);
    }
  }

  shared_ptr<Skeleton> construct() const {
    for (shared_ptr<GeometryCmd> cmd : this->cmds) {
      cmd->exec(context);
    }
    context->skeleton->update();   // 根据每个节点的相对位置姿态计算绝对位置姿态
    // 符号序列执行完毕，最后一步将生成的Skeleton返回
    return context->skeleton;
  }
};

};   // namespace GeometryInterpreter
