#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "constants.h"

#define RADIANS(a) ((a) * PI / 180.0f)

// #include <Eigen/Dense>
// #include <glm/glm.hpp>

namespace {
using namespace std;

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
  Vertex()
      : position{0.0f, 0.0f, 0.0f}, normal{0.0f, 0.0f, 0.0f},
        color{0.0f, 0.0f, 0.0f} {}
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
  vector<Vertex> vertices;
  // vector<Edge> edges;
  vector<Surface> surfaces;

  Geometry() = default;
  Geometry(const Geometry &) = default;
  ~Geometry() = default;
};

class Mesh : public Geometry {
private:
  uint32_t uNum{0}, vNum{0};

public:
  Mesh() : Mesh(50, 50) {}
  Mesh(uint32_t uNum, uint32_t vNum) {
    this->uNum = uNum;
    this->vNum = vNum;
    this->vertices.resize((uNum + 1) * (vNum + 1));
    // this->edges.resize(3*uNum*vNum + uNum + vNum);
    this->surfaces.resize(uNum * vNum * 2);
  }

  void updateVertex(function<Vertex(float, float)> func) {
    // 遍历i=0, 1, .., uNum，j=0, 1, ..., vNum
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
};

class Sphere : public Mesh {
private:
  float radius;

public:
  Sphere(float radius, uint32_t uNum = 100, uint32_t vNum = 100)
      : Mesh(uNum, vNum) {
    this->radius = radius;
    this->updateVertex([this](float u, float v) -> Vertex {
      Vertex vt;
      vt.nx = sin(PI * u) * cos(2 * PI * v);
      vt.ny = sin(PI * u) * sin(2 * PI * v);
      vt.nz = cos(PI * u);

      vt.x = this->radius * vt.normal[0];
      vt.y = this->radius * vt.normal[1];
      vt.z = this->radius * vt.normal[2];

      vt.r = 1.0f;
      vt.g = 0.0f;
      vt.b = 0.0f;

      return vt;
    });
  }
};

class CylinderEx : public Geometry {
private:
  float r1;
  float r2;
  float h;
  float phi;
  float rho;

  uint32_t RNum;
  uint32_t HNum;
  uint32_t PNum;

public:
  CylinderEx() : CylinderEx(1.0f, 1.0f, 3.0f, 0.0f, 0.0f) {}
  CylinderEx(float r1, float r2, float h, float phi, float rho,
             uint32_t RNum = 8, uint32_t HNum = 10, uint32_t PNum = 18)
      : r1(r1), r2(r2), h(h), phi(phi), rho(rho), RNum(RNum), HNum(HNum),
        PNum(PNum) {
    update();
  }

  void setR1(float r1) { this->r1 = r1; }
  void setR2(float r2) { this->r2 = r2; }
  void setH(float h) { this->h = h; }
  void setPhi(float phi){this->phi = phi;}
  void setRho(float rho){this->rho = rho;}
  void setRNum(uint32_t rNum){this->RNum = rNum;}
  void setHNum(uint32_t hNum){this->HNum = hNum;}
  void setPNum(uint32_t pNum){this->PNum = pNum;}

  void update() {
    this->vertices.clear();
    this->surfaces.clear();
    Mesh bottom(RNum, PNum), side(HNum, PNum), top(RNum, PNum);
    bottom.updateVertex([this](float u, float v) {
      Vertex vt;
      vt.nx = 0.0f;
      vt.ny = 0.0f;
      vt.nz = -1.0f;

      vt.x = r1 * u * cos(2 * PI * v);
      vt.y = r1 * u * sin(2 * PI * v);
      vt.z = 0.0f;

      vt.r = 0.0f;
      vt.g = 0.0f;
      vt.b = 1.0f;

      return vt;
    });
    top.updateVertex([this](float u, float v) {
      Vertex vt;
      vt.nx = 0.0f;
      vt.ny = 0.0f;
      vt.nz = 1.0f;

      vt.x = r2 * u * cos(-2 * PI * v);
      vt.y = r2 * u * sin(-2 * PI * v);
      vt.z = h;

      vt.r = 0.0f;
      vt.g = 0.0f;
      vt.b = 1.0f;

      // 向方位角phi、天顶角rho的弯曲变换
      glm::mat4 rot_mat(1.0f);
      rot_mat = glm::rotate(rho, glm::vec3(-cos(phi), sin(phi), 0));
      // 位置
      glm::vec4 pos(vt.x, vt.y, vt.z, 1.0f);
      pos = rot_mat * pos;
      vt.x = pos.x;
      vt.y = pos.y;
      vt.z = pos.z;
      // 法向量
      glm::vec4 norm_v(vt.nx,vt.ny,vt.nz, 0.0f);
      norm_v = rot_mat * norm_v;
      vt.nx = norm_v.x;
      vt.ny = norm_v.y;
      vt.nz = norm_v.z;

      return vt;
    });
    side.updateVertex([this](float u, float v) {
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

      vt.r = 0.0f;
      vt.g = 0.0f;
      vt.b = 1.0f;

      // 向方位角phi、天顶角rho的弯曲变换
      glm::mat4 rot_mat(1.0f);
      rot_mat = glm::rotate(u * rho, glm::vec3(-cos(phi), sin(phi), 0));
      // 位置
      glm::vec4 pos_v(vt.x, vt.y, vt.z, 1.0f);
      pos_v = rot_mat * pos_v;
      vt.x = pos_v.x;
      vt.y = pos_v.y;
      vt.z = pos_v.z;
      // 法向量
      glm::vec4 norm_v(vt.nx,vt.ny,vt.nz, 0.0f);
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

} // namespace