#pragma once
#include "Skeleton.h"

SkNode::SkNode(glm::vec3 position, float rot_angle, glm::vec3 rot_axis) {
  this->transform.translate(position);
  this->transform.rotate(rot_angle, rot_axis);
}

void SkNode::bindGeometry(std::shared_ptr<Geometry> obj) {
  // 绑定几何体对象用于可视化表示
  this->obj = obj;
}

void SkNode::setAttitude(float angle, glm::vec3 axis) {
  glm::quat attitude = glm::angleAxis(angle, axis);
  this->transform.setAttitude(attitude);
}

void SkNode::setAttitude(glm::quat attitude) {
  this->transform.setAttitude(attitude);
}

void SkNode::setPosition(glm::vec3 position) {
  this->transform.setPosition(position);
}

void SkNode::addChild(SkNode* child) {
  if (child == nullptr)
    throw std::invalid_argument("SkNode::addChild cannot pass nullptr!");
  // cout << this << " 加入" << child << endl;
  child->parent = this;
  this->children.push_back(child);
}

glm::mat4 SkNode::getAbsModel() const {
  return this->abs_transform.getModel();
}

Transform SkNode::getAbsTransform() const {
  return this->abs_transform;
}
Transform SkNode::getTransform() const {
  return this->transform;
}
void SkNode::setTransform(const Transform& transform) {
  this->transform = transform;
}


Skeleton::Skeleton()
  : root(new SkNode()) {
    // 默认生成一个空的根节点
  };
Skeleton::~Skeleton() {
  if (this->root == nullptr)
    return;
  // 广度优先遍历释放
  std::deque<SkNode*> buf{this->root};
  while (!buf.empty()) {
    SkNode* cur = buf.front();
    buf.pop_front();
    buf.insert(buf.end(), cur->children.begin(), cur->children.end());
    delete cur;
  }
}

void Skeleton::update() {
  std::deque<SkNode*> buf{this->root};
  while (!buf.empty()) {
    SkNode* cur = buf.front();
    buf.pop_front();
    buf.insert(buf.end(), cur->children.begin(), cur->children.end());

    if (cur->parent == nullptr) {   // 根节点
      cur->abs_transform = cur->transform;
      continue;
    }

    cur->abs_transform = cur->parent->abs_transform * cur->transform;

    // vec3 ps = cur->abs_transform.getPosition();
    // printf("节点%p 起点位置：(%.4f, %.4f, %.4f)\n", cur, ps.x, ps.y, ps.z);
  }
}

uint32_t Skeleton::calcNodeSize() {
  uint32_t num = 0;
  this->traverse([&num](SkNode* node) { num++; });
  return num;
}

void Skeleton::traverse(std::function<void(SkNode*)> visit) {
  // 广度优先遍历
  std::deque<SkNode*> buf{this->root};
  while (!buf.empty()) {
    SkNode* cur = buf.front();
    buf.pop_front();
    if (cur != this->root)
      visit(cur);
    if (cur->children.size() > 0)
      buf.insert(buf.end(), cur->children.begin(), cur->children.end());
  }
}
