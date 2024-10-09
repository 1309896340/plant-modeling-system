#pragma once

#include <algorithm>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "Geometry.hpp"
#include "Transform.hpp"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;

int partition(vector<int> &arr, int left, int right) {
  int pivot = arr[left];
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

int findKPosVal(vector<int> arr, int left, int right, int k) {
  vector<int> arr_cpy(arr);
  // 寻找第k小的元素位置
  int pos = partition(arr, left, right);
  while (true) {
    if (k > pos) {
      pos = partition(arr, pos + 1, right);
    } else if (k < pos) {
      pos = partition(arr, left, pos - 1);
    } else {
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
  enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS
  } dimension_type;
  vec3 min_bound{0.0f, 0.0f, 0.0f};
  vec3 max_bound{0.0f, 0.0f, 0.0f};
  BoundingBox() = default;
  BoundingBox(vec3 min_bound, vec3 max_bound)
      : min_bound(min_bound), max_bound(max_bound) {}
  BoundingBox(const vector<vec3> &vertices, const vector<Surface> &surfaces) {
    // 通过传入所有三角面元来初始化min_bound与max_bound
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

struct BvhNode {
  enum{
    NON_TERMINAL, TERMINAL
  }type;
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
    // 对每个三角面元生成一个包围盒
    

    // 先生成根节点包围盒
    this->root = new BvhNode();
    BvhNode *cur_node = this->root;

    // 构造节点
    cur_node->type = BvhNode::NON_TERMINAL;
    cur_node->box = BoundingBox(this->vertices, this->surfaces);
    for (int i = 0; i < this->surfaces.size(); i++) // 当前节点维护的三角
      cur_node->triangles.push_back(i);

    // 开始划分
    // 选择划分维度（根据包围盒的x、y、z宽度，选择最大宽的轴）
    vector<float> buf{cur_node->box.getXWidth(),cur_node->box.getYWidth(),cur_node->box.getZWidth()};
    uint32_t idx = std::distance(buf.begin(),std::max_element(buf.begin(),buf.end()));
    if(idx==0){
      cur_node->box.dimension_type = BoundingBox::X_AXIS;

    }else if(idx==1){
      cur_node->box.dimension_type = BoundingBox::Y_AXIS;

    }else{
      cur_node->box.dimension_type = BoundingBox::Z_AXIS;

    }
    
  }
  ~BvhTree() {
    // 广度优先遍历释放
  }
};

} // namespace
