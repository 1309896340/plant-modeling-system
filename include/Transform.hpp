#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "constants.h"

namespace {
    using glm::vec3;
    using glm::vec4;
    using glm::mat3;
    using glm::mat4;
    using glm::quat;

class Transform {
private:
  vec3 position;
  quat attitude{glm::angleAxis(-PI / 2, glm::vec3(1.0f, 0.0f, 0.0f))};

public:
  // 构造函数中默认做了一个绕x轴-90°旋转，将OpenGL坐标系(y朝上，z朝屏幕外)转换为z朝上，y朝屏幕内
  Transform(vec3 position, vec3 attitude_vec, float attitude_angle)
      : position(position) {
    this->attitude = glm::rotate(this->attitude, attitude_angle, attitude_vec);
  }
  Transform(vec3 position, quat attitude) : position(position) {
    // 未测试
    this->attitude = attitude * this->attitude * glm::conjugate(attitude);
  }

  Transform() : position{0.0f, 0.0f, 0.0f} {}

  void setPosition(vec3 new_position){this->position = new_position;};

  mat4 getModel() const {
    mat4 mvp(1.0f);
    mat4 rot_mat = glm::mat4_cast(this->attitude);
    mvp = glm::translate(mvp, this->position);
    mvp = mvp * rot_mat;
    return mvp;
  }
};

}