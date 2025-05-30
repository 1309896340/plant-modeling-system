#pragma once

#include "Bounding.h"
#include "Skeleton.h"

// struct Status {
//   bool selected{false};
//   bool visible{true};
//   bool collided{false};
//   bool listed{true};
//   bool lighted{true};
// };

enum ObjectFlag {
  ObjectFlag_None = 0x0,
  ObjectFlag_VISIBLE = 0x1,
  ObjectFlag_LISTED = 0x2,
  ObjectFlag_COLLIDED = 0x4,
  ObjectFlag_LIGHTED = 0x8
};
struct RadiosityResult {
  // float radiant_flux{0.0f};
  // 与GeometryContext::geometry::surfaces的元素一一对应
  std::vector<glm::vec3> radiant_flux;
};

namespace Component {
class GeometryObject {
private:
  std::string name;
  std::shared_ptr<Geometry> geometry{nullptr};
  std::unique_ptr<Skeleton> skeleton{
      nullptr}; // 加入Skeleton需要明确其与geometry之间的关系
  std::unique_ptr<BoundingBox> box{nullptr};
  std::unique_ptr<BvhTree> bvhtree{nullptr};

public:
  Transform transform;
  // Status status;
  uint32_t flag{ObjectFlag_None};

  RadiosityResult radiosity; // 由compute_radiosity()更新

  std::string getName() const;
  BvhTree *getBvhTree();
  BoundingBox *getBoundingBox();
  Geometry *getGeometry();
  GeometryObject(std::string name, std::shared_ptr<Geometry> geometry,
                 Transform transform = Transform{}, bool useBvh = false);

  void update();
  virtual ~GeometryObject();
};

}; // namespace Component
