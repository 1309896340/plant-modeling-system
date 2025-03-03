#pragma once

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstdint>

#include <deque>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <glm/gtx/string_cast.hpp>

#include <Eigen/Dense>

#include "Geometry.hpp"
#include "Transform.hpp"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;

struct Ray {
  vec3 origin{0.0f, 0.0f, 0.0f};
  vec3 dir{0.0f, 0.0f, 0.0f};
};

struct HitInfo {
  bool isHit{false};
  vec3 hitPos{0.0f, 0.0f, 0.0f};
  uint32_t triangle_idx{0};
  string geometry_name;
  float distance{FLT_MAX};
};

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
  // 寻找第k小的元素位置
  int pos = partition(arr, left, right);
  while (true) {
    if (k > pos) {
      left = pos + 1;
      pos = partition(arr, left, right);
    } else if (k < pos) {
      right = pos - 1;
      pos = partition(arr, left, right);
    } else {
      // 此时的arr[pos]是第k小元素的值，需要返回原序列找到它的位置（可能存在多个相同的中位值）
      auto ptr = std::find(arr_cpy.begin(), arr_cpy.end(), arr[pos]);
      if (ptr != arr_cpy.end()) {
        return ptr - arr_cpy.begin();
      } else {
        return -1;
      }
    }
  }
}

HitInfo hit_triangle(Ray ray, const vec3 &p1, const vec3 &p2, const vec3 &p3) {
  HitInfo target;
  vec3 _dir = glm::normalize(ray.dir);
  float det_base = glm::determinant(mat3(-_dir, p1 - p3, p2 - p3));
  if (glm::abs(det_base) < 1e-6) {
    // 平行视作未击中
    target.isHit = false;
    target.distance = 0;
    target.hitPos = ray.origin;
  } else {
    float t_det = glm::determinant(mat3(ray.origin - p3, p1 - p3, p2 - p3));
    float a1_det = glm::determinant(mat3(-_dir, ray.origin - p3, p2 - p3));
    float a2_det = glm::determinant(mat3(-_dir, p1 - p3, ray.origin - p3));
    float t = t_det / det_base;
    float a1 = a1_det / det_base;
    float a2 = a2_det / det_base;
    if (a1 >= 0 && a2 >= 0 && a1 + a2 <= 1 && t > 0.0f) {
      // 击中
      target.isHit = true;
      target.distance = t;
      target.hitPos = ray.origin + t * _dir;
    } else {
      // 未击中
      target.isHit = false;
      target.distance = 0;
      target.hitPos = ray.origin;
    }
  }

  return target;
}

class BoundingBox {
public:
  vec3 min_bound{0.0f, 0.0f, 0.0f};
  vec3 max_bound{0.0f, 0.0f, 0.0f};
  // BoundingBox() = default;
  BoundingBox(vec3 min_bound, vec3 max_bound)
      : min_bound(min_bound), max_bound(max_bound) {}
  BoundingBox(const vector<vec3> &vertices) {
    // 通过传入所有三角面元来初始化包围盒
    this->update(vertices);
  }
  BoundingBox(const vector<vec3> &vertices, const vector<Surface> &surfaces,
              const vector<uint32_t> &indices) {
    // 传入三角形面元来更新包围盒
    this->update(vertices, surfaces, indices);
  }

  void update(const vector<vec3> &vertices, const vector<Surface> &surfaces,
              const vector<uint32_t> &indices) {
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

  void update(const vector<vec3> &vertices) {
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

  vec3 getBoxCenter() const { return (min_bound + max_bound) / 2.0f; }
  float getXWidth() const { return max_bound.x - min_bound.x; }
  float getYWidth() const { return max_bound.y - min_bound.y; }
  float getZWidth() const { return max_bound.z - min_bound.z; }

  tuple<vector<vec3>, vector<uint32_t>>
  genOpenGLRenderInfo() const {
    // 用于将包围盒的min_bound,max_bound生成可用GL_LINES绘制的顶点和索引数据
    vec3 max_xyz = this->max_bound;
    vec3 min_xyz = this->min_bound;
    vector<vec3> vertices = {min_xyz,
                {min_xyz.x, min_xyz.y, max_xyz.z},
                {min_xyz.x, max_xyz.y, min_xyz.z},
                {min_xyz.x, max_xyz.y, max_xyz.z},
                {max_xyz.x, min_xyz.y, min_xyz.z},
                {max_xyz.x, min_xyz.y, max_xyz.z},
                {max_xyz.x, max_xyz.y, min_xyz.z},
                max_xyz};
    vector<uint32_t> indices = {0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3,
                                2, 6, 4, 6, 4, 5, 3, 7, 5, 7, 6, 7};
    return make_tuple(vertices, indices);
  }

  bool hit(Ray ray) {
    vec3 in_bound = this->min_bound;
    vec3 out_bound = this->max_bound;

    if (ray.dir.x < 0)
      swap(in_bound.x, out_bound.x);
    if (ray.dir.y < 0)
      swap(in_bound.y, out_bound.y);
    if (ray.dir.z < 0)
      swap(in_bound.z, out_bound.z);

    // 判断求交
    vec3 ndir = glm::normalize(ray.dir);
    vec3 t_min_xyz = (in_bound - ray.origin) / ndir;
    vec3 t_max_xyz = (out_bound - ray.origin) / ndir;
    float t_enter = glm::max(t_min_xyz.x, glm::max(t_min_xyz.y, t_min_xyz.z));
    float t_exit = glm::min(t_max_xyz.x, glm::min(t_max_xyz.y, t_max_xyz.z));
    if (t_enter <= t_exit && t_exit >= 0)
      return true;
    return false;
  }
};

struct BvhNode {
  BvhNode *parent{nullptr};
  BvhNode *left{nullptr}, *right{nullptr};

  // 记录该节点的包围盒所包含的三角的索引，被索引目标是BvhTree::surfaces
  vector<uint32_t> triangles;
  shared_ptr<BoundingBox> box;
};

class BvhTree {
private:
  BvhNode *root{nullptr};

  vector<vec3> vertices;
  vector<Surface> surfaces;

public:
  BvhTree() = default;

  BvhTree(const shared_ptr<Geometry> &geometry, Transform transform) {
    this->loadGeometry(geometry, transform);
  }
  BvhTree(const shared_ptr<Geometry> &geometry)
      : BvhTree(geometry, Transform()) {}

  void loadGeometry(const shared_ptr<Geometry> &geometry, Transform transform) {
    // 深拷贝一份geometry并对其顶点应用transform的变换
    this->vertices.resize(geometry->getVertices().size());
    mat4 model = transform.getModel();
    for (int i = 0; i < geometry->getVertices().size(); i++) {
      vec3 pt = vec3(
          model * vec4(glm::make_vec3(geometry->getVertices()[i].position), 1.0f));
      memcpy(&vertices[i], glm::value_ptr(pt), 3 * sizeof(float));
    }
    this->surfaces = geometry->getSurfaces(); // 拷贝构造
  }

  void construct() {
    // 先生成根节点
    BvhNode *cur_node = new BvhNode();
    for (int i = 0; i < this->surfaces.size(); i++)
      cur_node->triangles.push_back(i);
    cur_node->box = make_shared<BoundingBox>(this->vertices);
    cur_node->parent = nullptr;

    this->root = cur_node;
    // 开始划分，创建一个队列，以back进front出的顺序，记录待划分节点
    deque<BvhNode *> node_buf{this->root};
    while (!node_buf.empty()) {
      cur_node = node_buf.front();
      node_buf.pop_front();
      // 对cur_node的划分，选择分割维度
      vector<float> widths{cur_node->box->getXWidth(),
                           cur_node->box->getYWidth(),
                           cur_node->box->getZWidth()};
      uint32_t dimension_idx = std::distance(
          widths.begin(), std::max_element(widths.begin(), widths.end()));
      vector<float> comp_positions(cur_node->triangles.size());
      for (int k = 0; k < cur_node->triangles.size(); k++) { // 计算cur_node->triangles里每个三角面元质心位置
        Surface surf = surfaces[cur_node->triangles[k]];
        // 计算三角的质心的第i分量，dimension_idx决定按哪个维度的质心位置进行分割
        float center_pos = 0.0f;
        for (int i = 0; i < 3; i++)
          center_pos += glm::value_ptr(vertices[surf.tidx[i]])[dimension_idx];
        comp_positions[k] = center_pos / 3.0f;
      }

      vector<uint32_t> left_triangles, right_triangles;
      assert(comp_positions.size() > 0);
      if (comp_positions.size() == 1) {
        // cur_node为叶子节点，不进行划分
      } else if (comp_positions.size() == 2) {
        // 根据comp_positions分成左右节点
        if (comp_positions[0] < comp_positions[1]) {
          left_triangles.push_back(cur_node->triangles[0]);
          right_triangles.push_back(cur_node->triangles[1]);
        } else {
          left_triangles.push_back(cur_node->triangles[1]);
          right_triangles.push_back(cur_node->triangles[0]);
        }
      } else {
        // 面元大于3个的情况
        uint32_t mid_position_idx =
            findKPosVal(comp_positions, 0, comp_positions.size() - 1,
                        comp_positions.size() / 2);
        float mid_position = comp_positions[mid_position_idx];
        for (int k = 0; k < comp_positions.size(); k++)
          if (comp_positions[k] < mid_position) {
            left_triangles.push_back(cur_node->triangles[k]);
          } else if (comp_positions[k] > mid_position) {
            right_triangles.push_back(cur_node->triangles[k]);
          } else {
            // 为了尽可能平衡，当中位值为多个相等值时，往左右两侧平衡添加
            if (left_triangles.size() < right_triangles.size())
              left_triangles.push_back(cur_node->triangles[k]);
            else
              right_triangles.push_back(cur_node->triangles[k]);
          }
      }

      // printf("this: %p parent: %p left: %llu  right: %llu\n", cur_node,
      // cur_node->parent, left_triangles.size(), right_triangles.size());
      // 判断left_triangles和right_triangles是否为：1. 空 2.
      // 仅1个面元 3.2个以上面元。
      // 对于1的情况，不创建新node。对于2的情况，加入node但不加入node_buf。
      // 对于3的情况，加入node且将node加入node_buf
      if (left_triangles.size() > 0) {
        cur_node->left = new BvhNode();
        cur_node->left->parent = cur_node;
        cur_node->left->triangles = left_triangles;
        cur_node->left->box =
            make_shared<BoundingBox>(vertices, surfaces, left_triangles);
        if (left_triangles.size() > 1)
          node_buf.push_back(cur_node->left);
      }
      if (right_triangles.size() > 0) {
        cur_node->right = new BvhNode();
        cur_node->right->parent = cur_node;
        cur_node->right->triangles = right_triangles;
        cur_node->right->box =
            make_shared<BoundingBox>(vertices, surfaces, right_triangles);
        if (right_triangles.size() > 1)
          node_buf.push_back(cur_node->right);
      }
    }
  }

  void traverse(function<void(BvhNode *node)> visit) {
    // 广度优先遍历
    deque<BvhNode *> buf{this->root};
    while (!buf.empty()) {
      BvhNode *cur_node = buf.front();
      buf.pop_front();
      if (cur_node->left != nullptr)
        buf.push_back(cur_node->left);
      if (cur_node->right != nullptr)
        buf.push_back(cur_node->right);

      visit(cur_node);
    }
  }

  // tuple<bool, vec3, float, uint32_t> intersect(const vec3 &origin, const vec3 &dir, vec3 &hit_pos,float &distance, uint32_t &triangle_idx) {
  HitInfo intersect(Ray ray) {
    // 返回(是否击中, 击中位置, 距离, 三角索引)

    deque<BvhNode *> buf{this->root};
    vector<BvhNode *> hit_table; // 所有击中包围盒的叶节点
    while (!buf.empty()) {
      BvhNode *cur_node = buf.front();
      buf.pop_front();

      if (!cur_node->box->hit(ray))
        continue;
      // 击中，判断cur_node是否为叶节点，是则加入hit_table
      if (cur_node->left == nullptr && cur_node->right == nullptr) {
        hit_table.push_back(cur_node);
        continue;
      }
      // 击中，但不是叶节点
      if (cur_node->left != nullptr)
        buf.push_back(cur_node->left);
      if (cur_node->right != nullptr)
        buf.push_back(cur_node->right);
    }

    // 从hit_table中寻找最近击中三角
    HitInfo target_obj;
    for (int i = 0; i < hit_table.size(); i++) {
      BvhNode *cur_node = hit_table[i];
      Surface triangle = this->surfaces[cur_node->triangles[0]];
      vec3 pt[3];
      for (int j = 0; j < 3; j++)
        pt[j] = this->vertices[triangle.tidx[j]];

      HitInfo tmp_obj = hit_triangle(ray, pt[0], pt[1], pt[2]);
      if (tmp_obj.isHit && tmp_obj.distance < target_obj.distance) {
        tmp_obj.triangle_idx = cur_node->triangles[0];
        target_obj = tmp_obj;
      }
    }

    return target_obj;
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
