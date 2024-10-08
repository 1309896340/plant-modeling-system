#pragma once

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

class BoundingBox {
public:
  vec3 min_bound{0.0f, 0.0f, 0.0f};
  vec3 max_bound{0.0f, 0.0f, 0.0f};
  BoundingBox() = default;
  BoundingBox(vec3 min_bound, vec3 max_bound)
      : min_bound(min_bound), max_bound(max_bound) {}
  BoundingBox(const vector<vec3> &vertices, const vector<Surface> &surfaces) {
    // 通过传入所有三角面元来初始化min_bound与max_bound
    this->min_bound= vertices[0];
    this->max_bound =vertices[0];
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
    // 先生成根节点包围盒
    this->root = new BvhNode();
    this->root->box = BoundingBox(this->vertices, this->surfaces);  // 在构造函数中计算包围盒
    for (int i = 0; i < this->surfaces.size(); i++) // 最外层包围盒加入全部三角
      this->root->triangles.push_back(i);
    // 进行质量划分

    
  }
  ~BvhTree() {
    // 广度优先遍历释放
  }
};

} // namespace
