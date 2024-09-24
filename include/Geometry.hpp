#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

#define PI 3.14159265358f
#define ANGLE(a) ((a)*PI/180.0f)

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
  float R;
  float r;
  float H;
  float phi;
  float rho;

  uint32_t RNum;
  uint32_t HNum;
  uint32_t PNum;

public:
  CylinderEx() : CylinderEx(1.0f, 1.0f, 3.0f, 0.0f, 0.0f) {}
  CylinderEx(float R, float r, float H, float phi, float rho, uint32_t RNum=8, uint32_t HNum=10, uint32_t PNum=18)
      : R(R), r(r), H(H), phi(phi), rho(rho), RNum(RNum),HNum(HNum),PNum(PNum) {
    Mesh bottom(RNum,PNum), side(HNum,PNum), top(RNum,PNum);
    bottom.updateVertex([this](float u, float v) {
      Vertex vt;
      vt.nx = 0.0f;
      vt.ny = 0.0f;
      vt.nz = -1.0f;

      vt.x = this->R * u * cos(2 * PI * v);
      vt.y = this->R * u * sin(2 * PI * v);
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

      vt.x = this->r * u * cos(-2 * PI * v);
      vt.y = this->r * u * sin(-2 * PI * v);
      vt.z = this->H;

      vt.r = 0.0f;
      vt.g = 0.0f;
      vt.b = 1.0f;

      return vt;
    });
    side.updateVertex([this](float u, float v) {
      Vertex vt;
      // 法向量未测试是否正确
      float tmp = sqrt(this->H * this->H + pow(this->R - this->r,2.0f));
      vt.nx = sin(2 * PI * v) * this->H / tmp ;
      vt.ny = cos(2 * PI * v) * this->H / tmp ;
      vt.nz = (this->R - this->r)/ tmp;

      float interp_r = u * (this->r - this->R) + this->R;
      vt.x = interp_r * cos(2 * PI * v);
      vt.y = interp_r * sin(2 * PI * v);
      vt.z = this->H * u;

      vt.r = 0.0f;
      vt.g = 0.0f;
      vt.b = 1.0f;

      // 向方位角phi、天顶角rho的弯曲变换

      return vt;
    });

    // 将bottom, side, top组装起来
    uint32_t offset = 0;
    // 第1个mesh不需要偏移
    this->vertices.insert(this->vertices.end(), bottom.vertices.begin(),
                          bottom.vertices.end());
    this->surfaces.insert(this->surfaces.end(), bottom.surfaces.begin(),
                          bottom.surfaces.end());
    // 第2个mesh需要对surface中的元素值进行偏移
    offset += bottom.vertices.size();
    for (auto &sur : side.surfaces) {
      sur.tidx[0] += offset;
      sur.tidx[1] += offset;
      sur.tidx[2] += offset;
    }
    this->vertices.insert(this->vertices.end(), side.vertices.begin(),
                          side.vertices.end());
    this->surfaces.insert(this->surfaces.end(), side.surfaces.begin(),
                          side.surfaces.end());
    // 第3个mesh需要对surface中的元素值进行偏移
    offset += side.vertices.size();
    for (auto &sur : top.surfaces) {
      sur.tidx[0] += offset;
      sur.tidx[1] += offset;
      sur.tidx[2] += offset;
    }
    this->vertices.insert(this->vertices.end(), top.vertices.begin(),
                          top.vertices.end());
    this->surfaces.insert(this->surfaces.end(), top.surfaces.begin(),
                          top.surfaces.end());
  }
};

} // namespace