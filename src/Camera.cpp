#pragma once
#include "Camera.h"

void Camera::updateAttitude() {
  // 由改变后的toward，更新姿态属性 theta 和 phi，务必在每次实际旋转的操作后调用
  float cos_theta = glm::dot(this->toward, this->upDir);
  this->theta     = acos(cos_theta);

  glm::vec3 xoy_vec = glm::normalize(this->toward - cos_theta * this->upDir);
  float     cos_phi = glm::dot(xoy_vec, glm::vec3(1.0f, 0.0f, 0.0f));
  float     sin_phi = glm::dot(xoy_vec, glm::vec3(0.0f, 0.0f, -1.0f));

  this->phi = atan2(sin_phi, cos_phi);

  this->theta_s = this->theta * 180.0f / PI;
  this->phi_s   = this->phi * 180.0f / PI;
}


Camera::Camera() {
  updateAttitude();
  record();
};
Camera::Camera(glm::vec3 position, glm::vec3 target, float aspect)
  : position(position)
  , aspect(aspect) {
  this->toward    = glm::normalize(target - position);
  view_is_changed = true;
  updateAttitude();
  record();
}

std::tuple<float, float, float, float> Camera::getProperties() const {
  return std::make_tuple(fov, near, far, aspect);
}

// 所有会直接影响view矩阵结果的属性getter
glm::vec3& Camera::getPosition() {
  this->view_is_changed = true;
  return this->position;
}
glm::vec3& Camera::getToward() {
  this->view_is_changed = true;
  return this->toward;
}
glm::vec3& Camera::getUpDir() {
  this->view_is_changed = true;
  return this->upDir;
}
// 兼容性setter
void Camera::setPosition(glm::vec3 newPosition) {
  this->view_is_changed = true;
  this->position        = newPosition;
}

// 影响position, toward从而间接更新view矩阵的操作
void Camera::updateToward() {
  // 由改变后的theta_s和phi_s更新toward方向向量
  float fx          = sin(this->theta) * cos(this->phi);
  float fy          = cos(this->theta);
  float fz          = sin(this->theta) * sin(-this->phi);
  this->getToward() = glm::normalize(glm::vec3(fx, fy, fz));
  this->updateAttitude();
}
void Camera::updateAttitudeFromShadow() {
  this->theta = this->theta_s * PI / 180.0f;
  this->phi   = this->phi_s * PI / 180.0f;
  this->updateToward();
}
void Camera::setAttitude(float theta, float phi) {
  // 弧度制
  this->theta = theta;
  this->phi   = phi;
  this->updateToward();
}
void Camera::lookAt(glm::vec3 target) {
  this->getToward() = glm::normalize(target - this->position);
  this->updateAttitude();
}
void Camera::lookAt(float azimuth, float zenith) {
  // 角度制
  this->setAttitude(zenith / 180.0f * PI, azimuth / 180.0f * PI);
}
void Camera::move_relative(glm::vec3 offset) {
  // 局部坐标系上的平移
  glm::vec3 _right = glm::normalize(glm::cross(this->toward, this->upDir));
  glm::vec3 _up    = glm::normalize(glm::cross(_right, this->toward));
  this->getPosition() += offset.x * _right + offset.y * _up - offset.z * this->toward;
}
void Camera::move_absolute(glm::vec3 offset) {
  // 全局坐标系上的平移
  this->getPosition() += offset;
}
void Camera::rotate(glm::vec3 offset) {
  this->phi_s -= offset.x;
  this->theta_s += offset.y;
  if (this->phi_s < -180.0f)
    this->phi_s += 360.0f;
  else if (this->phi_s > 180.0f)
    this->phi_s -= 360.0f;
  this->theta_s = glm::clamp(this->theta_s, 1e-4f, 180.0f - 1e-4f);
  this->updateAttitudeFromShadow();
}
void Camera::surround(float dx, float dy) {
  // 配合record()使用
  this->surround_info.phi += dx;
  this->surround_info.theta -= dy;
  this->surround_info.theta = m_clamp(this->surround_info.theta, EPS, PI - EPS);
  glm::vec3 new_pos;
  new_pos.x           = this->surround_info.distance * sin(this->surround_info.theta) * cos(this->surround_info.phi);
  new_pos.y           = this->surround_info.distance * cos(this->surround_info.theta);
  new_pos.z           = this->surround_info.distance * sin(this->surround_info.theta) * sin(this->surround_info.phi);
  this->getPosition() = new_pos + this->surround_info.anchor;
  this->rotate(glm::vec3(dx * 180.0f / PI, dy * 180.0f / PI, 0.0f));
}

void Camera::setAnchor(glm::vec3 anchor) {
  this->surround_info.anchor = anchor;
}

void Camera::record() {
  // 配合surround()使用
  // 记录相机当前位置到锚点的距离、theta、phi
  glm::vec3 dist_v             = this->position - this->surround_info.anchor;
  this->surround_info.distance = glm::length(dist_v);
  this->surround_info.theta    = acos(glm::dot(glm::normalize(dist_v), glm::vec3(0.0f, 1.0f, 0.0f)));
  this->surround_info.phi      = atan2(dist_v.z, dist_v.x);
}

// 所有通过修改theta_s和phi_s意图更新theta和phi从而更改toward的操作
// 由于这两个值是角度值，因此需要考虑中间转换
// 另外，这两个值需要在修改之后立即更新，而不能等到一个render循环的末尾惰性更新，因此必须保证调用方在修改后立即调用 updateAttitudeFromShadow()
// 也正因为这需要即时的手动更新，所以不需要记录flag来延迟更新
// 由于范围是角度制，因此实际返回的是attitude shadow，所以调用的是 updateAttitudeFromShadow() 而不是 updateToward()
float& Camera::getTheta() {
  return this->theta_s;
}
float& Camera::getPhi() {
  return this->phi_s;
}

// 所有会直接影响projection矩阵结果的属性getter
float& Camera::getFov() {
  project_is_changed = true;
  return this->fov;
}
float& Camera::getNear() {
  project_is_changed = true;
  return this->near;
}
float& Camera::getFar() {
  project_is_changed = true;
  return this->far;
}
float& Camera::getAspect() {
  project_is_changed = true;
  return this->aspect;
}

// 计算view矩阵、projection矩阵
glm::mat4 Camera::getProject() const {
  return glm::perspective(this->fov, this->aspect, this->near, this->far);
}
glm::mat4 Camera::getView() const {
  return glm::lookAt(this->position, this->position + this->toward, this->upDir);
}

void Camera::apply_view_done() {
  this->view_is_changed = false;
}
void Camera::apply_projection_done() {
  this->project_is_changed = false;
}

bool Camera::isProjectionChanged() const {
  return this->project_is_changed;
}
bool Camera::isViewChanged() const {
  return this->view_is_changed;
}
