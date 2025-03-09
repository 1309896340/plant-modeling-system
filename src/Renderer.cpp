#pragma once
#include "Renderer.h"

OpenGLContext::OpenGLContext() {
  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);
  glGenBuffers(1, &this->vbo);
  glGenBuffers(1, &this->ebo);
  glBindVertexArray(0);
}

GLuint OpenGLContext::getVAO() const {
  return this->vao;
}
size_t OpenGLContext::getSize() const {
  return this->drawSize;
}

OpenGLContext::~OpenGLContext() {
  glDeleteBuffers(1, &this->vbo);
  glDeleteBuffers(1, &this->ebo);
  glDeleteVertexArrays(1, &this->vao);
}
