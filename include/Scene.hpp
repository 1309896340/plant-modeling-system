#pragma once

#include <cassert>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Scene {
private:
  GLFWwindow *window{nullptr};
  const int width = 1200;
  const int height = 900;

public:
  Scene() {}

  bool init() {
    std::cout << "进入init" <<std::endl;
    if (glfwInit() == GLFW_FALSE)
      return false;
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    std::cout << "glfwInit后" <<std::endl;

    this->window = glfwCreateWindow(width, height, "wheat", 0, 0);
    glfwMakeContextCurrent(window);
    std::cout << "glfwMakeContextCurrent后" <<std::endl;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      return false;

    // 其他配置
    glEnable(GL_DEPTH);     // 开启深度测试

    return true;
  }

  void mainloop(){
    while(!glfwWindowShouldClose(this->window)){
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
  }
};
