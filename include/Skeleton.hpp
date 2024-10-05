﻿#pragma once

#include <deque>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "glm/ext/quaternion_trigonometric.hpp"
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
  friend class Skeleton;

private:
  Transform transform;     // 相对parent的位置姿态
  Transform abs_transform; // 绝对位置姿态
public:
  vector<SkNode *> children;
  SkNode *parent{nullptr};

  SkNode() = default;
  SkNode(vec3 position, float rot_angle, vec3 rot_axis) {
    this->transform.translate(position);
    this->transform.rotate(rot_angle, rot_axis);
  }

  void setAttitude(float angle, vec3 axis) {
    quat attitude = glm::angleAxis(angle, axis);
    this->transform.setAttitude(attitude);
  }

  void setAttitude(quat attitude) { this->transform.setAttitude(attitude); }

  void setPosition(vec3 position) { this->transform.setPosition(position); }

  void addChild(SkNode *child) {
    if (child == nullptr)
      throw invalid_argument("SkNode::addChild cannot pass nullptr!");
    // cout << this << " 加入" << child << endl;
    child->parent = this;
    this->children.push_back(child);
  }

  mat4 getAbsModel() const { return this->abs_transform.getModel(); }

  Transform getAbsTransform() const { return this->abs_transform; }
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

  void update() {
    deque<SkNode *> buf{this->root};
    while (!buf.empty()) {
      SkNode *cur = buf.front();
      buf.pop_front();
      buf.insert(buf.end(), cur->children.begin(), cur->children.end());

      if(cur->parent==nullptr){
        cur->abs_transform = cur->transform;
        continue;
      }

      // 1. 更新姿态
      quat cur_attitude = cur->transform.getAttitude();
      quat accum_attitude = cur->parent->abs_transform.getAttitude();
      cur->abs_transform.setAttitude(accum_attitude * cur_attitude);

      // 2. 更新位置
      vec3 cur_position = cur->transform.getPosition();
      vec3 accum_position = cur->parent->abs_transform.getPosition();

      vec3 right = accum_attitude * _right;
      vec3 up = accum_attitude * _up;
      vec3 front = accum_attitude * _front;

      vec3 new_cur_position = accum_position + 
          right * cur_position.x + up * cur_position.y + front * cur_position.z;
      cur->abs_transform.setPosition(new_cur_position);
    }
  }
};

} // namespace
