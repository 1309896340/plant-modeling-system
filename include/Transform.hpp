#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "constants.h"
#include "glm/ext/matrix_transform.hpp"

// namespace {
// using glm::mat3;
// using glm::mat4;
// using glm::quat;
// using glm::vec3;
// using glm::vec4;

class Transform {
private:
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  // quat attitude{glm::angleAxis(-PI / 2, glm::vec3(1.0f, 0.0f, 0.0f))};
  glm::quat attitude{glm::identity<glm::quat>()};

public:
  friend Transform operator*(Transform t2, Transform t1);

  Transform(glm::vec3 position, glm::vec3 attitude_vec, float attitude_angle)
      : position(position),
        attitude(glm::angleAxis(attitude_angle, attitude_vec)) {}
  Transform(glm::vec3 position, glm::quat attitude)
      : position(position), attitude(attitude) {}
  Transform(glm::quat attitude) : attitude(attitude) {}
  Transform(glm::vec3 position) : position(position) {}
  Transform(const Transform &transform)
      : position(transform.position), attitude(transform.attitude) {}
  Transform() = default;

  void setPosition(glm::vec3 new_position) { this->position = new_position; };
  glm::vec3 getPosition() const { return this->position; };
  glm::vec3 &getPosition() { return this->position; };

  void setAttitude(glm::quat attitude) { this->attitude = attitude; }
  glm::quat getAttitude() const { return this->attitude; };
  glm::quat &getAttitude(){ return this->attitude; };

  void translate(glm::vec3 offset) {
    // 绝对空间偏移
    this->position += offset;
  }
  void translate_relative(glm::vec3 offset) {
    // 当前姿态相关的空间偏移（未测试）
    glm::mat3 rot_mat = glm::mat3_cast(this->attitude);
    glm::vec3 rel_up = rot_mat * _up;
    glm::vec3 rel_right = rot_mat * _right;
    glm::vec3 rel_front = rot_mat * _front;

    glm::vec3 rel_offset =
        offset.x * rel_right + offset.y * rel_up + offset.z * rel_front;

    this->position += rel_offset;
  }

  void rotate(float rot_angle, glm::vec3 rot_axis) {
    glm::quat rot = glm::angleAxis(rot_angle, rot_axis);
    // this->attitude = rot * this->attitude;
    this->attitude = this->attitude * rot;    // 为什么这样才是对的？
  }

  glm::mat4 getModel() const {
    glm::mat4 mvp(1.0f);
    glm::mat4 rot_mat = glm::mat4_cast(this->attitude);
    mvp = glm::translate(mvp, this->position);    // 先旋转，后移动
    mvp = mvp * rot_mat;
    return mvp;
  }

};


inline Transform operator*(Transform t2, Transform t1){

    t1.attitude = t2.getAttitude() * t1.attitude;
    
    // 以下operator*被重载为 q*v*q^-1
    glm::vec3 offset = t2.getAttitude() * t1.position;
    t1.position = t2.position + offset;

    return t1;
}

// } // namespace