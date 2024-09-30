#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "constants.h"

namespace {
using namespace std;
using param_variant = variant<uint32_t, int, float, bool, glm::vec3>;

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

  void translate(float x_offset, float y_offset, float z_offset) {
    for (auto &vertex : this->vertices) {
      vertex.x += x_offset;
      vertex.y += y_offset;
      vertex.z += z_offset;
    }
  }

  void rotate(float angle, glm::vec3 axis) {
    glm::mat3 rot_mat = glm::mat3(glm::rotate(glm::mat4(1.0f),angle, axis));
    for (auto &vertex : this->vertices) {
      glm::vec3 new_pos = rot_mat * glm::make_vec3(vertex.position);
      glm::vec3 new_normal = rot_mat * glm::make_vec3(vertex.normal);
      vertex.x = new_pos.x;
      vertex.y = new_pos.y;
      vertex.z = new_pos.z;
      vertex.nx = new_normal.x;
      vertex.ny = new_normal.y;
      vertex.nz = new_normal.z;
    }
  }

  void setColor(float r, float g, float b) {
    // 设置纯色
    for (auto &vertex : this->vertices) {
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

class FixedGeometry : public Geometry {
public:
  FixedGeometry() { this->parameters.clear(); }

  FixedGeometry(const Geometry &geometry) {
    // 可以从Geometry构造（这是个有点奇怪的操作，引入了从父类到子类的隐式转换）
    this->vertices = geometry.vertices;
    this->surfaces = geometry.surfaces;
    this->parameters.clear();
  }

  FixedGeometry(Geometry &&geometry) noexcept {
    this->vertices = geometry.vertices;
    this->surfaces = geometry.surfaces;
    geometry.vertices.clear();
    geometry.surfaces.clear();
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

  FixedGeometry operator+(const Geometry &b) const {
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
  function<Vertex(float, float)> meshUpdator;

public:
  Mesh() : Mesh(50, 50) {}
  Mesh(uint32_t uNum, uint32_t vNum) {
    this->parameters["uNum"] = uNum;
    this->parameters["vNum"] = vNum;
    resize();
  }

  FixedGeometry operator+(const Mesh &other) const {
    FixedGeometry b(other);
    return FixedGeometry(*this) + b;
  }

  void resize() {
    uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]);
    uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]);

    this->vertices.resize((uNum + 1) * (vNum + 1));
    // this->edges.resize(3*uNum*vNum + uNum + vNum);
    this->surfaces.resize(uNum * vNum * 2);
  }

  void updateVertex(function<Vertex(float, float)> func) {
    resize();
    uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]);
    uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]);
    this->meshUpdator = func; // 缓存更新函数用于resize中保留uv更新顶点属性
    for (int i = 0; i <= uNum; i++) {
      for (int j = 0; j <= vNum; j++) {
        float u = static_cast<float>(i) / uNum;
        float v = static_cast<float>(j) / vNum;
        Vertex &vt = this->vertices[j + i * (vNum + 1)];
        vt = func(u, v);
        // 待优化，不必每次更新顶点都要重置网格
        if (i != uNum && j != vNum) {
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
    uint32_t uNum = std::get<uint32_t>(this->parameters["uNum"]);
    uint32_t vNum = std::get<uint32_t>(this->parameters["vNum"]);
    for (int i = 0; i <= uNum; i++) {
      for (int j = 0; j <= vNum; j++) {
        float u = static_cast<float>(i) / uNum;
        float v = static_cast<float>(j) / vNum;
        Vertex &vt = this->vertices[j + i * (vNum + 1)];
        vt = func(vt, u, v);
      }
    }
  }

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

      vt.r = vt.x;
      vt.g = vt.y;
      vt.b = vt.z;

      vt.u = u;
      vt.v = v;
      return vt;
    });
  }
};

class Disk : public Mesh {
private:
public:
  Disk(float r, uint32_t RNum = 8, uint32_t PNum = 18) : Mesh(RNum, PNum) {
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
public:
  ConeSide(float r, float h, uint32_t HNum = 10, uint32_t PNum = 18)
      : Mesh(HNum, PNum) {
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

class Cone : public Geometry {
public:
  Cone(float radius, float height, uint32_t RNum = 8, uint32_t HNum = 10,
       uint32_t PNum = 18) {

    this->parameters["radius"] = radius;
    this->parameters["height"] = height;
    this->parameters["PNum"] = PNum;
    this->parameters["RNum"] = RNum;
    this->parameters["HNum"] = HNum;

    this->update();
  }

  virtual void update() {
    float radius = std::get<float>(this->parameters["radius"]);
    float height = std::get<float>(this->parameters["height"]);
    uint32_t RNum = std::get<uint32_t>(this->parameters["RNum"]);
    uint32_t HNum = std::get<uint32_t>(this->parameters["HNum"]);
    uint32_t PNum = std::get<uint32_t>(this->parameters["PNum"]);

    ConeSide cs(radius, height, HNum, PNum);
    Disk ds(radius, RNum, PNum);

    ds.rotate(glm::radians(180.0f), {1.0f, 0.0f, 0.0f});

    // 组合
    FixedGeometry &&res = cs + ds;
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
};

class CylinderSide : public Mesh {
private:
public:
  CylinderSide(float r, float h, uint32_t HNum = 10, uint32_t PNum = 18)
      : Mesh(HNum, PNum) {
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
      vt.b = 1.0f;

      vt.u = u;
      vt.v = v;

      return vt;
    });
  }
};

class TruncatedConeSide : public Mesh {
private:
public:
  TruncatedConeSide(float r1, float r2, float h, uint32_t RNum = 8,
                    uint32_t HNum = 10, uint32_t PNum = 18)
      : Mesh(HNum, PNum) {
    this->parameters["r1"] = r1;
    this->parameters["r2"] = r2;
    this->parameters["h"] = h;
    update();
  }
  virtual void update() {
    this->updateVertex([this](float u, float v) {
      Vertex vt;
      float r1 = std::get<float>(this->parameters["r1"]);
      float r2 = std::get<float>(this->parameters["r2"]);
      float h = std::get<float>(this->parameters["h"]);

      float interp_r = u * (r2 - r1) + r1;
      vt.x = interp_r * cos(2 * PI * v);
      vt.y = interp_r * sin(2 * PI * v);
      vt.z = h * u;

      float tmp = sqrt(h * h + pow(r1 - r2, 2.0f));
      vt.nx = h / tmp * cos(2 * PI * v);
      vt.ny = h / tmp * sin(2 * PI * v);
      vt.nz = (r1 - r2) / tmp;

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
    this->parameters["radius"] = radius;
    this->parameters["height"] = height;
    this->parameters["RNum"] = RNum;
    this->parameters["HNum"] = HNum;
    this->parameters["PNum"] = PNum;

    this->update();
  }

  virtual void update() {
    float radius = std::get<float>(this->parameters["radius"]);
    float height = std::get<float>(this->parameters["height"]);
    uint32_t RNum = std::get<uint32_t>(this->parameters["RNum"]);
    uint32_t HNum = std::get<uint32_t>(this->parameters["HNum"]);
    uint32_t PNum = std::get<uint32_t>(this->parameters["PNum"]);

    CylinderSide cs(radius, height, HNum, PNum);
    Disk ds_bottom(radius, RNum, PNum), ds_top(radius, RNum, PNum);

    cs.setColor(0.0f, 0.0f, 1.0f);
    ds_bottom.rotate(glm::radians(180.0f), {1.0f, 0.0f, 0.0f});
    ds_top.translate(0.0f, 0.0f, height);

    // 组合
    FixedGeometry &&res = cs + ds_top + ds_bottom;
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
};

class TruncatedConeEx : public Geometry {
private:

public:
  TruncatedConeEx() : TruncatedConeEx(1.0f, 1.0f, 3.0f, 0.0f, 0.0f) {}
  TruncatedConeEx(float r1, float r2, float h, float phi, float rho,
                  uint32_t RNum = 8, uint32_t HNum = 10, uint32_t PNum = 18)
       {
    this->parameters["r1"] = r1;
    this->parameters["r2"] = r2;
    this->parameters["h"] = h;
    this->parameters["phi"] = phi;
    this->parameters["rho"] = rho;
    this->parameters["RNum"] = RNum;
    this->parameters["HNum"] = HNum;
    this->parameters["PNum"] = PNum;
    update();
  }

  virtual void update() {
    float r1 = std::get<float>(this->parameters["r1"]);
    float r2 = std::get<float>(this->parameters["r2"]);
    float h = std::get<float>(this->parameters["h"]);
    float phi = std::get<float>(this->parameters["phi"]);
    float rho = std::get<float>(this->parameters["rho"]);
    float RNum = std::get<uint32_t>(this->parameters["RNum"]);
    float HNum = std::get<uint32_t>(this->parameters["HNum"]);
    float PNum = std::get<uint32_t>(this->parameters["PNum"]);

    Disk bottom(r1, RNum, PNum);
    Disk top(r2, RNum, PNum);
    TruncatedConeSide side(r1, r2, h, RNum, HNum, PNum);
    side.setColor(0.8f, 0.2f, 0.6f);
    top.setColor(1.0f, 1.0f, 0.0f);
    bottom.rotate(glm::radians(180.0f), {1.0f, 0.0f, 0.0f});
    top.translate(0.0f, 0.0f, h);

    top.transformVertex([phi, rho](const Vertex &vt0, float u, float v) {
      Vertex vt(vt0);
      glm::mat3 rot_mat =
          glm::mat3(glm::rotate(glm::mat4(1.0f),rho, glm::vec3(cos(phi), sin(phi), 0.0f)));
          
      glm::vec3 n_pos = rot_mat * glm::make_vec3(vt.position);
      glm::vec3 n_norm = rot_mat * glm::make_vec3(vt.normal);
      vt.x = n_pos.x;
      vt.y = n_pos.y;
      vt.z = n_pos.z;
      vt.nx = n_norm.x;
      vt.ny = n_norm.y;
      vt.nz = n_norm.z;
      return vt;
    });
    side.transformVertex([phi, rho](const Vertex &vt0, float u, float v) {
      Vertex vt(vt0);
      glm::mat3 rot_mat =
          glm::mat3(glm::rotate(glm::mat4(1.0f),u * rho, glm::vec3(cos(phi), sin(phi), 0.0f)));
      glm::vec3 n_pos = rot_mat * glm::make_vec3(vt.position);
      glm::vec3 n_norm = rot_mat * glm::make_vec3(vt.normal);
      vt.x = n_pos.x;
      vt.y = n_pos.y;
      vt.z = n_pos.z;
      vt.nx = n_norm.x;
      vt.ny = n_norm.y;
      vt.nz = n_norm.z;
      return vt;
    });

    // 组合
    FixedGeometry &&res = bottom + top + side;
    this->vertices = res.vertices;
    this->surfaces = res.surfaces;
  }
};

class Arrow : public Geometry {
private:
  const float body_ratio = 0.9f;
  const float radius_ratio = 0.5f;

public:
  Arrow(float radius, float length, glm::vec3 arrow_color, glm::vec3 body_color) {
    this->parameters["radius"] = radius;
    this->parameters["length"] = length;
    this->parameters["arrow_color"] = arrow_color;
    this->parameters["body_color"] = body_color;
    update();
  }
  Arrow(float radius, float length):Arrow(radius, length, {0.0f,1.0f,1.0f}, {1.0f,0.0f,0.0f}){}

  virtual void update(){
    float radius = std::get<float>(this->parameters["radius"]);
    float length = std::get<float>(this->parameters["length"]);
    glm::vec3 arrow_color = std::get<glm::vec3>(this->parameters["arrow_color"]);
    glm::vec3 body_color = std::get<glm::vec3>(this->parameters["body_color"]);

    Cone arrow(radius, 1 - body_ratio);
    Cylinder body(radius_ratio * radius, body_ratio * length);

    arrow.setColor(arrow_color.r,arrow_color.g,arrow_color.b);
    body.setColor(body_color.r,body_color.g,body_color.b);

    arrow.translate(0.0f, 0.0f, body_ratio * length);

    FixedGeometry a = FixedGeometry(std::move(arrow)) + body;
    this->vertices = a.vertices;
    this->surfaces = a.surfaces;
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