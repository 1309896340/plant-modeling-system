#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Geometry.hpp"

#include "constants.h"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::vec3;

class CoordinateAxis : public FixedGeometry {
public:
  CoordinateAxis(float radius, float length){
    // Arrow a_x(radius, length);
    // Arrow a_y(radius, length);
    // Arrow a_z(radius, length);
    auto a_x = CompositeMesh::Arrow(radius,length);
    auto a_y = CompositeMesh::Arrow(radius,length);
    auto a_z = CompositeMesh::Arrow(radius,length);
    
    // 由于使用了响应式变量，Geometry本身的这些变换操作可能失效
    a_x->rotate(glm::radians(90.0f), _front);
    a_z->rotate(glm::radians(90.0f), _right);

    a_x->setColor(1.0f, 0.0f, 0.0f);
    a_y->setColor(0.0f, 1.0f, 0.0f);
    a_z->setColor(0.0f, 0.0f, 1.0f);

    FixedGeometry &&res = (*a_x) + (*a_y) + (*a_z);
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
  CoordinateAxis():CoordinateAxis(0.06f,1.0f){}

};

class Ground : public FixedGeometry {
public:
  Ground(float width, float height) {
    // Plane ground(width, height);
    // this->vertices = ground.vertices;
    // this->surfaces = ground.surfaces;
    auto ground = Mesh::Plane(width, height, 10, 10);
    this->vertices = ground->getVertices();
    this->surfaces = ground->getSurfaces();
  }
  Ground() : Ground(20.0f, 20.0f) {}
};

// class Beam : public FixedGeometry{
//   // 用于可视化平行光
//   public:
//   Beam(){

//   }
// };

} // namespace
