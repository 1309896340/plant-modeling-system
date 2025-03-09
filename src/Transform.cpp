#pragma once
#include "Transform.h"


Transform::Transform(glm::vec3 position, glm::vec3 attitude_vec, float attitude_angle)
  : position(position)
  , attitude(glm::angleAxis(attitude_angle, attitude_vec)) {}
Transform::Transform(glm::vec3 position, glm::quat attitude)
  : position(position)
  , attitude(attitude) {}
Transform::Transform(glm::quat attitude)
  : attitude(attitude) {}
Transform::Transform(glm::vec3 position)
  : position(position) {}
Transform::Transform(const Transform& transform)
  : position(transform.position)
  , attitude(transform.attitude) {}


void Transform::setPosition(glm::vec3 new_position) {
  this->position = new_position;
};
glm::vec3 Transform::getPosition() const {
  return this->position;
};
glm::vec3& Transform::getPosition() {
  return this->position;
};

void Transform::setAttitude(glm::quat attitude) {
  this->attitude = attitude;
}
glm::quat Transform::getAttitude() const {
  return this->attitude;
};
glm::quat& Transform::getAttitude() {
  return this->attitude;
};

void Transform::translate(glm::vec3 offset) {
  // 绝对空间偏移
  this->position += offset;
}
void Transform::translate_relative(glm::vec3 offset) {
  // 当前姿态相关的空间偏移（未测试）
  glm::mat3 rot_mat   = glm::mat3_cast(this->attitude);
  glm::vec3 rel_up    = rot_mat * _up;
  glm::vec3 rel_right = rot_mat * _right;
  glm::vec3 rel_front = rot_mat * _front;

  glm::vec3 rel_offset =
    offset.x * rel_right + offset.y * rel_up + offset.z * rel_front;

  this->position += rel_offset;
}

void Transform::rotate(float rot_angle, glm::vec3 rot_axis) {
  glm::quat rot = glm::angleAxis(rot_angle, rot_axis);
  // this->attitude = rot * this->attitude;
  this->attitude = this->attitude * rot;   // 为什么这样才是对的？
}

glm::mat4 Transform::getModel() const {
  glm::mat4 mvp(1.0f);
  glm::mat4 rot_mat = glm::mat4_cast(this->attitude);
  mvp               = glm::translate(mvp, this->position);   // 先旋转，后移动
  mvp               = mvp * rot_mat;
  return mvp;
}

Transform operator*(Transform t2, Transform t1) {
  t1.attitude = t2.getAttitude() * t1.attitude;
  // 以下operator*被重载为 q*v*q^-1
  glm::vec3 offset = t2.getAttitude() * t1.position;
  t1.position      = t2.position + offset;
  return t1;
}
