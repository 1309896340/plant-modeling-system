#pragma once
#include <QOpenGLFunctions_4_5_Core>
#include <glm/glm.hpp>

#include "Shader.h"

class LineDrawer: protected QOpenGLFunctions_4_5_Core {
private:
  GLuint vao{0};
  GLuint vbo{0};
  std::vector<glm::vec3> rays;
  std::vector<uint32_t> v_nums;
  std::vector<glm::vec3> colors;

public:
  LineDrawer();
  void addLine(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 color);
  void addLine(glm::vec3 pt1, glm::vec3 pt2);
  void addPolygon(const std::vector<glm::vec3> &vert);

  void addPolygon(const std::vector<glm::vec3> &vert, glm::vec3 color);
  void clear();
  void update();
  size_t size() const;

  void draw(Shader *sd, float linewidth = 1.0f);
  ~LineDrawer();
};