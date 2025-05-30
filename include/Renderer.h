#pragma once

#include <QOpenGLFunctions_4_5_Core>

#include "Bounding.h"
#include "Component.h"

class OpenGLContext : protected QOpenGLFunctions_4_5_Core {
  protected:
  GLuint   vao{0}, vbo{0}, ebo{0};
  uint32_t drawSize{0};

  public:
  GLuint texture{0};
  OpenGLContext();

  virtual void init()   = 0;
  virtual void update() = 0;

  GLuint getVAO() const;
  size_t getSize() const;

  virtual ~OpenGLContext();
};

class BoundingBoxContext : public OpenGLContext {
private:
  BoundingBox *box{nullptr};
  Transform *transform;

public:
  BoundingBoxContext() = delete;

  BoundingBoxContext(BoundingBox *box, Transform *transform);
  std::tuple<std::vector<glm::vec3>, std::vector<uint32_t>> genOpenGLRawData();

  virtual void init();
  virtual void update();
};

class GeometryContext : public OpenGLContext {
private:
  Component::GeometryObject *obj{nullptr};
  BvhTree *bvhtree{nullptr};

public:
  std::unique_ptr<BoundingBoxContext> box{nullptr};
  std::vector<std::unique_ptr<BoundingBoxContext>> boxes;
  GeometryContext(Component::GeometryObject *obj);

  virtual void init();
  virtual void update();
};
