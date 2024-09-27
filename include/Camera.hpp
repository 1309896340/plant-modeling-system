#pragma once

#include "constants.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace {
using glm::mat4;
using glm::vec3;
using glm::vec4;

class Camera {
private:
  vec3 position{glm::vec3(0.0f, 0.0f, 5.0f)};
  vec3 toward{glm::vec3(0.0f, 0.0f, -1.0f)}; // 单位向量
  vec3 upDir{glm::vec3(0.0f, 1.0f, 0.0f)};   // 单位向量

  float fov{glm::radians(35.0f)};
  float near{0.1f};
  float far{100.0f};
  float aspect{1.0f};

  // 天顶角，范围[0,pi]
  float theta{0.0f};
  // 方向角，与+x的夹角，范围(-pi,pi]
  float phi{0.0f};

  bool view_is_changed{true};
  bool project_is_changed{true};


  void updateAttitude() {
    // 更新姿态属性 theta 和 phi，务必在每次实际旋转的操作后调用
    float cos_theta = glm::dot(this->toward, this->upDir);
    this->theta = acos(cos_theta);

    vec3 xoy_vec = glm::normalize(this->toward - cos_theta * this->upDir);
    float cos_phi = glm::dot(xoy_vec, vec3(1.0f, 0.0f, 0.0f));
    float sin_phi = glm::dot(xoy_vec, vec3(0.0f, 0.0f, -1.0f));

    this->phi = atan2(sin_phi, cos_phi);

    this->theta_s = this->theta * 180.0f / PI;
    this->phi_s = this->phi * 180.0f / PI;
  }

  void updatePositionToShadow() {
    this->position_s.x = this->position.x;
    this->position_s.y = this->position.y;
    this->position_s.z = this->position.z;
  }

public:
  // 用于外部访问修改某些属性的影子属性
  float theta_s{0.0f}; // 角度制
  float phi_s{0.0f};
  vec3 position_s{0.0f, 0.0f, 0.0f};


  Camera() { updateAttitude(); };
  Camera(vec3 position, vec3 target, float aspect)
      : position(position), aspect(aspect) {
    this->toward = glm::normalize(target - position);
    updateAttitude();
  }

  void updatePositionFromShadow() {
    this->position.x = this->position_s[0];
    this->position.y = this->position_s[1];
    this->position.z = this->position_s[2];
    view_is_changed = true;
  }

  void updateToward() {
    float fx =
        sin(this->theta_s * PI / 180.0f) * cos(this->phi_s * PI / 180.0f);
    float fy = cos(this->theta_s * PI / 180.0f);
    float fz =
        sin(this->theta_s * PI / 180.0f) * sin(-this->phi_s * PI / 180.0f);
    this->toward = glm::normalize(vec3(fx, fy, fz));
    // printf("front: (%.2f, %.2f, %.2f)\n", toward.x, toward.y, toward.z);
    this->view_is_changed = true;
  }

  void setPosition(vec3 new_position) {
    this->position = new_position;
    updatePositionToShadow();
    view_is_changed = true;
  }

  void move_relative(vec3 offset) {
    // 局部坐标系上的平移
    vec3 _right = glm::normalize(glm::cross(this->toward, this->upDir));
    vec3 _up = glm::normalize(glm::cross(_right, this->toward));
    this->position +=
        offset.x * _right + offset.y * _up - offset.z * this->toward;
    updatePositionToShadow();
    view_is_changed = true;
  }

  void move_absolute(vec3 offset) {
    // 全局坐标系上的平移
    this->position += offset;
    updatePositionToShadow();
    view_is_changed = true;
  }

  // void rotate_relative(vec3 offset) {
  //   // 以相机局部坐标系的up为竖直旋转轴
  //   this->phi_s += offset.x;
  //   this->theta_s -= offset.y;
  //   this->updateToward();
  // }

  void rotate(vec3 offset) {
    this->phi_s -= offset.x;
    this->theta_s += offset.y;
    this->updateToward();
    view_is_changed = true;
  }

  void lookAt(vec3 target) {
    this->toward = glm::normalize(target - this->position);
    // std::cout << "view vec: " << glm::to_string(this->toward) << std::endl;
    updateAttitude();
    view_is_changed = true;
  }

  mat4 getProject() const {
    return glm::perspective(this->fov, this->aspect, this->near, this->far);
  }

  mat4 getView() const {
    // printf("front: (%.2f, %.2f, %.2f)\n", toward.x, toward.y, toward.z);
    return glm::lookAt(this->position, this->position + this->toward,
                       this->upDir);
  }

  void apply_view_done(){
    this->view_is_changed = false;
  }
  void apply_projection_done(){
    this->project_is_changed = false;
  }

  void setAspect(float aspect){
    this->aspect = aspect;
    this->project_is_changed = true;
  }

  bool isProjectionChanged()const{
    return this->project_is_changed;
  }
  bool isViewChanged()const{
    return this->view_is_changed;
  }
};
} // namespace
