#pragma once

// #include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// #include <glm/gtx/string_cast.hpp>

namespace {
using glm::mat4;
using glm::vec3;
using glm::vec4;

class Camera {
private:
  vec3 position{glm::vec3(0.0f, 0.0f, 5.0f)};
  vec3 toward{glm::vec3(0.0f, 0.0f, -1.0f)}; // 单位向量
  vec3 upDir{glm::vec3(0.0f, 1.0f, 0.0f)};   // 单位向量

  float fov{45.0f};
  float near{100.0f};
  float far{0.1f};
  float aspect{1.0f};

public:
  Camera() = default;
  Camera(vec3 position, vec3 target, float aspect)
      : position(position), aspect(aspect) {
    this->toward = glm::normalize(target - position);
  }

  void setPosition(vec3 new_position) { this->position = new_position; }

  void move_relative(vec3 offset) {
    // 局部坐标系上的平移
    vec3 _right = glm::cross(this->toward, this->upDir);
    vec3 _up = glm::cross(_right, this->toward);
    this->position +=
        offset.x * _right + offset.y * _up - offset.z * this->toward;
  }

  void move_absolute(vec3 offset) {
    // 全局坐标系上的平移
    this->position += offset;
  }

  void lookAt(vec3 target) {
    this->toward = glm::normalize(target - this->position);
    // std::cout << "view vec: " << glm::to_string(this->toward) << std::endl;
  }

  mat4 getProject() const {
    return glm::perspective(this->fov, this->aspect, this->near, this->far);
  }

  mat4 getView() const {
    return glm::lookAt(this->position, this->position + this->toward,
                       this->upDir);
  }
};
} // namespace
