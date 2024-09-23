#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

#define PI 3.14159265358f

// #include <Eigen/Dense>
// #include <glm/glm.hpp>

namespace {
using namespace std;

struct Vertex {
  float position[3]{0.0f, 0.0f, 0.0f};
  float normal[3]{0.0f, 0.0f, 0.0f};
  float color[3]{0.0f, 0.0f, 0.0f};
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
  Mesh(uint32_t uNum, uint32_t vNum) {
    this->uNum = uNum;
    this->vNum = vNum;
    this->vertices.resize((uNum + 1) * (vNum + 1));
    // this->edges.resize(3*uNum*vNum + uNum + vNum);
    this->surfaces.resize(uNum * vNum * 2);
  }

  void genVertex(function<Vertex(float, float)> func) {
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
  Sphere(float radius, uint32_t uNum = 100, uint32_t vNum = 100) : Mesh(uNum, vNum) {
    this->radius = radius;
    this->genVertex([this](float u, float v) -> Vertex {
      Vertex vt;
      vt.normal[0] = sin(PI * u) * cos(2 * PI * v);
      vt.normal[1] = sin(PI * u) * sin(2 * PI * v);
      vt.normal[2] = cos(PI * u);

      vt.position[0] = this->radius * vt.normal[0];
      vt.position[1] = this->radius * vt.normal[1];
      vt.position[2] = this->radius * vt.normal[2];

      vt.color[0] = 1.0f;
      vt.color[1] = 0.0f;
      vt.color[2] = 0.0f;

      return vt;
    });
  }
};

} // namespace