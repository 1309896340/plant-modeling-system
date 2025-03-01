#pragma once

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
  function<float(prop)>      func{nullptr};   // 可能存在函数关系的映射

  public:
  ReflectValue(const string& name, prop init_value)
    : name(name)
    , value(init_value) {}
  ReflectValue(const string& name, prop init_value, weak_ptr<Observer> observer)
    : name(name)
    , value(init_value) {
    this->observers.emplace_back(observer);
  }
  // ReflectValue(const string& name, prop init_value, shared_ptr<Observer>
  // observer)
  //   : name(name)
  //   , value(init_value) {
  //   this->observers.emplace_back(observer);
  // }
  prop& getProp() { return this->value; }

  virtual void notify(const string& name, const prop& param) {
    // 实现ReflectValue之间的单向绑定
    if (this->func != nullptr)
      this->value = this->func(param);
    else
      this->value = param;
    // 然后连锁触发
    this->notifyAll();
  }

  void notifyAll() {   // 外部有修改时手动调用
    // 移除this->observers中已经失效的元素
    erase_if(this->observers,
             [](weak_ptr<Observer> obs) { return obs.expired(); });
    for (auto& obs : this->observers) {
      assert(!obs.expired() &&
             "ReflectValue::notifyAll(): observer pointer is expired!");
      // std::visit([this](auto&& arg) {
      //   using T = std::decay_t<decltype(arg)>;
      //   if constexpr (std::is_same_v<T, float>) {
      //     printf("name: %s value: %f\n", this->name.c_str(), arg);
      //   }
      //   else if constexpr (std::is_same_v<T, uint32_t>) {
      //     printf("name: %s value: %u\n", this->name.c_str(), arg);
      //   }
      // }, this->value);
      obs.lock()->notify(
        this->name,
        this->value);   // Geometry需要定义notify(string, prop)接口
    }
  }
  string getName() const { return this->name; }
  void   addObserver(weak_ptr<Observer> observer) {
    assert(!observer.expired() &&
           "ReflectVal::addObserver(): observer is expired!");
    this->observers.emplace_back(observer);
  }
};

class Geometry : public Observer {
  protected:
  vector<Vertex>  vertices;
  vector<Surface> surfaces;

  public:
    // map<string, shared_ptr<ReflectValue>> parameters;
    // map<string, shared_ptr<ReflectValue>> parameters;
    // map<string, param_variant> parameters;
    // prop parameters;
    
    map<string, shared_ptr<ReflectValue>> topo_parameters;
    map<string, shared_ptr<ReflectValue>> geom_parameters;

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


struct FixedGeometry {
  vector<Vertex>  vertices;
  vector<Surface> surfaces;
  FixedGeometry() = default;
  FixedGeometry(const vector<Vertex>& vertices, const vector<Surface>& surfaces)
    : vertices(vertices)
    , surfaces(surfaces) {}
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
    this->topo_parameters["uNum"] = make_shared<ReflectValue>("uNum", uNum);
    this->topo_parameters["vNum"] = make_shared<ReflectValue>("vNum", vNum);

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
    this->topo_parameters["uNum"] = make_shared<ReflectValue>("uNum", uNum);
    this->topo_parameters["vNum"] = make_shared<ReflectValue>("vNum", vNum);
    this->updater            = updater;
  }

  virtual void update() {
    uint32_t uNum = std::get<uint32_t>(this->topo_parameters["uNum"]->getProp());
    uint32_t vNum = std::get<uint32_t>(this->topo_parameters["vNum"]->getProp());
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
          //   this->surfaces[ptr + 0] = {j + i * (vNum + 1), 1 + j + i * (vNum
          //   + 1), 1 + j + (i + 1) * (vNum + 1)}; this->surfaces[ptr + 1] = {j
          //   + i * (vNum + 1),
          //                             1 + j + (i + 1) * (vNum + 1),
          //                             j + (i + 1) * (vNum + 1)};
          // }
        }
      }
    }
  }

  // 由于updater的参数构建比较复杂，且比较抽象，以下的简单赋值并不足以正确完成updater的替换
  // 下面这种做法是存在问题的，先移除
  void setUpdater(MeshUpdater updater) { this->updater = updater; }

  void resize() {
    uint32_t uNum = std::get<uint32_t>(this->topo_parameters["uNum"]->getProp());
    uint32_t vNum = std::get<uint32_t>(this->topo_parameters["vNum"]->getProp());

    this->vertices.resize((uNum + 1) * (vNum + 1));
    this->surfaces.resize(uNum * vNum * 2);
  }

  virtual void notify(const string& name, const prop& param) {
    // todo
    // 需要根据name进行判断
    // 如果name=="uNum"或name=="vNum"，则要先调用rebuildTopo()
    // 之后统一调用一次this->update更新参数曲面的顶点数据
    // 注意：暂时无法确定Observer处定义的这个接口是否被正确重写


    // std::visit([name](auto&& arg) {
    //   using T = std::decay_t<decltype(arg)>;
    //   if constexpr (std::is_same_v<T, float>) {
    //     printf("Mesh更新变量\"%s\" 新值：%f\n", name.c_str(), arg);
    //   }
    //   else if constexpr (std::is_same_v<T, uint32_t>) {
    //     printf("Composition更新变量\"%s\" 新值：%u\n", name.c_str(), arg);
    //   }
    // },
    //            param);
    
    if (name == "uNum" || name == "vNum")
      this->rebuildTopo();
    this->update();
  }

  void rebuildTopo() {
    // 外部修改了uNum和vNum，下一次update()调用将重新生成拓扑
    uint32_t uNum = std::get<uint32_t>(this->topo_parameters["uNum"]->getProp());
    uint32_t vNum = std::get<uint32_t>(this->topo_parameters["vNum"]->getProp());

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
  //       this->surfaces[ptr + 0] = {j + i * (vNum + 1), 1 + j + i * (vNum +
  //       1), 1 + j + (i + 1) * (vNum + 1)}; this->surfaces[ptr + 1] = {j + i *
  //       (vNum + 1),
  //                                  1 + j + (i + 1) * (vNum + 1),
  //                                  j + (i + 1) * (vNum + 1)};
  //     }
  //   }
  // }
  // }

  // void transformVertex(function<Vertex(const Vertex &, float, float)> func) {
  //   uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]->getProp());
  //   uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]->getProp());
  //   for (int i = 0; i <= uNum; i++) {
  //     for (int j = 0; j <= vNum; j++) {
  //       float u = static_cast<float>(i) / uNum;
  //       float v = static_cast<float>(j) / vNum;
  //       Vertex &vt = this->vertices[j + i * (vNum + 1)];
  //       vt = func(vt, u, v);
  //     }
  //   }
  // }

  static shared_ptr<Mesh> Sphere(float radius, uint32_t PNum, uint32_t VNum) {
    auto mesh = make_shared<Mesh>(PNum, VNum);
    auto updater = [mesh](float u, float v) {
      float radius = std::get<float>(mesh->geom_parameters["radius"]->getProp());
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
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    radius_val->addObserver(mesh);
    mesh->geom_parameters["radius"] = radius_val;
    mesh->topo_parameters["uNum"]->addObserver(mesh);
    mesh->topo_parameters["vNum"]->addObserver(mesh);
    // 进行一次初始的更新
    mesh->update();
    return mesh;
  }

  static shared_ptr<Mesh> Disk(float radius, uint32_t PNum, uint32_t RNum) {
    auto mesh = make_shared<Mesh>(PNum, RNum);
    auto updater = [mesh](float u, float v) {
      float radius = std::get<float>(mesh->geom_parameters["radius"]->getProp());

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
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    radius_val->addObserver(mesh);
    mesh->geom_parameters["radius"] = radius_val;
    mesh->topo_parameters["uNum"]->addObserver(mesh);
    mesh->topo_parameters["vNum"]->addObserver(mesh);
    // 进行一次初始的更新
    mesh->update();
    return mesh;
  }

  static shared_ptr<Mesh> ConeSide(float radius, float height, uint32_t PNum, uint32_t HNum) {
    auto mesh = make_shared<Mesh>(PNum, HNum);
    auto updater = [mesh](float u, float v) {
      float radius = std::get<float>(mesh->geom_parameters["radius"]->getProp());
      float height = std::get<float>(mesh->geom_parameters["height"]->getProp());
      Vertex vt;

      vt.x = radius * (1 - v) * cos(-2 * PI * u);
      vt.y = height * v;
      vt.z = radius * (1 - v) * sin(-2 * PI * u);

      float tmp = sqrt(radius * radius + height * height);
      vt.nx = height / tmp * cos(-2 * PI * u);
      vt.ny = radius / tmp;
      vt.nz = height / tmp * sin(-2 * PI * u);

      vt.r = 1.0f;
      vt.g = 0.0f;
      vt.b = 0.0f;

      vt.u = u;
      vt.v = v;

      return vt;
    };
    mesh->setUpdater(updater);
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto height_val = make_shared<ReflectValue>("height", height);
    radius_val->addObserver(mesh);
    height_val->addObserver(mesh);
    mesh->geom_parameters["radius"] = radius_val;
    mesh->geom_parameters["height"] = height_val;
    mesh->topo_parameters["uNum"]->addObserver(mesh);
    mesh->topo_parameters["vNum"]->addObserver(mesh);
    // 进行一次初始的更新
    mesh->update();
    return mesh;
  }

  static shared_ptr<Mesh> CylinderSide(float radius, float height, uint32_t PNum, uint32_t HNum) {
    auto mesh = make_shared<Mesh>(PNum, HNum);
    auto updater = [mesh](float u, float v) -> Vertex {
      float radius = std::get<float>(mesh->geom_parameters["radius"]->getProp());
      float height = std::get<float>(mesh->geom_parameters["height"]->getProp());
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
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto height_val = make_shared<ReflectValue>("height", height);
    radius_val->addObserver(mesh);
    height_val->addObserver(mesh);
    mesh->geom_parameters["radius"] = radius_val;
    mesh->geom_parameters["height"] = height_val;
    mesh->topo_parameters["uNum"]->addObserver(mesh);
    mesh->topo_parameters["vNum"]->addObserver(mesh);
    // 进行一次初始的更新
    mesh->update();
    return mesh;
  }

  static shared_ptr<Mesh> Plane(float width, float height, uint32_t VNum, uint32_t HNum) {
    auto mesh = make_shared<Mesh>(VNum, HNum);
    auto updater = [mesh](float u, float v) {
      float width = std::get<float>(mesh->geom_parameters["width"]->getProp());
      float height =
          std::get<float>(mesh->geom_parameters["height"]->getProp());

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
    auto width_val  = make_shared<ReflectValue>("width", width);
    auto height_val = make_shared<ReflectValue>("height", height);
    width_val->addObserver(mesh);
    height_val->addObserver(mesh);
    mesh->geom_parameters["width"]  = width_val;
    mesh->geom_parameters["height"] = height_val;
    mesh->topo_parameters["uNum"]->addObserver(mesh);
    mesh->topo_parameters["vNum"]->addObserver(mesh);
    // 进行一次初始的更新
    mesh->update();
    return mesh;
  }
};

using TransformUpdater = function<glm::mat4()>;

class Composition : public Geometry {
  private:
  vector<shared_ptr<Geometry>> goemetries;
  vector<TransformUpdater>     transforms;

  public:
  Composition() = default;

  void pushGeometry(shared_ptr<Geometry> mesh, const TransformUpdater& transform) {
    // if (this->goemetries.empty()) {
    //   this->parameters = mesh->parameters;
    //   for (auto& param_pair : mesh->parameters) {
    //     param_pair.second->addObserver(shared_from_this());
    //     // 似乎无法将this转换为weak_ptr，因此只能将Observer继承enable_shared_from_this
    //   }
    // }
    // else {
    //   for (auto& param_pair : mesh->parameters) {
    //     // 如果this->parameters中已经存在同名变量，则以this->parameters中的为准
    //     if (!this->parameters.contains(param_pair.first)) {
    //       this->parameters[param_pair.first] = param_pair.second;
    //       param_pair.second->addObserver(shared_from_this());
    //     }
    //     // todo，此处需要仔细考虑当Composition::parameters的元素发生改变时，如何驱动Mesh::parameters动作的问题
    //   }
    // }

    this->goemetries.emplace_back(mesh);
    this->transforms.emplace_back(transform);
  }

  void popMesh() {
    this->goemetries.pop_back();
    this->transforms.pop_back();
  }

  virtual void notify(const string& name, const prop& param) {
    // 调整this->meshes和this->transforms中受影响的Mesh实例
    std::visit([name](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, float>) {
        printf("Composition更新变量\"%s\" 新值：%f\n", name.c_str(), arg);
      }
      else if constexpr (std::is_same_v<T, uint32_t>) {
        printf("Composition更新变量\"%s\" 新值：%u\n", name.c_str(), arg);
      }
    },
               param);

    this->update();
  }

  virtual void update() {
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

  static shared_ptr<Composition> Cube(float xWidth, float yWidth, float zWidth, uint32_t xNum, uint32_t yNum, uint32_t zNum) {
    auto xwidth_val = make_shared<ReflectValue>("xWidth", xWidth);
    auto ywidth_val = make_shared<ReflectValue>("yWidth", yWidth);
    auto zwidth_val = make_shared<ReflectValue>("zWidth", zWidth);
    auto xnum_val   = make_shared<ReflectValue>("XNum", xNum);
    auto ynum_val   = make_shared<ReflectValue>("YNum", yNum);
    auto znum_val   = make_shared<ReflectValue>("ZNum", zNum);

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
    comp->pushGeometry(xz1,
                       [xwidth_val, ywidth_val, zwidth_val]() {   // xz1 下
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
    xnum_val->addObserver(xy1->topo_parameters["uNum"]);
    xnum_val->addObserver(xy2->topo_parameters["uNum"]);
    xnum_val->addObserver(xz1->topo_parameters["uNum"]);
    xnum_val->addObserver(xz2->topo_parameters["uNum"]);
    ynum_val->addObserver(yz1->topo_parameters["uNum"]);
    ynum_val->addObserver(yz2->topo_parameters["uNum"]);

    ynum_val->addObserver(xy1->topo_parameters["vNum"]);
    ynum_val->addObserver(xy2->topo_parameters["vNum"]);
    znum_val->addObserver(xz1->topo_parameters["vNum"]);
    znum_val->addObserver(xz2->topo_parameters["vNum"]);
    znum_val->addObserver(yz1->topo_parameters["vNum"]);
    znum_val->addObserver(yz2->topo_parameters["vNum"]);

    xwidth_val->addObserver(xy1->geom_parameters["width"]);
    xwidth_val->addObserver(xy2->geom_parameters["width"]);
    xwidth_val->addObserver(xz1->geom_parameters["width"]);
    xwidth_val->addObserver(xz2->geom_parameters["width"]);
    ywidth_val->addObserver(yz1->geom_parameters["width"]);
    ywidth_val->addObserver(yz2->geom_parameters["width"]);

    ywidth_val->addObserver(xy1->geom_parameters["height"]);
    ywidth_val->addObserver(xy2->geom_parameters["height"]);
    zwidth_val->addObserver(xz1->geom_parameters["height"]);
    zwidth_val->addObserver(xz2->geom_parameters["height"]);
    zwidth_val->addObserver(yz1->geom_parameters["height"]);
    zwidth_val->addObserver(yz2->geom_parameters["height"]);

    // 添加观察者
    xwidth_val->addObserver(comp);
    ywidth_val->addObserver(comp);
    zwidth_val->addObserver(comp);
    xnum_val->addObserver(comp);
    ynum_val->addObserver(comp);
    znum_val->addObserver(comp);
    comp->geom_parameters["xWidth"] = xwidth_val;
    comp->geom_parameters["yWidth"] = ywidth_val;
    comp->geom_parameters["zWidth"] = zwidth_val;
    comp->topo_parameters["XNum"]   = xnum_val;
    comp->topo_parameters["YNum"]   = ynum_val;
    comp->topo_parameters["ZNum"]   = znum_val;

    // 初始化更新
    comp->update();
    return comp;
  }

  static shared_ptr<Composition> Cylinder(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum) {
    // 声明响应式参数
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto height_val = make_shared<ReflectValue>("height", height);

    auto pnum_val = make_shared<ReflectValue>("PNum", PNum);
    auto rnum_val = make_shared<ReflectValue>("RNum", RNum);
    auto hnum_val = make_shared<ReflectValue>("HNum", HNum);
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
    radius_val->addObserver(bottom->geom_parameters["radius"]);
    radius_val->addObserver(top->geom_parameters["radius"]);
    radius_val->addObserver(side->geom_parameters["radius"]);

    height_val->addObserver(side->geom_parameters["height"]);

    pnum_val->addObserver(bottom->topo_parameters["uNum"]);
    pnum_val->addObserver(top->topo_parameters["uNum"]);
    pnum_val->addObserver(side->topo_parameters["uNum"]);

    rnum_val->addObserver(bottom->topo_parameters["vNum"]);
    rnum_val->addObserver(top->topo_parameters["vNum"]);

    hnum_val->addObserver(side->topo_parameters["vNum"]);

    // 注册观察者
    radius_val->addObserver(comp);
    height_val->addObserver(comp);
    pnum_val->addObserver(comp);
    rnum_val->addObserver(comp);
    hnum_val->addObserver(comp);

    // 注册目标变量
    comp->geom_parameters["radius"] = radius_val;
    comp->geom_parameters["height"] = height_val;
    comp->topo_parameters["PNum"]   = pnum_val;
    comp->topo_parameters["RNum"]   = rnum_val;
    comp->topo_parameters["HNum"]   = hnum_val;

    comp->update();
    return comp;
  }

  static shared_ptr<Composition> Cone(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum) {
    auto radius_val = make_shared<ReflectValue>("radius", radius);
    auto height_val = make_shared<ReflectValue>("height", height);

    auto pnum_val = make_shared<ReflectValue>("PNum", PNum);
    auto rnum_val = make_shared<ReflectValue>("RNum", RNum);
    auto hnum_val = make_shared<ReflectValue>("HNum", HNum);

    auto bottom = Mesh::Disk(radius, PNum, RNum);
    // auto top    = Mesh::Disk(radius, PNum, RNum);
    auto side = Mesh::ConeSide(radius, height, PNum, HNum);
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
    pnum_val->addObserver(bottom->topo_parameters["uNum"]);
    pnum_val->addObserver(side->topo_parameters["uNum"]);
    rnum_val->addObserver(bottom->topo_parameters["vNum"]);
    hnum_val->addObserver(side->topo_parameters["vNum"]);

    radius_val->addObserver(bottom->geom_parameters["radius"]);
    radius_val->addObserver(side->geom_parameters["radius"]);
    height_val->addObserver(side->geom_parameters["height"]);

    radius_val->addObserver(comp);
    height_val->addObserver(comp);
    pnum_val->addObserver(comp);
    rnum_val->addObserver(comp);
    hnum_val->addObserver(comp);

    comp->geom_parameters["radius"] = radius_val;
    comp->geom_parameters["height"] = height_val;
    comp->topo_parameters["PNum"]   = pnum_val;
    comp->topo_parameters["RNum"]   = rnum_val;
    comp->topo_parameters["HNum"]   = hnum_val;

    comp->update();
    return comp;
  }

  static shared_ptr<Composition> Arrow(float radius, float length ) {
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
};

}   // namespace