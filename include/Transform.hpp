#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "constants.h"
#include "glm/ext/matrix_transform.hpp"

namespace {
using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

class Transform {
private:
  vec3 position;
  // quat attitude{glm::angleAxis(-PI / 2, glm::vec3(1.0f, 0.0f, 0.0f))};
  quat attitude{glm::identity<quat>()};

public:
  // 构造函数中默认做了一个绕x轴-90°旋转，将OpenGL坐标系(y朝上，z朝屏幕外)转换为z朝上，y朝屏幕内
  Transform(vec3 position, vec3 attitude_vec, float attitude_angle)
      : position(position) {
    // this->attitude = glm::rotate(this->attitude, attitude_angle, attitude_vec);
    this->attitude = glm::angleAxis(attitude_angle, attitude_vec);
  }
  Transform(vec3 position, quat attitude) : position(position) {
    // this->attitude = attitude * this->attitude * glm::inverse(attitude);
    this->attitude = attitude;
  }

  Transform() : position{0.0f, 0.0f, 0.0f} {}
  Transform(quat origin_attitude) : attitude(origin_attitude) {
    // 无预置姿态的初始化
  }

  void setPosition(vec3 new_position) { this->position = new_position; };
  vec3 getPosition() const { return this->position; };

  quat getAttitude() const { return this->attitude; };

  void translate(vec3 offset) {
    // 绝对空间偏移
    this->position += offset;
  }
  void translate_relative(vec3 offset) {
    // 当前姿态相关的空间偏移
    mat3 rot_mat = glm::mat3_cast(this->attitude);
    vec3 rel_up = rot_mat * _up;
    vec3 rel_right = rot_mat * _right;
    vec3 rel_front = rot_mat * _front;

    vec3 rel_offset =
        offset.x * rel_right + offset.y * rel_up - offset.z * rel_front;

    this->position += rel_offset;
  }

  void rotate(float rot_angle, vec3 rot_axis) {
    quat rot = glm::angleAxis(rot_angle, rot_axis);
    this->attitude = rot * this->attitude;
  }

  mat4 getModel() const {
    mat4 mvp(1.0f);
    mat4 rot_mat = glm::mat4_cast(this->attitude);
    mvp = glm::translate(mvp, this->position);
    mvp = mvp * rot_mat;
    return mvp;
  }
};

} // namespace