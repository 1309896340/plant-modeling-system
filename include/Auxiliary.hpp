#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "Geometry.hpp"
#include "constants.h"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::vec3;

class Arrow : public FixedGeometry {
private:
  const float body_ratio = 0.9f;
  const float radius_ratio = 0.5f;

public:
  Arrow(float radius, float length, vec3 arrow_color, vec3 body_color) {
    Cone arrow(radius, 1 - body_ratio);
    Cylinder body(radius_ratio * radius, body_ratio * length);

    arrow.setColor(arrow_color.r,arrow_color.g,arrow_color.b);
    body.setColor(body_color.r,body_color.g,body_color.b);

    arrow.translate(0.0f, 0.0f, body_ratio * length);

    FixedGeometry a = FixedGeometry(std::move(arrow)) + body;
    this->vertices = a.vertices;
    this->surfaces = a.surfaces;
  }
  Arrow(float radius, float length):Arrow(radius, length, {0.0f,1.0f,1.0f}, {0.0f,1.0f,0.0f}){}

  // 静态方法
  static Arrow getAxisX() {
    Arrow axis_x(0.05f, 1.0f);
    axis_x.setColor(1.0f, 0.0f, 0.0f);
    axis_x.rotate(glm::radians(90.0f),{0.0f,1.0f,0.0f});
    return axis_x;
  }
  static Arrow getAxisY() {
    Arrow axis_y(0.05f, 1.0f);
    axis_y.setColor(0.0f, 1.0f, 0.0f);
    axis_y.rotate(glm::radians(90.0f),{-1.0f,0.0f,0.0f});
    return axis_y;
  }
  static Arrow getAxisZ() {
    Arrow axis_z(0.05f, 1.0f);
    axis_z.setColor(0.0f, 0.0f, 1.0f);
    // axis_z.rotate(glm::radians(90.0f),{1.0f,0.0f,0.0f});
    return axis_z;
  }
};

} // namespace
