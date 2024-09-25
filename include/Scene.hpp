#pragma once

// #define NDEBUG

// #include "glm/ext/quaternion_common.hpp"
// #include "glm/ext/quaternion_transform.hpp"
// #include "glm/ext/quaternion_trigonometric.hpp"

#include "GLFW/glfw3.h"
#include "glm/geometric.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "proj.h"

#include "Camera.hpp"
#include "Geometry.hpp"
#include "Shader.hpp"

#define MOUSE_VIEW_ROTATE_SENSITIVITY 0.1f
#define MOUSE_VIEW_TRANSLATE_SENSITIVITY 0.02f

namespace {
using namespace std;
// namespace fs = std::filesystem;

using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

class Transform {
private:
  vec3 position;
  quat attitude{glm::angleAxis(-PI / 2, glm::vec3(1.0f, 0.0f, 0.0f))};

public:
  // 构造函数中默认做了一个绕x轴-90°旋转，将OpenGL坐标系(y朝上，z朝屏幕外)转换为z朝上，y朝屏幕内
  Transform(vec3 position, vec3 attitude_vec, float attitude_angle)
      : position(position) {
    this->attitude = glm::rotate(this->attitude, attitude_angle, attitude_vec);
  }
  Transform(vec3 position, quat attitude) : position(position) {
    this->attitude = attitude * this->attitude * glm::conjugate(attitude);
  }

  Transform() : position{0.0f, 0.0f, 0.0f} {}

  mat4 getModel() const {
    mat4 mvp(1.0f);
    mat4 rot_mat = glm::mat4_cast(this->attitude);
    mvp = glm::translate(mvp, this->position);
    mvp = mvp * rot_mat;
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

  GeometryObj() = default;
  GeometryObj(Geometry *geo, Transform trans)
      : geometry(geo), transform(trans) {
    initBuffer();
  }

  GeometryObj(GeometryObj &&geo) noexcept { // 实现移动语义
    this->transform = geo.transform;
    this->vao = geo.vao;
    this->vbo = geo.vbo;
    this->ebo = geo.ebo;
    this->texture = geo.texture;
    this->geometry = geo.geometry;
    geo.geometry = nullptr;
  }

  GeometryObj &operator=(GeometryObj &&geo) noexcept {
    this->transform = geo.transform;
    this->vao = geo.vao;
    this->vbo = geo.vbo;
    this->ebo = geo.ebo;
    this->texture = geo.texture;
    this->geometry = geo.geometry;
    geo.geometry = nullptr;
    return *this;
  }
};

void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void errorCallback(int code, const char *msg) {
  cerr << "errors occured! error code: " << code << endl;
  cout << msg << endl;
}

class Scene {
private:
  GLFWwindow *window{nullptr};

  ImGuiIO *io{nullptr};

public:
  const int width = 1200;
  const int height = 900;

  map<string, Shader *> shaders;
  map<string, GeometryObj> objs;

  Camera camera{vec3(0.0f, 0.0f, 10.0f), vec3{0.0f, 0.0f, 0.0f},
                static_cast<float>(width) / static_cast<float>(height)};
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
    glEnable(GL_DEPTH);     // 开启深度测试
    glEnable(GL_CULL_FACE); // 开启面剔除
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    loadIcon();

    initImgui();

    glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);
    glfwSetErrorCallback(errorCallback);

    loadAllShader();
  }
  Scene(const Scene &sc) = delete;
  ~Scene() {
    for (const pair<string, Shader *> &sd : this->shaders)
      delete sd.second;
  }

  void loadIcon() {
    GLFWimage image;
    image.pixels =
        stbi_load("favicon.png", &image.width, &image.height, nullptr, 4);
    glfwSetWindowIcon(this->window, 1, &image);
    stbi_image_free(image.pixels);
  }
  void initImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    this->io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // io->Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 24.0f);
    // io->Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Microsoft YaHei
    // UI/msyh.ttc", 24.0f,nullptr,io->Fonts->GetGlyphRangesChineseSimplifiedCommon());
    io->Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/simhei.ttf", 24.0f, nullptr,
        io->Fonts->GetGlyphRangesChineseSimplifiedCommon());

    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGuiStyle &style = ImGui::GetStyle();
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init();
  }
  void loadAllShader() {
    // // 读取shaders目录下的所有文件
    // set<string> shader_names;
    // fs::path shader_dir = "shaders";
    // for (auto &file : fs::directory_iterator(shader_dir)) {
    //   fs::path pp = file.path();
    //   if (!fs::is_regular_file(pp))
    //     throw runtime_error("\"/shaders\" cannot contain directory");
    //   fs::path fname = pp.filename();
    //   string ext = fname.extension().string();
    //   if (ext != ".vert" && ext != ".frag" && ext != ".geo") {
    //     stringstream err_msg;
    //     err_msg << "invalid glsl extension: " << fname;
    //     throw err_msg.str();
    //   }
    //   shader_names.insert(fname.stem().string());
    // }
    // for (const string &sdns : shader_names)
    //   shaders[sdns] = new Shader(sdns + ".vert", sdns + ".frag");

    shaders["default"] = new Shader("default.vert", "default.frag");
    shaders["normal"] = new Shader("normal.vert", "normal.geo", "normal.frag");

#ifndef NDEBUG
    cout << "all shaders compile finished!" << endl;
#endif
  }

  void add(const string &name, Geometry *geo, Transform trans = Transform()) {
    GeometryObj obj(geo, trans);
    this->objs[name] = std::move(obj);
  }

  void imgui_menu() {

    ImGui::ShowDemoWindow();

    ImGui::Begin("相机");

    bool is_theta_changed = ImGui::SliderFloat("天顶角", &this->camera.theta_s,
                                               0.0f, 180.0f, "%.1f");
    bool is_phi_changed = ImGui::SliderFloat("方向角", &this->camera.phi_s,
                                             -180.0f, 180.0f, "%.1f");
    if (is_theta_changed || is_phi_changed)
      this->camera.updateToward();

    if (ImGui::InputFloat3("位置", glm::value_ptr(this->camera.position_s),
                           "%.2f", 0)) {
      this->camera.updatePositionFromShadow();
    }

    // 鼠标交互动作
    static vec3 anchor = {0.0f, 4.0f, 0.0f};
    // 记录下“相机环绕”时的相机极坐标
    static float theta_c = 0.0f;
    static float phi_c = 0.0f;
    static float dist = 0.0f;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
        vec3 dist_v = this->camera.position_s - anchor;
        dist = glm::length(dist_v);
        theta_c =
            acos(glm::dot(glm::normalize(dist_v), vec3(0.0f, 1.0f, 0.0f)));
        phi_c = atan2(dist_v.z, dist_v.x);
      }
    }

    if (!io->WantCaptureMouse) {
      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
          // 以世界坐标锚点为中心做旋转
          phi_c += MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.x;
          theta_c -= MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.y;

          vec3 new_pos;
          new_pos.x = dist * sin(theta_c) * cos(phi_c);
          new_pos.y = dist * cos(theta_c);
          new_pos.z = dist * sin(theta_c) * sin(phi_c);
          this->camera.setPosition(new_pos + anchor);
          this->camera.lookAt({0.0f, 4.0f, 0.0f});
          // this->camera.rotate(
          //     {-MOUSE_VIEW_ROTATE_SENSITIVITY *0.01* io->MouseDelta.x,
          //      MOUSE_VIEW_ROTATE_SENSITIVITY*0.01 * io->MouseDelta.y, 0.0f});
        } else {
          // 以相机为中心旋转
          this->camera.rotate(
              {MOUSE_VIEW_ROTATE_SENSITIVITY  * io->MouseDelta.x,
               MOUSE_VIEW_ROTATE_SENSITIVITY* io->MouseDelta.y, 0.0});
        }
      }
      if (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f)) {
        this->camera.move_relative(
            {-MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.x,
             MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.y, 0.0f});
      }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      // cout << "弹起鼠标左键  " << mouse_left_click_cnt++ << endl;
    }

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
      this->camera.move_relative({0.0f, 0.0f, -0.1f});
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
      this->camera.move_relative({0.0f, 0.0f, 0.1f});
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
      this->camera.move_relative({-0.1f, 0.0f, 0.0f});
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
      this->camera.move_relative({0.1f, 0.0f, 0.0f});
    }

    ImGui::End();
  }

  // void imgui_interaction(){

  // }

  void render() {
    static uint32_t count = 0;
    count++;
    float t = static_cast<float>(count % 360) / 360;

    for (auto &pair_obj : this->objs) {
      // 计算pvm矩阵
      GeometryObj *cur_obj = &pair_obj.second;
      mat4 view_model = this->camera.getView() * cur_obj->transform.getModel();
      mat4 projection = this->camera.getProject();
      mat4 pvm = projection * view_model;

      // 渲染本体
      this->shaders["default"]->use();
      GLuint pvm_loc =
          glGetUniformLocation(this->shaders["default"]->program(), "PVM");
      assert(pvm_loc != -1);
      glUniformMatrix4fv(pvm_loc, 1, GL_FALSE, glm::value_ptr(pvm));
      glBindVertexArray(cur_obj->vao);
      glDrawElements(GL_TRIANGLES, cur_obj->geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, (void *)0);

      // 渲染法向量
      this->shaders["normal"]->use();
      GLuint projection_loc = glGetUniformLocation(
          this->shaders["normal"]->program(), "projection");
      GLuint model_view_loc = glGetUniformLocation(
          this->shaders["normal"]->program(), "view_model");
      glUniformMatrix4fv(projection_loc, 1, GL_FALSE,
                         glm::value_ptr(projection));
      glUniformMatrix4fv(model_view_loc, 1, GL_FALSE,
                         glm::value_ptr(view_model));
      glBindVertexArray(cur_obj->vao);
      glDrawArrays(GL_POINTS, 0, cur_obj->geometry->vertices.size());
    }
  }

  void mainloop() {
    glfwShowWindow(this->window);
    while (!glfwWindowShouldClose(this->window)) {
      glfwPollEvents();
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // this->camera.move_relative(vec3(0.05f, 0.0f, 0.0f));
      // this->camera.lookAt(vec3(0.0f, 4.0f, 0.0f));

      render();

      // imgui_interaction();
      imgui_menu();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(this->window);
      }

      glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
  }
};

} // namespace
