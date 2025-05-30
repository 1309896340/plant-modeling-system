#pragma once
#include "Renderer.h"

OpenGLContext::OpenGLContext() {
  initializeOpenGLFunctions();
  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);
  glGenBuffers(1, &this->vbo);
  glGenBuffers(1, &this->ebo);
  glBindVertexArray(0);
}

GLuint OpenGLContext::getVAO() const { return this->vao; }
size_t OpenGLContext::getSize() const { return this->drawSize; }

OpenGLContext::~OpenGLContext() {
  glDeleteBuffers(1, &this->vbo);
  glDeleteBuffers(1, &this->ebo);
  glDeleteVertexArrays(1, &this->vao);
}

BoundingBoxContext::BoundingBoxContext(BoundingBox *box, Transform *transform)
    : OpenGLContext(), transform(transform), box(box) {}
std::tuple<std::vector<glm::vec3>, std::vector<uint32_t>>
BoundingBoxContext::genOpenGLRawData() {
  glm::vec3 max_xyz = this->box->max_bound;
  glm::vec3 min_xyz = this->box->min_bound;
  std::vector<glm::vec3> vertices = {min_xyz,
                                     {min_xyz.x, min_xyz.y, max_xyz.z},
                                     {min_xyz.x, max_xyz.y, min_xyz.z},
                                     {min_xyz.x, max_xyz.y, max_xyz.z},
                                     {max_xyz.x, min_xyz.y, min_xyz.z},
                                     {max_xyz.x, min_xyz.y, max_xyz.z},
                                     {max_xyz.x, max_xyz.y, min_xyz.z},
                                     max_xyz};
  glm::mat4 transMat = this->transform->getModel();
  for (uint32_t i = 0; i < vertices.size(); i++)
    vertices[i] = glm::vec3(transMat * glm::vec4(vertices[i], 1.0f));
  std::vector<uint32_t> indices = {0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3,
                                   2, 6, 4, 6, 4, 5, 3, 7, 5, 7, 6, 7};
  return make_tuple(vertices, indices);
}
void BoundingBoxContext::init() {
  auto [vertices, indices] = genOpenGLRawData();

  glBindVertexArray(this->vao);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
               indices.data(), GL_STATIC_DRAW);

  this->drawSize = indices.size();
}
void BoundingBoxContext::update() {
  auto [vertices, indices] = genOpenGLRawData();

  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3),
                  vertices.data());
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t),
                  indices.data());
}

GeometryContext::GeometryContext(Component::GeometryObject *obj)
    : OpenGLContext(), obj(obj) {
  BoundingBox *bbox = this->obj->getBoundingBox();
  assert(bbox != nullptr);
  this->box = std::make_unique<BoundingBoxContext>(bbox, &obj->transform);
  this->bvhtree = this->obj->getBvhTree();
}
void GeometryContext::init() {
  if (this->obj == nullptr) {
    printf("GeometryContext没有初始化有效的GeometryObject实例!\n");
    return;
  }
  Geometry *geometry = this->obj->getGeometry();

  assert(geometry != nullptr && "GeometryContext::init() geometry is null!");

  glBindVertexArray(this->vao);
  auto &vertices = geometry->getVertices();
  auto &surfaces = geometry->getSurfaces();
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);

  size_t stride = sizeof(Vertex);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)0); // 位置
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)(3 * sizeof(float))); // 法向量
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)(6 * sizeof(float))); // 颜色
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
                        (void *)(9 * sizeof(float))); // 纹理坐标
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaces.size() * sizeof(Surface),
               surfaces.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);

  this->drawSize = surfaces.size() * 3;

  this->box->init();
  if (this->bvhtree != nullptr) {
    this->bvhtree->traverse([this](BvhNode *node) {
      auto box_context = std::make_unique<BoundingBoxContext>(
          node->box.get(), &this->obj->transform);
      box_context->init();
      this->boxes.emplace_back(std::move(box_context));
    });
  }
}
void GeometryContext::update() {
  if (this->obj == nullptr) {
    printf("GeometryContext没有初始化有效的GeometryObject实例!\n");
    return;
  }

  Geometry *geometry = this->obj->getGeometry();
  assert(geometry != nullptr &&
         "GeometryRenderObjecy::update() geometry is null!");
  // 将geometry中的顶点属性更新到顶点缓冲区
  auto &vertices = geometry->getVertices();
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_DYNAMIC_DRAW);
  auto &surfaces = geometry->getSurfaces();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaces.size() * sizeof(Surface),
               surfaces.data(), GL_DYNAMIC_DRAW);
  glBindVertexArray(0);

  this->drawSize = surfaces.size() * 3;

  this->box->update();
  if (this->bvhtree != nullptr) {
    // 由于bvhtree的更新回重构所有BoundingBox，因此其对应context也只能全部重建
    this->boxes.clear();
    this->bvhtree->traverse([this](BvhNode *node) {
      auto box_context = std::make_unique<BoundingBoxContext>(
          node->box.get(), &this->obj->transform);
      box_context->init();
      this->boxes.emplace_back(std::move(box_context));
    });
  }
}