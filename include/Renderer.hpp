#pragma once

#include "Transform.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>


class OpenGLContext {
  protected:
  GLuint   vao{0}, vbo{0}, ebo{0};
  uint32_t drawSize{0};

  public:
  // Transform transform{};
  GLuint texture{0};
  OpenGLContext() {
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
    glGenBuffers(1, &this->vbo);
    glGenBuffers(1, &this->ebo);
    glBindVertexArray(0);
  }

  virtual void init()   = 0;
  virtual void update() = 0;

  GLuint getVAO() const { return this->vao; }
  size_t getSize() const { return this->drawSize; }

  virtual ~OpenGLContext() {
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
    glDeleteVertexArrays(1, &this->vao);
  }
};
