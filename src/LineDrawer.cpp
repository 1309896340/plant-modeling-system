#include "LineDrawer.h"

#include <random>

std::random_device rdgen;

LineDrawer::LineDrawer() {
  initializeOpenGLFunctions();
  glGenVertexArrays(1, &this->vao);
  glGenBuffers(1, &this->vbo);
}

void LineDrawer::addLine(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 color) {
  std::vector<glm::vec3> minibuf(2);
  minibuf[0] = pt1;
  minibuf[1] = pt2;
  this->addPolygon(minibuf, color);
}

void LineDrawer::addLine(glm::vec3 pt1, glm::vec3 pt2) {
  std::vector<glm::vec3> minibuf(2);
  minibuf[0] = pt1;
  minibuf[1] = pt2;
  this->addPolygon(minibuf);
}

void LineDrawer::addPolygon(const std::vector<glm::vec3> &vert) {
  this->rays.insert(this->rays.end(), vert.begin(), vert.end());
  this->v_nums.push_back(vert.size());
  std::uniform_real_distribution<> distr(0.1f, 0.9f);
  glm::vec3 color = glm::vec3(distr(rdgen), distr(rdgen), distr(rdgen));
  this->colors.push_back(color);
}

void LineDrawer::addPolygon(const std::vector<glm::vec3> &vert,
                            glm::vec3 color) {
  this->rays.insert(this->rays.end(), vert.begin(), vert.end());
  this->v_nums.push_back(vert.size());
  this->colors.push_back(color);
}

void LineDrawer::clear() {
  this->rays.clear();
  this->v_nums.clear();
  this->colors.clear();
}
void LineDrawer::update() {
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, this->rays.size() * sizeof(glm::vec3),
               this->rays.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);
}

size_t LineDrawer::size() const {
  // 返回线条数量
  return this->v_nums.size();
}

void LineDrawer::draw(Shader *sd, float linewidth) {
  GLfloat currentLineWidth;
  glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);

  glLineWidth(linewidth);
  glBindVertexArray(this->vao);
  uint32_t offset = 0u;
  for (int i = 0; i < this->v_nums.size(); i++) {
    sd->set("lineColor", this->colors[i]);
    glDrawArrays(GL_LINE_STRIP, offset, v_nums[i]);
    offset += v_nums[i]; // 累加上第i段的顶点数
  }
  glLineWidth(currentLineWidth);
}
LineDrawer::~LineDrawer() {
  glDeleteVertexArrays(1, &this->vao);
  glDeleteBuffers(1, &this->vbo);
}
