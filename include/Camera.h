#pragma once

#include <tuple>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// #include "constants.h"
// #include "utils.hpp"

class Camera {
  private:
  glm::vec3 position{glm::vec3(0.0f, 0.0f, 5.0f)};
  glm::vec3 toward{glm::vec3(0.0f, 0.0f, -1.0f)};   // 单位向量
  glm::vec3 upDir{glm::vec3(0.0f, 1.0f, 0.0f)};     // 单位向量

  float fov{glm::radians(55.0f)};
  float near{0.1f};
  float far{200.0f};
  float aspect{1.0f};

  // 相机环绕的锚点
  struct {
    glm::vec3 anchor{0.0f, 0.0f, 0.0f};   // 环绕的目标
    float     distance{0.0f};
    float     theta{0.0f};
    float     phi{0.0f};
  } surround_info;

  // 天顶角，范围[0,pi]
  float theta{0.0f};
  // 方位角，与+x的夹角，范围(-pi,pi]
  float phi{0.0f};

  // 用于外部访问修改某些属性的影子属性
  float theta_s{0.0f};   // 角度制
  float phi_s{0.0f};

  // 用于给外部提供两个矩阵是否发生变更的标志
  bool view_is_changed{true};
  bool project_is_changed{true};

  void updateAttitude();

  public:
  Camera();
  Camera(glm::vec3 position, glm::vec3 target, float aspect);
  std::tuple<float, float, float, float> getProperties() const;

  // 所有会直接影响view矩阵结果的属性getter
  glm::vec3& getPosition();
  glm::vec3& getToward();
  glm::vec3& getUpDir();
  // 兼容性setter
  void setPosition(glm::vec3 newPosition);

  // 影响position, toward从而间接更新view矩阵的操作
  void updateToward();
  void updateAttitudeFromShadow();
  void setAttitude(float theta, float phi);
  void lookAt(glm::vec3 target);
  void lookAt(float azimuth, float zenith);
  void move_relative(glm::vec3 offset);
  void move_absolute(glm::vec3 offset);
  void rotate(glm::vec3 offset);
  void surround(float dx, float dy);

  void setAnchor(glm::vec3 anchor);
  void record();

  // 所有通过修改theta_s和phi_s意图更新theta和phi从而更改toward的操作
  // 由于这两个值是角度值，因此需要考虑中间转换
  // 另外，这两个值需要在修改之后立即更新，而不能等到一个render循环的末尾惰性更新，因此必须保证调用方在修改后立即调用 updateAttitudeFromShadow()
  // 也正因为这需要即时的手动更新，所以不需要记录flag来延迟更新
  // 由于范围是角度制，因此实际返回的是attitude shadow，所以调用的是 updateAttitudeFromShadow() 而不是 updateToward()
  float& getTheta();
  float& getPhi();

  // 所有会直接影响projection矩阵结果的属性getter
  float& getFov();
  float& getNear();
  float& getFar();
  float& getAspect();

  // 计算view矩阵、projection矩阵
  glm::mat4 getProject() const;
  glm::mat4 getView() const;

  void apply_view_done();
  void apply_projection_done();

  bool isProjectionChanged() const;
  bool isViewChanged() const;
};
// }   // namespace
