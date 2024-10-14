#pragma once

#include <glm/glm.hpp>
namespace {
using namespace std;
using glm::vec3;

class Light {
public:
  vec3 color{1.0f, 1.0f, 1.0f};
  enum LightType {
    POINT,
    PARALLEL,
    SPOT
  } type;
  Light(vec3 color, LightType type) : color(color), type(type) {}
};
class PointLight : public Light {
public:
  vec3 position{0.0f, 0.0f, 0.0f};
  PointLight(vec3 color, vec3 position) : Light(color, POINT), position(position) {}
};
class ParallelLight : public Light {
public:
  vec3 direction{0.0f, 0.0f, 0.0f};
  ParallelLight(vec3 color, vec3 direction) : Light(color, PARALLEL), direction(direction) {}
};
class SpotLight : public Light {
public:
  vec3 direction{0.0f, 0.0f, 0.0f};
  float theta{1.0f}; // 聚光灯最大范围夹角
  SpotLight(vec3 color, vec3 direction, float theta) : Light(color, SPOT), direction(direction), theta(theta) {}
};
} // namespace
