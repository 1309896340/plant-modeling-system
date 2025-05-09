﻿#include "Geometry.h"

#include "constants.h"

using namespace std;

ReflectValue::ReflectValue(const string& name, prop init_value, float min_val, float max_val)
  : name(name)
  , value(init_value)
  , region(make_pair(min_val, max_val)) {}
ReflectValue::ReflectValue(const string& name, prop init_value, float min_val, float max_val, weak_ptr<Observer> observer)
  : name(name)
  , value(init_value)
  , region(make_pair(min_val, max_val)) {
  this->observers.emplace_back(observer);
}

ReflectValue::ReflectValue(const std::string& name, prop init_value, float min_val, float max_val, bool isTopo, std::weak_ptr<Observer> observer)
  : name(name)
  , value(init_value)
  , region(make_pair(min_val, max_val))
  , topoFlag(isTopo) {
  this->observers.emplace_back(observer);
}

ReflectValue::ReflectValue(const std::string& name, prop init_value, float min_val, float max_val, bool isTopo)
  : name(name)
  , value(init_value)
  , region(make_pair(min_val, max_val))
  , topoFlag(isTopo) {}

pair<float, float> ReflectValue::getRegion() const {
  return this->region;
}
prop& ReflectValue::getProp() {
  return this->value;
}

bool ReflectValue::isTopo() const {
  return this->topoFlag;
}

void ReflectValue::notify(const string& name, const prop& param) {
  // 实现ReflectValue之间的单向绑定
  if (this->func != nullptr)
    this->value = this->func(param);
  else
    this->value = param;
  // 然后连锁触发
  this->notifyAll();
}

void ReflectValue::notifyAll() {   // 外部有修改时手动调用
  // 移除this->observers中已经失效的元素
  erase_if(this->observers,
           [](weak_ptr<Observer> obs) { return obs.expired(); });
  for (auto& obs : this->observers) {
    assert(!obs.expired() &&
           "ReflectValue::notifyAll(): observer pointer is expired!");
    obs.lock()->notify(
      this->name,
      this->value);   // Geometry需要定义notify(string, prop)接口
  }
}
string ReflectValue::getName() const {
  return this->name;
}
void ReflectValue::addObserver(weak_ptr<Observer> observer) {
  assert(!observer.expired() &&
         "ReflectVal::addObserver(): observer is expired!");
  this->observers.emplace_back(observer);
}

vector<Vertex>& Geometry::getVertices() {
  return this->vertices;
}
vector<Surface>& Geometry::getSurfaces() {
  return this->surfaces;
}
const vector<Vertex>& Geometry::getVertices() const {
  return this->vertices;
}
const vector<Surface>& Geometry::getSurfaces() const {
  return this->surfaces;
}

void Geometry::translate(float x_offset, float y_offset, float z_offset) {
  for (auto& vertex : this->vertices) {
    vertex.x += x_offset;
    vertex.y += y_offset;
    vertex.z += z_offset;
  }
}

void Geometry::rotate(float angle, glm::vec3 axis) {
  glm::mat3 rot_mat = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, axis));
  for (auto& vertex : this->vertices) {
    glm::vec3 new_pos    = rot_mat * glm::make_vec3(vertex.position);
    glm::vec3 new_normal = rot_mat * glm::make_vec3(vertex.normal);
    vertex.x             = new_pos.x;
    vertex.y             = new_pos.y;
    vertex.z             = new_pos.z;
    vertex.nx            = new_normal.x;
    vertex.ny            = new_normal.y;
    vertex.nz            = new_normal.z;
  }
}

void Geometry::setColor(float r, float g, float b) {
  // 设置纯色
  for (auto& vertex : this->vertices) {
    vertex.r = r;
    vertex.g = g;
    vertex.b = b;
  }
}

void Geometry::reset() {
  this->vertices.clear();
  this->surfaces.clear();
}

FixedGeometry::FixedGeometry(const vector<Vertex>& vertices, const vector<Surface>& surfaces)
  : vertices(vertices)
  , surfaces(surfaces) {}
FixedGeometry::FixedGeometry(const Geometry& geo)
  : vertices(geo.getVertices())
  , surfaces(geo.getSurfaces()) {}


FixedGeometry operator+(const FixedGeometry& a, const FixedGeometry& b) {
  FixedGeometry res = a;
  // 1. 将两组vertices进行简单拼接
  res.vertices.insert(res.vertices.end(), b.vertices.begin(), b.vertices.end());
  // 2. 将b的surfaces的索引偏移this->vertices.size()，然后拼接
  const uint32_t  offset = a.vertices.size();
  vector<Surface> sfs    = b.surfaces;
  for (auto& surf_ids : sfs) {
    surf_ids.tidx[0] += offset;
    surf_ids.tidx[1] += offset;
    surf_ids.tidx[2] += offset;
  }
  res.surfaces.insert(res.surfaces.end(), sfs.begin(), sfs.end());
  return res;
}

Mesh::Mesh(uint32_t uNum, uint32_t vNum)
  : topo_flag(true) {
  this->parameters["uNum"] =
      make_shared<ReflectValue>("uNum", uNum, 1.0f, FLT_MAX, true);
  this->parameters["vNum"] =
      make_shared<ReflectValue>("vNum", vNum, 1.0f, FLT_MAX, true);

  // 默认updater生成一个平面
  MeshUpdater updater = [](float u, float v) {
    // 默认尺寸
    float width  = 5;
    float height = 5;

    Vertex vt;
    vt.x = (0.5f - u) * width;
    vt.y = (v - 0.5f) * height;
    vt.z = 0.0f;

    // 默认朝上
    vt.nx = 0.0f;
    vt.ny = 0.0f;
    vt.nz = 1.0f;

    // 默认黄色
    vt.r = 1.0f;
    vt.g = 1.0f;
    vt.b = 0.0f;

    vt.u = u;
    vt.v = v;
    return vt;
  };
}
Mesh::Mesh(uint32_t uNum, uint32_t vNum, MeshUpdater updater)
  : topo_flag(true) {
  this->parameters["uNum"] = make_shared<ReflectValue>("uNum", EPS, FLT_MAX, uNum, true);
  this->parameters["vNum"] = make_shared<ReflectValue>("vNum", EPS, FLT_MAX, vNum, true);
  this->updater            = updater;
}

void Mesh::update() {
  uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
  uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());
  if (this->topo_flag) {
    this->resize();
    uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
    vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());
    for (int i = 0; i < uNum; i++) {
      for (int j = 0; j < vNum; j++) {
        uint32_t ptr            = (j + i * vNum) * 2;
        this->surfaces[ptr + 0] = {j + i * (vNum + 1), 1 + j + i * (vNum + 1), 1 + j + (i + 1) * (vNum + 1)};
        this->surfaces[ptr + 1] = {j + i * (vNum + 1),
                                   1 + j + (i + 1) * (vNum + 1),
                                   j + (i + 1) * (vNum + 1)};
      }
    }
    this->topo_flag = false;
  }
  if (this->updater) {
    for (int i = 0; i <= uNum; i++) {
      for (int j = 0; j <= vNum; j++) {
        float u                            = static_cast<float>(i) / uNum;
        float v                            = static_cast<float>(j) / vNum;
        this->vertices[j + i * (vNum + 1)] = this->updater(u, v);
      }
    }
  }
}

// 由于updater的参数构建比较复杂，且比较抽象，以下的简单赋值并不足以正确完成updater的替换
// 下面这种做法是存在问题的，使用时注意lambda内部捕获的变量需要重新绑定到parameters中
void Mesh::setUpdater(MeshUpdater updater) {
  this->updater = updater;
}

void Mesh::resize() {
  uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
  uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());

  this->vertices.resize((uNum + 1) * (vNum + 1));
  this->surfaces.resize(uNum * vNum * 2);
}
void Mesh::notify(const string& name, const prop& param) {
  if (name == "uNum" || name == "vNum")
    this->topo_flag = true;
  this->update();
}
shared_ptr<Mesh> Mesh::Sphere(float radius, uint32_t PNum, uint32_t VNum) {
  auto mesh    = make_shared<Mesh>(PNum, VNum);
  auto updater = [mesh](float u, float v) {
    float  radius = std::get<float>(mesh->parameters["radius"]->getProp());
    Vertex vt;

    vt.nx = sin(PI * v) * cos(2 * PI * u);
    vt.ny = cos(PI * v);
    vt.nz = sin(PI * v) * sin(2 * PI * u);

    vt.x = radius * vt.normal[0];
    vt.y = radius * vt.normal[1];
    vt.z = radius * vt.normal[2];

    vt.r = vt.x;
    vt.g = vt.y;
    vt.b = vt.z;

    vt.u = u;
    vt.v = v;
    return vt;
  };
  mesh->setUpdater(updater);
  auto radius_val = make_shared<ReflectValue>("radius", radius, EPS, FLT_MAX);
  radius_val->addObserver(mesh);
  mesh->parameters["radius"] = radius_val;
  mesh->parameters["uNum"]->addObserver(mesh);
  mesh->parameters["vNum"]->addObserver(mesh);
  // 进行一次初始的更新
  mesh->update();
  return mesh;
}

shared_ptr<Mesh> Mesh::Disk(float radius, uint32_t PNum, uint32_t RNum) {
  auto mesh    = make_shared<Mesh>(PNum, RNum);
  auto updater = [mesh](float u, float v) {
    float radius = std::get<float>(mesh->parameters["radius"]->getProp());

    Vertex vt;

    vt.x = radius * v * cos(2 * PI * u);
    vt.y = 0.0f;
    vt.z = radius * v * sin(2 * PI * u);

    vt.nx = 0.0f;
    vt.ny = 1.0f;
    vt.nz = 0.0f;

    vt.r = vt.x;
    vt.g = vt.y;
    vt.b = vt.z;

    vt.u = u;
    vt.v = v;
    return vt;
  };
  mesh->setUpdater(updater);
  auto radius_val = make_shared<ReflectValue>("radius", radius, EPS, FLT_MAX);
  radius_val->addObserver(mesh);
  mesh->parameters["radius"] = radius_val;
  mesh->parameters["uNum"]->addObserver(mesh);
  mesh->parameters["vNum"]->addObserver(mesh);
  // 进行一次初始的更新
  mesh->update();
  return mesh;
}

shared_ptr<Mesh> Mesh::ConeSide(float radius, float height, uint32_t PNum, uint32_t HNum) {
  auto mesh    = make_shared<Mesh>(PNum, HNum);
  auto updater = [mesh](float u, float v) {
    float  radius = std::get<float>(mesh->parameters["radius"]->getProp());
    float  height = std::get<float>(mesh->parameters["height"]->getProp());
    Vertex vt;

    vt.x = radius * (1 - v) * cos(-2 * PI * u);
    vt.y = height * v;
    vt.z = radius * (1 - v) * sin(-2 * PI * u);

    float tmp = sqrt(radius * radius + height * height);
    vt.nx     = height / tmp * cos(-2 * PI * u);
    vt.ny     = radius / tmp;
    vt.nz     = height / tmp * sin(-2 * PI * u);

    vt.r = 1.0f;
    vt.g = 0.0f;
    vt.b = 0.0f;

    vt.u = u;
    vt.v = v;

    return vt;
  };
  mesh->setUpdater(updater);
  auto radius_val = make_shared<ReflectValue>("radius", radius, EPS, FLT_MAX);
  auto height_val = make_shared<ReflectValue>("height", height, EPS, FLT_MAX);
  radius_val->addObserver(mesh);
  height_val->addObserver(mesh);
  mesh->parameters["radius"] = radius_val;
  mesh->parameters["height"] = height_val;
  mesh->parameters["uNum"]->addObserver(mesh);
  mesh->parameters["vNum"]->addObserver(mesh);
  // 进行一次初始的更新
  mesh->update();
  return mesh;
}

shared_ptr<Mesh> Mesh::CylinderSide(float radius, float height, uint32_t PNum, uint32_t HNum) {
  auto mesh    = make_shared<Mesh>(PNum, HNum);
  auto updater = [mesh](float u, float v) -> Vertex {
    float  radius = std::get<float>(mesh->parameters["radius"]->getProp());
    float  height = std::get<float>(mesh->parameters["height"]->getProp());
    Vertex vt;
    vt.x = radius * cos(-2 * PI * u);
    vt.y = height * v;
    vt.z = radius * sin(-2 * PI * u);

    vt.nx = cos(-2 * PI * u);
    vt.ny = 0.0f;
    vt.nz = sin(-2 * PI * u);

    vt.r = 1.0f;
    vt.g = 0.0f;
    vt.b = 1.0f;

    vt.u = u;
    vt.v = v;

    return vt;
  };
  mesh->setUpdater(updater);
  auto radius_val = make_shared<ReflectValue>("radius", radius, EPS, FLT_MAX);
  auto height_val = make_shared<ReflectValue>("height", height, EPS, FLT_MAX);
  radius_val->addObserver(mesh);
  height_val->addObserver(mesh);
  mesh->parameters["radius"] = radius_val;
  mesh->parameters["height"] = height_val;
  mesh->parameters["uNum"]->addObserver(mesh);
  mesh->parameters["vNum"]->addObserver(mesh);
  // 进行一次初始的更新
  mesh->update();
  return mesh;
}

shared_ptr<Mesh> Mesh::Plane(float width, float height, uint32_t VNum, uint32_t HNum) {
  auto mesh    = make_shared<Mesh>(VNum, HNum);
  auto updater = [mesh](float u, float v) {
    float width  = std::get<float>(mesh->parameters["width"]->getProp());
    float height = std::get<float>(mesh->parameters["height"]->getProp());

    Vertex vt;
    vt.x = (0.5f - u) * width;
    vt.y = 0.0f;
    vt.z = (v - 0.5f) * height;

    vt.nx = 0.0f;
    vt.ny = 1.0f;
    vt.nz = 0.0f;

    vt.r = 1.0f;
    vt.g = 1.0f;
    vt.b = 0.0f;

    vt.u = u;
    vt.v = v;
    return vt;
  };
  mesh->setUpdater(updater);
  auto width_val  = make_shared<ReflectValue>("width", width, EPS, FLT_MAX);
  auto height_val = make_shared<ReflectValue>("height", height, EPS, FLT_MAX);
  width_val->addObserver(mesh);
  height_val->addObserver(mesh);
  mesh->parameters["width"]  = width_val;
  mesh->parameters["height"] = height_val;
  mesh->parameters["uNum"]->addObserver(mesh);
  mesh->parameters["vNum"]->addObserver(mesh);
  // 进行一次初始的更新
  mesh->update();
  return mesh;
}



using TransformUpdater = function<glm::mat4()>;

void Composition::pushGeometry(shared_ptr<Geometry>    mesh,
                               const TransformUpdater& transform) {

  this->goemetries.emplace_back(mesh);
  this->transforms.emplace_back(transform);
}

void Composition::popMesh() {
  this->goemetries.pop_back();
  this->transforms.pop_back();
}
void Composition::notify(const string& name, const prop& param) {
  // 调整this->meshes和this->transforms中受影响的Mesh实例

  // std::visit([name](auto&& arg) {
  //   using T = std::decay_t<decltype(arg)>;
  //   if constexpr (std::is_same_v<T, float>) {
  //     printf("Composition更新变量\"%s\" 新值：%f\n", name.c_str(), arg);
  //   }
  //   else if constexpr (std::is_same_v<T, uint32_t>) {
  //     printf("Composition更新变量\"%s\" 新值：%u\n", name.c_str(), arg);
  //   }
  // },
  //            param);

  this->update();
}

void Composition::update() {
  assert(this->goemetries.size() == this->transforms.size() &&
         "Composition goemetries.size() != transforms.size()!");
  uint32_t                          num = this->goemetries.size();
  vector<shared_ptr<FixedGeometry>> meshes;
  for (uint32_t i = 0; i < num; i++) {
    // 从meshes中取出mesh，取出vertices并做相应的变换，然后相加得到FixedGeometry并将vertices和surfaces更新this
    vector<Vertex> mesh_vertices = this->goemetries[i]->getVertices();
    // 此处要进行矩阵乘法效率可能比较低(后续再考虑是否需要优化)
    glm::mat4 trans = this->transforms[i]();
    for (uint32_t j = 0; j < mesh_vertices.size(); j++) {
      glm::vec4 pt(mesh_vertices[j].x, mesh_vertices[j].y, mesh_vertices[j].z, 1);
      glm::vec4 pt_norm(mesh_vertices[j].nx, mesh_vertices[j].ny, mesh_vertices[j].nz, 0);
      glm::vec4 new_pt      = trans * pt;
      glm::vec4 new_pt_norm = trans * pt_norm;
      mesh_vertices[j].x    = new_pt.x;
      mesh_vertices[j].y    = new_pt.y;
      mesh_vertices[j].z    = new_pt.z;
      mesh_vertices[j].nx   = new_pt_norm.x;
      mesh_vertices[j].ny   = new_pt_norm.y;
      mesh_vertices[j].nz   = new_pt_norm.z;
    }
    meshes.emplace_back(make_shared<FixedGeometry>(
      mesh_vertices,
      this->goemetries[i]->getSurfaces()));
  }
  // todo 需要将所有变换后的vertices及其对应surfaces拼接到一起
  assert(!meshes.empty() && "Composition::update() meshes.size()==0!");
  shared_ptr<FixedGeometry> fgeo = meshes[0];
  for (uint32_t i = 1; i < meshes.size(); i++)
    (*fgeo) = (*fgeo) + (*meshes[i]);
  this->vertices = fgeo->vertices;
  this->surfaces = fgeo->surfaces;
}

shared_ptr<Composition> Composition::Cube(float xWidth, float yWidth, float zWidth, uint32_t xNum, uint32_t yNum, uint32_t zNum) {
  auto xwidth_val = make_shared<ReflectValue>("xWidth", xWidth, EPS, FLT_MAX);
  auto ywidth_val = make_shared<ReflectValue>("yWidth", yWidth, EPS, FLT_MAX);
  auto zwidth_val = make_shared<ReflectValue>("zWidth", zWidth, EPS, FLT_MAX);
  auto xnum_val = make_shared<ReflectValue>("XNum", xNum, 1.0f, FLT_MAX, true);
  auto ynum_val = make_shared<ReflectValue>("YNum", yNum, 1.0f, FLT_MAX, true);
  auto znum_val = make_shared<ReflectValue>("ZNum", zNum, 1.0f, FLT_MAX, true);

  auto xy1 = Mesh::Plane(xWidth, yWidth, xNum, yNum);
  auto xy2 = Mesh::Plane(xWidth, yWidth, xNum, yNum);
  auto xz1 = Mesh::Plane(xWidth, zWidth, xNum, zNum);
  auto xz2 = Mesh::Plane(xWidth, zWidth, xNum, zNum);
  auto yz1 = Mesh::Plane(yWidth, zWidth, yNum, zNum);
  auto yz2 = Mesh::Plane(yWidth, zWidth, yNum, zNum);
  // 组装comp
  auto comp = make_shared<Composition>();
  comp->pushGeometry(xy1, [xwidth_val, ywidth_val,
                           zwidth_val]() {   // xy1  前
    float     xwidth = std::get<float>(xwidth_val->getProp());
    float     ywidth = std::get<float>(ywidth_val->getProp());
    float     zwidth = std::get<float>(zwidth_val->getProp());
    glm::mat4 trans(1.0f);
    trans =
      glm::translate(trans, glm::vec3(0.0f, ywidth / 2.0f, zwidth / 2.0f));
    trans = glm::rotate(trans, glm::radians(90.0f), _right);
    return trans;
  });

  comp->pushGeometry(xy2, [xwidth_val, ywidth_val,
                           zwidth_val]() {   // xy2  后
    float     xwidth = std::get<float>(xwidth_val->getProp());
    float     ywidth = std::get<float>(ywidth_val->getProp());
    float     zwidth = std::get<float>(zwidth_val->getProp());
    glm::mat4 trans(1.0f);
    trans =
      glm::translate(trans, glm::vec3(0.0f, ywidth / 2.0f, -zwidth / 2.0f));
    trans = glm::rotate(trans, glm::radians(90.0f), -_right);
    return trans;
  });
  comp->pushGeometry(xz1, [xwidth_val, ywidth_val, zwidth_val]() {   // xz1 下
    float     xwidth = std::get<float>(xwidth_val->getProp());
    float     ywidth = std::get<float>(ywidth_val->getProp());
    float     zwidth = std::get<float>(zwidth_val->getProp());
    glm::mat4 trans(1.0f);
    trans = glm::rotate(trans, glm::radians(180.0f), _right);
    return trans;
  });
  comp->pushGeometry(xz2, [xwidth_val, ywidth_val, zwidth_val]() {   // xz2 上
    float     xwidth = std::get<float>(xwidth_val->getProp());
    float     ywidth = std::get<float>(ywidth_val->getProp());
    float     zwidth = std::get<float>(zwidth_val->getProp());
    glm::mat4 trans(1.0f);
    trans = glm::translate(trans, glm::vec3(0.0f, ywidth, 0.0f));
    return trans;
  });
  comp->pushGeometry(yz2, [xwidth_val, ywidth_val,
                           zwidth_val]() {   // yz1 左
    float     xwidth = std::get<float>(xwidth_val->getProp());
    float     ywidth = std::get<float>(ywidth_val->getProp());
    float     zwidth = std::get<float>(zwidth_val->getProp());
    glm::mat4 trans(1.0f);
    trans =
      glm::translate(trans, glm::vec3(-xwidth / 2.0f, ywidth / 2.0f, 0.0f));
    trans = glm::rotate(trans, glm::radians(90.0f), -_front);
    return trans;
  });
  comp->pushGeometry(yz2, [xwidth_val, ywidth_val,
                           zwidth_val]() {   // yz2 右
    float     xwidth = std::get<float>(xwidth_val->getProp());
    float     ywidth = std::get<float>(ywidth_val->getProp());
    float     zwidth = std::get<float>(zwidth_val->getProp());
    glm::mat4 trans(1.0f);
    trans =
      glm::translate(trans, glm::vec3(xwidth / 2.0f, ywidth / 2.0f, 0.0f));
    trans = glm::rotate(trans, glm::radians(90.0f), _front);
    return trans;
  });

  // 反向绑定
  xnum_val->addObserver(xy1->parameters["uNum"]);
  xnum_val->addObserver(xy2->parameters["uNum"]);
  xnum_val->addObserver(xz1->parameters["uNum"]);
  xnum_val->addObserver(xz2->parameters["uNum"]);
  ynum_val->addObserver(yz1->parameters["uNum"]);
  ynum_val->addObserver(yz2->parameters["uNum"]);

  ynum_val->addObserver(xy1->parameters["vNum"]);
  ynum_val->addObserver(xy2->parameters["vNum"]);
  znum_val->addObserver(xz1->parameters["vNum"]);
  znum_val->addObserver(xz2->parameters["vNum"]);
  znum_val->addObserver(yz1->parameters["vNum"]);
  znum_val->addObserver(yz2->parameters["vNum"]);

  xwidth_val->addObserver(xy1->parameters["width"]);
  xwidth_val->addObserver(xy2->parameters["width"]);
  xwidth_val->addObserver(xz1->parameters["width"]);
  xwidth_val->addObserver(xz2->parameters["width"]);
  ywidth_val->addObserver(yz1->parameters["width"]);
  ywidth_val->addObserver(yz2->parameters["width"]);

  ywidth_val->addObserver(xy1->parameters["height"]);
  ywidth_val->addObserver(xy2->parameters["height"]);
  zwidth_val->addObserver(xz1->parameters["height"]);
  zwidth_val->addObserver(xz2->parameters["height"]);
  zwidth_val->addObserver(yz1->parameters["height"]);
  zwidth_val->addObserver(yz2->parameters["height"]);

  // 添加观察者
  xwidth_val->addObserver(comp);
  ywidth_val->addObserver(comp);
  zwidth_val->addObserver(comp);
  xnum_val->addObserver(comp);
  ynum_val->addObserver(comp);
  znum_val->addObserver(comp);
  comp->parameters["xWidth"] = xwidth_val;
  comp->parameters["yWidth"] = ywidth_val;
  comp->parameters["zWidth"] = zwidth_val;
  comp->parameters["XNum"]   = xnum_val;
  comp->parameters["YNum"]   = ynum_val;
  comp->parameters["ZNum"]   = znum_val;

  // 初始化更新
  comp->update();
  return comp;
}

shared_ptr<Composition> Composition::Cylinder(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum) {
  // 声明响应式参数
  auto radius_val = make_shared<ReflectValue>("radius", radius, EPS, FLT_MAX);
  auto height_val = make_shared<ReflectValue>("height", height, EPS, FLT_MAX);

  auto pnum_val = make_shared<ReflectValue>("PNum", PNum, 1.0f, FLT_MAX, true);
  auto rnum_val = make_shared<ReflectValue>("RNum", RNum, 1.0f, FLT_MAX, true);
  auto hnum_val = make_shared<ReflectValue>("HNum", HNum, 1.0f, FLT_MAX, true);
  // 定义构成曲面
  auto bottom = Mesh::Disk(radius, PNum, RNum);
  auto top    = Mesh::Disk(radius, PNum, RNum);
  auto side   = Mesh::CylinderSide(radius, height, PNum, HNum);
  // 创建目标
  auto comp = make_shared<Composition>();
  comp->pushGeometry(bottom, [radius_val, height_val]() {   // bottom
    float     radius = std::get<float>(radius_val->getProp());
    float     height = std::get<float>(height_val->getProp());
    glm::mat4 trans(1.0f);
    // trans = glm::rotate(trans, glm::radians(-90.0f), _right);
    trans = glm::rotate(trans, glm::radians(180.0f), _right);
    return trans;
  });
  comp->pushGeometry(top, [radius_val, height_val]() {   // top
    float     radius = std::get<float>(radius_val->getProp());
    float     height = std::get<float>(height_val->getProp());
    glm::mat4 trans(1.0f);
    // trans = glm::rotate(trans, glm::radians(-90.0f), _right);
    trans = glm::translate(trans, glm::vec3(0.0f, height, 0.0f));
    return trans;
  });
  comp->pushGeometry(side, [radius_val, height_val]() {   // side
    float     radius = std::get<float>(radius_val->getProp());
    float     height = std::get<float>(height_val->getProp());
    glm::mat4 trans(1.0f);
    // trans = glm::rotate(trans, glm::radians(-90.0f), _right);
    return trans;
  });

  // 参数绑定
  radius_val->addObserver(bottom->parameters["radius"]);
  radius_val->addObserver(top->parameters["radius"]);
  radius_val->addObserver(side->parameters["radius"]);

  height_val->addObserver(side->parameters["height"]);

  pnum_val->addObserver(bottom->parameters["uNum"]);
  pnum_val->addObserver(top->parameters["uNum"]);
  pnum_val->addObserver(side->parameters["uNum"]);

  rnum_val->addObserver(bottom->parameters["vNum"]);
  rnum_val->addObserver(top->parameters["vNum"]);

  hnum_val->addObserver(side->parameters["vNum"]);

  // 注册观察者
  radius_val->addObserver(comp);
  height_val->addObserver(comp);
  pnum_val->addObserver(comp);
  rnum_val->addObserver(comp);
  hnum_val->addObserver(comp);

  // 注册目标变量
  comp->parameters["radius"] = radius_val;
  comp->parameters["height"] = height_val;
  comp->parameters["PNum"]   = pnum_val;
  comp->parameters["RNum"]   = rnum_val;
  comp->parameters["HNum"]   = hnum_val;

  comp->update();
  return comp;
}

shared_ptr<Composition> Composition::Cone(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum) {
  auto radius_val = make_shared<ReflectValue>("radius", radius, EPS, FLT_MAX);
  auto height_val = make_shared<ReflectValue>("height", height, EPS, FLT_MAX);

  auto pnum_val = make_shared<ReflectValue>("PNum", PNum, 1.0f, FLT_MAX, true);
  auto rnum_val = make_shared<ReflectValue>("RNum", RNum, 1.0f, FLT_MAX, true);
  auto hnum_val = make_shared<ReflectValue>("HNum", HNum, 1.0f, FLT_MAX, true);

  auto bottom = Mesh::Disk(radius, PNum, RNum);
  auto side   = Mesh::ConeSide(radius, height, PNum, HNum);
  // 组装comp
  auto comp = make_shared<Composition>();
  comp->pushGeometry(bottom, [radius_val, height_val]() {   // bottom
    float     radius = std::get<float>(radius_val->getProp());
    float     height = std::get<float>(height_val->getProp());
    glm::mat4 trans(1.0f);
    trans = glm::rotate(trans, glm::radians(180.0f), _right);
    return trans;
  });
  comp->pushGeometry(side, [radius_val, height_val]() {   // side
    float     radius = std::get<float>(radius_val->getProp());
    float     height = std::get<float>(height_val->getProp());
    glm::mat4 trans(1.0f);
    // trans = glm::rotate(trans, glm::radians(-90.0f), _right);
    return trans;
  });

  // 反向映射
  pnum_val->addObserver(bottom->parameters["uNum"]);
  pnum_val->addObserver(side->parameters["uNum"]);
  rnum_val->addObserver(bottom->parameters["vNum"]);
  hnum_val->addObserver(side->parameters["vNum"]);

  radius_val->addObserver(bottom->parameters["radius"]);
  radius_val->addObserver(side->parameters["radius"]);
  height_val->addObserver(side->parameters["height"]);

  radius_val->addObserver(comp);
  height_val->addObserver(comp);
  pnum_val->addObserver(comp);
  rnum_val->addObserver(comp);
  hnum_val->addObserver(comp);

  comp->parameters["radius"] = radius_val;
  comp->parameters["height"] = height_val;
  comp->parameters["PNum"]   = pnum_val;
  comp->parameters["RNum"]   = rnum_val;
  comp->parameters["HNum"]   = hnum_val;

  comp->update();
  return comp;
}

shared_ptr<Composition> Composition::Arrow(float radius, float length) {
  // todo 后续考虑实现参数之间存在计算关系的响应式
  float bodyRatio   = 0.8f;
  float radiusRatio = 0.5f;

  // 定义构成曲面
  auto arrow = Composition::Cone(radius, length * (1 - bodyRatio), 16, 1, 2);
  auto body  = Composition::Cylinder(radius * radiusRatio, length * bodyRatio, 16, 1, 2);

  // 创建目标
  auto comp = make_shared<Composition>();
  comp->pushGeometry(arrow, [=]() {
    glm::mat4 trans(1.0f);
    trans = glm::translate(trans, glm::vec3(0.0f, length * bodyRatio, 0.0f));
    return trans;
  });
  comp->pushGeometry(body, [=]() {
    glm::mat4 trans(1.0f);
    return trans;
  });

  comp->update();
  return comp;
}


// } // namespace