﻿#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <iostream>

#include "constants.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
using namespace std;
// using param_variant = variant<uint32_t, int, float, bool, glm::vec3>;
using prop = variant<uint32_t, int32_t, float, double, bool, char, glm::vec3>;

struct Vertex {
  union {
    float position[3];
    struct {
      float x;
      float y;
      float z;
    };
  };
  union {
    float normal[3];
    struct {
      float nx;
      float ny;
      float nz;
    };
  };
  union {
    float color[3];
    struct {
      float r;
      float g;
      float b;
    };
  };
  union {
    unsigned int uv[2];
    struct {
      float u;
      float v;
    };
  };
};

// struct Edge {
//   uint32_t pidx[2]; // 2个Vertex构成
// };

struct Surface {
  uint32_t tidx[3];   // 3个Vertex构成
};

class Observer : public enable_shared_from_this<Observer> {
  public:
  virtual void notify(const string& name, const prop& parameter) = 0;
};

// class Geometry;
class ReflectValue : public Observer {
  private:
  string                     name;
  prop                       value;
  vector<weak_ptr<Observer>> observers;

  public:
  ReflectValue(const string& name, prop init_value)
    : name(name)
    , value(init_value) {}
  ReflectValue(const string& name, prop init_value, weak_ptr<Observer> observer)
    : name(name)
    , value(init_value) {
    this->observers.emplace_back(observer);
  }
  // ReflectValue(const string& name, prop init_value, shared_ptr<Observer> observer)
  //   : name(name)
  //   , value(init_value) {
  //   this->observers.emplace_back(observer);
  // }
  prop& getProp() { return this->value; }

  virtual void notify(const string& name, const prop& param) {
    // 实现ReflectValue之间的单向绑定
    this->value = param;
  }

  void notifyAll() {   // 外部有修改时调用
    // 移除this->observers中已经失效的元素
    erase_if(this->observers, [](weak_ptr<Observer> obs) {
      return obs.expired();
    });
    for (auto& obs : this->observers) {
      assert(!obs.expired() && "ReflectValue::notifyAll(): observer pointer is expired!");
      obs.lock()->notify(this->name, this->value);   // Geometry需要定义notify(string, prop)接口
    }
  }
  string getName() const { return this->name; }
  void   addObserver(weak_ptr<Observer> observer) {
    assert(!observer.expired() && "ReflectVal::addObserver(): observer is expired!");
    this->observers.emplace_back(observer);
  }
};

class Geometry : public Observer {
  protected:
  vector<Vertex>  vertices;
  vector<Surface> surfaces;

  public:
  // map<string, shared_ptr<ReflectValue>> parameters;
  map<string, shared_ptr<ReflectValue>> parameters;
  // map<string, param_variant> parameters;
  // prop parameters;

  Geometry()                = default;
  Geometry(const Geometry&) = default;
  ~Geometry()               = default;

  vector<Vertex>&        getVertices() { return this->vertices; }
  vector<Surface>&       getSurfaces() { return this->surfaces; }
  const vector<Vertex>&  getVertices() const { return this->vertices; }
  const vector<Surface>& getSurfaces() const { return this->surfaces; }

  void translate(float x_offset, float y_offset, float z_offset) {
    for (auto& vertex : this->vertices) {
      vertex.x += x_offset;
      vertex.y += y_offset;
      vertex.z += z_offset;
    }
  }

  void rotate(float angle, glm::vec3 axis) {
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

  void setColor(float r, float g, float b) {
    // 设置纯色
    for (auto& vertex : this->vertices) {
      vertex.r = r;
      vertex.g = g;
      vertex.b = b;
    }
  }

  virtual void update() = 0;
  virtual void reset() {
    this->vertices.clear();
    this->surfaces.clear();
  }
};

// class FixedGeometry : public Geometry {
// public:
//   FixedGeometry() { this->parameters.clear(); }

//   FixedGeometry(const Geometry &geometry) {
//     // 可以从Geometry构造（这是个有点奇怪的操作，引入了从父类到子类的隐式转换）
//     this->vertices = geometry.vertices;
//     this->surfaces = geometry.surfaces;
//     this->parameters.clear();
//   }

//   FixedGeometry(Geometry &&geometry) noexcept {
//     this->vertices = geometry.vertices;
//     this->surfaces = geometry.surfaces;
//     geometry.vertices.clear();
//     geometry.surfaces.clear();
//     this->parameters.clear();
//   }

//   FixedGeometry(const vector<Vertex> &vertices,
//                 const vector<Surface> &surfaces) {
//     this->vertices = vertices;
//     this->surfaces = surfaces;
//     // 没有任何可修改属性
//     this->parameters.clear();
//   }

//   // 默认正常拷贝 vertices 和 surfaces
//   FixedGeometry(const FixedGeometry &) = default;

//   virtual void update() {
//     // 空实现，不会修改网格及其顶点的任何属性
//   }
//   virtual void reset() {
//     // 空实现，直到对象自动销毁前，不会有人为的清空操作
//   }

//   FixedGeometry operator+(const Geometry &b) const {
//     FixedGeometry c(*this);
//     // 1. 将两组vertices进行简单拼接
//     c.vertices.insert(c.vertices.end(), b.vertices.begin(),
//     b.vertices.end());
//     // 2. 将b的surfaces的索引偏移this->vertices.size()，然后拼接
//     const uint32_t offset = this->vertices.size();
//     vector<Surface> sfs = b.surfaces;
//     for (auto &surf_ids : sfs) {
//       surf_ids.tidx[0] += offset;
//       surf_ids.tidx[1] += offset;
//       surf_ids.tidx[2] += offset;
//     }
//     c.surfaces.insert(c.surfaces.end(), sfs.begin(), sfs.end());
//     return c;
//   }
// };

struct FixedGeometry {
  vector<Vertex>  vertices;
  vector<Surface> surfaces;
  FixedGeometry() = default;
  FixedGeometry(const vector<Vertex>& vertices,const vector<Surface>& surfaces):vertices(vertices),surfaces(surfaces){}
  FixedGeometry(const Geometry& geo)
    : vertices(geo.getVertices())
    , surfaces(geo.getSurfaces()) {}
};

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


class Mesh : public Geometry {
  using MeshUpdater = function<Vertex(float, float)>;

  private:
  MeshUpdater updater;
  bool        topo_flag{true};

  public:
  Mesh(uint32_t uNum, uint32_t vNum)
    : topo_flag(true) {
    // 不能在构造函数中shared_from_this()
    // this->parameters["uNum"] = make_shared<ReflectValue>("uNum", uNum, shared_from_this());
    // this->parameters["vNum"] = make_shared<ReflectValue>("vNum", vNum, shared_from_this());
    this->parameters["uNum"] = make_shared<ReflectValue>("uNum", uNum);
    this->parameters["vNum"] = make_shared<ReflectValue>("vNum", vNum);

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
  Mesh(uint32_t uNum, uint32_t vNum, MeshUpdater updater)
    : topo_flag(true) {
    // 不能在构造函数中shared_from_this()
    // this->parameters["uNum"] = make_shared<ReflectValue>("uNum", uNum, shared_from_this());
    // this->parameters["vNum"] = make_shared<ReflectValue>("vNum", vNum, shared_from_this());
    this->parameters["uNum"] = make_shared<ReflectValue>("uNum", uNum);
    this->parameters["vNum"] = make_shared<ReflectValue>("vNum", vNum);
    this->updater            = updater;
  }

  // FixedGeometry operator+(const Mesh &other) const {
  //   FixedGeometry b(other);
  //   return FixedGeometry(*this) + b;
  // }

  virtual void update() {
    uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
    uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());
    if (this->topo_flag) {
      this->resize();
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
          // 待优化，不必每次更新顶点都要重置网格
          // if (i != uNum && j != vNum) {
          //   uint32_t ptr            = (j + i * vNum) * 2;
          //   this->surfaces[ptr + 0] = {j + i * (vNum + 1), 1 + j + i * (vNum + 1), 1 + j + (i + 1) * (vNum + 1)};
          //   this->surfaces[ptr + 1] = {j + i * (vNum + 1),
          //                             1 + j + (i + 1) * (vNum + 1),
          //                             j + (i + 1) * (vNum + 1)};
          // }
        }
      }
    }
  }

  void changeUpdater(MeshUpdater updater) {
    this->updater = updater;
  }

  void resize() {
    uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
    uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());

    this->vertices.resize((uNum + 1) * (vNum + 1));
    this->surfaces.resize(uNum * vNum * 2);
  }

  virtual void notify(const string& name, const prop& param) {
    // todo
    // 需要根据name进行判断
    // 如果name=="uNum"或name=="vNum"，则要先调用rebuildTopo()
    // 之后统一调用一次this->update更新参数曲面的顶点数据
    // 注意：暂时无法确定Observer处定义的这个接口是否被正确重写
    if (name == "uNum" || name == "vNum")
      this->rebuildTopo();
    this->update();
  }

  void rebuildTopo() {
    // 外部修改了uNum和vNum，下一次update()调用将重新生成拓扑
    uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
    uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());

    this->vertices.resize((uNum + 1) * (vNum + 1));
    this->surfaces.resize(uNum * vNum * 2);

    this->topo_flag = true;
  }

  // void updateVertex(MeshUpdater func) {
  // resize();
  // uint32_t uNum     = std::get<uint32_t>(this->parameters["uNum"]);
  // uint32_t vNum     = std::get<uint32_t>(this->parameters["vNum"]);
  // this->updater = func;   // 缓存更新函数用于resize中保留uv更新顶点属性
  // for (int i = 0; i <= uNum; i++) {
  //   for (int j = 0; j <= vNum; j++) {
  //     float   u  = static_cast<float>(i) / uNum;
  //     float   v  = static_cast<float>(j) / vNum;
  //     Vertex& vt = this->vertices[j + i * (vNum + 1)];
  //     vt         = func(u, v);
  //     // 待优化，不必每次更新顶点都要重置网格
  //     if (i != uNum && j != vNum) {
  //       uint32_t ptr            = (j + i * vNum) * 2;
  //       this->surfaces[ptr + 0] = {j + i * (vNum + 1), 1 + j + i * (vNum + 1), 1 + j + (i + 1) * (vNum + 1)};
  //       this->surfaces[ptr + 1] = {j + i * (vNum + 1),
  //                                  1 + j + (i + 1) * (vNum + 1),
  //                                  j + (i + 1) * (vNum + 1)};
  //     }
  //   }
  // }
  // }

  void transformVertex(function<Vertex(const Vertex&, float, float)> func) {
    uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
    uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());
    for (int i = 0; i <= uNum; i++) {
      for (int j = 0; j <= vNum; j++) {
        float   u  = static_cast<float>(i) / uNum;
        float   v  = static_cast<float>(j) / vNum;
        Vertex& vt = this->vertices[j + i * (vNum + 1)];
        vt         = func(vt, u, v);
      }
    }
  }


  static shared_ptr<Mesh> Sphere(float radius, uint32_t PNum, uint32_t VNum) {
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto mesh       = make_shared<Mesh>(PNum, VNum, [radius_val](float u, float v) {
      float  radius = std::get<float>(radius_val->getProp());
      Vertex vt;

      vt.nx = sin(PI * v) * cos(-2 * PI * u);
      vt.ny = sin(PI * v) * sin(-2 * PI * u);
      vt.nz = cos(PI * v);

      vt.x = radius * vt.normal[0];
      vt.y = radius * vt.normal[1];
      vt.z = radius * vt.normal[2];

      vt.r = vt.x;
      vt.g = vt.y;
      vt.b = vt.z;

      vt.u = u;
      vt.v = v;
      return vt;
    });
    radius_val->addObserver(mesh);
    mesh->parameters["radius"] = radius_val;
    mesh->parameters["uNum"]->addObserver(mesh);
    mesh->parameters["vNum"]->addObserver(mesh);
    // 进行一次初始的更新
    mesh->update();
    return mesh;
  }

  static shared_ptr<Mesh> Disk(float radius, uint32_t PNum, uint32_t RNum) {
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto mesh       = make_shared<Mesh>(PNum, RNum, [radius_val](float u, float v) {
      float radius = std::get<float>(radius_val->getProp());

      Vertex vt;

      vt.nx = sin(PI * v) * cos(-2 * PI * u);
      vt.ny = sin(PI * v) * sin(-2 * PI * u);
      vt.nz = cos(PI * v);

      vt.x = radius * vt.normal[0];
      vt.y = radius * vt.normal[1];
      vt.z = radius * vt.normal[2];

      vt.r = vt.x;
      vt.g = vt.y;
      vt.b = vt.z;

      vt.u = u;
      vt.v = v;
      return vt;
    });
    radius_val->addObserver(mesh);
    mesh->parameters["radius"] = radius_val;
    mesh->parameters["uNum"]->addObserver(mesh);
    mesh->parameters["vNum"]->addObserver(mesh);
    // 进行一次初始的更新
    mesh->update();
    return mesh;
  }

  static shared_ptr<Mesh> ConeSide(float radius, float height, uint32_t PNum, uint32_t HNum) {
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto height_val = make_shared<ReflectValue>("height", height);
    auto mesh       = make_shared<Mesh>(PNum, HNum, [radius_val, height_val](float u, float v) {
      float  radius = std::get<float>(radius_val->getProp());
      float  height = std::get<float>(height_val->getProp());
      Vertex vt;

      vt.x = radius * (1 - v) * cos(2 * PI * u);
      vt.y = radius * (1 - v) * sin(2 * PI * u);
      vt.z = height * v;

      float tmp = sqrt(radius * radius + height * height);
      vt.nx     = height / tmp * cos(2 * PI * u);
      vt.ny     = height / tmp * sin(2 * PI * u);
      vt.nz     = radius / tmp;

      vt.r = 1.0f;
      vt.g = 0.0f;
      vt.b = 0.0f;

      vt.u = u;
      vt.v = v;

      return vt;
    });
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

  static shared_ptr<Mesh> CylinderSide(float radius, float height, uint32_t PNum, uint32_t HNum) {
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto height_val = make_shared<ReflectValue>("height", height);
    auto mesh       = make_shared<Mesh>(
      PNum,
      HNum,
      [radius_val, height_val](float u, float v) -> Vertex {
        float  radius = std::get<float>(radius_val->getProp());
        float  height = std::get<float>(height_val->getProp());
        Vertex vt;
        vt.x = radius * cos(2 * PI * u);
        vt.y = radius * sin(2 * PI * u);
        vt.z = height * v;

        vt.nx = cos(2 * PI * u);
        vt.ny = sin(2 * PI * u);
        vt.nz = 0.0f;

        vt.r = 1.0f;
        vt.g = 0.0f;
        vt.b = 1.0f;

        vt.u = u;
        vt.v = v;

        return vt;
      });
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

  static shared_ptr<Mesh> Plane(float width, float height, uint32_t VNum, uint32_t HNum) {
    auto width_val  = make_shared<ReflectValue>("width", width);
    auto height_val = make_shared<ReflectValue>("height", height);
    auto mesh       = make_shared<Mesh>(VNum, HNum, [width_val, height_val](float u, float v) {
      float width  = std::get<float>(width_val->getProp());
      float height = std::get<float>(height_val->getProp());

      Vertex vt;
      vt.x = (0.5f - u) * width;
      vt.y = (v - 0.5f) * height;
      vt.z = 0.0f;

      vt.nx = 0.0f;
      vt.ny = 0.0f;
      vt.nz = 1.0f;

      vt.r = 1.0f;
      vt.g = 1.0f;
      vt.b = 0.0f;

      vt.u = u;
      vt.v = v;
      return vt;
    });
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
};

using TransformUpdater = function<glm::mat4()>;

class CompositeMesh : public Geometry {
  private:
  vector<shared_ptr<Mesh>> meshes;
  vector<TransformUpdater> transforms;

  public:
  CompositeMesh() = default;

  void pushMesh(shared_ptr<Mesh> mesh, const TransformUpdater& transform) {
    if (this->meshes.empty()) {
      this->parameters = mesh->parameters;
      for (auto& param_pair : mesh->parameters) {
        param_pair.second->addObserver(shared_from_this());
        // 似乎无法将this转换为weak_ptr，因此只能将Observer继承enable_shared_from_this
      }
    }
    else {
      for (auto& param_pair : mesh->parameters) {
        // 如果this->parameters中已经存在同名变量，则以this->parameters中的为准
        if (!this->parameters.contains(param_pair.first)) {
          this->parameters[param_pair.first] = param_pair.second;
          param_pair.second->addObserver(shared_from_this());
        }
        // todo，此处需要仔细考虑当CompositeMesh::parameters的元素发生改变时，如何驱动Mesh::parameters动作的问题
      }
    }

    this->meshes.emplace_back(mesh);
    this->transforms.emplace_back(transform);
  }

  void popMesh() {
    this->meshes.pop_back();
    this->transforms.pop_back();
  }

  virtual void notify(const string& name, const prop& param) {
    this->update();
  }

  virtual void update() {
    assert(this->meshes.size() == this->transforms.size() && "CompositeMesh meshes.size() != transforms.size()!");
    uint32_t num = this->meshes.size();
    vector<shared_ptr<FixedGeometry>> meshes;
    for (uint32_t i = 0; i < num; i++) {
      // 从meshes中取出mesh，取出vertices并做相应的变换，然后相加得到FixedGeometry并将vertices和surfaces更新this
      vector<Vertex> mesh_vertices = this->meshes[i]->getVertices();
      // 此处要进行矩阵乘法效率可能比较低(后续再考虑是否需要优化)
      glm::mat4 trans = this->transforms[i]();
      for (uint32_t j = 0; j < mesh_vertices.size(); j++) {
        glm::vec4 pt(mesh_vertices[j].x, mesh_vertices[j].y, mesh_vertices[j].z, 1);
        // 这里可能存在乘法顺序的错误，需要检查验证。glm应该是行变换乘法，也就是矩阵左乘列向量
        glm::vec4 res      = trans * pt;
        mesh_vertices[j].x = res.x;
        mesh_vertices[j].y = res.y;
        mesh_vertices[j].z = res.z;
      }
      meshes.emplace_back(make_shared<FixedGeometry>(mesh_vertices, this->meshes[i]->getSurfaces()));
    }
    // todo 需要将所有变换后的vertices及其对应surfaces拼接到一起
    assert(!meshes.empty() && "CompositeMesh::update() meshes.size()==0!");
    shared_ptr<FixedGeometry> fgeo = meshes[0];
    for (uint32_t i = 1; i < meshes.size() - 1; i++) 
      (*fgeo) = (*fgeo) + (*meshes[i]);
    this->vertices = fgeo->vertices;
    this->surfaces = fgeo->surfaces;
    // todo 检查
  }

  static shared_ptr<CompositeMesh> Cube(float xWidth, float yWidth, float zWidth, uint32_t xNum, uint32_t yNum, uint32_t zNum) {
    auto xwidth_val = make_shared<ReflectValue>("xWidth", xWidth);
    auto ywidth_val = make_shared<ReflectValue>("yWidth", yWidth);
    auto zwidth_val = make_shared<ReflectValue>("zWidth", zWidth);
    auto xnum_val   = make_shared<ReflectValue>("xNum", xNum);
    auto ynum_val   = make_shared<ReflectValue>("yNum", yNum);
    auto znum_val   = make_shared<ReflectValue>("zNum", zNum);

    auto xy1 = Mesh::Plane(xWidth, yWidth, xNum, yNum);
    auto xy2 = Mesh::Plane(xWidth, yWidth, xNum, yNum);
    auto xz1 = Mesh::Plane(xWidth, zWidth, xNum, zNum);
    auto xz2 = Mesh::Plane(xWidth, zWidth, xNum, zNum);
    auto yz1 = Mesh::Plane(yWidth, zWidth, yNum, zNum);
    auto yz2 = Mesh::Plane(yWidth, zWidth, yNum, zNum);
    // 组装cmesh
    auto cmesh = make_shared<CompositeMesh>();
    cmesh->meshes.emplace_back(xy1);
    cmesh->meshes.emplace_back(xy2);
    cmesh->meshes.emplace_back(xz1);
    cmesh->meshes.emplace_back(xz2);
    cmesh->meshes.emplace_back(yz1);
    cmesh->meshes.emplace_back(yz2);
    // 构建坐标变换
    cmesh->transforms.emplace_back([xwidth_val, ywidth_val, zwidth_val]() {   // xy1  前
      float     xwidth = std::get<float>(xwidth_val->getProp());
      float     ywidth = std::get<float>(ywidth_val->getProp());
      float     zwidth = std::get<float>(zwidth_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(90.0f), _right);
      trans = glm::translate(trans, glm::vec3(0.0f, ywidth / 2.0f, zwidth / 2.0f));
      return trans;
    });
    cmesh->transforms.emplace_back([xwidth_val, ywidth_val, zwidth_val]() {   // xy2  后
      float     xwidth = std::get<float>(xwidth_val->getProp());
      float     ywidth = std::get<float>(ywidth_val->getProp());
      float     zwidth = std::get<float>(zwidth_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(90.0f), -_right);
      trans = glm::translate(trans, glm::vec3(0.0f, ywidth / 2.0f, -zwidth / 2.0f));
      return trans;
    });
    cmesh->transforms.emplace_back([xwidth_val, ywidth_val, zwidth_val]() {   // xz1 下
      float     xwidth = std::get<float>(xwidth_val->getProp());
      float     ywidth = std::get<float>(ywidth_val->getProp());
      float     zwidth = std::get<float>(zwidth_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(180.0f), _right);
      return trans;
    });
    cmesh->transforms.emplace_back([xwidth_val, ywidth_val, zwidth_val]() {   // xz2 上
      float     xwidth = std::get<float>(xwidth_val->getProp());
      float     ywidth = std::get<float>(ywidth_val->getProp());
      float     zwidth = std::get<float>(zwidth_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::translate(trans, glm::vec3(0.0f, ywidth, 0.0f));
      return trans;
    });
    cmesh->transforms.emplace_back([xwidth_val, ywidth_val, zwidth_val]() {   // yz1 左
      float     xwidth = std::get<float>(xwidth_val->getProp());
      float     ywidth = std::get<float>(ywidth_val->getProp());
      float     zwidth = std::get<float>(zwidth_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(90.0f), -_front);
      trans = glm::translate(trans, glm::vec3(0.0f, ywidth / 2.0f, 0.0f));
      return trans;
    });
    cmesh->transforms.emplace_back([xwidth_val, ywidth_val, zwidth_val]() {   // yz2 右
      float     xwidth = std::get<float>(xwidth_val->getProp());
      float     ywidth = std::get<float>(ywidth_val->getProp());
      float     zwidth = std::get<float>(zwidth_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(90.0f), _front);
      trans = glm::translate(trans, glm::vec3(0.0f, ywidth / 2.0f, 0.0f));
      return trans;
    });

    // 参数映射方案，将ReflectValue也视作观察者进行监视
    // 当xwidth_val,ywidth_val,zwidth_val被imgui修改后，会通知xy1,xy2,xz1,xz2,yz1,yz2进行更新
    // 但是暂时没有建立反向的更新，也就是修改每个Mesh的属性来更新CompositeMesh
    xy1->parameters["width"]->addObserver(xwidth_val);
    xy1->parameters["height"]->addObserver(ywidth_val);
    xy1->parameters["uNum"]->addObserver(xnum_val);
    xy1->parameters["vNum"]->addObserver(ynum_val);
    xy2->parameters["width"]->addObserver(xwidth_val);
    xy2->parameters["height"]->addObserver(ywidth_val);
    xy2->parameters["uNum"]->addObserver(xnum_val);
    xy2->parameters["vNum"]->addObserver(ynum_val);

    xz1->parameters["width"]->addObserver(xwidth_val);
    xz1->parameters["height"]->addObserver(zwidth_val);
    xz1->parameters["uNum"]->addObserver(xnum_val);
    xz1->parameters["vNum"]->addObserver(znum_val);
    xz2->parameters["width"]->addObserver(xwidth_val);
    xz2->parameters["height"]->addObserver(zwidth_val);
    xz2->parameters["uNum"]->addObserver(xnum_val);
    xz2->parameters["vNum"]->addObserver(znum_val);

    yz1->parameters["width"]->addObserver(ywidth_val);
    yz1->parameters["height"]->addObserver(zwidth_val);
    yz1->parameters["uNum"]->addObserver(ynum_val);
    yz1->parameters["vNum"]->addObserver(znum_val);
    yz2->parameters["width"]->addObserver(ywidth_val);
    yz2->parameters["height"]->addObserver(zwidth_val);
    yz2->parameters["uNum"]->addObserver(ynum_val);
    yz2->parameters["vNum"]->addObserver(znum_val);


    xwidth_val->addObserver(cmesh);
    ywidth_val->addObserver(cmesh);
    zwidth_val->addObserver(cmesh);
    xnum_val->addObserver(cmesh);
    ynum_val->addObserver(cmesh);
    znum_val->addObserver(cmesh);
    cmesh->parameters["xWidth"] = xwidth_val;
    cmesh->parameters["yWidth"] = ywidth_val;
    cmesh->parameters["zWidth"] = zwidth_val;
    cmesh->parameters["xNum"]   = xnum_val;
    cmesh->parameters["yNum"]   = ynum_val;
    cmesh->parameters["zNum"]   = znum_val;

    // 初始化更新
    cmesh->update();
    return cmesh;
  }

  static shared_ptr<CompositeMesh> Cylinder(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum) {
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto height_val = make_shared<ReflectValue>("height", height);

    auto pnum_val = make_shared<ReflectValue>("PNum", PNum);
    auto rnum_val = make_shared<ReflectValue>("RNum", RNum);
    auto hnum_val = make_shared<ReflectValue>("HNum", HNum);

    auto bottom = Mesh::Disk(radius, PNum, RNum);
    auto top    = Mesh::Disk(radius, PNum, RNum);
    auto side   = Mesh::CylinderSide(radius, height, PNum, HNum);
    // 组装cmesh
    auto cmesh = make_shared<CompositeMesh>();
    cmesh->meshes.emplace_back(bottom);
    cmesh->meshes.emplace_back(top);
    cmesh->meshes.emplace_back(side);
    // 构建坐标变换
    cmesh->transforms.emplace_back([radius_val, height_val]() {   // bottom
      float     radius = std::get<float>(radius_val->getProp());
      float     height = std::get<float>(height_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(-90.0f), _right);
      trans = glm::rotate(trans, glm::radians(180.0f), _right);
      return trans;
    });
    cmesh->transforms.emplace_back([radius_val, height_val]() {   // top
      float     radius = std::get<float>(radius_val->getProp());
      float     height = std::get<float>(height_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(-90.0f), _right);
      trans = glm::translate(trans, glm::vec3(0.0f, height, 0.0f));
      return trans;
    });
    cmesh->transforms.emplace_back([radius_val, height_val]() {   // side
      float     radius = std::get<float>(radius_val->getProp());
      float     height = std::get<float>(height_val->getProp());
      glm::mat4 trans(1.0f);
      trans = glm::rotate(trans, glm::radians(-90.0f), _right);
      return trans;
    });

    // 参数映射方案
    bottom->parameters["radius"]->addObserver(radius_val);
    bottom->parameters["uNum"]->addObserver(pnum_val);
    bottom->parameters["vNum"]->addObserver(rnum_val);

    top->parameters["radius"]->addObserver(radius_val);
    top->parameters["uNum"]->addObserver(pnum_val);
    top->parameters["vNum"]->addObserver(rnum_val);

    side->parameters["radius"]->addObserver(radius_val);
    side->parameters["height"]->addObserver(height_val);
    side->parameters["uNum"]->addObserver(pnum_val);
    side->parameters["vNum"]->addObserver(hnum_val);

    // 添加观察者
    radius_val->addObserver(cmesh);
    height_val->addObserver(cmesh);
    pnum_val->addObserver(cmesh);
    rnum_val->addObserver(cmesh);
    hnum_val->addObserver(cmesh);

    cmesh->parameters["radius"] = radius_val;
    cmesh->parameters["height"] = height_val;
    cmesh->parameters["PNum"]   = pnum_val;
    cmesh->parameters["RNum"]   = rnum_val;
    cmesh->parameters["HNum"]   = hnum_val;

    return cmesh;
  }

  static shared_ptr<CompositeMesh> Cone(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum) {

    return nullptr;
  }

  static shared_ptr<CompositeMesh> Arrow(float radius, float length) {

    // 定义lambda变量


    // 定义构成的曲面


    // 组装曲面


    // 构建坐标变换


    // 参数映射方案

    // 配置观察者


    // cmesh添加parameters


    return nullptr;
  }
};

// class Sphere : public Mesh {
//   public:
//   Sphere(float radius, uint32_t uNum = 100, uint32_t vNum = 100)
//     : Mesh(uNum, vNum) {
//     this->parameters["radius"] = radius;
//     update();
//   }

//   // virtual void update() {
//   //   this->reset();
//   //   this->updateVertex([this](float u, float v) -> Vertex {
//   //     Vertex vt;
//   //     float  radius = std::get<float>(this->parameters["radius"]);

//   //     vt.nx = sin(PI * u) * cos(-2 * PI * v);
//   //     vt.ny = sin(PI * u) * sin(-2 * PI * v);
//   //     vt.nz = cos(PI * u);

//   //     vt.x = radius * vt.normal[0];
//   //     vt.y = radius * vt.normal[1];
//   //     vt.z = radius * vt.normal[2];

//   //     vt.r = vt.x;
//   //     vt.g = vt.y;
//   //     vt.b = vt.z;

//   //     vt.u = u;
//   //     vt.v = v;
//   //     return vt;
//   //   });

//   //   this->rotate(glm::radians(-90.0f), _right);
//   // }
// };

// class Disk : public Mesh {
//   private:
//   public:
//   Disk(float r, uint32_t RNum = 8, uint32_t PNum = 18)
//     : Mesh(RNum, PNum) {
//     this->parameters["r"] = r;
//     update();
//   }
//   // virtual void update() {
//   //   this->updateVertex([this](float u, float v) {
//   //     Vertex vt;
//   //     float  r = std::get<float>(this->parameters["r"]);
//   //     vt.x     = r * u * cos(-2 * PI * v);
//   //     vt.y     = r * u * sin(-2 * PI * v);
//   //     vt.z     = 0.0f;

//   //     vt.nx = 0.0f;
//   //     vt.ny = 0.0f;
//   //     vt.nz = 1.0f;

//   //     vt.r = 0.0f;
//   //     vt.g = 0.0f;
//   //     vt.b = 1.0f;

//   //     vt.u = u;
//   //     vt.v = v;

//   //     return vt;
//   //   });

//   //   this->rotate(glm::radians(-90.0f), _right);
//   // }
// };

// class ConeSide : public Mesh {
//   private:
//   public:
//   ConeSide(float r, float h, uint32_t HNum = 10, uint32_t PNum = 18)
//     : Mesh(HNum, PNum) {
//     this->parameters["r"] = r;
//     this->parameters["h"] = h;
//     update();
//   }
//   // virtual void update() {
//   //   this->updateVertex([this](float u, float v) {
//   //     Vertex vt;
//   //     float  r = std::get<float>(this->parameters["r"]);
//   //     float  h = std::get<float>(this->parameters["h"]);
//   //     vt.x     = r * (1 - u) * cos(2 * PI * v);
//   //     vt.y     = r * (1 - u) * sin(2 * PI * v);
//   //     vt.z     = h * u;

//   //     float tmp = sqrt(r * r + h * h);
//   //     vt.nx     = h / tmp * cos(2 * PI * v);
//   //     vt.ny     = h / tmp * sin(2 * PI * v);
//   //     vt.nz     = r / tmp;

//   //     vt.r = 1.0f;
//   //     vt.g = 0.0f;
//   //     vt.b = 0.0f;

//   //     vt.u = u;
//   //     vt.v = v;

//   //     return vt;
//   //   });

//   //   this->rotate(glm::radians(-90.0f), _right);
//   // }
// };

// class Cone : public Geometry {
//   public:
//   Cone(float radius, float height, uint32_t RNum = 8, uint32_t HNum = 10, uint32_t PNum = 18) {

//     this->parameters["radius"] = radius;
//     this->parameters["height"] = height;
//     this->parameters["PNum"]   = PNum;
//     this->parameters["RNum"]   = RNum;
//     this->parameters["HNum"]   = HNum;

//     this->update();
//   }

//   virtual void update() {
//     float    radius = std::get<float>(this->parameters["radius"]);
//     float    height = std::get<float>(this->parameters["height"]);
//     uint32_t RNum   = std::get<uint32_t>(this->parameters["RNum"]);
//     uint32_t HNum   = std::get<uint32_t>(this->parameters["HNum"]);
//     uint32_t PNum   = std::get<uint32_t>(this->parameters["PNum"]);

//     ConeSide cs(radius, height, HNum, PNum);
//     Disk     ds(radius, RNum, PNum);

//     ds.rotate(glm::radians(180.0f), {1.0f, 0.0f, 0.0f});

//     // 组合
//     // todo

//     // FixedGeometry &&res = cs + ds;
//     // // res.rotate(glm::radians(-90.0f), _right);
//     // this->vertices = res.vertices;
//     // this->surfaces = res.surfaces;
//   }
// };

// class CylinderSide : public Mesh {
//   private:
//   public:
//   CylinderSide(float r, float h, uint32_t HNum = 10, uint32_t PNum = 18)
//     : Mesh(HNum, PNum) {
//     this->parameters["r"] = r;
//     this->parameters["h"] = h;
//     update();
//   }
//   // virtual void update() {
//   //   this->updateVertex([this](float u, float v) {
//   //     Vertex vt;
//   //     float  r = std::get<float>(this->parameters["r"]);
//   //     float  h = std::get<float>(this->parameters["h"]);
//   //     vt.x     = r * cos(2 * PI * v);
//   //     vt.y     = r * sin(2 * PI * v);
//   //     vt.z     = h * u;

//   //     vt.nx = cos(2 * PI * v);
//   //     vt.ny = sin(2 * PI * v);
//   //     vt.nz = 0.0f;

//   //     vt.r = 1.0f;
//   //     vt.g = 0.0f;
//   //     vt.b = 1.0f;

//   //     vt.u = u;
//   //     vt.v = v;

//   //     return vt;
//   //   });
//   //   this->rotate(glm::radians(-90.0f), _right);
//   // }
// };

// class TruncatedConeSide : public Mesh {
//   private:
//   public:
//   TruncatedConeSide(float r1, float r2, float h, uint32_t RNum = 8, uint32_t HNum = 10, uint32_t PNum = 18)
//     : Mesh(HNum, PNum) {
//     this->parameters["r1"] = r1;
//     this->parameters["r2"] = r2;
//     this->parameters["h"]  = h;
//     update();
//   }
//   // virtual void update() {
//   //   this->updateVertex([this](float u, float v) {
//   //     Vertex vt;
//   //     float  r1 = std::get<float>(this->parameters["r1"]);
//   //     float  r2 = std::get<float>(this->parameters["r2"]);
//   //     float  h  = std::get<float>(this->parameters["h"]);

//   //     float interp_r = u * (r2 - r1) + r1;
//   //     vt.x           = interp_r * cos(2 * PI * v);
//   //     vt.y           = interp_r * sin(2 * PI * v);
//   //     vt.z           = h * u;

//   //     float tmp = sqrt(h * h + pow(r1 - r2, 2.0f));
//   //     vt.nx     = h / tmp * cos(2 * PI * v);
//   //     vt.ny     = h / tmp * sin(2 * PI * v);
//   //     vt.nz     = (r1 - r2) / tmp;

//   //     vt.r = 1.0f;
//   //     vt.g = 0.0f;
//   //     vt.b = 0.0f;

//   //     vt.u = u;
//   //     vt.v = v;

//   //     return vt;
//   //   });

//   //   this->rotate(glm::radians(-90.0f), _right);
//   // }
// };

// class Cylinder : public Geometry {
//   // todo 改为惰性构造
//   public:
//   Cylinder(float radius, float height, uint32_t RNum = 8, uint32_t HNum = 10, uint32_t PNum = 18) {
//     this->parameters["radius"] = radius;
//     this->parameters["height"] = height;
//     this->parameters["RNum"]   = RNum;
//     this->parameters["HNum"]   = HNum;
//     this->parameters["PNum"]   = PNum;

//     this->update();
//   }

//   virtual void update() {
//     float    radius = std::get<float>(this->parameters["radius"]);
//     float    height = std::get<float>(this->parameters["height"]);
//     uint32_t RNum   = std::get<uint32_t>(this->parameters["RNum"]);
//     uint32_t HNum   = std::get<uint32_t>(this->parameters["HNum"]);
//     uint32_t PNum   = std::get<uint32_t>(this->parameters["PNum"]);

//     CylinderSide cs(radius, height, HNum, PNum);
//     Disk         ds_bottom(radius, RNum, PNum), ds_top(radius, RNum, PNum);

//     ds_bottom.rotate(glm::radians(180.0f), {1.0f, 0.0f, 0.0f});
//     ds_top.translate(0.0f, height, 0.0f);

//     // 组合
//     // todo
//     // FixedGeometry &&res = cs + ds_top + ds_bottom;
//     // // res.rotate(glm::radians(90.0f), _right);
//     // this->vertices = res.vertices;
//     // this->surfaces = res.surfaces;
//   }
// };

// class TruncatedConeEx : public Geometry {
//   private:
//   public:
//   TruncatedConeEx()
//     : TruncatedConeEx(1.0f, 1.0f, 3.0f, 0.0f, 0.0f) {}
//   TruncatedConeEx(float r1, float r2, float h, float phi, float rho, uint32_t RNum = 8, uint32_t HNum = 10, uint32_t PNum = 18) {
//     this->parameters["r1"]   = r1;
//     this->parameters["r2"]   = r2;
//     this->parameters["h"]    = h;
//     this->parameters["phi"]  = phi;
//     this->parameters["rho"]  = rho;
//     this->parameters["RNum"] = RNum;
//     this->parameters["HNum"] = HNum;
//     this->parameters["PNum"] = PNum;
//     update();
//   }

//   virtual void update() {
//     float r1   = std::get<float>(this->parameters["r1"]);
//     float r2   = std::get<float>(this->parameters["r2"]);
//     float h    = std::get<float>(this->parameters["h"]);
//     float phi  = std::get<float>(this->parameters["phi"]);
//     float rho  = std::get<float>(this->parameters["rho"]);
//     float RNum = std::get<uint32_t>(this->parameters["RNum"]);
//     float HNum = std::get<uint32_t>(this->parameters["HNum"]);
//     float PNum = std::get<uint32_t>(this->parameters["PNum"]);

//     Disk              bottom(r1, RNum, PNum);
//     Disk              top(r2, RNum, PNum);
//     TruncatedConeSide side(r1, r2, h, RNum, HNum, PNum);
//     side.setColor(0.8f, 0.2f, 0.6f);
//     top.setColor(1.0f, 1.0f, 0.0f);
//     bottom.rotate(glm::radians(180.0f), {1.0f, 0.0f, 0.0f});
//     top.translate(0.0f, h, 0.0f);

//     top.transformVertex([phi, rho](const Vertex& vt0, float u, float v) {
//       Vertex    vt(vt0);
//       glm::mat3 rot_mat = glm::mat3(glm::rotate(
//         glm::mat4(1.0f),
//         rho,
//         glm::vec3(cos(phi), 0.0f, sin(phi))));

//       glm::vec3 n_pos  = rot_mat * glm::make_vec3(vt.position);
//       glm::vec3 n_norm = rot_mat * glm::make_vec3(vt.normal);
//       vt.x             = n_pos.x;
//       vt.y             = n_pos.y;
//       vt.z             = n_pos.z;
//       vt.nx            = n_norm.x;
//       vt.ny            = n_norm.y;
//       vt.nz            = n_norm.z;
//       return vt;
//     });
//     side.transformVertex([phi, rho](const Vertex& vt0, float u, float v) {
//       Vertex    vt(vt0);
//       glm::mat3 rot_mat = glm::mat3(glm::rotate(
//         glm::mat4(1.0f),
//         u * rho,
//         glm::vec3(cos(phi), 0.0f, sin(phi))));
//       glm::vec3 n_pos   = rot_mat * glm::make_vec3(vt.position);
//       glm::vec3 n_norm  = rot_mat * glm::make_vec3(vt.normal);
//       vt.x              = n_pos.x;
//       vt.y              = n_pos.y;
//       vt.z              = n_pos.z;
//       vt.nx             = n_norm.x;
//       vt.ny             = n_norm.y;
//       vt.nz             = n_norm.z;
//       return vt;
//     });

//     // 组合
//     // todo
//     // FixedGeometry &&res = bottom + top + side;
//     // // res.rotate(glm::radians(-90.0f), _right);
//     // this->vertices = res.vertices;
//     // this->surfaces = res.surfaces;
//   }
// };

// class Arrow : public Geometry {
//   private:
//   const float body_ratio   = 0.8f;
//   const float radius_ratio = 0.5f;

//   public:
//   Arrow(float radius, float length, glm::vec3 arrow_color, glm::vec3 body_color) {
//     this->parameters["radius"]      = radius;
//     this->parameters["length"]      = length;
//     this->parameters["arrow_color"] = arrow_color;
//     this->parameters["body_color"]  = body_color;
//     update();
//   }
//   Arrow(float radius, float length)
//     : Arrow(radius, length, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}) {}

//   virtual void update() {
//     float     radius = std::get<float>(this->parameters["radius"]);
//     float     length = std::get<float>(this->parameters["length"]);
//     glm::vec3 arrow_color =
//       std::get<glm::vec3>(this->parameters["arrow_color"]);
//     glm::vec3 body_color = std::get<glm::vec3>(this->parameters["body_color"]);

//     Cone     arrow(radius, 1 - body_ratio);
//     Cylinder body(radius_ratio * radius, body_ratio * length);

//     arrow.setColor(arrow_color.r, arrow_color.g, arrow_color.b);
//     body.setColor(body_color.r, body_color.g, body_color.b);

//     arrow.translate(0.0f, body_ratio * length, 0.0f);

//     // todo
//     // FixedGeometry a = FixedGeometry(std::move(arrow)) + body;
//     // // a.rotate(glm::radians(-90.0f), _right);
//     // this->vertices = a.vertices;
//     // this->surfaces = a.surfaces;
//   }
// };

// class Plane : public Mesh {
//   public:
//   Plane(float width, float height, uint32_t uNum = 10, uint32_t vNum = 10)
//     : Mesh(uNum, vNum) {
//     this->parameters["width"]  = width;
//     this->parameters["height"] = height;
//     update();
//   }

//   // virtual void update() {
//   //   this->updateVertex([this](float u, float v) {
//   //     float width  = std::get<float>(this->parameters["width"]);
//   //     float height = std::get<float>(this->parameters["height"]);

//   //     Vertex vt;
//   //     vt.x = (0.5f - u) * width;
//   //     vt.y = (v - 0.5f) * height;
//   //     vt.z = 0.0f;

//   //     vt.nx = 0.0f;
//   //     vt.ny = 0.0f;
//   //     vt.nz = 1.0f;

//   //     vt.r = 1.0f;
//   //     vt.g = 1.0f;
//   //     vt.b = 0.0f;

//   //     vt.u = u;
//   //     vt.v = v;
//   //     return vt;
//   //   });
//   //   this->rotate(glm::radians(-90.0f), _right);
//   // }
// };

// class Cube : public Geometry {
//   public:
//   Cube(float xWidth, float yWidth, float zWidth, uint32_t xNum = 10, uint32_t yNum = 10, uint32_t zNum = 10) {
//     this->parameters["xWidth"] = xWidth;
//     this->parameters["yWidth"] = yWidth;
//     this->parameters["zWidth"] = zWidth;
//     this->parameters["xNum"]   = xNum;
//     this->parameters["yNum"]   = yNum;
//     this->parameters["zNum"]   = zNum;
//     update();
//   }

//   virtual void update() {
//     float xWidth = std::get<float>(this->parameters["xWidth"]);
//     float yWidth = std::get<float>(this->parameters["yWidth"]);
//     float zWidth = std::get<float>(this->parameters["zWidth"]);

//     uint32_t xNum = std::get<uint32_t>(this->parameters["xNum"]);
//     uint32_t yNum = std::get<uint32_t>(this->parameters["yNum"]);
//     uint32_t zNum = std::get<uint32_t>(this->parameters["zNum"]);

//     Plane back(xWidth, yWidth, xNum, yNum), front(xWidth, yWidth, xNum, yNum);
//     Plane left(yWidth, zWidth, yNum, zNum), right(yWidth, zWidth, yNum, zNum);
//     Plane bottom(xWidth, zWidth, xNum, zNum), top(xWidth, zWidth, xNum, zNum);

//     bottom.rotate(glm::radians(180.0f), _right);

//     top.translate(0.0f, yWidth, 0.0f);

//     left.rotate(glm::radians(90.0f), -_front);
//     left.translate(-xWidth / 2.0f, yWidth / 2.0f, 0.0f);

//     right.rotate(glm::radians(90.0f), _front);
//     right.translate(xWidth / 2.0f, yWidth / 2.0f, 0.0f);

//     front.rotate(glm::radians(90.0f), _right);
//     front.translate(0.0f, yWidth / 2.0f, zWidth / 2.0f);

//     back.rotate(glm::radians(90.0f), -_right);
//     back.translate(0.0f, yWidth / 2.0f, -zWidth / 2.0f);

//     // FixedGeometry &&res =
//     //     FixedGeometry(left) + right + top + bottom + front + back;
//     // this->vertices = res.vertices;
//     // this->surfaces = res.surfaces;
//     // todo
//   }
// };

}   // namespace