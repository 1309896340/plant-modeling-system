#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

// #include <Eigen/Dense>
#include <glm/glm.hpp>

namespace {
  using namespace std;
  using glm::vec3;
  using glm::vec4;

struct Vertex{
  float position[3]{0.0f,0.0f,0.0f};
  float normal[3]{0.0f,0.0f,0.0f};
  float color[3]{0.0f,0.0f,0.0f};
};

struct Edge{
  uint32_t pidx[2]; // 2个Vertex构成
};

struct Surface{
  uint32_t tidx[3]; // 3个Vertex构成
};



class Geometry {
protected:
  vector<Vertex> vertices;
  vector<Edge> edges;
  vector<Surface> surfaces;
public:
  Geometry() {}
};

} // namespace