#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "LSysConfig.h"

#include "Geometry.h"
#include "Skeleton.h"
#include "Transform.h"

namespace GeometryInterpreter {
struct GraphicsContext {
  Transform transform;
  SkNode*   cur_node{nullptr};

  // 用以实现栈操作的上下文保存
  std::vector<Transform>    backup_transforms;
  std::vector<SkNode*>      backup_nodes;
  std::shared_ptr<Skeleton> skeleton{nullptr};

  GraphicsContext();
};


class GeometryCmd {
  protected:
  enum cmd_type { GEOMETRY,
                  CONTROL } type{GEOMETRY};

  public:
  GeometryCmd(cmd_type type);
  virtual void exec(std::shared_ptr<GraphicsContext> context) = 0;
};

class GeometryController : public GeometryCmd {
  private:
  char ctrl_sym;   // 约定punct是单字符

  public:
  GeometryController(char ctrl_sym);
  virtual void exec(std::shared_ptr<GraphicsContext> context);
};

class GeometryGenerater : public GeometryCmd {
  private:
  std::string        name;
  std::vector<float> args;

  public:
  GeometryGenerater(const std::string& name, const std::vector<float>& args);
  virtual void exec(std::shared_ptr<GraphicsContext> context);
};


class GraphicsStructure {
  private:
  std::shared_ptr<GraphicsContext>          context{nullptr};
  std::vector<std::shared_ptr<GeometryCmd>> cmds;

  public:
  GraphicsStructure(const std::shared_ptr<LSysConfig::SymSeq>& symSeq);

  std::shared_ptr<Skeleton> construct() const;
};

};   // namespace GeometryInterpreter
