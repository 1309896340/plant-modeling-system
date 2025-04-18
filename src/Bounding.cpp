#pragma once
#include "Bounding.h"

#include <algorithm>
#include <cstdint>
#include <deque>

HitInfo hit_triangle(Ray ray, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
  HitInfo   target;
  glm::vec3 _dir     = glm::normalize(ray.dir);
  float     det_base = glm::determinant(glm::mat3(-_dir, p1 - p3, p2 - p3));
  if (glm::abs(det_base) < 1e-6) {
    // 平行视作未击中
    target.isHit    = false;
    target.distance = 0;
    target.hitPos   = ray.origin;
  }
  else {
    float t_det  = glm::determinant(glm::mat3(ray.origin - p3, p1 - p3, p2 - p3));
    float a1_det = glm::determinant(glm::mat3(-_dir, ray.origin - p3, p2 - p3));
    float a2_det = glm::determinant(glm::mat3(-_dir, p1 - p3, ray.origin - p3));
    float t      = t_det / det_base;
    float a1     = a1_det / det_base;
    float a2     = a2_det / det_base;
    if (a1 >= 0 && a2 >= 0 && a1 + a2 <= 1 && t > 0.0f) {
      // 击中
      target.isHit    = true;
      target.distance = t;
      target.hitPos   = ray.origin + t * _dir;
    }
    else {
      // 未击中
      target.isHit    = false;
      target.distance = 0;
      target.hitPos   = ray.origin;
    }
  }

  return target;
}

BoundingBox::BoundingBox(glm::vec3 min_bound, glm::vec3 max_bound)
  : min_bound(min_bound)
  , max_bound(max_bound) {}
BoundingBox::BoundingBox(const std::vector<Vertex>& vertices) {
  // 通过传入所有三角面元来初始化包围盒
  this->update(vertices);
}
BoundingBox::BoundingBox(const std::vector<Vertex>& vertices, const std::vector<Surface>& surfaces, const std::vector<uint32_t>& indices) {
  // 传入三角形面元来更新包围盒
  this->update(vertices, surfaces, indices);
}

void BoundingBox::update(const std::vector<Vertex>& vertices, const std::vector<Surface>& surfaces, const std::vector<uint32_t>& indices) {
  glm::vec3 default_bound = glm::make_vec3(vertices[surfaces[indices[0]].tidx[0]].position);
  this->min_bound         = default_bound;
  this->max_bound         = default_bound;
  for (uint32_t tri_idx : indices) {
    Surface surf = surfaces[tri_idx];
    for (int i = 0; i < 3; i++) {
      Vertex vert = vertices[surf.tidx[i]];
      min_bound.x = std::min(min_bound.x, vert.x);
      min_bound.y = std::min(min_bound.y, vert.y);
      min_bound.z = std::min(min_bound.z, vert.z);
      max_bound.x = std::max(max_bound.x, vert.x);
      max_bound.y = std::max(max_bound.y, vert.y);
      max_bound.z = std::max(max_bound.z, vert.z);
    }
  }
}

void BoundingBox::update(const std::vector<Vertex>& vertices) {
  this->min_bound = glm::make_vec3(vertices[0].position);
  this->max_bound = glm::make_vec3(vertices[0].position);
  for (const Vertex& vert : vertices) {
    this->max_bound.x = std::max(this->max_bound.x, vert.x);
    this->max_bound.y = std::max(this->max_bound.y, vert.y);
    this->max_bound.z = std::max(this->max_bound.z, vert.z);
    this->min_bound.x = std::min(this->min_bound.x, vert.x);
    this->min_bound.y = std::min(this->min_bound.y, vert.y);
    this->min_bound.z = std::min(this->min_bound.z, vert.z);
  }
}

glm::vec3 BoundingBox::getBoxCenter() const {
  return (min_bound + max_bound) / 2.0f;
}
float BoundingBox::getXWidth() const {
  return max_bound.x - min_bound.x;
}
float BoundingBox::getYWidth() const {
  return max_bound.y - min_bound.y;
}
float BoundingBox::getZWidth() const {
  return max_bound.z - min_bound.z;
}


bool BoundingBox::hit(Ray ray) {
  glm::vec3 in_bound  = this->min_bound;
  glm::vec3 out_bound = this->max_bound;

  if (ray.dir.x < 0)
    std::swap(in_bound.x, out_bound.x);
  if (ray.dir.y < 0)
    std::swap(in_bound.y, out_bound.y);
  if (ray.dir.z < 0)
    std::swap(in_bound.z, out_bound.z);

  // 判断求交
  glm::vec3 ndir      = glm::normalize(ray.dir);
  glm::vec3 t_min_xyz = (in_bound - ray.origin) / ndir;
  glm::vec3 t_max_xyz = (out_bound - ray.origin) / ndir;
  float     t_enter   = glm::max(t_min_xyz.x, glm::max(t_min_xyz.y, t_min_xyz.z));
  float     t_exit    = glm::min(t_max_xyz.x, glm::min(t_max_xyz.y, t_max_xyz.z));
  if (t_enter <= t_exit && t_exit >= 0)
    return true;
  return false;
}

bool BoundingBox::hit(Ray ray, Transform trans) {
  Ray local_ray = ray;
  glm::mat4 model = trans.getModel();
  glm::mat4 rmodel = glm::inverse(model);
  local_ray.origin = glm::vec3(rmodel * glm::vec4(ray.origin, 1.0f));
  local_ray.dir = glm::normalize(glm::vec3(rmodel * glm::vec4(ray.dir, 0.0f)));
  return this->hit(local_ray);
}


BvhTree::BvhTree(Geometry* geometry)
  : geometry(geometry) {}

void BvhTree::construct() {
  if (this->root != nullptr) {
    std::cerr << "BvhTree::construct()失败,已经存在构造好的bvh树!" << std::endl;
    return;
  }

  // 先生成根节点
  BvhNode* cur_node = new BvhNode();
  for (int i = 0; i < this->geometry->getSurfaces().size(); i++)
    cur_node->triangles.push_back(i);
  cur_node->box    = make_unique<BoundingBox>(this->geometry->getVertices());
  cur_node->parent = nullptr;

  this->root = cur_node;
  // 开始划分，创建一个队列，以back进front出的顺序，记录待划分节点
  std::deque<BvhNode*> node_buf{this->root};
  while (!node_buf.empty()) {
    cur_node = node_buf.front();
    node_buf.pop_front();
    // 对cur_node的划分，选择分割维度
    std::vector<float> widths{cur_node->box->getXWidth(),
                              cur_node->box->getYWidth(),
                              cur_node->box->getZWidth()};
    uint32_t           dimension_idx = std::distance(
      widths.begin(),
      std::max_element(widths.begin(), widths.end()));
    std::vector<float> comp_positions(cur_node->triangles.size());
    for (int k = 0; k < cur_node->triangles.size(); k++) {   // 计算cur_node->triangles里每个三角面元质心位置
      Surface surf = this->geometry->getSurfaces()[cur_node->triangles[k]];
      // 计算三角的质心的第i分量，dimension_idx决定按哪个维度的质心位置进行分割
      float center_pos = 0.0f;
      for (int i = 0; i < 3; i++)
        center_pos += this->geometry->getVertices()[surf.tidx[i]].position[dimension_idx];
      comp_positions[k] = center_pos / 3.0f;
    }

    std::vector<uint32_t> left_triangles, right_triangles;
    assert(comp_positions.size() > 0);
    if (comp_positions.size() == 1) {
      // cur_node为叶子节点，不进行划分
    }
    else if (comp_positions.size() == 2) {
      // 根据comp_positions分成左右节点
      if (comp_positions[0] < comp_positions[1]) {
        left_triangles.push_back(cur_node->triangles[0]);
        right_triangles.push_back(cur_node->triangles[1]);
      }
      else {
        left_triangles.push_back(cur_node->triangles[1]);
        right_triangles.push_back(cur_node->triangles[0]);
      }
    }
    else {
      // 面元大于3个的情况
      // uint32_t mid_position_idx = findKPosVal(comp_positions, 0, comp_positions.size() - 1, comp_positions.size() / 2);
      std::vector<float> comp_cpy = comp_positions;
      auto mid_idx = comp_cpy.size()/2;
      std::nth_element(comp_cpy.begin(),comp_cpy.begin()+mid_idx,comp_cpy.end());
      uint32_t mid_position_idx = std::distance(comp_positions.begin(), std::find(comp_positions.begin(),comp_positions.end(),comp_cpy[mid_idx]));
      
      float mid_position = comp_positions[mid_position_idx];
      for (int k = 0; k < comp_positions.size(); k++)
        if (comp_positions[k] < mid_position) {
          left_triangles.push_back(cur_node->triangles[k]);
        }
        else if (comp_positions[k] > mid_position) {
          right_triangles.push_back(cur_node->triangles[k]);
        }
        else {
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
      cur_node->left            = new BvhNode();
      cur_node->left->parent    = cur_node;
      cur_node->left->triangles = left_triangles;
      cur_node->left->box =
        make_unique<BoundingBox>(this->geometry->getVertices(), this->geometry->getSurfaces(), left_triangles);
      if (left_triangles.size() > 1)
        node_buf.push_back(cur_node->left);
    }
    if (right_triangles.size() > 0) {
      cur_node->right            = new BvhNode();
      cur_node->right->parent    = cur_node;
      cur_node->right->triangles = right_triangles;
      cur_node->right->box =
        make_unique<BoundingBox>(this->geometry->getVertices(), this->geometry->getSurfaces(), right_triangles);
      if (right_triangles.size() > 1)
        node_buf.push_back(cur_node->right);
    }
  }
}

void BvhTree::traverse(std::function<void(BvhNode* node)> visit) {
  // 广度优先遍历
  std::deque<BvhNode*> buf{this->root};
  while (!buf.empty()) {
    BvhNode* cur_node = buf.front();
    buf.pop_front();
    if (cur_node->left != nullptr)
      buf.push_back(cur_node->left);
    if (cur_node->right != nullptr)
      buf.push_back(cur_node->right);

    visit(cur_node);
  }
}

void BvhTree::update() {
  // geometry发生了变化需要重新构造
  this->destruct();    // 如果存在旧的记录，则删除，不存在也不会出错
  this->construct();   // 重新构造
}

// tuple<bool, vec3, float, uint32_t> intersect(const vec3 &origin, const vec3 &dir, vec3 &hit_pos,float &distance, uint32_t &triangle_idx) {
HitInfo BvhTree::hit(Ray ray, Transform trans) {
  // 参数：ray为世界坐标系下的光线，trans为model变换
  // 返回(是否击中, 击中位置, 距离, 三角索引)

  Ray       local_ray = ray;
  glm::mat4 model     = trans.getModel();
  glm::mat4 rmodel    = glm::inverse(model);
  local_ray.origin    = glm::vec3(rmodel * glm::vec4(ray.origin, 1.0f));
  local_ray.dir       = glm::normalize(glm::vec3(rmodel * glm::vec4(ray.dir, 0.0f)));

  std::deque<BvhNode*>  buf{this->root};
  std::vector<BvhNode*> hit_table;   // 所有击中包围盒的叶节点
  while (!buf.empty()) {
    BvhNode* cur_node = buf.front();
    buf.pop_front();

    if (!cur_node->box->hit(local_ray))
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
    BvhNode*  cur_node = hit_table[i];
    Surface   triangle = this->geometry->getSurfaces()[cur_node->triangles[0]];
    glm::vec3 pt[3];
    for (int j = 0; j < 3; j++)
      pt[j] = glm::make_vec3(this->geometry->getVertices()[triangle.tidx[j]].position);

    HitInfo tmp_obj = hit_triangle(local_ray, pt[0], pt[1], pt[2]);
    if (tmp_obj.isHit && tmp_obj.distance < target_obj.distance) {
      tmp_obj.triangleIdx = cur_node->triangles[0];
      target_obj          = tmp_obj;
    }
  }
  // 将交点反变换回世界坐标，并计算相应距离

  target_obj.hitPos   = glm::vec3(model * glm::vec4(target_obj.hitPos, 1.0f));
  target_obj.distance = glm::distance(ray.origin, target_obj.hitPos);

  return target_obj;
}

void BvhTree::destruct() {
  if (this->root == nullptr)
    return;
  // 广度优先遍历释放
  std::deque<BvhNode*> tmp{this->root};
  while (!tmp.empty()) {
    BvhNode* cur_node = tmp.front();
    tmp.pop_front();
    if (cur_node->left != nullptr)
      tmp.push_back(cur_node->left);
    if (cur_node->right != nullptr)
      tmp.push_back(cur_node->right);
    delete cur_node;
  }
  this->root = nullptr;
}

BvhTree::~BvhTree() {
  if (this->root == nullptr)
    return;
  // 广度优先遍历释放
  std::deque<BvhNode*> tmp{this->root};
  while (!tmp.empty()) {
    BvhNode* cur_node = tmp.front();
    tmp.pop_front();
    if (cur_node->left != nullptr)
      tmp.push_back(cur_node->left);
    if (cur_node->right != nullptr)
      tmp.push_back(cur_node->right);
    delete cur_node;
  }
}
