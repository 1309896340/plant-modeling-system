#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "constants.h"
#include "glm/ext/matrix_transform.hpp"


class Transform {
  private:
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  // quat attitude{glm::angleAxis(-PI / 2, glm::vec3(1.0f, 0.0f, 0.0f))};
  glm::quat attitude{glm::identity<glm::quat>()};

  public:
  friend Transform operator*(Transform t2, Transform t1);

  Transform(glm::vec3 position, glm::vec3 attitude_vec, float attitude_angle);
  Transform(glm::vec3 position, glm::quat attitude);
  Transform(glm::quat attitude);
  Transform(glm::vec3 position);
  Transform(const Transform& transform);
  Transform() = default;

  void       setPosition(glm::vec3 new_position);
  glm::vec3  getPosition() const;
  glm::vec3& getPosition();

  void       setAttitude(glm::quat attitude);
  glm::quat  getAttitude() const;
  glm::quat& getAttitude();

  void translate(glm::vec3 offset);
  void translate_relative(glm::vec3 offset);

  void rotate(float rot_angle, glm::vec3 rot_axis);

  glm::mat4 getModel() const;
};


Transform operator*(Transform t2, Transform t1);
