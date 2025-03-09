#pragma once

#include <cstdint>
#include <cstdio>
#include <deque>
// #include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Geometry.h"
#include "Transform.h"


class SkNode {
  friend class Skeleton;

  private:
  // 当前节点相对于上一个节点的位置姿态偏移
  Transform transform;
  Transform abs_transform;   // 当前节点的起始位置与姿态
  public:
  std::vector<SkNode*> children;
  SkNode*              parent{nullptr};

  std::shared_ptr<Geometry> obj{nullptr};   // 绑定的几何体

  SkNode() = default;
  SkNode(glm::vec3 position, float rot_angle, glm::vec3 rot_axis);

  void bindGeometry(std::shared_ptr<Geometry> obj);

  void setAttitude(float angle, glm::vec3 axis);

  void setAttitude(glm::quat attitude);

  void setPosition(glm::vec3 position);

  void addChild(SkNode* child);

  glm::mat4 getAbsModel() const;

  Transform getAbsTransform() const;
  Transform getTransform() const;
  void      setTransform(const Transform& transform);
};

class Skeleton {
  // 目前没有任何添加节点的方法，添加节点的过程是完全暴露的
  private:
  public:
  SkNode* root{nullptr};
  Skeleton();
  ~Skeleton();

  void update();

  uint32_t calcNodeSize();

  void traverse(std::function<void(SkNode*)> visit);
};
