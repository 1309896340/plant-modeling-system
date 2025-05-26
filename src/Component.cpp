#include "Component.h"

#include <string>

std::string Component::GeometryObject::getName() const { return this->name; }
BvhTree *Component::GeometryObject::getBvhTree() { return this->bvhtree.get(); }
BoundingBox *Component::GeometryObject::getBoundingBox() {
  return this->box.get();
}
Geometry *Component::GeometryObject::getGeometry() {
  return this->geometry.get();
}

void Component::GeometryObject::update() {
  // 1. 组件更新
  // geometry更新(由imgui激活对应参数的观察者实现)

  // bangdingbox更新
  this->box->update(this->geometry->getVertices());
  // bvhtree更新
  if (this->bvhtree != nullptr) {
    this->bvhtree->update();
  }
}
Component::GeometryObject::~GeometryObject() {
  printf("销毁GeometryObject: \"%s\" geometry: %p  box: %p  bvhtree: %p\n",
         this->name.c_str(), this->geometry.get(), this->box.get(),
         this->bvhtree.get());
}

Component::GeometryObject::GeometryObject(std::string name,
                                          std::shared_ptr<Geometry> geometry,
                                          Transform transform, bool useBvh)
    : name(name), geometry(geometry), transform(transform) {
  // 构造普通包围盒
  this->box = make_unique<BoundingBox>(geometry->getVertices());
  if (useBvh) {
    this->bvhtree = make_unique<BvhTree>(geometry.get());
    this->bvhtree->construct();
  }
  printf("创建GeometryObject: \"%s\" geometry: %p  box: %p  bvhtree: %p\n",
         this->name.c_str(), this->geometry.get(), this->box.get(),
         this->bvhtree.get());
}
