#pragma once

#include "GeometryInterpreter.h"

namespace GeometryInterpreter {

GraphicsContext::GraphicsContext() {
  // skeleton伴随着上下文创建而创建，最终会被作为parse的结果传递出去，由引用计数自动释放
  this->skeleton = std::make_shared<Skeleton>();
  this->cur_node = this->skeleton->root;   // 初始指向root节点
}

GeometryCmd::GeometryCmd(cmd_type type)
  : type(type) {}


GeometryController::GeometryController(char ctrl_sym)
  : GeometryCmd{CONTROL}
  , ctrl_sym{ctrl_sym} {}
void GeometryController::exec(std::shared_ptr<GraphicsContext> context) {
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

GeometryGenerater::GeometryGenerater(const std::string& name, const std::vector<float>& args)
  : GeometryCmd{GEOMETRY}
  , name{name}
  , args{args} {}
void GeometryGenerater::exec(std::shared_ptr<GraphicsContext> context) {
  // 暂时将Primitive生成逻辑封装在这里
  if (name == "S") {
    // 构造SkNode
    SkNode* node = new SkNode();
    node->parent = context->cur_node;
    context->cur_node->addChild(node);
    context->cur_node = node;
    // 创建几何体
    auto geometry = Mesh::Sphere(args[0], 36, 18);
    // std::shared_ptr<Sphere> geometry = std::make_shared<Sphere>(args[0]);
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
    // std::shared_ptr<Cylinder> geometry = std::make_shared<Cylinder>(args[0], args[1]);
    context->cur_node->obj = static_pointer_cast<Geometry>(geometry);
    // 更新偏移量
    context->cur_node->setAttitude(context->transform.getAttitude());
    context->cur_node->setPosition(context->transform.getPosition());
    // 为新节点重置上下文的transform
    context->transform = Transform();
    // 当前几何体的位置偏移将更新到下一个节点中
    glm::vec3 offset = std::get<float>(geometry->parameters["height"]->getProp()) * _up;
    context->transform.translate(offset);
  }
  else if (name == "F") {   // 不构造几何体，也不生成节点
    glm::vec3 offset = _up;
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
GraphicsStructure::GraphicsStructure(const std::shared_ptr<LSysConfig::SymSeq>& symSeq) {
  this->context = std::make_shared<GraphicsContext>();
  for (uint32_t i = 0; i < symSeq->syms.size(); i++) {
    std::shared_ptr<LSysConfig::Sym> sym = symSeq->syms[i];
    std::shared_ptr<GeometryCmd>     new_cmd{nullptr};
    switch (sym->type) {
    case LSysConfig::Sym::NORMAL:
      new_cmd = std::make_shared<GeometryGenerater>(sym->name, sym->args);
      break;
    case LSysConfig::Sym::PUNCT:
      new_cmd = std::make_shared<GeometryController>(sym->punct);
      break;
    default:
      std::cerr << "GraphicsStructure: unknown sym->type" << std::endl;
    }
    this->cmds.emplace_back(new_cmd);
  }
}

std::shared_ptr<Skeleton> GraphicsStructure::construct() const {
  for (std::shared_ptr<GeometryCmd> cmd : this->cmds) {
    cmd->exec(context);
  }
  context->skeleton->update();   // 根据每个节点的相对位置姿态计算绝对位置姿态
  // 符号序列执行完毕，最后一步将生成的Skeleton返回
  return context->skeleton;
}

};   // namespace GeometryInterpreter
