#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <deque>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Geometry.hpp"
#include "Transform.hpp"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;

template <typename T>
int partition(vector<T> &arr, int left, int right) {
  T pivot = arr[left];
  // printf("left: %d right: %d\n", left, right);
  while (left < right) {
    while (left < right && arr[right] >= pivot)
      right--;
    if (left < right)
      swap(arr[left], arr[right]);

    while (left < right && arr[left] <= pivot)
      left++;
    if (left < right)
      swap(arr[left], arr[right]);
  }
  return left;
}

template <typename T>
int findKPosVal(vector<T> arr, int left, int right, int k) {
  vector<float> arr_cpy(arr);
  if (right - left < 0) {
    string msg = "findKPosVal: left cannot greater than right!";
    cerr << msg << endl;
    throw runtime_error(msg);
  } else if (right - left < 2) {
    // 若只有1个、2个元素，直接返回第一个元素位置
    return left;
  }
  printf("================================开始\n");
  // 寻找第k小的元素位置
  int pos = partition(arr, left, right);
  while (true) {
    printf("left: %d right: %d pos: %d\n", left, right, pos);
    if (k > pos) {
      printf("k=%d 大于 pos=%d 向右半寻找\n", k, pos);
      pos = pos + 1 + partition(arr, pos + 1, right);
    } else if (k < pos) {
      printf("k=%d 小于 pos=%d 向左半寻找\n", k, pos);
      pos = partition(arr, left, pos - 1);
    } else {
      printf("k==pos=%d 返回\n", k);
      // 此时的arr[pos]是第k小元素的值，需要返回原序列找到它的位置
      auto ptr = std::find(arr_cpy.begin(), arr_cpy.end(), arr[pos]);
      if (ptr != arr_cpy.end()) {
        return ptr - arr_cpy.begin();
      } else {
        return -1;
      }
    }
  }
}

class BoundingBox {
public:
  vec3 min_bound{0.0f, 0.0f, 0.0f};
  vec3 max_bound{0.0f, 0.0f, 0.0f};
  BoundingBox() = default;
  BoundingBox(vec3 min_bound, vec3 max_bound)
      : min_bound(min_bound), max_bound(max_bound) {}
  BoundingBox(const vector<vec3> &vertices) {
    // 通过传入所有三角面元来初始化包围盒
    this->min_bound = vertices[0];
    this->max_bound = vertices[0];
    for (const vec3 &vert : vertices) {
      this->max_bound.x = std::max(this->max_bound.x, vert.x);
      this->max_bound.y = std::max(this->max_bound.y, vert.y);
      this->max_bound.z = std::max(this->max_bound.z, vert.z);
      this->min_bound.x = std::min(this->min_bound.x, vert.x);
      this->min_bound.y = std::min(this->min_bound.y, vert.y);
      this->min_bound.z = std::min(this->min_bound.z, vert.z);
    }
  }
  BoundingBox(const vector<vec3> &vertices, const vector<Surface> &surfaces, const vector<uint32_t> &indices) {
    // 通过传入所有三角面元，通过索引指定其子集，创建子集的包围盒
    vec3 default_bound = vertices[surfaces[indices[0]].tidx[0]];
    this->min_bound = default_bound;
    this->max_bound = default_bound;
    for (uint32_t tri_idx : indices) {
      Surface surf = surfaces[tri_idx];
      for (int i = 0; i < 3; i++) {
        vec3 vert = vertices[surf.tidx[i]];
        min_bound.x = std::min(min_bound.x, vert.x);
        min_bound.y = std::min(min_bound.y, vert.y);
        min_bound.z = std::min(min_bound.z, vert.z);
        max_bound.x = std::max(max_bound.x, vert.x);
        max_bound.y = std::max(max_bound.y, vert.y);
        max_bound.z = std::max(max_bound.z, vert.z);
      }
    }
  }
  vec3 getBoxCenter() const { return (min_bound + max_bound) / 2.0f; }
  float getXWidth() const { return max_bound.x - min_bound.x; }
  float getYWidth() const { return max_bound.y - min_bound.y; }
  float getZWidth() const { return max_bound.z - min_bound.z; }

  bool hit(const vec3 &origin, const vec3 &dir) {

    // 选择就近边界为near_bound，就远边界为far_bound
    vec3 near_bound = this->min_bound, far_bound = this->max_bound;
    vec3 near_dist = glm::abs(origin - near_bound);
    vec3 far_dist = glm::abs(origin - far_bound);
    if (far_dist.x < near_dist.x)
      swap(near_bound.x, far_bound.x);
    if (far_dist.y < near_dist.y)
      swap(near_bound.y, far_bound.y);
    if (far_dist.z < near_dist.z)
      swap(near_bound.z, far_bound.z);

    // 判断求交
    vec3 ndir = glm::normalize(dir);
    vec3 t_min_xyz = (near_bound - origin) / ndir;
    vec3 t_max_xyz = (far_bound - origin) / ndir;
    float t_enter = glm::max(t_min_xyz.x, glm::max(t_min_xyz.y, t_min_xyz.z));
    float t_exit = glm::min(t_max_xyz.x, glm::min(t_max_xyz.y, t_max_xyz.z));
    if (t_enter < t_exit && t_exit >= 0)
      return true;
    return false;
  }
};

// class TrianglePrimitive {
// public:
//   uint32_t triangle_idx{0};
//   BoundingBox box;

//   TrianglePrimitive(uint32_t triangle_id) : triangle_idx(triangle_id) {
//     // 传递可以访问所有三角面元的的上下文，并指定该面元对应的索引
//   }
// };

struct BvhNode {
  BvhNode *parent{nullptr};
  BvhNode *left{nullptr}, *right{nullptr};

  // 记录该节点的包围盒所包含的三角的索引，被索引目标是BvhTree::surfaces
  vector<uint32_t> triangles;
  BoundingBox box;
};

class BvhTree {
private:
  BvhNode *root{nullptr};

  vector<vec3> vertices;
  vector<Surface> surfaces;

public:
  BvhTree(const shared_ptr<Geometry> &geometry, Transform transform) {
    // 深拷贝一份geometry并对其顶点应用transform的变换
    this->vertices.resize(geometry->vertices.size());
    mat4 model = transform.getModel();
    for (int i = 0; i < geometry->vertices.size(); i++) {
      vec3 pt = vec3(model * vec4(glm::make_vec3(geometry->vertices[i].position), 1.0f));
      memcpy(&vertices[i], glm::value_ptr(pt), 3 * sizeof(float));
    }
    this->surfaces = geometry->surfaces; // 拷贝构造
  };
  BvhTree(const shared_ptr<Geometry> &geometry) : BvhTree(geometry, Transform()) {}

  void construct() {
    // 先生成根节点
    BvhNode *cur_node = new BvhNode();
    for (int i = 0; i < this->surfaces.size(); i++)
      cur_node->triangles.push_back(i);
    cur_node->box = BoundingBox(this->vertices);
    cur_node->parent = nullptr;

    this->root = cur_node;

    // 开始划分，创建一个队列，以back进front出的顺序，记录待划分节点
    deque<BvhNode *> node_buf{this->root};

    // 处理node_buf直到为空
    while (!node_buf.empty()) {
      cur_node = node_buf.front();
      node_buf.pop_front();
      // 进行对cur_node的划分，判断聚类维度
      vector<float> widths{cur_node->box.getXWidth(), cur_node->box.getYWidth(), cur_node->box.getZWidth()};
      uint32_t dimension_idx = std::distance(widths.begin(), std::max_element(widths.begin(), widths.end()));
      vector<float> comp_positions(cur_node->triangles.size());
      for (int k = 0; k < cur_node->triangles.size(); k++) { // 计算cur_node->triangles里每个三角面元质心位置
        Surface surf = surfaces[cur_node->triangles[k]];
        // 计算三角的质心的x分量，dimension_idx决定了计算哪个维度的质心位置分量
        float center_pos = 0.0f;
        for (int i = 0; i < 3; i++)
          center_pos += glm::value_ptr(vertices[surf.tidx[i]])[dimension_idx];
        comp_positions[k] = center_pos / 3.0f;
      }
      printf("开始找中位点\n");
      // 找到中位点
      uint32_t mid_position_idx = findKPosVal(comp_positions, 0, comp_positions.size() - 1, comp_positions.size() / 2);
      float mid_position = comp_positions[mid_position_idx];
      printf("找到中位点\n");
      // 对cur_node->triangles中的三角分组
      vector<uint32_t> left_triangles, right_triangles;
      for (int k = 0; k < comp_positions.size(); k++)
        if (comp_positions[k] <= mid_position)
          left_triangles.push_back(cur_node->triangles[k]);
        else
          right_triangles.push_back(cur_node->triangles[k]);
      // 调试输出分组信息
      printf("this: %p parent: %p left: %llu  right: %llu\n", cur_node, cur_node->parent, left_triangles.size(), right_triangles.size());
      // 判断left_triangles和right_triangles是否为：1. 空 2. 仅1个面元 3. 2个以上面元。
      // 对于1的情况，不创建新node。对于2的情况，加入node但不加入node_buf。 对于3的情况，加入node且将node加入node_buf
      printf("当前node_buf.size(): %llu\n", node_buf.size());
      if (left_triangles.size() > 0) {
        cur_node->left = new BvhNode();
        cur_node->left->parent = cur_node;
        cur_node->left->triangles = left_triangles;
        cur_node->left->box = BoundingBox(vertices, surfaces, left_triangles);
        if (left_triangles.size() > 1)
          node_buf.push_back(cur_node->left);
      }
      if (right_triangles.size() > 0) {
        cur_node->right = new BvhNode();
        cur_node->right->parent = cur_node;
        cur_node->right->triangles = right_triangles;
        cur_node->right->box = BoundingBox(vertices, surfaces, right_triangles);
        if (right_triangles.size() > 1)
          node_buf.push_back(cur_node->right);
      }
    }
  }

  ~BvhTree() {
    // 广度优先遍历释放
    deque<BvhNode *> tmp{this->root};
    while (!tmp.empty()) {
      BvhNode *cur_node = tmp.front();
      tmp.pop_front();
      if (cur_node->left != nullptr)
        tmp.push_back(cur_node->left);
      if (cur_node->right != nullptr)
        tmp.push_back(cur_node->right);
      delete cur_node;
    }
  }
};

} // namespace
