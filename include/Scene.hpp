#pragma once

// #define NDEBUG

#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

#include "proj.h"

#include "Shader.hpp"

namespace {
using namespace std;
namespace fs = std::filesystem;
class Scene {
private:
  GLFWwindow *window{nullptr};
  const int width = 1200;
  const int height = 900;

  map<string, Shader *> shaders;

public:
  Scene() {
    if (glfwInit() == GLFW_FALSE)
      throw runtime_error("failed to init glfw!");
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    this->window = glfwCreateWindow(width, height, "wheat", 0, 0);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      throw runtime_error("failed to init glad!");
    glEnable(GL_DEPTH); // 开启深度测试
    loadAllShader();
  }
  Scene(const Scene &sc) = delete;
  ~Scene(){
    for(const pair<string, Shader *> &sd : this->shaders)
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

  void mainloop() {
    while (!glfwWindowShouldClose(this->window)) {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glfwPollEvents();
      glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
  }
};

} // namespace
