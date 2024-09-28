#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


#include "constants.h"

#define RADIANS(a) ((a) * PI / 180.0f)

namespace {
using namespace std;
using param_variant = variant<unsigned int, int, float, bool>;

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
  Vertex()
      : position{0.0f, 0.0f, 0.0f}, normal{0.0f, 0.0f, 0.0f},
        color{0.0f, 0.0f, 0.0f}, uv{0u, 0u} {}
};

struct Edge {
  uint32_t pidx[2]; // 2个Vertex构成
};

struct Surface {
  uint32_t tidx[3]; // 3个Vertex构成
};

class Geometry {
protected:
public:
  map<string, param_variant> parameters;

  vector<Vertex> vertices;
  // vector<Edge> edges;
  vector<Surface> surfaces;

  Geometry() = default;
  Geometry(const Geometry &) = default;
  ~Geometry() = default;

  virtual void update() = 0;
  virtual void reset() {
    this->vertices.clear();
    this->surfaces.clear();
  }
};

class FixedGeometry : public Geometry {
public:
  FixedGeometry() {
    // 没有任何可修改属性
    this->parameters.clear();
  }

  FixedGeometry(const Geometry &geometry) {
    // 可以从Geometry构造（这是个有点奇怪的操作，引入了从父类到子类的隐式转换）
    this->vertices = geometry.vertices;
    this->surfaces = geometry.surfaces;
    // 没有任何可修改属性
    this->parameters.clear();
  }

  FixedGeometry(const vector<Vertex> &vertices,
                const vector<Surface> &surfaces) {
    this->vertices = vertices;
    this->surfaces = surfaces;
    // 没有任何可修改属性
    this->parameters.clear();
  }

  // 默认正常拷贝 vertices 和 surfaces
  FixedGeometry(const FixedGeometry &) = default;

  virtual void update() {
    // 空实现，不会修改网格及其顶点的任何属性
  }
  virtual void reset() {
    // 空实现，直到对象自动销毁前，不会有人为的清空操作
  }

  FixedGeometry operator+(const FixedGeometry &b) const {
    FixedGeometry c(*this);
    // 1. 将两组vertices进行简单拼接
    c.vertices.insert(c.vertices.end(), b.vertices.begin(), b.vertices.end());
    // 2. 将b的surfaces的索引偏移this->vertices.size()，然后拼接
    const uint32_t offset = this->vertices.size();
    vector<Surface> sfs = b.surfaces;
    for (auto &surf_ids : sfs) {
      surf_ids.tidx[0] += offset;
      surf_ids.tidx[1] += offset;
      surf_ids.tidx[2] += offset;
    }
    c.surfaces.insert(c.surfaces.end(), sfs.begin(), sfs.end());
    return c;
  }
};

class Mesh : public Geometry {
private:
  uint32_t uNum{0}, vNum{0};

public:
  Mesh() : Mesh(50, 50) {}
  Mesh(uint32_t uNum, uint32_t vNum) : uNum(uNum), vNum(vNum) { reset(); }

  FixedGeometry operator+(const Mesh &other) const {
    FixedGeometry a(*this), b(other);
    return a + b;
  }

  virtual void reset() {
    this->vertices.resize((uNum + 1) * (vNum + 1));
    // this->edges.resize(3*uNum*vNum + uNum + vNum);
    this->surfaces.resize(uNum * vNum * 2);
  }

  void updateVertex(function<Vertex(float, float)> func) {
    reset();
    for (int i = 0; i <= this->uNum; i++) {
      for (int j = 0; j <= this->vNum; j++) {
        this->vertices[j + i * (vNum + 1)] =
            func(static_cast<float>(i) / uNum, static_cast<float>(j) / vNum);
        if (i != this->uNum && j != this->vNum) {
          uint32_t ptr = (j + i * vNum) * 2;
          this->surfaces[ptr + 0] = {j + i * (vNum + 1), 1 + j + i * (vNum + 1),
                                     1 + j + (i + 1) * (vNum + 1)};
          this->surfaces[ptr + 1] = {j + i * (vNum + 1),
                                     1 + j + (i + 1) * (vNum + 1),
                                     j + (i + 1) * (vNum + 1)};
        }
      }
    }
  }

  void transformVertex(function<Vertex(const Vertex &, float, float)> func) {
    // 假定uNum和vNum没有发生改变，该函数也不会修改索引
    for (int i = 0; i <= this->uNum; i++) {
      for (int j = 0; j <= this->vNum; j++) {
        float u = static_cast<float>(i) / uNum;
        float v = static_cast<float>(j) / vNum;
        Vertex &vt = this->vertices[j + i * (vNum + 1)];
        vt = func(vt, u, v);
      }
    }
  }

  virtual void update() {
    // 空实现，将要移除，将Mesh当作接口使用
    // 在没有重构CylinderEx前先保留该空实现
  };
};

class Sphere : public Mesh {
public:
  Sphere(float radius, uint32_t uNum = 100, uint32_t vNum = 100)
      : Mesh(uNum, vNum) {
    this->parameters["radius"] = radius; // radius为float
    update();
  }

  virtual void update() {
    this->reset();
    this->updateVertex([this](float u, float v) -> Vertex {
      Vertex vt;
      float radius = std::get<float>(this->parameters["radius"]);

      vt.nx = sin(PI * u) * cos(-2 * PI * v);
      vt.ny = sin(PI * u) * sin(-2 * PI * v);
      vt.nz = cos(PI * u);

      vt.x = radius * vt.normal[0];
      vt.y = radius * vt.normal[1];
      vt.z = radius * vt.normal[2];

      return vt;
    });
  }
};

// class Cone : public Geometry {
// private:
//   uint32_t RNum; // 半径细分
//   uint32_t HNum; // 高度细分
//   uint32_t PNum; // 圆周细分
// public:
//   Cone() : Cone(0.2f, 1.0f) {}
//   Cone(float r, float h, uint32_t RNum = 8, uint32_t HNum = 10,
//        uint32_t PNum = 36)
//       : RNum(RNum), HNum(HNum), PNum(PNum) {
//     this->parameters["r"] = r;
//     this->parameters["h"] = h;
//     update();
//   }
//   virtual void update() {}
// };

class Disk : public Mesh {
private:
  uint32_t RNum; // 半径细分
  uint32_t PNum; // 圆周细分

public:
  Disk(float r, uint32_t RNum = 8, uint32_t PNum = 18)
      : RNum(RNum), PNum(PNum) {
    this->parameters["r"] = r;
    update();
  }
  virtual void update() {
    this->updateVertex([this](float u, float v) {
      Vertex vt;
      float r = std::get<float>(this->parameters["r"]);
      vt.x = r * u * cos(-2 * PI * v);
      vt.y = r * u * sin(-2 * PI * v);
      vt.z = 0.0f;

      vt.nx = 0.0f;
      vt.ny = 0.0f;
      vt.nz = 1.0f;

      vt.r = 0.0f;
      vt.g = 1.0f;
      vt.b = 0.0f;

      vt.u = u;
      vt.v = v;

      return vt;
    });
  }
};

class ConeSide : public Mesh {
private:
  uint32_t HNum; // 高度细分
  uint32_t PNum; // 圆周细分

public:
  ConeSide(float r, float h, uint32_t HNum = 10, uint32_t PNum = 18)
      : HNum(HNum), PNum(PNum) {
    this->parameters["r"] = r;
    this->parameters["h"] = h;
    update();
  }
  virtual void update() {
    this->updateVertex([this](float u, float v) {
      Vertex vt;
      float r = std::get<float>(this->parameters["r"]);
      float h = std::get<float>(this->parameters["h"]);
      vt.x = r * (1 - u) * cos(2 * PI * v);
      vt.y = r * (1 - u) * sin(2 * PI * v);
      vt.z = h * u;

      float tmp = sqrt(r * r + h * h);
      vt.nx = h / tmp * cos(2 * PI * v);
      vt.ny = h / tmp * sin(2 * PI * v);
      vt.nz = r / tmp;

      vt.r = 1.0f;
      vt.g = 0.0f;
      vt.b = 0.0f;

      vt.u = u;
      vt.v = v;

      return vt;
    });
  }
};

class Cone : public FixedGeometry {
public:
  Cone(float radius, float height, uint32_t RNum = 8, uint32_t HNum = 10,
       uint32_t PNum = 18) {
    ConeSide cs(radius, height, HNum, PNum);
    Disk ds(radius, RNum, PNum);

    // 圆盘方向旋转朝下
    glm::mat3 rot_mat = glm::mat3(
        glm::rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    ds.transformVertex([rot_mat](const Vertex &vt0, float u, float v) {
      Vertex vt(vt0);
      glm::vec3 new_position = rot_mat * glm::make_vec3(vt.position);
      glm::vec3 new_normal = rot_mat * glm::make_vec3(vt.normal);
      vt.x = new_position.x;
      vt.y = new_position.y;
      vt.z = new_position.z;
      vt.nx = new_normal.x;
      vt.ny = new_normal.y;
      vt.nz = new_normal.z;
      return vt;
    });

    // 组合
    FixedGeometry &&res = cs + ds;
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
};

class CylinderSide : public Mesh {
private:
  uint32_t HNum; // 高度细分
  uint32_t PNum; // 圆周细分

public:
  CylinderSide(float r, float h, uint32_t RNum = 8, uint32_t HNum = 10,
               uint32_t PNum = 18)
      : HNum(HNum), PNum(PNum) {
    this->parameters["r"] = r;
    this->parameters["h"] = h;
    update();
  }
  virtual void update() {
    this->updateVertex([this](float u, float v) {
      Vertex vt;
      float r = std::get<float>(this->parameters["r"]);
      float h = std::get<float>(this->parameters["h"]);
      vt.x = r * cos(2 * PI * v);
      vt.y = r * sin(2 * PI * v);
      vt.z = h * u;

      vt.nx = cos(2 * PI * v);
      vt.ny = sin(2 * PI * v);
      vt.nz = 0.0f;

      vt.r = 1.0f;
      vt.g = 0.0f;
      vt.b = 0.0f;

      vt.u = u;
      vt.v = v;

      return vt;
    });
  }
};

class Cylinder : public FixedGeometry {
public:
  Cylinder(float radius, float height, uint32_t RNum = 8, uint32_t HNum = 10,
           uint32_t PNum = 18) {
    CylinderSide cs(radius, height, RNum, HNum, PNum);
    Disk ds_bottom(radius, RNum, PNum), ds_top(radius, RNum, PNum);

    // 圆盘方向旋转朝下
    glm::mat3 rot_mat = glm::mat3(
        glm::rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    ds_bottom.transformVertex([rot_mat](const Vertex &vt0, float u, float v) {
      Vertex vt(vt0);
      glm::vec3 new_position = rot_mat * glm::make_vec3(vt.position);
      glm::vec3 new_normal = rot_mat * glm::make_vec3(vt.normal);
      vt.x = new_position.x;
      vt.y = new_position.y;
      vt.z = new_position.z;
      vt.nx = new_normal.x;
      vt.ny = new_normal.y;
      vt.nz = new_normal.z;
      return vt;
    });

    ds_top.transformVertex([height](const Vertex &vt0, float u, float v) {
      Vertex vt(vt0);
      vt.z += height;
      return vt;
    });

    // 组合
    FixedGeometry &&res = cs + ds_top + ds_bottom;
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
};

// 暂时先不修改，但是update的逻辑后面必须处理一下
class CylinderEx : public Geometry {
private:
  uint32_t RNum; // 半径细分
  uint32_t HNum; // 高度细分
  uint32_t PNum; // 圆周细分

public:
  CylinderEx() : CylinderEx(1.0f, 1.0f, 3.0f, 0.0f, 0.0f) {}
  CylinderEx(float r1, float r2, float h, float phi, float rho,
             uint32_t RNum = 8, uint32_t HNum = 10, uint32_t PNum = 18)
      : RNum(RNum), HNum(HNum), PNum(PNum) {
    this->parameters["r1"] = r1;
    this->parameters["r2"] = r2;
    this->parameters["h"] = h;
    this->parameters["phi"] = phi;
    this->parameters["rho"] = rho;
    update();
  }

  virtual void update() {
    this->reset();
    Mesh bottom(RNum, PNum), side(HNum, PNum), top(RNum, PNum);
    bottom.updateVertex([this](float u, float v) {
      float r1 = std::get<float>(this->parameters["r1"]);
      Vertex vt;
      vt.nx = 0.0f;
      vt.ny = 0.0f;
      vt.nz = -1.0f;

      vt.x = r1 * u * cos(2 * PI * v);
      vt.y = r1 * u * sin(2 * PI * v);
      vt.z = 0.0f;
      return vt;
    });
    top.updateVertex([this](float u, float v) {
      float r1 = std::get<float>(this->parameters["r1"]);
      float r2 = std::get<float>(this->parameters["r2"]);
      float h = std::get<float>(this->parameters["h"]);
      float rho = std::get<float>(this->parameters["rho"]);
      float phi = std::get<float>(this->parameters["phi"]);
      Vertex vt;
      vt.nx = 0.0f;
      vt.ny = 0.0f;
      vt.nz = 1.0f;

      vt.x = r2 * u * cos(-2 * PI * v);
      vt.y = r2 * u * sin(-2 * PI * v);
      vt.z = h;

      // 向方位角phi、天顶角rho的弯曲变换
      glm::mat4 rot_mat(1.0f);
      rot_mat = glm::rotate(rho, glm::vec3(-sin(phi), cos(phi), 0));
      // 位置
      glm::vec4 pos(vt.x, vt.y, vt.z, 1.0f);
      pos = rot_mat * pos;
      vt.x = pos.x;
      vt.y = pos.y;
      vt.z = pos.z;
      // 法向量
      glm::vec4 norm_v(vt.nx, vt.ny, vt.nz, 0.0f);
      norm_v = rot_mat * norm_v;
      vt.nx = norm_v.x;
      vt.ny = norm_v.y;
      vt.nz = norm_v.z;

      return vt;
    });
    side.updateVertex([this](float u, float v) {
      float r1 = std::get<float>(this->parameters["r1"]);
      float r2 = std::get<float>(this->parameters["r2"]);
      float h = std::get<float>(this->parameters["h"]);
      float rho = std::get<float>(this->parameters["rho"]);
      float phi = std::get<float>(this->parameters["phi"]);
      Vertex vt;
      // 法向量未测试是否正确
      float tmp = sqrt(h * h + pow(r1 - r2, 2.0f));
      vt.nx = cos(2 * PI * v) * h / tmp;
      vt.ny = sin(2 * PI * v) * h / tmp;
      vt.nz = (r1 - r2) / tmp;

      float interp_r = u * (r2 - r1) + r1;
      vt.x = interp_r * cos(2 * PI * v);
      vt.y = interp_r * sin(2 * PI * v);
      vt.z = h * u;

      // 向方位角phi、天顶角rho的弯曲变换
      glm::mat4 rot_mat(1.0f);
      rot_mat = glm::rotate(u * rho, glm::vec3(-sin(phi), cos(phi), 0));
      // 位置
      glm::vec4 pos_v(vt.x, vt.y, vt.z, 1.0f);
      pos_v = rot_mat * pos_v;
      vt.x = pos_v.x;
      vt.y = pos_v.y;
      vt.z = pos_v.z;
      // 法向量
      glm::vec4 norm_v(vt.nx, vt.ny, vt.nz, 0.0f);
      norm_v = rot_mat * norm_v;
      vt.nx = norm_v.x;
      vt.ny = norm_v.y;
      vt.nz = norm_v.z;

      return vt;
    });

    // 将bottom, side, top组装起来
    uint32_t offset = 0;
    // 第1个mesh不需要偏移
    vertices.insert(vertices.end(), bottom.vertices.begin(),
                    bottom.vertices.end());
    surfaces.insert(surfaces.end(), bottom.surfaces.begin(),
                    bottom.surfaces.end());
    // 第2个mesh需要对surface中的元素值进行偏移
    offset += bottom.vertices.size();
    for (auto &sur : side.surfaces) {
      sur.tidx[0] += offset;
      sur.tidx[1] += offset;
      sur.tidx[2] += offset;
    }
    vertices.insert(vertices.end(), side.vertices.begin(), side.vertices.end());
    surfaces.insert(surfaces.end(), side.surfaces.begin(), side.surfaces.end());
    // 第3个mesh需要对surface中的元素值进行偏移
    offset += side.vertices.size();
    for (auto &sur : top.surfaces) {
      sur.tidx[0] += offset;
      sur.tidx[1] += offset;
      sur.tidx[2] += offset;
    }
    vertices.insert(vertices.end(), top.vertices.begin(), top.vertices.end());
    surfaces.insert(surfaces.end(), top.surfaces.begin(), top.surfaces.end());
  }
};

class Plane : public Mesh {
private:
  float width{0.0f};
  float height{0.0f};

public:
  Plane(float width, float height)
      : Mesh(10, 10), width(width), height(height) {
    update();
  }

  virtual void update() {
    reset();
    this->updateVertex([this](float u, float v) {
      Vertex vt;
      vt.x = (0.5f - u) * this->width;
      vt.y = (v - 0.5f) * this->height;
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
  }
};

} // namespace