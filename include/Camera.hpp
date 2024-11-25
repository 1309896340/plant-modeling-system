#pragma once

#include <tuple>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "constants.h"
#include "utils.hpp"

namespace {
using namespace std;
using glm::mat4;
using glm::vec3;
using glm::vec4;

// const float MOUSE_VIEW_ROTATE_SENSITIVITY = 0.1f;

class Camera {
  private:
    vec3 position{glm::vec3(0.0f, 0.0f, 5.0f)};
    vec3 toward{glm::vec3(0.0f, 0.0f, -1.0f)}; // 单位向量
    vec3 upDir{glm::vec3(0.0f, 1.0f, 0.0f)};   // 单位向量

    float fov{glm::radians(55.0f)};
    float near{0.1f};
    float far{200.0f};
    float aspect{1.0f};

    // 相机环绕的锚点
    struct {
        vec3 anchor{0.0f, 0.0f, 0.0f}; // 环绕的目标
        float distance{0.0f};
        float theta{0.0f};
        float phi{0.0f};
    } surround_info;

    // 天顶角，范围[0,pi]
    float theta{0.0f};
    // 方向角，与+x的夹角，范围(-pi,pi]
    float phi{0.0f};

    // 用于外部访问修改某些属性的影子属性
    float theta_s{0.0f}; // 角度制
    float phi_s{0.0f};

    // 用于给外部提供两个矩阵是否发生变更的标志
    bool view_is_changed{true};
    bool project_is_changed{true};

    void updateAttitude() {
        // 由改变后的toward，更新姿态属性 theta 和 phi，务必在每次实际旋转的操作后调用
        float cos_theta = glm::dot(this->toward, this->upDir);
        this->theta = acos(cos_theta);

        vec3 xoy_vec = glm::normalize(this->toward - cos_theta * this->upDir);
        float cos_phi = glm::dot(xoy_vec, vec3(1.0f, 0.0f, 0.0f));
        float sin_phi = glm::dot(xoy_vec, vec3(0.0f, 0.0f, -1.0f));

        this->phi = atan2(sin_phi, cos_phi);

        this->theta_s = this->theta * 180.0f / PI;
        this->phi_s = this->phi * 180.0f / PI;
    }

  public:
    Camera() {
        updateAttitude();
        record();
    };
    Camera(vec3 position, vec3 target, float aspect)
        : position(position), aspect(aspect) {
        this->toward = glm::normalize(target - position);
        view_is_changed = true;
        updateAttitude();
        record();
    }

    tuple<float, float, float, float> getProperties() const {
        return make_tuple(fov, near, far, aspect);
    }

    // 所有会直接影响view矩阵结果的属性getter
    vec3 &getPosition() {
        this->view_is_changed = true;
        return this->position;
    }
    vec3 &getToward() {
        this->view_is_changed = true;
        return this->toward;
    }
    vec3 &getUpDir() {
        this->view_is_changed = true;
        return this->upDir;
    }
    // 兼容性setter
    void setPosition(vec3 newPosition) {
        this->view_is_changed = true;
        this->position = newPosition;
    }

    // 影响position, toward从而间接更新view矩阵的操作
    void updateToward() {
        // 由改变后的theta_s和phi_s更新toward方向向量
        float fx = sin(this->theta) * cos(this->phi);
        float fy = cos(this->theta);
        float fz = sin(this->theta) * sin(-this->phi);
        this->getToward() = glm::normalize(vec3(fx, fy, fz));
        this->updateAttitude();
    }
    void updateAttitudeFromShadow() {
        this->theta = this->theta_s * PI / 180.0f;
        this->phi = this->phi_s * PI / 180.0f;
        this->updateToward();
    }
    void setAttitude(float theta, float phi) {
        this->theta = theta;
        this->phi = phi;
        this->updateToward();
    }
    void lookAt(vec3 target) {
        this->getToward() = glm::normalize(target - this->position);
        this->updateAttitude();
    }
    void move_relative(vec3 offset) {
        // 局部坐标系上的平移
        vec3 _right = glm::normalize(glm::cross(this->toward, this->upDir));
        vec3 _up = glm::normalize(glm::cross(_right, this->toward));
        this->getPosition() += offset.x * _right + offset.y * _up - offset.z * this->toward;
    }
    void move_absolute(vec3 offset) {
        // 全局坐标系上的平移
        this->getPosition() += offset;
    }
    void rotate(vec3 offset) {
        this->phi_s -= offset.x;
        this->theta_s += offset.y;
        if (this->phi_s < -180.0f)
            this->phi_s += 360.0f;
        else if (this->phi_s > 180.0f)
            this->phi_s -= 360.0f;
        this->theta_s = glm::clamp(this->theta_s, 1e-4f, 180.0f - 1e-4f);
        this->updateAttitudeFromShadow();
    }
    void surround(float dx, float dy) {
        // 配合record()使用
        this->surround_info.phi += dx;
        this->surround_info.theta -= dy;
        this->surround_info.theta = m_clamp(this->surround_info.theta, EPS, PI - EPS);
        vec3 new_pos;
        new_pos.x = this->surround_info.distance * sin(this->surround_info.theta) * cos(this->surround_info.phi);
        new_pos.y = this->surround_info.distance * cos(this->surround_info.theta);
        new_pos.z = this->surround_info.distance * sin(this->surround_info.theta) * sin(this->surround_info.phi);
        this->getPosition() = new_pos + this->surround_info.anchor;
        this->rotate(vec3(dx*180.0f/PI, dy*180.0f/PI, 0.0f));
    }

    void setAnchor(vec3 anchor) { this->surround_info.anchor = anchor; }

    void record() {
        // 配合surround()使用
        // 记录相机当前位置到锚点的距离、theta、phi
        vec3 dist_v = this->position - this->surround_info.anchor;
        this->surround_info.distance = glm::length(dist_v);
        this->surround_info.theta = acos(glm::dot(glm::normalize(dist_v), vec3(0.0f, 1.0f, 0.0f)));
        this->surround_info.phi = atan2(dist_v.z, dist_v.x);
    }

    // 所有通过修改theta_s和phi_s意图更新theta和phi从而更改toward的操作
    // 由于这两个值是角度值，因此需要考虑中间转换
    // 另外，这两个值需要在修改之后立即更新，而不能等到一个render循环的末尾惰性更新，因此必须保证调用方在修改后立即调用 updateAttitudeFromShadow()
    // 也正因为这需要即时的手动更新，所以不需要记录flag来延迟更新
    // 由于范围是角度制，因此实际返回的是attitude shadow，所以调用的是 updateAttitudeFromShadow() 而不是 updateToward()
    float &getTheta() {
        return this->theta_s;
    }
    float &getPhi() {
        return this->phi_s;
    }

    // 所有会直接影响projection矩阵结果的属性getter
    float &getFov() {
        project_is_changed = true;
        return this->fov;
    }
    float &getNear() {
        project_is_changed = true;
        return this->near;
    }
    float &getFar() {
        project_is_changed = true;
        return this->far;
    }
    float &getAspect() {
        project_is_changed = true;
        return this->aspect;
    }

    // 计算view矩阵、projection矩阵
    mat4 getProject() const {
        return glm::perspective(this->fov, this->aspect, this->near, this->far);
    }
    mat4 getView() const {
        return glm::lookAt(this->position, this->position + this->toward,
                           this->upDir);
    }

    void apply_view_done() { this->view_is_changed = false; }
    void apply_projection_done() { this->project_is_changed = false; }

    bool isProjectionChanged() const { return this->project_is_changed; }
    bool isViewChanged() const { return this->view_is_changed; }
};
} // namespace
