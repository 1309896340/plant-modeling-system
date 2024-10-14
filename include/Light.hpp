#pragma once

#include <glm/glm.hpp>
namespace {
using namespace std;
using glm::vec3;

class Light {
public:
  vec3 color{1.0f, 1.0f, 1.0f};
  enum {
    POINT,
    PARALLEL,
    SPOT
  } type;
};
class PointLight : public Light {
public:
  vec3 position{0.0f, 0.0f, 0.0f};
};
class ParallelLight : public Light {
public:
  vec3 direction{0.0f, 0.0f, 0.0f};
};
class SpotLight : public Light {
public:
  vec3 direction{0.0f, 0.0f, 0.0f};
  float theta{1.0f}; // 聚光灯最大范围夹角
};
} // namespace
