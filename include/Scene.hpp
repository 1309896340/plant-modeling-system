#pragma once

// #define NDEBUG

#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLFW/glfw3.h"
#include "Scene.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "proj.h"

#include "Camera.hpp"
#include "Geometry.hpp"
#include "Shader.hpp"

namespace {
using namespace std;
namespace fs = std::filesystem;

using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

class Transform {
private:
  vec3 position;
  quat attitude;

public:
  Transform()
      : position{vec3(0.0f, 0.0f, 0.0f)}, attitude(1.0f, 0.0f, 0.0f, 0.0f) {}

  mat4 getModel() const {
    mat4 mvp(1.0f);
    mat4 rot_mat = glm::mat4_cast(this->attitude);
    mvp = glm::translate(mvp, this->position);
    return mvp;
  }
};

class GeometryObj {
private:
  void initBuffer() {

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 this->geometry->vertices.size() * sizeof(Vertex),
                 this->geometry->vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                          (void *)0); // 位置
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                          (void *)(3 * sizeof(float))); // 法向量
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                          (void *)(6 * sizeof(float))); // 颜色
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &this->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 this->geometry->surfaces.size() * sizeof(Surface),
                 this->geometry->surfaces.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
  }

public:
  Geometry *geometry{nullptr};
  Transform transform;

  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};
  GLuint texture{0};

  GeometryObj() = delete;
  GeometryObj(Geometry *geo, Transform trans)
      : geometry(geo), transform(trans) {
    initBuffer();
  }
};

void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

class Scene {
private:
  GLFWwindow *window{nullptr};
  const int width = 1200;
  const int height = 900;

  map<string, Shader *> shaders;
  vector<GeometryObj> objs;

  Camera camera{vec3(0.0f, 0.0f, 10.0f), vec3{0.0f, 0.0f, 0.0f},
                static_cast<float>(width) / static_cast<float>(height)};

public:
  Scene() {
    if (glfwInit() == GLFW_FALSE)
      throw runtime_error("failed to init glfw!");
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    this->window = glfwCreateWindow(width, height, "demo", 0, 0);
    glfwMakeContextCurrent(this->window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      throw runtime_error("failed to init glad!");
    glEnable(GL_DEPTH); // 开启深度测试
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);

    loadAllShader();
  }
  Scene(const Scene &sc) = delete;
  ~Scene() {
    for (const pair<string, Shader *> &sd : this->shaders)
      delete sd.second;
  }
  void loadAllShader() {
    // 读取shaders目录下的所有文件
    set<string> shader_names;
    fs::path shader_dir = "shaders";
    for (auto &file : fs::directory_iterator(shader_dir)) {
      fs::path pp = file.path();
      if (!fs::is_regular_file(pp))
        throw runtime_error("\"/shaders\" cannot contain directory");
      fs::path fname = pp.filename();
      string ext = fname.extension().string();
      if (ext != ".vert" && ext != ".frag") {
        stringstream err_msg;
        err_msg << "invalid shader source extension: " << fname;
        throw err_msg.str();
      }
      shader_names.insert(fname.stem().string());
    }
    for (const string &sdns : shader_names)
      shaders[sdns] = new Shader(sdns + ".vert", sdns + ".frag");
#ifndef NDEBUG
    cout << "all shaders compile finished!" << endl;
#endif
  }

  void add(Geometry *geo) {
    GeometryObj obj(geo, Transform());
    this->objs.emplace_back(obj);
  }

  void render() {
    this->shaders["default"]->use();
    GLuint pvm_loc =
        glGetUniformLocation(this->shaders["default"]->program(), "PVM");
    assert(pvm_loc != -1);

    for (int i = 0; i < this->objs.size(); i++) {
      // 计算pvm矩阵
      GeometryObj *cur_obj = &objs[i];
      mat4 pvm = this->camera.getProject() * this->camera.getView() *
                 cur_obj->transform.getModel();

      glUniformMatrix4fv(pvm_loc, 1, GL_FALSE, glm::value_ptr(pvm));
      glBindVertexArray(cur_obj->vao);
      glDrawElements(GL_TRIANGLES, cur_obj->geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, (void *)0);
    }
  }

  void mainloop() {
    glfwShowWindow(this->window);
    while (!glfwWindowShouldClose(this->window)) {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      this->camera.move_relative(vec3(0.05f, 0.0f, 0.0f));
      this->camera.lookAt(vec3(0.0f, 0.0f, 0.0f));

      render();

      glfwPollEvents();
      glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
  }
};

} // namespace
