#pragma once

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstdint>

// #include <deque>
// #include <iterator>
#include <iostream>
#include <memory>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <glm/gtx/string_cast.hpp>

#include <Eigen/Dense>

#include "Geometry.h"
// #include "Renderer.h"
#include "Transform.h"



struct Ray {
  glm::vec3 origin{0.0f, 0.0f, 0.0f};
  glm::vec3 dir{0.0f, 0.0f, 0.0f};
};

struct HitInfo {
  bool        isHit{false};
  glm::vec3   hitPos{0.0f, 0.0f, 0.0f};
  uint32_t    triangleIdx{0};
  std::string geometryName;
  float       distance{FLT_MAX};
};

template<typename T>
inline int partition(std::vector<T>& arr, int left, int right) {
  T pivot = arr[left];
  // printf("left: %d right: %d\n", left, right);
  while (left < right) {
    while (left < right && arr[right] >= pivot)
      right--;
    if (left < right)
      std::swap(arr[left], arr[right]);
    while (left < right && arr[left] <= pivot)
      left++;
    if (left < right)
      std::swap(arr[left], arr[right]);
  }
  return left;
}

template<typename T>
inline int findKPosVal(std::vector<T> arr, int left, int right, int k) {
  std::vector<float> arr_cpy(arr);
  if (right - left < 0) {
    std::string msg = "findKPosVal: left cannot greater than right!";
    std::cerr << msg << std::endl;
    throw std::runtime_error(msg);
  }
  else if (right - left < 2) {
    // 若只有1个、2个元素，直接返回第一个元素位置
    return left;
  }
  // 寻找第k小的元素位置
  int pos = partition(arr, left, right);
  while (true) {
    if (k > pos) {
      left = pos + 1;
      pos  = partition(arr, left, right);
    }
    else if (k < pos) {
      right = pos - 1;
      pos   = partition(arr, left, right);
    }
    else {
      // 此时的arr[pos]是第k小元素的值，需要返回原序列找到它的位置（可能存在多个相同的中位值）
      auto ptr = std::find(arr_cpy.begin(), arr_cpy.end(), arr[pos]);
      if (ptr != arr_cpy.end()) {
        return ptr - arr_cpy.begin();
      }
      else {
        return -1;
      }
    }
  }
}

HitInfo hit_triangle(Ray ray, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);


class BoundingBox {
  private:
  public:
  // unique_ptr<OpenGLContext> context{nullptr};
  glm::vec3 min_bound{0.0f, 0.0f, 0.0f};
  glm::vec3 max_bound{0.0f, 0.0f, 0.0f};
  BoundingBox() = delete;
  BoundingBox(glm::vec3 min_bound, glm::vec3 max_bound);
  BoundingBox(const std::vector<Vertex>& vertices) ;
  BoundingBox(const std::vector<Vertex>& vertices, const std::vector<Surface>& surfaces, const std::vector<uint32_t>& indices);

  void      update(const std::vector<Vertex>& vertices, const std::vector<Surface>& surfaces, const std::vector<uint32_t>& indices);
  void      update(const std::vector<Vertex>& vertices);
  glm::vec3 getBoxCenter() const;
  float     getXWidth() const;
  float     getYWidth() const;
  float     getZWidth() const;


  bool hit(Ray ray);
  bool hit(Ray ray, Transform transform);
};

struct BvhNode {
  BvhNode* parent{nullptr};
  BvhNode *left{nullptr}, *right{nullptr};

  // 记录该节点的包围盒所包含的三角的索引，被索引目标是BvhTree::surfaces
  std::vector<uint32_t>        triangles;
  std::unique_ptr<BoundingBox> box;
};

class BvhTree {
  // 由构造时传入的shared_ptr<Geometry>的顶点构造的层次包围盒，为目标的局部坐标系下的顶点位置
  private:
  BvhNode* root{nullptr};

  Geometry* geometry{nullptr};

  public:
  BvhTree() = delete;

  BvhTree(Geometry* geometry);

  void construct();

  void traverse(std::function<void(BvhNode* node)> visit);

  void update();

  // tuple<bool, vec3, float, uint32_t> intersect(const vec3 &origin, const vec3 &dir, vec3 &hit_pos,float &distance, uint32_t &triangle_idx) {
  HitInfo hit(Ray ray, Transform trans);

  void destruct();

  ~BvhTree();
};

// }   // namespace
