#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Geometry.h"

#include "constants.h"

// namespace {
// using namespace std;
// using glm::mat3;
// using glm::mat4;
// using glm::vec3;

class CoordinateAxis : public Geometry {
  std::shared_ptr<Geometry> axis_x{nullptr};
  std::shared_ptr<Geometry> axis_y{nullptr};
  std::shared_ptr<Geometry> axis_z{nullptr};
public:
  CoordinateAxis(float radius, float length){
    // Arrow a_x(radius, length);
    // Arrow a_y(radius, length);
    // Arrow a_z(radius, length);
    axis_x = Composition::Arrow(radius,length);
    axis_y = Composition::Arrow(radius,length);
    axis_z = Composition::Arrow(radius,length);
    
    // 由于使用了响应式变量，Geometry本身的这些变换操作可能失效
    axis_x->rotate(glm::radians(90.0f), _front);
    axis_z->rotate(glm::radians(90.0f), _right);

    axis_x->setColor(1.0f, 0.0f, 0.0f);
    axis_y->setColor(0.0f, 1.0f, 0.0f);
    axis_z->setColor(0.0f, 0.0f, 1.0f);

    FixedGeometry &&res = (*axis_x) + (*axis_y) + (*axis_z);
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
  CoordinateAxis():CoordinateAxis(0.06f,1.0f){}

  virtual void update() {
    // // 不会改变，无需更新
    // FixedGeometry &&res = (*axis_x) + (*axis_y) + (*axis_z);
    // this->vertices = res.vertices;
    // this->surfaces = res.surfaces;
  }
  virtual void notify(const std::string& name, const prop& parameter){}
};

class Ground : public Geometry {
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
  virtual void update() {}
  virtual void notify(const std::string& name, const prop& parameter){}
};

// class Beam : public FixedGeometry{
//   // 用于可视化平行光
//   public:
//   Beam(){

//   }
// };

// } // namespace
