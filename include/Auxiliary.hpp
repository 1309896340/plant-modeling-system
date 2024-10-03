﻿#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Geometry.hpp"

#include "constants.h"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::vec3;

class AuxiliaryObject : public FixedGeometry {
public:
  AuxiliaryObject() = default;

  static AuxiliaryObject from_geometry(const Geometry &geometry) {
    AuxiliaryObject auxObj;
    auxObj.vertices = geometry.vertices;
    auxObj.surfaces = geometry.surfaces;
    return auxObj;
  }

  // virtual void update() {};
};

class CoordinaryAxis : public AuxiliaryObject {
public:
  CoordinaryAxis() {

    Arrow a_x(0.06f, 1.0f);
    Arrow a_y(0.06f, 1.0f);
    Arrow a_z(0.06f, 1.0f);

    a_x.rotate(glm::radians(90.0f), _front);
    a_z.rotate(glm::radians(90.0f), _right);

    a_x.setColor(1.0f, 0.0f, 0.0f);
    a_y.setColor(0.0f, 1.0f, 0.0f);
    a_z.setColor(0.0f, 0.0f, 1.0f);

    FixedGeometry &&res = FixedGeometry(a_x) + a_y + a_z;
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
};

class Ground : public AuxiliaryObject {
public:
  Ground() {
    Plane ground(10.0f,10.0f);
    this->vertices = ground.vertices;
    this->surfaces = ground.surfaces;
  }
};

} // namespace
