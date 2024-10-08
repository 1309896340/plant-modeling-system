#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace{
    using namespace std;
    using glm::vec3;
    using glm::vec4;
    using glm::mat4;
    using glm::mat3;


class BoundingBox {
public:
  vec3 min_bound{0.0f, 0.0f, 0.0f};
  vec3 max_bound{0.0f, 0.0f, 0.0f};
  BoundingBox() = default;
  BoundingBox(vec3 min_bound, vec3 max_bound)
      : min_bound(min_bound), max_bound(max_bound) {}
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

class BVH{

};

}
