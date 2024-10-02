#pragma once

#include <cstddef>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Transform.hpp"
namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

class SkNode {
public:
  vector<SkNode *> children;
  SkNode *parent{nullptr};
  // Transform transform{glm::identity<quat>()};
  Transform transform;

  SkNode() = default;
  SkNode(vec3 position, float rot_angle, vec3 rot_axis) {
    this->transform.translate(position);
    this->transform.rotate(rot_angle, rot_axis);
  }

  mat4 getModel() const {
    const SkNode *cur = this;
    mat4 res(1.0f);
    while (cur != nullptr) {
      res = glm::inverse(this->transform.getModel()) * res;
      cur = cur->parent;
    }
    return res;
  }

  void addChild(SkNode *child) {
    if (child == nullptr)
      throw invalid_argument("SkNode::addChild cannot pass nullptr!");
    // cout << this << " 加入" << child << endl;
    child->parent = this;
    this->children.push_back(child);
  }
};

class Skeleton {
private:
public:
  SkNode *root{nullptr};
  Skeleton() = default;
  ~Skeleton() {
    if (this->root == nullptr)
      return;
    // 广度优先遍历释放
    deque<SkNode *> buf{this->root};
    while (!buf.empty()) {
      SkNode *cur = buf.front();
      buf.pop_front();
      for (SkNode *n : cur->children) {
        buf.insert(buf.end(), n->children.begin(), n->children.end());
        delete n;
      }
      delete cur;
    }
  }
};

} // namespace
