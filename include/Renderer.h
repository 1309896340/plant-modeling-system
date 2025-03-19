#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>


class OpenGLContext {
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

