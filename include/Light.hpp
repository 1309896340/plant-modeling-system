#pragma once

#include <glm/glm.hpp>
// namespace {


class Light {
public:
  glm::vec3 color{1.0f, 1.0f, 1.0f};
  enum LightType {
    POINT,
    PARALLEL,
    SPOT
  } type;
  Light(glm::vec3 color, LightType type) : color(color), type(type) {}
  virtual const char *toString() const {
    return "light";
  }
};
class PointLight : public Light {
public:
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  float radiance{1.0f};
  PointLight(glm::vec3 color, glm::vec3 position, float radiance) : Light(color, POINT), position(position), radiance(radiance) {}
};
class ParallelLight : public Light {
public:
  glm::vec3 direction{0.0f, 0.0f, 0.0f};
  float irradiance{1.0f};
  ParallelLight(glm::vec3 color, glm::vec3 direction, float irradiance) : Light(color, PARALLEL), direction(direction), irradiance(irradiance) {}
};
class SpotLight : public Light {
public:
  glm::vec3 direction{0.0f, 0.0f, 0.0f};
  float theta{1.0f}; // 聚光灯最大范围夹角
  SpotLight(glm::vec3 color, glm::vec3 direction, float theta) : Light(color, SPOT), direction(direction), theta(theta) {}
};
// } // namespace
